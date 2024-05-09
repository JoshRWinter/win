#include <filesystem>

#include <win/FileReadStream.hpp>

namespace win
{

FileReadStream::FileReadStream(const std::string &path)
	: path(path)
{
	file = std::move(std::ifstream(path, std::ifstream::binary));
	if (!file.good())
		win::bug("FileReadStream: can't read " + path);
}

unsigned long long FileReadStream::size() const
{
	return std::filesystem::file_size(path);
}

void FileReadStream::read(void *buf, unsigned long long size)
{
	file.read((char*)buf, size);
	if (file.gcount() != size)
		win::bug("FileReadStream: couldn't read " + std::to_string(size) + " bytes");
}

std::unique_ptr<unsigned char[]> FileReadStream::read_all()
{
	const auto fsize = size();
	std::unique_ptr<unsigned char[]> bytes(new unsigned char[fsize]);
	file.read((char*)bytes.get(), fsize);
	if (file.gcount() != fsize)
		win::bug("FileReadStream: couldn't read " + std::to_string(fsize) + " bytes");

	return bytes;
}

std::string FileReadStream::read_all_as_string()
{
	const auto fsize = size();
	std::unique_ptr<char[]> bytes(new char[fsize]);
	file.read(bytes.get(), fsize);
	if (file.gcount() != fsize)
		win::bug("FileReadStream: couldn't read " + std::to_string(fsize) + " bytes");

	return std::string(bytes.get());
}

void FileReadStream::seek(unsigned long long pos)
{
	file.seekg(pos);
}

unsigned long long FileReadStream::tell()
{
	return file.tellg();
}

}
