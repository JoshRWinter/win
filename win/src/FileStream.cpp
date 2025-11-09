#include <win/FileStream.hpp>

namespace win
{

FileStream::FileStream(const std::filesystem::path &path)
	: FileStream(path, 0, std::filesystem::file_size(path))
{}

FileStream::FileStream(const std::filesystem::path &path, unsigned long long start, unsigned long long length)
	: path(path)
	, start(start)
	, length(length)
{
}

unsigned long long FileStream::size() const
{
	return length;
}

void FileStream::read(void *dest, unsigned long long len)
{
	lazy_init();

	const auto position = (unsigned long long)file.tellg() - start;
	if (position + len > length)
		bug("FileStream: overread position = " + std::to_string(position) + ", length = " + std::to_string(length) + ", requested read = " + std::to_string(len));

	file.read((char*)dest, len);
	if (file.gcount() != len)
		bug("FileStream: couldn't read " + std::to_string(len) + " bytes");
}

std::unique_ptr<unsigned char[]> FileStream::read_all()
{
	lazy_init();

	const auto last_pos = file.tellg();

	file.seekg(start);

	std::unique_ptr<unsigned char[]> bytes(new unsigned char[length]);
	file.read((char*)bytes.get(), length);

	if (file.gcount() != length)
		bug("FileStream: couldn't read " + std::to_string(length) + " bytes");

	file.seekg(last_pos);

	return bytes;
}

std::string FileStream::read_all_as_string()
{
	lazy_init();

	const auto last_pos = file.tellg();

	file.seekg(start);

	std::unique_ptr<char[]> bytes(new char[length + 1]);
	file.read(bytes.get(), length);
	bytes[length] = 0;

	if (file.gcount() != length)
		bug("FileStream: couldn't read " + std::to_string(length) + " bytes");

	file.seekg(last_pos);

	return bytes.get();
}

void FileStream::seek(unsigned long long pos)
{
	lazy_init();

	if (pos > length)
		bug("FileStream: overseek length = " + std::to_string(length) + ", requested seek = " + std::to_string(pos));

	file.seekg(start + pos);
}

unsigned long long FileStream::tell()
{
	lazy_init();

	return (unsigned long long)file.tellg() - start;
}

void FileStream::lazy_init()
{
	if (!file.is_open())
	{
		file = std::move(std::ifstream(path, std::ifstream::binary));

		if (!file.good())
			bug("FileStream: can't open " + path.string());

		if (start != 0)
			file.seekg(start);
	}
}

}
