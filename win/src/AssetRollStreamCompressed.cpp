#include <cstring>

#include <win/Win.hpp>
#include <win/AssetRollStreamCompressed.hpp>

namespace win
{

AssetRollStreamCompressed::AssetRollStreamCompressed(unsigned char *buffer, unsigned long long len)
	: buffer(buffer)
	, position(0)
	, length(len)
{
}

unsigned long long AssetRollStreamCompressed::size() const
{
	return length;
}

void AssetRollStreamCompressed::read(void *buf, unsigned long long len)
{
	if (position + len > length)
		win::bug("AssetRollStreamCompressed: read past end");

	memcpy(buf, buffer.get() + position, len);
	position += len;
}

std::unique_ptr<unsigned char[]> AssetRollStreamCompressed::read_all()
{
	std::unique_ptr<unsigned char[]> buf(new unsigned char[length]);
	memcpy(buf.get(), buffer.get(), length);
	return buf;
}

std::string AssetRollStreamCompressed::read_all_as_string()
{
	std::unique_ptr<char[]> buf(new char[length + 1]);
	memcpy(buf.get(), buffer.get(), length);
	buf[length] = 0;
	return buf.get();
}

void AssetRollStreamCompressed::seek(unsigned long long pos)
{
	if (pos > length)
		win::bug("AssetRollStreamCompressed: seek past end");

	position = pos;
}

unsigned long long AssetRollStreamCompressed::tell()
{
	return position;
}

}
