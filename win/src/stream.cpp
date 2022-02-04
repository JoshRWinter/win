#include <win/stream.hpp>

namespace win
{

Stream::Stream(StreamImpl *provider)
	: inner(provider)
{
}

unsigned long long Stream::size() const
{
	return inner->size();
}

void Stream::read(void *buf, unsigned long long len)
{
	inner->read(buf, len);
}

std::unique_ptr<unsigned char[]> Stream::read_all()
{
	return inner->read_all();
}

std::string Stream::read_all_as_string()
{
	return inner->read_all_as_string();
}

void Stream::seek(unsigned long long pos)
{
	inner->seek(pos);
}

unsigned long long Stream::tell()
{
	return inner->tell();
}

StreamImpl::~StreamImpl() {}

}
