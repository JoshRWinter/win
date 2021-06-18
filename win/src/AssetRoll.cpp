#include <fstream>
#include <memory>
#include <string>

#include <string.h>
#include <zlib.h>

#include <win.h>

using namespace std::string_literals;

static void corrupt()
{
	win::bug("Corrupt asset roll");
}

namespace win
{

AssetRoll::AssetRoll(const char *file)
	: asset_roll_name(file)
{
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
		rh.assetroll = file;

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

AssetRollResource AssetRoll::operator[](const char *resourcename)
{
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

	return resources[index];
}

bool AssetRoll::exists(const char *resourcename) const
{
	for(const AssetRollResource &rh : resources)
		if(rh.name == resourcename)
			return true;

	return false;
}

AssetRollStream::AssetRollStream(const AssetRollResource &resource)
	: resource(resource)
{
	name = resource.name;
	std::ifstream rollstream(resource.assetroll, std::ifstream::binary);
	rollstream.seekg(resource.begin);

	if (resource.compressed)
	{
		kind = AssetRollStreamKind::memory;
		position = 0;

		uLongf uncompressed_size = resource.uncompressed_size;

		auto compressed_data = std::make_unique<unsigned char[]>(resource.size);
		memory = std::make_unique<unsigned char[]>(uncompressed_size);
		rollstream.read((char*)compressed_data.get(), resource.size);

		if(uncompress(memory.get(), &uncompressed_size, compressed_data.get(), resource.size) != Z_OK)
			win::bug("Could not inflate " + resource.name + " (" + resource.assetroll + ")");
	}
	else
	{
		kind = AssetRollStreamKind::file;
		file_stream = std::move(rollstream);
	}
}

unsigned long long AssetRollStream::size() const
{
	return resource.uncompressed_size;
}

void AssetRollStream::read(void *buf, unsigned long long len)
{
	if (kind == AssetRollStreamKind::file)
	{
		unsigned long long offset_into_stream = file_stream.tellg();
		unsigned long long offset_into_resource = offset_into_stream - resource.begin;
		unsigned long long left = resource.uncompressed_size - offset_into_resource;
		if (len > left)
			win::bug("Reading too many bytes from AssetRollStream " + resource.name + " (" + resource.assetroll + ")");

		file_stream.read((char*)buf, len);
		if (file_stream.gcount() != len)
			win::bug("Could not read " + std::to_string(len) + " bytes from AssetRollStream " + resource.name + " (" + resource.assetroll + ")");
	}
	else
	{
		unsigned long long left = resource.uncompressed_size - position;
		if (len > left)
			win::bug("Reading too many bytes from AssetRollStream " + resource.name + " (" + resource.assetroll + ")");

		memcpy(buf, memory.get() + position, len);
		position += len;
	}
}

void AssetRollStream::read_all(void *buf)
{
	if (kind == AssetRollStreamKind::file)
	{
		file_stream.seekg(resource.begin);
		file_stream.read((char*)buf, resource.uncompressed_size);
		if (file_stream.gcount() != resource.uncompressed_size)
			win::bug("Reading too many bytes from AssetRollStream " + resource.name + " (" + resource.assetroll + ")");
	}
	else
	{
		memcpy(buf, memory.get(), resource.uncompressed_size);
		position = resource.uncompressed_size;
	}
}

std::unique_ptr<unsigned char[]> AssetRollStream::read_all()
{
	auto dest = std::make_unique<unsigned char[]>(resource.uncompressed_size);

	if (kind == AssetRollStreamKind::file)
	{
		file_stream.seekg(resource.begin);
		file_stream.read((char*)dest.get(), resource.uncompressed_size);
	}
	else
	{
		memcpy(dest.get(), memory.get(), resource.uncompressed_size);
		position = resource.uncompressed_size;
	}

	return dest;
}

void AssetRollStream::seek(unsigned long long pos)
{
	if (kind == AssetRollStreamKind::file)
		file_stream.seekg(pos + resource.begin);
	else
		position = pos;
}

unsigned long long AssetRollStream::tell()
{
	if (kind == AssetRollStreamKind::file)
	{
		unsigned long long strpos = file_stream.tellg();

		return strpos - resource.begin;
	}
	else
		return position;
}

}
