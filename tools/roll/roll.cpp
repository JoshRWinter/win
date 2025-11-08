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
"       roll [--hex] outputfile recipefile\n"
"       roll --inspect input.roll\n"
"       roll --help\n"
;

const char *MAGIC = "ASSETROLL";
const char *HEXMAGIC = "0x41,0x53,0x53,0x45,0x54,0x52,0x4F,0x4C,0x4C";

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

static bool is_asset_roll(const std::string &filename, bool hex)
{
	std::ifstream in(filename, std::ifstream::binary);

	const char *magic = hex ? HEXMAGIC : MAGIC;
	const auto len = strlen(magic);

	char test[50];

	if (sizeof(test) < len + 1)
		throw std::runtime_error("oops");

	in.read(test, len);

	if(in.gcount() != len)
		return false;

	test[len] = 0;

	return !strcmp(test, magic);
}

static void inspect(const std::string &roll)
{
	if(!std::filesystem::exists(roll))
		throw std::runtime_error("File \"" + roll + "\" does not exist");

	if(!is_asset_roll(roll, false))
		throw std::runtime_error("File \"" + roll + "\" does not appear to be an asset roll");

	std::ifstream in(roll, std::ifstream::binary);
	if(!in)
		throw std::runtime_error("Could not open file \"" + roll + "\" in read mode");

	in.seekg(strlen(MAGIC));

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

std::filesystem::path get_random_temp_file()
{
	std::random_device rd;

	char r[11];

	for (int i = 0; i < 10; ++i)
		r[i] = (char)std::uniform_int_distribution<int>('a', 'z')(rd);

	r[10] = 0;

	return std::filesystem::temp_directory_path() / r;
}

// do the work
static void create(bool hex, const std::filesystem::path &out_file, const std::vector<RollItem> &in_files)
{
	// number of input files
	const int file_count = in_files.size();

	// make sure the output file doesn't exist, or is already an asset roll
	if(std::filesystem::exists(out_file) && !is_asset_roll(out_file, hex))
		throw std::runtime_error("Not willing to overwrite file \""s + out_file.string() + "\"");

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
	const auto destfile = hex ? get_random_temp_file() : out_file;
	std::ofstream out(destfile, std::ofstream::binary);

	// write magic
	out.write(MAGIC, strlen(MAGIC));

	// embedded file count
	const std::uint16_t file_count_short = file_count;
	out.write((char*)&file_count_short, sizeof(file_count_short));

	// write the headers
	std::uint64_t offset = strlen(MAGIC) + sizeof(std::uint16_t); // magic length plus filecount length
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
	out.seekp(strlen(MAGIC) + sizeof(std::uint16_t));
	for(const Header &h : headers)
	{
		out.write((const char*)&h.compressed, sizeof(h.compressed));
		out.write((const char*)&h.uncompressed_size, sizeof(h.uncompressed_size));
		out.write((const char*)&h.begin, sizeof(h.begin));
		out.write((const char*)&h.size, sizeof(h.size));

		out.seekp(sizeof(h.filename_length) + h.filename_length, std::ofstream::cur);
	}

	out.close();

	// rewrite the binary in hex csv
	if (hex)
	{
		std::ifstream in(destfile, std::ifstream::binary);
		if (!in)
			throw std::runtime_error("Couldn't open " + destfile.string() + " for reading");

		const auto len = std::filesystem::file_size(destfile);
		std::unique_ptr<unsigned char[]> binary(new unsigned char[len]);
		in.read((char*)binary.get(), len);
		if (in.gcount() != len)
			throw std::runtime_error("Couldn't read " + std::to_string(len) + " from " + destfile.string());

		std::filesystem::remove(destfile);

		std::ofstream hexout(out_file, std::ofstream::binary);
		if (!hexout)
			throw std::runtime_error("Couldn't open " + out_file.string() + " for writing");

		char conv[10];
		for (long long i = 0; i < len; ++i)
		{
			if (i % 60 == 0 && i != 0)
				hexout.write("\n", 1);

			const bool last = i == len - 1;
			snprintf(conv, sizeof(conv), last ? "0x%X" : "0x%X,", binary[i]);
			hexout.write(conv, strlen(conv));
		}

		hexout.close();
	}

	std::cout << file_count << " files written to \"" << out_file.string() << "\" (" << format(std::filesystem::file_size(out_file)) << ")" << std::endl;
}

struct CmdLineOptions
{
	bool showhelp = false;
	std::filesystem::path outfile;
	std::filesystem::path recipe;
	std::filesystem::path inspectfile;
	bool hex = false;
};

CmdLineOptions get_options(int argc, char **argv)
{
	CmdLineOptions opts;

	if (argc < 3)
	{
		opts.showhelp = true;
		return opts;
	}

	std::vector<std::string> args;
	for (int i = 0; i < argc; ++i)
		args.emplace_back(argv[i]);

	// inspect
	if (args.at(1) == "--inspect")
	{
		if (args.size() != 3)
		{
			opts.showhelp = true;
			return opts;
		}

		opts.inspectfile = args.at(2);
	}
	// recipe with hex
	else if (args.at(1) == "--hex")
	{
		if (args.size() != 4)
		{
			opts.showhelp = true;
			return opts;
		}

		opts.outfile = args.at(2);
		opts.recipe = args.at(3);
		opts.hex = true;
	}
	// recipe no hex
	else
	{
		if (args.size() != 3)
		{
			opts.showhelp = true;
			return opts;
		}

		opts.outfile = args[1];
		opts.recipe = args[2];
	}

	return opts;
}

static int go(int argc, char **argv)
{
	const auto opts = get_options(argc, argv);

	if (opts.showhelp)
	{
		std::cout << helptext << std::endl;
		return 1;
	}

	else if (opts.inspectfile != "")
	{
		inspect(opts.inspectfile);
		return 0;
	}
	else
	{
		try
		{
			Recipe recipe(opts.recipe, opts.outfile);

			bool update = true;
			auto infiles = recipe.get_items(update);
			if (!update)
			{
				std::cout << opts.outfile << " is up-to-date" << std::endl;
				return 0;
			}

			create(opts.hex, opts.outfile, infiles);
			return 0;
		}
		catch (const std::exception &e)
		{
			throw std::runtime_error(std::string("recipe: ") + e.what());
		}
	}
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
