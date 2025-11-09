#pragma once

#include <filesystem>
#include <fstream>

#include <win/Win.hpp>
#include <win/Stream.hpp>

namespace win
{

class FileStream : public StreamBase
{
public:
	explicit FileStream(const std::filesystem::path &path);
	explicit FileStream(const std::filesystem::path &path, unsigned long long start, unsigned long long length);

	unsigned long long size() const override;

	void read(void *dest, unsigned long long len) override;
	std::unique_ptr<unsigned char[]> read_all() override;
	std::string read_all_as_string() override;
	void seek(unsigned long long pos) override;
	unsigned long long tell() override;

private:
	void lazy_init();

	std::filesystem::path path;
	std::ifstream file;
	unsigned long long start;
	unsigned long long length;
};

}
