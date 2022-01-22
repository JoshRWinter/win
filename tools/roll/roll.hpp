#ifndef ROLL_HPP
#define ROLL_HPP

#include <string>

struct RollItem
{
	RollItem(const std::string &source_file, const std::string &recorded_file, bool compress)
		: source_file(recorded_file), recorded_file(recorded_file), compress(compress) {}

	std::string source_file;
	std::string recorded_file;
    bool compress;
};

struct Header
{
	std::uint8_t compressed;
	std::uint64_t uncompressed_size;
	std::uint64_t begin;
	std::uint64_t size;
	std::uint16_t filename_length;
	std::string filename;

	std::uint64_t length() const { return sizeof(compressed) + sizeof(uncompressed_size) + sizeof(begin) + sizeof(size) + sizeof(filename_length) + filename_length; }
};

#endif
