#include <iostream>
#include <fstream>
#include <vector>
#include <memory>
#include <filesystem>

#include <string.h>

#include <zlib.h>

#include "roll.hpp"
#include "recipe/recipe.hpp"

using namespace std::string_literals;

static constexpr const char *const helptext =
"Asset Roll\n"
"Usage: roll output-file.roll inputfile [...]\n"
"       roll output-file.roll --recipe recipe-file\n"
"       roll --inspect input.roll\n"
"       roll --help\n"
;

std::vector<const char*> compressible_file_exts =
{
	".frag",
	".vert",
	".glsl",
	".tga",
	".txt",
	".atlas"
};

const char *magic = "ASSETROLL";

static bool compress_file_ext(const std::string &ext)
{
	for (const char *const e : compressible_file_exts)
		if (ext == e)
			return true;

	return false;
}

static std::string format(size_t size)
{
	char convert[26];
	if(size < 1000)
		snprintf(convert, sizeof(convert), "%dB", (int)size);
	else if(size < 1000 * 1000)
		snprintf(convert, sizeof(convert), "%.2fKB", (double)size / 1000);
	else
		snprintf(convert, sizeof(convert), "%.2fMB", (double)size / 1000 / 1000);

	return convert;
}

static bool is_asset_roll(const std::string &filename)
{
	std::ifstream in(filename, std::ifstream::binary);

	char test[50];
	in.read(test, strlen(magic));
	if(in.gcount() != strlen(magic))
		return false;
	test[strlen(magic)] = 0;

	return std::string(test) == std::string(magic);
}

static void inspect(const std::string &roll)
{
	if(!std::filesystem::exists(roll))
		throw std::runtime_error("File \"" + roll + "\" does not exist");

	if(!is_asset_roll(roll))
		throw std::runtime_error("File \"" + roll + "\" does not appear to be an asset roll");

	std::ifstream in(roll, std::ifstream::binary);
	if(!in)
		throw std::runtime_error("Could not open file \"" + roll + "\" in read mode");

	in.seekg(strlen(magic));

	// read number of files
	std::uint16_t file_count = 0;
	in.read((char*)&file_count, sizeof(file_count));
	if(in.gcount() != sizeof(file_count))
		throw std::runtime_error("File \"" + roll + "\" is corrupt");

	std::cout << roll << ": " << file_count << " files" << std::endl;

	for(int i = 0; i < file_count; ++i)
	{
		Header rh;

		in.read((char*)&rh.compressed, sizeof(rh.compressed));
		if(in.gcount() != sizeof(rh.compressed))
			throw std::runtime_error("File \"" + roll + "\" is corrupt");

		in.read((char*)&rh.uncompressed_size, sizeof(rh.uncompressed_size));
		if(in.gcount() != sizeof(rh.uncompressed_size))
			throw std::runtime_error("File \"" + roll + "\" is corrupt");

		in.seekg(sizeof(rh.begin), std::ifstream::cur);

		in.read((char*)&rh.size, sizeof(rh.size));
		if(in.gcount() != sizeof(rh.size))
			throw std::runtime_error("File \"" + roll + "\" is corrupt");

		in.read((char*)&rh.filename_length, sizeof(rh.filename_length));
		if(in.gcount() != sizeof(rh.filename_length))
			throw std::runtime_error("File \"" + roll + "\" is corrupt");

		std::unique_ptr<char[]> fname = std::make_unique<char[]>(rh.filename_length + 1);
		in.read(fname.get(), rh.filename_length);
		if(in.gcount() != rh.filename_length)
			throw std::runtime_error("File \"" + roll + "\" is corrupt");
		fname[rh.filename_length] = 0;
		rh.filename = fname.get();

		std::cout << (i + 1) << ": \"" << rh.filename << "\" (" << format(rh.compressed ? rh.uncompressed_size : rh.size) << (rh.compressed ? "/" + format(rh.size) + " compressed" : "") << ")" << std::endl;
	}
}

// replace backslashes with forward slashes
static std::string forward_slash(const std::string &name)
{
	if(name.find('\\') == std::string::npos)
		return name;

	std::string newname = name;
	for(char &c : newname)
		if(c == '\\')
			c = '/';

	return newname;
}

// do the work
static void create(const std::string &out_file, const std::vector<RollItem> &in_files)
{
	// number of input files
	const int file_count = in_files.size();

	// make sure the output file doesn't exist, or is already an asset roll
	if(std::filesystem::exists(out_file) && !is_asset_roll(out_file))
		throw std::runtime_error("Not willing to overwrite file \""s + out_file + "\"");

	// make sure all the input files exist
	for(const RollItem &in : in_files)
	{
		if(!std::filesystem::exists(in.real_file))
			throw std::runtime_error("File \"" + in.real_file + "\" does not exist");
	}

	// construct the headers

	std::vector<Header> headers;
	for(int i = 0; i < file_count; ++i)
	{
		Header h;

		// mostly placeholder data
		h.compressed = 0;
		h.uncompressed_size = 0;
		h.begin = 0;
		h.size = 0;
		h.filename = forward_slash(in_files[i].recorded_file);
		h.filename_length = h.filename.length();

		headers.push_back(h);
	}

	// prepare output file
	std::ofstream out(out_file, std::ofstream::binary);

	// write magic
	out.write(magic, strlen(magic));

	// embedded file count
	const std::uint16_t file_count_short = file_count;
	out.write((char*)&file_count_short, sizeof(file_count_short));

	// write the headers
	std::uint64_t offset = strlen(magic) + sizeof(std::uint16_t); // magic length plus filecount length
	for(const Header &h : headers)
	{
		out.write((const char*)&h.compressed, sizeof(h.compressed));
		out.write((const char*)&h.uncompressed_size, sizeof(h.uncompressed_size));
		out.write((const char*)&h.begin, sizeof(h.begin));
		out.write((const char*)&h.size, sizeof(h.size));
		out.write((const char*)&h.filename_length, sizeof(h.filename_length));
		out.write((const char*)h.filename.c_str(), h.filename.length());

		offset += h.length();
	}

	// write the files
	for(int i = 0; i < file_count; ++i)
	{
		const long long fsize = std::filesystem::file_size(in_files.at(i).real_file);

		// fill in some missing header details
		headers[i].compressed = in_files[i].compress;
		headers[i].uncompressed_size = fsize;
		headers[i].begin = offset;
		if(!headers[i].compressed)
			headers[i].size = fsize;

		std::ifstream in(in_files.at(i).real_file, std::ifstream::binary);
		if(!in)
			throw std::runtime_error("Could not open file \"" + in_files.at(i).real_file + "\" for reading");

		// write the file contents
		if(headers[i].compressed)
		{
			// compress with zlib; load the entire file into memory
			std::vector<unsigned char> contents(fsize);
			in.read((char*)contents.data(), fsize);
			if(in.gcount() != fsize)
				throw std::runtime_error("Could not read the contents of file \"" + in_files.at(i).real_file + "\"");

			// go go gadget zlib
			uLongf compressed_size = (fsize * 1.1) + 12;
			unsigned char *const compressed_data = (unsigned char*)malloc(compressed_size);
			if(compress(compressed_data, &compressed_size, contents.data(), fsize) != Z_OK)
			{
				free(compressed_data);
				throw std::runtime_error("Zlib error: Could not compress the data");
			}

			// more missing header info
			headers[i].size = compressed_size;

			// write
			out.write((const char*)compressed_data, compressed_size);
			free(compressed_data);

			offset += compressed_size;
		}
		else
		{
			// write the file normally
			unsigned char buffer[4096];
			long long written = 0;
			while(written != fsize)
			{
				in.read((char*)buffer, sizeof(buffer));
				const int got = in.gcount();
				out.write((const char*)buffer, got);
				written += got;
			}

			offset += fsize;
		}
	}

	// rewrite all the headers with proper info
	out.seekp(strlen(magic) + sizeof(std::uint16_t));
	for(const Header &h : headers)
	{
		out.write((const char*)&h.compressed, sizeof(h.compressed));
		out.write((const char*)&h.uncompressed_size, sizeof(h.uncompressed_size));
		out.write((const char*)&h.begin, sizeof(h.begin));
		out.write((const char*)&h.size, sizeof(h.size));

		out.seekp(sizeof(h.filename_length) + h.filename_length, std::ofstream::cur);
	}

	out.close();

	std::cout << file_count << " files written to \"" << out_file << "\" (" << format(std::filesystem::file_size(out_file)) << ")" << std::endl;
}

static int go(int argc, char **argv)
{
	std::vector<std::string> args;
	for (int i = 0; i < argc; ++i)
		args.emplace_back(argv[i]);

	if(args.size() < 2 || (args.size() == 2 && (args.at(1) == "--help" || args.at(1) == "-h")))
	{
		std::cout << helptext << std::endl;
		return 0;
	}

	// collect the arguments
	std::vector<RollItem> infiles;

	// examine an existing rollfile
	if(args.size() == 3 && args.at(1) == "--inspect")
	{
		inspect(args.at(2));

		return 0;
	}
	// use rollfile
	else if(args.size() == 4 && args.at(2) == "--recipe")
	{
		const std::string out_file = args.at(1);
		const std::string recipe_file = args.at(3);

		try
		{
			Recipe recipe(recipe_file, out_file);
			bool update = true;

			infiles = recipe.get_items(update);
			if (!update)
			{
				std::cout << recipe_file << " is up-to-date" << std::endl;
				return 0;
			}
		}
		catch (const std::exception &e)
		{
			throw std::runtime_error(std::string("recipe: ") + e.what());
		}
	}
	// create a roll file from cmd args
	else
	{
		const std::string out_file = args.at(1);

		for(int i = 2; i < args.size(); ++i)
		{
			const std::string real_file = std::filesystem::path(out_file).parent_path() / args.at(i);
			const bool compress = compress_file_ext(std::filesystem::path(real_file).extension());

			infiles.emplace_back(real_file, args.at(i), compress);
		}
	}

	create(args.at(1), infiles);

	return 0;
}

int main(int argc, char **argv)
{
	try
	{
		return go(argc, argv);
	}
	catch(const std::exception &e)
	{
		std::cout << "Fatal: " << e.what() << std::endl;
		return 1;
	}
}
