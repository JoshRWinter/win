#include <iostream>
#include <fstream>
#include <stdexcept>
#include <string>
#include <cstdint>
#include <vector>
#include <memory>
#include <cctype>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <zlib.h>

using namespace std::string_literals;

static constexpr const char *const helptext =
"Asset Roll\n"
"Usage: roll output-file.roll inputfile[:z] [ ... ]\n"
"       roll output-file.roll\n"
"       roll --inspect input.roll\n"
"       roll --help\n"
"\n"
"If the second 1-arg form is used, a 'rollfile' is\n"
"expected to be found in the current direcory.\n"
"\n"
"The rollfile should have one file (with optional\n"
"[:z] compression flag) per line. #Comments, blank\n"
"lines, and end-of-line #Comments are supported here."
;

static const std::string magic = "ASSETROLL";

struct header
{
	std::uint8_t compressed;
	std::uint64_t uncompressed_size;
	std::uint64_t begin;
	std::uint64_t size;
	std::uint16_t filename_length;
	std::string filename;

	std::uint64_t length() const { return sizeof(compressed) + sizeof(uncompressed_size) + sizeof(begin) + sizeof(size) + sizeof(filename_length) + filename_length; }
};

static std::string strip_options(const std::string&);
static std::string forward_slash(const std::string&);
static std::string trim(const std::string&);
static bool request_compression(const std::string&);
static bool is_asset_roll(const std::string&);
static long long filesize(const std::string&);
static bool exists(const std::string&);
static std::string format(size_t);
static void create(const std::string&, const std::vector<std::string>&);
static void inspect(const std::string&);
static int go(int argc, char **argv);

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

int go(int argc, char **argv)
{
	if(argc < 2 || (argc == 2 && (!strcmp(argv[1], "--help") || !strcmp(argv[1], "-h"))))
	{
		std::cout << helptext << std::endl;
		return 0;
	}

	// collect the arguments
	std::vector<std::string> infiles;

	// examine an existing rollfile
	if(argc == 3 && !strcmp(argv[1], "--inspect"))
	{
		inspect(argv[2]);

		return 0;
	}
	// use rollfile
	else if(argc == 2)
	{
		std::ifstream rollfile("rollfile");
		if(!rollfile)
			throw std::runtime_error("No rollfile could be found in the current directory");

		while(rollfile.good())
		{
			std::string line;
			std::getline(rollfile, line);

			line = forward_slash(trim(line));

			// ignore blanks
			if(line.length() == 0)
				continue;

			// ignore lines starting with #
			if(line[0] == '#')
				continue;

			// cut the comment end off a line (e.g. "filepath.txt #comment")
			const auto position = line.find("#");
			if(position != std::string::npos)
				line = line.substr(0, position);

			infiles.push_back(line);
		}
	}
	// create a roll file from cmd args
	else
	{
		for(int i = 2; i < argc; ++i)
			infiles.push_back(trim(forward_slash(argv[i])));
	}

	create(argv[1], infiles);

	return 0;
}

// do the work
void create(const std::string &out_file, const std::vector<std::string> &in_files)
{
	// number of input files
	const int file_count = in_files.size();

	// make sure the output file doesn't exist, or is already an asset roll
	if(exists(out_file) && !is_asset_roll(out_file))
		throw std::runtime_error("Not willing to overwrite file \""s + out_file + "\"");

	// make sure all the input files exist
	for(const std::string &file : in_files)
	{
		if(!exists(strip_options(file)))
			throw std::runtime_error("File \"" + strip_options(file) + "\" does not exist");
	}

	// construct the headers

	std::vector<header> headers;
	for(int i = 0; i < file_count; ++i)
	{
		header h;

		// mostly placeholder data
		h.compressed = 0;
		h.uncompressed_size;
		h.begin = 0;
		h.size = 0;
		h.filename = strip_options(in_files[i]);
		h.filename_length = h.filename.length();

		headers.push_back(h);
	}

	// prepare output file
	std::ofstream out(out_file, std::ofstream::binary);

	// write magic
	out.write(magic.c_str(), magic.length());

	// embedded file count
	const std::uint16_t file_count_short = file_count;
	out.write((char*)&file_count_short, sizeof(file_count_short));

	// write the headers
	std::uint64_t offset = magic.length() + sizeof(std::uint16_t); // magic length plus filecount length
	for(const header &h : headers)
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
		const long long fsize = filesize(headers[i].filename);

		// fill in some missing header details
		headers[i].compressed = request_compression(in_files[i]);
		headers[i].uncompressed_size = fsize;
		headers[i].begin = offset;
		if(!headers[i].compressed)
			headers[i].size = fsize;

		std::ifstream in(headers[i].filename, std::ifstream::binary);
		if(!in)
			throw std::runtime_error("Could not open file \"" + headers[i].filename + "\" for reading");

		// write the file contents
		if(headers[i].compressed)
		{
			// compress with zlib; load the entire file into memory
			std::vector<unsigned char> contents(fsize);
			in.read((char*)contents.data(), fsize);
			if(in.gcount() != fsize)
				throw std::runtime_error("Could not read the contents of file \"" + headers[i].filename + "\"");

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
	out.seekp(magic.length() + sizeof(std::uint16_t));
	for(const header &h : headers)
	{
		out.write((const char*)&h.compressed, sizeof(h.compressed));
		out.write((const char*)&h.uncompressed_size, sizeof(h.uncompressed_size));
		out.write((const char*)&h.begin, sizeof(h.begin));
		out.write((const char*)&h.size, sizeof(h.size));

		out.seekp(sizeof(h.filename_length) + h.filename_length, std::ofstream::cur);
	}

	out.close();

	std::cout << file_count << " files written to \"" << out_file << "\" (" << format(filesize(out_file)) << ")" << std::endl;
}

void inspect(const std::string &roll)
{
	if(!exists(roll))
		throw std::runtime_error("File \"" + roll + "\" does not exist");
	else if(!is_asset_roll(roll))
		throw std::runtime_error("File \"" + roll + "\" does not appear to be an asset roll");

	std::ifstream in(roll, std::ifstream::binary);
	if(!in)
		throw std::runtime_error("Could not open file \"" + roll + "\" in read mode");

	in.seekg(magic.size());

	// read number of files
	std::uint16_t file_count = 0;
	in.read((char*)&file_count, sizeof(file_count));
	if(in.gcount() != sizeof(file_count))
		throw std::runtime_error("File \"" + roll + "\" is corrupt");

	std::cout << roll << ": " << file_count << " files" << std::endl;

	for(int i = 0; i < file_count; ++i)
	{
		header rh;

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

std::string format(size_t size)
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

// strip the suffix options on filename (e.g. "image1.png:z" -> "image1.png")
std::string strip_options(const std::string &filename)
{
	const std::string::size_type position = filename.rfind(":z");
	if(position == std::string::npos)
		return filename;
	else
		return filename.substr(0, filename.length() - 2);
}

// replace backslashes with forward slashes
std::string forward_slash(const std::string &name)
{
	if(name.find('\\') == std::string::npos)
		return name;

	std::string newname = name;
	for(char &c : newname)
		if(c == '\\')
			c = '/';

	return newname;
}

// remove whitespace on both sides
std::string trim(const std::string &s)
{
	std::string str = s;

	// trim beginning
	for(int i = 0; i < str.size(); ++i)
	{
		if(isspace(str[i]))
		{
			str.erase(str.begin() + i);
			--i;
		}
	}

	// trim end
	for(int i = str.size() - 1; i >= 0; --i)
	{
		if(isspace(str[i]))
		{
			str.erase(str.begin() + i);
			++i;
		}
	}

	return str;
}

bool request_compression(const std::string &filename)
{
	return filename.rfind(":z") != std::string::npos;
}

// is the file an asset roll file?
bool is_asset_roll(const std::string &filename)
{
	std::ifstream in(filename, std::ifstream::binary);
	char magic[10];
	in.read(magic, sizeof(magic) - 1);
	if(in.gcount() != sizeof(magic) - 1)
		return false;
	magic[9] = 0;
	return !strcmp(magic, "ASSETROLL");
}

#if defined __linux__
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

long long filesize(const std::string &filename)
{
	struct stat s;
	if(stat(filename.c_str(), &s))
		throw std::runtime_error("stat() failed (\"" + filename + "\"): " + strerror(errno));

	return s.st_size;
}

bool exists(const std::string &filename)
{
	struct stat s;
	if(stat(filename.c_str(), &s))
	{
		if(errno == ENOENT)
			return false;
		else
			throw std::runtime_error("stat() failed (\"" + filename + "\"): " + strerror(errno));
	}
	else
		return true;
}

#elif defined _WIN32
#include <windows.h>

long long filesize(const std::string &filename)
{
	HANDLE file = CreateFile(filename.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(file == INVALID_HANDLE_VALUE)
		throw std::runtime_error("CreateFile() failed (\"" + filename + "\")");

	LARGE_INTEGER li;
	if(!GetFileSizeEx(file, &li))
		throw std::runtime_error("GetFileSizeEx() failed (\"" + filename + "\")");

	CloseHandle(file);
	return li.QuadPart;
}

bool exists(const std::string &filename)
{
	return GetFileAttributes(filename.c_str()) != INVALID_FILE_ATTRIBUTES;
}

#endif
