#include <win.h>

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
