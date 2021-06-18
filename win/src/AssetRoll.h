#ifndef WIN_ROLL_H
#define WIN_ROLL_H

#include <vector>
#include <fstream>
#include <string>
#include <initializer_list>
#include <memory>

namespace win
{


struct AssetRollResource
{
	std::uint8_t compressed;
	std::uint64_t uncompressed_size;
	std::uint64_t begin;
	std::uint64_t size;
	std::string name;

	std::string assetroll;
};

class AssetRollStream;
class AssetRoll
{
public:
	explicit AssetRoll(const char*);
	AssetRoll(const AssetRoll&) = delete;
	AssetRoll(AssetRoll&&) = delete;

	void operator=(const AssetRoll&) = delete;
	AssetRoll &operator=(AssetRoll&&) = delete;

	AssetRollResource operator[](const char*);
	bool exists(const char*) const;

private:
	std::string asset_roll_name;
	std::vector<AssetRollResource> resources;
	std::ifstream stream;
};

enum class AssetRollStreamKind
{
	file,
	memory
};

class AssetRollStream
{
	friend class AssetRoll;
public:
	AssetRollStream(const AssetRollResource&);
	AssetRollStream(const AssetRollStream&) = delete;
	AssetRollStream(AssetRollStream&&) = default;

	AssetRollStream &operator=(const AssetRollStream&) = delete;
	AssetRollStream &operator=(AssetRollStream&&) = default;

	unsigned long long size() const;
	void read(void*, unsigned long long len);
	void read_all(void*);
	std::unique_ptr<unsigned char[]> read_all();
	void seek(unsigned long long pos);
	unsigned long long tell();

	std::string name;

private:
	std::ifstream file_stream;

	std::unique_ptr<unsigned char[]> memory;
	unsigned long long position;

	AssetRollStreamKind kind;
	AssetRollResource resource;
};

}

#endif
