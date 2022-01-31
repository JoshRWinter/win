#ifndef WIN_ROLL_HPP
#define WIN_ROLL_HPP

#include <vector>
#include <fstream>
#include <string>
#include <initializer_list>
#include <memory>
#include <mutex>

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
class AssetRoll
{
public:
	explicit AssetRoll(const char*);
	AssetRoll(const AssetRoll&) = delete;
	AssetRoll(AssetRoll&&) = delete;

	void operator=(const AssetRoll&) = delete;
	AssetRoll &operator=(AssetRoll&&) = delete;

	Stream operator[](const char*);
	bool exists(const char*);

private:
	std::mutex guard;
	std::string asset_roll_name;
	std::vector<AssetRollResource> resources;
	std::ifstream stream;
};

class StreamImpl;
class Stream
{
	friend class AssetRoll;
public:
	Stream(StreamImpl*);
	Stream(const Stream&) = delete;
	Stream(Stream&&) = default;

	Stream &operator=(const Stream&) = delete;

	unsigned long long size() const;
	void read(void*, unsigned long long len);
	void read_all(void*);
	std::unique_ptr<unsigned char[]> read_all();
	void seek(unsigned long long pos);
	unsigned long long tell();

private:
	std::unique_ptr<StreamImpl> inner;
};

class StreamImpl
{
public:
	StreamImpl() = default;
	StreamImpl(const StreamImpl&) = delete;
	StreamImpl(StreamImpl&&) = delete;
	virtual ~StreamImpl() = 0;

	void operator=(const StreamImpl&) = delete;
	void operator=(StreamImpl&&) = delete;

	virtual unsigned long long size() const = 0;
	virtual void read(void*, unsigned long long) = 0;
	virtual std::unique_ptr<unsigned char[]> read_all() = 0;
	virtual void seek(unsigned long long) = 0;
	virtual unsigned long long tell() = 0;
};

class AssetRollStreamRaw : public StreamImpl
{
public:
	AssetRollStreamRaw(const std::string&, unsigned long long, unsigned long long);
	AssetRollStreamRaw(const AssetRollStreamRaw&) = delete;
	AssetRollStreamRaw(AssetRollStreamRaw&&) = delete;
	~AssetRollStreamRaw() {}

	void operator=(const AssetRollStreamRaw&) = delete;
	void operator=(AssetRollStreamRaw&&) = delete;

	unsigned long long size() const;
	void read(void*, unsigned long long);
	std::unique_ptr<unsigned char[]> read_all();
	void seek(unsigned long long);
	unsigned long long tell();

private:
	std::ifstream stream;
	unsigned long long begin;
	unsigned long long length;
};

class AssetRollStreamCompressed : public StreamImpl
{
public:
	AssetRollStreamCompressed(unsigned char*, unsigned long long);
	AssetRollStreamCompressed(const AssetRollStreamCompressed&) = delete;
	AssetRollStreamCompressed(AssetRollStreamCompressed&&) = delete;
	~AssetRollStreamCompressed() {}

	void operator=(const AssetRollStreamCompressed&) = delete;
	void operator=(AssetRollStreamCompressed&&) = delete;

	unsigned long long size() const;
	void read(void*, unsigned long long);
	std::unique_ptr<unsigned char[]> read_all();
	void seek(unsigned long long);
	unsigned long long tell();

private:
	std::unique_ptr<unsigned char[]> buffer;
	unsigned long long length;
	unsigned long long position;
};

}

#endif
