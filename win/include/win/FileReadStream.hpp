#pragma once

#include <string>
#include <fstream>

#include <win/Win.hpp>
#include <win/Stream.hpp>

namespace win
{

class FileReadStream : public StreamBase
{
	WIN_NO_COPY_MOVE(FileReadStream);

public:
	explicit FileReadStream(const std::string &path);

	unsigned long long size() const override;

	void read(void*, unsigned long long) override;
	std::unique_ptr<unsigned char[]> read_all() override;
	std::string read_all_as_string() override;
	void seek(unsigned long long) override;
	unsigned long long tell() override;

private:
	std::string path;
	std::ifstream file;
};

}
