#include <fstream>
#include <memory>

#include "win.h"

using namespace std::string_literals;

static void corrupt()
{
	throw win::exception("Corrupt asset-roll");
}

win::roll::roll(const char *file)
{
	std::ifstream in(file, std::ifstream::binary);
	if(!in)
		throw exception("Could not open roll file \""s + file + "\"");

	// read the headers
	char magic[9];
	in.read(magic, sizeof(magic));
	if(in.gcount() != sizeof(magic))
		corrupt();

	// number of files stored within
	std::uint16_t file_count;
	in.read((char*)&file_count, sizeof(file_count));
	if(in.gcount() != sizeof(file_count))
		corrupt();

	for(int i = 0; i < file_count; ++i)
	{
		roll_header rh;

		in.read((char*)&rh.compressed, sizeof(rh.compressed));
		if(in.gcount() != sizeof(rh.compressed))
			corrupt();

		in.read((char*)&rh.uncompressed_size, sizeof(rh.uncompressed_size));
		if(in.gcount() != sizeof(rh.uncompressed_size))
			corrupt();

		in.read((char*)&rh.begin, sizeof(rh.begin));
		if(in.gcount() != sizeof(rh.begin))
			corrupt();

		in.read((char*)&rh.size, sizeof(rh.size));
		if(in.gcount() != sizeof(rh.size))
			corrupt();

		in.read((char*)&rh.filename_length, sizeof(rh.filename_length));
		if(in.gcount() != sizeof(rh.filename_length))
			corrupt();

		std::unique_ptr<char[]> fname = std::make_unique<char[]>(rh.filename_length + 1);
		in.read((char*)fname.get(), rh.filename_length);
		if(in.gcount() != rh.filename_length)
			corrupt();
		fname[rh.filename_length] = 0;
		rh.filename = fname.get();

		files.push_back(rh);
	}

	std::cerr << "read " << file_count << " files:" << std::endl;
	int index = 1;
	for(const roll_header &rh : files)
	{
		std::cerr << "file " << index << std::endl;
		std::cerr << "\"" << rh.filename << "\" (" << rh.filename_length << ")" << std::endl;
		std::cerr << "compressed: " << (rh.compressed ? "yes" : "no") << std::endl;
		std::cerr << "uncompressed_size: " << rh.uncompressed_size << std::endl;
		std::cerr << "size: " << rh.size << std::endl;
		std::unique_ptr<char[]> contents = std::make_unique<char[]>(rh.size + 1);
		in.seekg(rh.begin);
		in.read(contents.get(), rh.size);
		contents[rh.size] = 0;
		++index;
	}
}
