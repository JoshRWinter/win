#ifndef WIN_STREAM_HPP
#define WIN_STREAM_HPP

#include <memory>
#include <string>

namespace win
{

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
	virtual std::string read_all_as_string() = 0;
	virtual void seek(unsigned long long) = 0;
	virtual unsigned long long tell() = 0;
};

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
	std::unique_ptr<unsigned char[]> read_all();
	std::string read_all_as_string();
	void seek(unsigned long long pos);
	unsigned long long tell();

private:
	std::unique_ptr<StreamImpl> inner;
};

}

#endif
