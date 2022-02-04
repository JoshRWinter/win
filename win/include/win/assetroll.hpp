#ifndef WIN_ROLL_HPP
#define WIN_ROLL_HPP

#include <fstream>
#include <string>
#include <vector>
#include <memory>
#include <mutex>

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

class Stream;

class AssetRollStreamRaw : public StreamImpl
{
	WIN_NO_COPY_MOVE(AssetRollStreamRaw);

public:
	AssetRollStreamRaw(const std::string&, unsigned long long, unsigned long long);
	~AssetRollStreamRaw() {}

	unsigned long long size() const override;
	void read(void*, unsigned long long) override;
	std::unique_ptr<unsigned char[]> read_all() override;
	std::string read_all_as_string() override;
	void seek(unsigned long long) override;
	unsigned long long tell() override;

private:
	std::ifstream stream;
	unsigned long long begin;
	unsigned long long length;
};

class AssetRollStreamCompressed : public StreamImpl
{
	WIN_NO_COPY_MOVE(AssetRollStreamCompressed);

public:
	AssetRollStreamCompressed(unsigned char*, unsigned long long);
	~AssetRollStreamCompressed() {}

	unsigned long long size() const override;
	void read(void*, unsigned long long) override;
	std::unique_ptr<unsigned char[]> read_all() override;
	std::string read_all_as_string() override;
	void seek(unsigned long long) override;
	unsigned long long tell() override;

private:
	std::unique_ptr<unsigned char[]> buffer;
	unsigned long long length;
	unsigned long long position;
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
