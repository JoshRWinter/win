#ifndef WIN_ROLL_HPP
#define WIN_ROLL_HPP

#include <fstream>
#include <string>
#include <vector>
#include <memory>
#include <mutex>

#include <win/win.hpp>
#include <win/stream.hpp>

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
	explicit AssetRoll(const char*);

	Stream operator[](const char*);
	bool exists(const char*);

private:
	std::mutex guard;
	std::string asset_roll_name;
	std::vector<AssetRollResource> resources;
	std::ifstream stream;
};

}

#endif
