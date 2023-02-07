#include <win/Win.hpp>
#include <win/AssetRollStreamRaw.hpp>

namespace win
{

AssetRollStreamRaw::AssetRollStreamRaw(const std::string &name, unsigned long long begin, unsigned long long length)
	: stream(std::move(std::ifstream(name, std::ifstream::binary)))
	, begin(begin)
	, length(length)
{
	if (!stream)
		win::bug("Couldn't open \"" + name + "\" for reading");

	stream.seekg(begin);
}

unsigned long long AssetRollStreamRaw::size() const
{
	return length;
}

void AssetRollStreamRaw::read(void *buf, unsigned long long len)
{
	if ((unsigned long long)stream.tellg() - begin > length)
		win::bug("AssetRollStreamRaw: read past end");

	stream.read((char*)buf, len);

	if (stream.gcount() != len)
		win::bug("AssetRollStreamRaw: read length mismatch");
}

std::unique_ptr<unsigned char[]> AssetRollStreamRaw::read_all()
{
	std::unique_ptr<unsigned char[]> memory(new unsigned char[length]);

	stream.read((char*)memory.get(), length);

	if (stream.gcount() != length)
		win::bug("AssetRollStreamRaw: read length mismatch");

	return memory;
}

std::string AssetRollStreamRaw::read_all_as_string()
{
	std::unique_ptr<char[]> memory(new char[length + 1]);

	stream.read(memory.get(), length);

	if (stream.gcount() != length)
		win::bug("AssetRollStreamRaw: read length mismatch");

	memory[length] = 0;

	return memory.get();
}

void AssetRollStreamRaw::seek(unsigned long long pos)
{
	stream.seekg(begin + pos);
}

unsigned long long AssetRollStreamRaw::tell()
{
	const long long spot = stream.tellg();
	if (spot == -1)
		win::bug("AssetRollStreamRaw: tellg() failure");

	return spot - begin;
}

}
