#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <mutex>

#include <win/Win.hpp>
#include <win/Stream.hpp>

namespace win
{

struct AssetRollResource
{
	std::uint8_t compressed;
	std::uint64_t uncompressed_size;
	std::uint64_t begin;
	std::uint64_t size;
	std::string name;
};

class AssetRoll
{
	WIN_NO_COPY_MOVE(AssetRoll);

public:
	explicit AssetRoll(const std::filesystem::path &filepath);
	AssetRoll(const unsigned char *data, unsigned long long len);

private:
	explicit AssetRoll(Stream stream);

public:

	Stream operator[](const char*);
	bool exists(const char*);

private:
	Stream substream(unsigned long long start, unsigned long long length);

	std::mutex guard;
	std::vector<AssetRollResource> resources;
	Stream stream;

	std::filesystem::path original_file;
	const unsigned char *original_data;
	unsigned long long original_data_length;
};

}
