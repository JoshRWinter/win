#include <fstream>
#include <memory>
#include <string>

#include <string.h>
#include <zlib.h>

#include <win.h>

using namespace std::string_literals;

static void corrupt()
{
	throw win::exception("Corrupt asset-roll");
}

win::roll::roll(const char *file)
{
	stream_.open(file, std::ifstream::binary);
	if(!stream_)
		throw exception("Could not open roll file \""s + file + "\"");

	// read the headers
	char magic[10];
	stream_.read(magic, sizeof(magic) - 1);
	if(stream_.gcount() != sizeof(magic) - 1)
		corrupt();
	magic[9] = 0;
	if(strcmp(magic, "ASSETROLL"))
		throw exception("File \""s + file + "\" is not an asset roll");

	// number of files stored within
	std::uint16_t file_count = 0;
	stream_.read((char*)&file_count, sizeof(file_count));
	if(stream_.gcount() != sizeof(file_count))
		corrupt();

	for(int i = 0; i < file_count; ++i)
	{
		roll_header rh;

		stream_.read((char*)&rh.compressed, sizeof(rh.compressed));
		if(stream_.gcount() != sizeof(rh.compressed))
			corrupt();

		stream_.read((char*)&rh.uncompressed_size, sizeof(rh.uncompressed_size));
		if(stream_.gcount() != sizeof(rh.uncompressed_size))
			corrupt();

		stream_.read((char*)&rh.begin, sizeof(rh.begin));
		if(stream_.gcount() != sizeof(rh.begin))
			corrupt();

		stream_.read((char*)&rh.size, sizeof(rh.size));
		if(stream_.gcount() != sizeof(rh.size))
			corrupt();

		stream_.read((char*)&rh.filename_length, sizeof(rh.filename_length));
		if(stream_.gcount() != sizeof(rh.filename_length))
			corrupt();

		std::unique_ptr<char[]> fname = std::make_unique<char[]>(rh.filename_length + 1);
		stream_.read((char*)fname.get(), rh.filename_length);
		if(stream_.gcount() != rh.filename_length)
			corrupt();
		fname[rh.filename_length] = 0;
		rh.filename = fname.get();

		files_.push_back(rh);
	}
}

win::roll::roll(roll &&rhs)
{
	move(rhs);
}

win::roll &win::roll::operator=(roll &&rhs)
{
	move(rhs);
	return *this;
}

win::data win::roll::operator[](const char *filename)
{
	// make sure the file exists
	int index = -1;
	for(int i = 0; i < (int)files_.size(); ++i)
	{
		if(files_[i].filename == filename)
		{
			index = i;
			break;
		}
	}

	if(index == -1)
		return data();

	// read and return
	std::unique_ptr<unsigned char[]> contents = std::make_unique<unsigned char[]>(files_[index].size);
	stream_.seekg(files_[index].begin);
	stream_.read((char*)contents.get(), files_[index].size);
	if((long unsigned int)stream_.gcount() != files_[index].size)
		corrupt();

	if(files_[index].compressed)
	{
		unsigned long uncompressed_size = files_[index].uncompressed_size;
		unsigned char *rawdata = new unsigned char[uncompressed_size];
		if(uncompress(rawdata, &uncompressed_size, contents.get(), files_[index].size) != Z_OK)
		{
			delete[] rawdata;
			throw exception("Zlib: could not uncompress the file data");
		}

		return data(rawdata, uncompressed_size);
	}
	else
	{
		return data(contents.release(), files_[index].size);
	}
}

bool win::roll::exists(const char *filename) const
{
	for(const roll_header &rh : files_)
		if(rh.filename == filename)
			return true;

	return false;
}

// return a list of all the filenames in the roll
win::data_list win::roll::all()
{
	data_list list(this);

	for(const roll_header &header : files_)
		list.add(header.filename);

	return list;
}

win::data_list win::roll::select(const std::initializer_list<const char*> &files)
{
	data_list list(this);

	for(const char *file : files)
	{
		// see if the file exists in files_
		bool found = false;
		for(const roll_header &rh : files_)
			if(rh.filename == file)
			{
				found = true;
				break;
			}
		if(!found)
			bug("File: \""s + file + "\" -- no such entity");

		list.add(file);
	}

	return list;
}

void win::roll::move(roll &rhs)
{
	files_ = std::move(rhs.files_);
	stream_ = std::move(rhs.stream_);
}
