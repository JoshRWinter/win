#include <fstream>
#include <string>
#include <cstring>

#include <zlib.h>

#include <win/Win.hpp>
#include <win/AssetRoll.hpp>
#include <win/MemoryStream.hpp>
#include <win/FileStream.hpp>

using namespace std::string_literals;

namespace win
{

AssetRoll::AssetRoll(const unsigned char *data, unsigned long long len)
	: AssetRoll(Stream(new MemoryStream(data, len, false)))
{
	original_file = "";
	original_data = data;
	original_data_length = len;
}

AssetRoll::AssetRoll(const std::filesystem::path &filepath)
	: AssetRoll(Stream(new FileStream(filepath)))
{
	original_file = filepath;
	original_data = NULL;
	original_data_length = 0;
}

AssetRoll::AssetRoll(Stream stream2)
	: stream(std::move(stream2))
{
	// read the headers
	char magic[10];
	stream.read(magic, sizeof(magic) - 1);

	magic[9] = 0;
	if(strcmp(magic, "ASSETROLL"))
		bug("Not an asset roll");

	// number of files stored within
	std::uint16_t file_count = 0;
	stream.read((char*)&file_count, sizeof(file_count));

	for(int i = 0; i < file_count; ++i)
	{
		AssetRollResource rh;
		stream.read((char*)&rh.compressed, sizeof(rh.compressed));
		stream.read((char*)&rh.uncompressed_size, sizeof(rh.uncompressed_size));
		stream.read((char*)&rh.begin, sizeof(rh.begin));
		stream.read((char*)&rh.size, sizeof(rh.size));

		std::uint16_t filename_length;
		stream.read((char*)&filename_length, sizeof(filename_length));

		auto fname = std::make_unique<char[]>(filename_length + 1);
		stream.read((char*)fname.get(), filename_length);
		fname[filename_length] = 0;
		rh.name = fname.get();

		resources.push_back(rh);
	}
}

Stream AssetRoll::operator[](const char *resourcename)
{
	std::lock_guard lock(guard);

	// make sure the file exists
	int index = -1;
	for (int i = 0; i < (int)resources.size(); ++i)
	{
		if(resources[i].name == resourcename)
		{
			index = i;
			break;
		}
	}

	if(index == -1)
		bug("AssetRoll: no asset " + std::string(resourcename));

	const AssetRollResource &resource = resources[index];

	if (resource.compressed)
	{
		uLongf uncompressed_size = resource.uncompressed_size;

		auto compressed_data = std::make_unique<unsigned char[]>(resource.size);
		stream.seek(resource.begin);
		stream.read((char*)compressed_data.get(), resource.size);

		auto data = new unsigned char[uncompressed_size];

		if(uncompress(data, &uncompressed_size, compressed_data.get(), resource.size) != Z_OK)
			bug("Could not inflate " + resource.name + " (" + resourcename + ")");

		return Stream(new MemoryStream(data, resource.uncompressed_size, true));
	}
	else
	{
		return substream(resource.begin, resource.size);
	}
}

bool AssetRoll::exists(const char *resourcename)
{
	std::lock_guard lock(guard);

	for (const AssetRollResource &rh : resources)
		if(rh.name == resourcename)
			return true;

	return false;
}

Stream AssetRoll::substream(unsigned long long start, unsigned long long length)
{
	if (original_file != "")
	{
		return Stream(new FileStream(original_file, start, length));
	}
	else if (original_data != NULL)
	{
		if (start + length > original_data_length)
			bug("AssetRoll: cannot substream length = " + std::to_string(original_data_length) + ", requested start = " + std::to_string(start) + ", requested length = " + std::to_string(length));

		return Stream(new MemoryStream(original_data + start, length, false));
	}
	else
	{
		bug("Asset roll: cannot substream");
	}
}

}
