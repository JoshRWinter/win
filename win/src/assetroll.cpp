#include <fstream>
#include <string>

#include <string.h>
#include <zlib.h>

#include <win/win.hpp>
#include <win/assetroll.hpp>
#include <win/assetrollstreamcompressed.hpp>
#include <win/assetrollstreamraw.hpp>

using namespace std::string_literals;

static void corrupt()
{
	win::bug("Corrupt asset roll");
}

namespace win
{

AssetRoll::AssetRoll(const char *file)
{
	std::lock_guard lock(guard);
	asset_roll_name = file;
	stream.open(file, std::ifstream::binary);
	if(!stream)
		win::bug("Could not read asset roll file \""s + file + "\"");

	// read the headers
	char magic[10];
	stream.read(magic, sizeof(magic) - 1);
	if(stream.gcount() != sizeof(magic) - 1)
		corrupt();
	magic[9] = 0;
	if(strcmp(magic, "ASSETROLL"))
		win::bug("File \""s + file + "\" is not an asset roll");

	// number of files stored within
	std::uint16_t file_count = 0;
	stream.read((char*)&file_count, sizeof(file_count));
	if(stream.gcount() != sizeof(file_count))
		corrupt();

	for(int i = 0; i < file_count; ++i)
	{
		AssetRollResource rh;
		stream.read((char*)&rh.compressed, sizeof(rh.compressed));
		if(stream.gcount() != sizeof(rh.compressed))
			corrupt();

		stream.read((char*)&rh.uncompressed_size, sizeof(rh.uncompressed_size));
		if(stream.gcount() != sizeof(rh.uncompressed_size))
			corrupt();

		stream.read((char*)&rh.begin, sizeof(rh.begin));
		if(stream.gcount() != sizeof(rh.begin))
			corrupt();

		stream.read((char*)&rh.size, sizeof(rh.size));
		if(stream.gcount() != sizeof(rh.size))
			corrupt();

		std::uint16_t filename_length;
		stream.read((char*)&filename_length, sizeof(filename_length));
		if(stream.gcount() != sizeof(filename_length))
			corrupt();

		auto fname = std::make_unique<char[]>(filename_length + 1);
		stream.read((char*)fname.get(), filename_length);
		if(stream.gcount() != filename_length)
			corrupt();
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
	for(int i = 0; i < (int)resources.size(); ++i)
	{
		if(resources[i].name == resourcename)
		{
			index = i;
			break;
		}
	}

	if(index == -1)
		win::bug("AssetRoll: no asset " + std::string(resourcename));

	const AssetRollResource &resource = resources[index];

	if (resource.compressed)
	{
		uLongf uncompressed_size = resource.uncompressed_size;

		auto compressed_data = std::make_unique<unsigned char[]>(resource.size);
		auto data = new unsigned char[uncompressed_size];
		stream.seekg(resource.begin);
		stream.read((char*)compressed_data.get(), resource.size);

		if(uncompress(data, &uncompressed_size, compressed_data.get(), resource.size) != Z_OK)
			win::bug("Could not inflate " + resource.name + " (" + resourcename + ")");

		return Stream(new AssetRollStreamCompressed(data, resource.uncompressed_size));
	}
	else
	{
		return Stream(new AssetRollStreamRaw(asset_roll_name, resource.begin, resource.size));
	}
}

bool AssetRoll::exists(const char *resourcename)
{
	std::lock_guard lock(guard);
	for(const AssetRollResource &rh : resources)
		if(rh.name == resourcename)
			return true;

	return false;
}

}
