#include <iostream>
#include <fstream>
#include <stdexcept>
#include <string>
#include <cstdint>
#include <vector>
#include <memory>

#include <string.h>
#include <stdlib.h>

#include <zlib.h>

using namespace std::string_literals;

static constexpr const char *const helptext =
"Asset Roll\n"
"Usage: roll output-file inputfile[:z] [ ... ]"
;
static const std::string magic = "ASSETROLL";

static std::string strip_options(const std::string&);
static bool request_compression(const std::string&);
static bool is_asset_roll(const std::string&);
static long long filesize(const std::string&);
static bool exists(const std::string&);
static int go(int, char**);

int main(int argc, char **argv)
{
	if(argc < 3)
	{
		std::cout << helptext << std::endl;
		return 0;
	}

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

// do the work
int go(int argc, char **argv)
{
	// number of input files
	const int file_count = argc - 2;

	// make sure the output file doesn't exist, or is already an asset roll
	if(exists(argv[1]) && !is_asset_roll(argv[1]))
		throw std::runtime_error("Not willing to overwrite file \""s + argv[1] + "\"");

	// make sure all the input files exist
	for(int i = 2; i < argc; ++i)
	{
		const std::string stripped = strip_options(argv[i]);
		if(!exists(stripped))
			throw std::runtime_error("File \"" + stripped + "\" does not exist");
	}

	// construct the headers
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

	std::vector<header> headers;
	for(int i = 0; i < file_count; ++i)
	{
		header h;

		// mostly placeholder data
		h.compressed = 0;
		h.uncompressed_size;
		h.begin = 0;
		h.size = 0;
		h.filename = strip_options(argv[i + 2]);
		h.filename_length = h.filename.length();

		headers.push_back(h);
	}

	// prepare output file
	std::ofstream out(argv[1], std::ofstream::binary);

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
		headers[i].compressed = request_compression(argv[i + 2]);
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
	out.seekp(sizeof(magic) + sizeof(std::uint16_t));
	for(const header &h : headers)
	{
		out.write((const char*)&h.compressed, sizeof(h.compressed));
		out.write((const char*)&h.uncompressed_size, sizeof(h.uncompressed_size));
		out.write((const char*)&h.begin, sizeof(h.begin));
		out.write((const char*)&h.size, sizeof(h.size));
	}

	return 0;
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

bool request_compression(const std::string &filename)
{
	return filename.rfind(":z") != std::string::npos;
}

// is the file an asset roll file?
bool is_asset_roll(const std::string &filename)
{
	std::ifstream in(filename, std::ifstream::binary);
	char magic[10];
	in.read(magic, sizeof(magic));
	if(in.gcount() != sizeof(magic))
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

#endif
