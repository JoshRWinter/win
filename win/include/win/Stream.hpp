#pragma once

#include <memory>

namespace win
{

class StreamBase
{
	WIN_NO_COPY_MOVE(StreamBase);
public:
	StreamBase() = default;
	virtual ~StreamBase() = default;

	virtual unsigned long long size() const = 0;
	virtual void read(void*, unsigned long long) = 0;
	virtual std::unique_ptr<unsigned char[]> read_all() = 0;
	virtual std::string read_all_as_string() = 0;
	virtual void seek(unsigned long long) = 0;
	virtual unsigned long long tell() = 0;
};

class Stream
{
	WIN_NO_COPY(Stream);
public:
	explicit Stream(StreamBase *inner) : inner(inner) {}
	Stream(Stream&&) = default;

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
