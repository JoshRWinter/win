#pragma once

#include <memory>
#include <string>

namespace win
{

class StreamBase
{
public:
	StreamBase() = default;
	StreamBase(const StreamBase&) = delete;
	StreamBase(StreamBase&&) = delete;
	virtual ~StreamBase() = default;

	void operator=(const StreamBase&) = delete;
	void operator=(StreamBase&&) = delete;

	virtual unsigned long long size() const = 0;
	virtual void read(void*, unsigned long long) = 0;
	virtual std::unique_ptr<unsigned char[]> read_all() = 0;
	virtual std::string read_all_as_string() = 0;
	virtual void seek(unsigned long long) = 0;
	virtual unsigned long long tell() = 0;
};

class Stream
{
	friend class AssetRoll;
public:
	explicit Stream(StreamBase *inner) : inner(inner) {}
	Stream(const Stream&) = delete;
	Stream(Stream&&) = default;

	Stream &operator=(const Stream&) = delete;

	unsigned long long size() const { return inner->size(); }
	void read(void *b, unsigned long long len) { inner->read(b, len); }
	std::unique_ptr<unsigned char[]> read_all() { return inner->read_all(); }
	std::string read_all_as_string() { return inner->read_all_as_string(); }
	void seek(unsigned long long pos) { inner->seek(pos); }
	unsigned long long tell() { return inner->tell(); }

private:
	std::unique_ptr<StreamBase> inner;
};

}
