#ifndef WIN_ROLL_H
#define WIN_ROLL_H

#include <vector>
#include <fstream>
#include <string>

namespace win
{

struct roll_header
{
	std::uint8_t compressed;
	std::uint64_t uncompressed_size;
	std::uint64_t begin;
	std::uint64_t size;
	std::uint16_t filename_length;
	std::string filename;
};

class roll
{
public:
	roll(const char*);
	roll(const roll&) = delete;
	roll(roll&&);

	void operator=(const roll&) = delete;
	roll &operator=(roll&&);

	data operator[](const char*);
	data operator[](const std::string&);

	data_list all(const char*);
	data_list all();

private:
	std::vector<roll_header> files_;
	std::ifstream stream_;
};

}

#endif
