#include "win.h"

#include <string.h>

#if defined WINPLAT_LINUX
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

static long long getsize(const char *filename)
{
	struct stat s;
	if(::stat(filename, &s))
		throw win::exception(std::string("stat() returned error on file \"") + filename + "\"");

	return s.st_size;
}

#endif

win::resource::resource(const char *filename)
	: in_(filename, std::ifstream::binary)
	, filename_(filename)
{
	if(!in_)
		throw error((std::string("File \"") + filename + "\" not found").c_str());
}

win::resource::resource(resource &&rhs)
	: in_(std::move(rhs.in_))
	, filename_(std::move(rhs.filename_))
{
}

win::resource &win::resource::operator=(resource &&rhs)
{
	in_ = std::move(rhs.in_);
	filename_ = std::move(rhs.filename_);

	return *this;
}

long long win::resource::size() const
{
	return getsize(filename_.c_str());
}

long long win::resource::read(void *buffer, long long count)
{
	in_.read((std::ifstream::char_type*)buffer, count);
	return in_.gcount();
}

std::vector<unsigned char> win::resource::read()
{
	const auto len = size();
	std::cerr << "size is " << len;
	std::vector<unsigned char> data(len);

	in_.seekg(0);

	int sofar = 0;
	while(sofar != len)
	{
		in_.read((std::ifstream::char_type*)(data.data() + sofar), len - sofar);
		const int got = in_.gcount();
		if(got < 0)
			throw exception("could not read from file");
		sofar += got;
	}

	return data;
}
