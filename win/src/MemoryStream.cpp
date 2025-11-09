#include <cstring>

#include <win/MemoryStream.hpp>

namespace win
{

MemoryStream::MemoryStream(const unsigned char *data, unsigned long long len, bool owned)
	: owned(owned)
	, data(data)
	, position(0)
	, data_len(len)
{}

MemoryStream::~MemoryStream()
{
	if (owned)
		delete[] data;
}

unsigned long long MemoryStream::size() const
{
	return data_len;
}

void MemoryStream::read(void *dest, unsigned long long len)
{
	if (position + len > data_len)
		bug("MemoryStream: overread position = " + std::to_string(position) + ", data_len = " + std::to_string(data_len) + ", requested read = " + std::to_string(len));

	memcpy(dest, data + position, len);
	position += len;
}

std::unique_ptr<unsigned char[]> MemoryStream::read_all()
{
	std::unique_ptr<unsigned char[]> all(new unsigned char[data_len]);
	memcpy(all.get(), data, data_len);
	return all;
}

std::string MemoryStream::read_all_as_string()
{
	if (data[data_len - 1] == 0)
	{
		return (char*)data;
	}
	else
	{
		std::unique_ptr<char[]> all(new char[data_len + 1]);
		memcpy(all.get(), data, data_len);
		all[data_len] = 0;
		return all.get();
	}
}

void MemoryStream::seek(unsigned long long pos)
{
	if (pos > data_len)
		bug("MemoryStream: overseek length = " + std::to_string(data_len) + ", requested seek = " + std::to_string(pos));

	position = pos;
}

unsigned long long MemoryStream::tell()
{
	return position;
}

}
