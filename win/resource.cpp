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
	: in_(filename)
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
	std::vector<unsigned char> data(len);

	for(long long i = 0; i < len; ++i)
	{
		in_.read((std::ifstream::char_type*)&data[i], 1);
		if(in_.gcount() != 1)
			throw exception("could not read 1 byte");
	}

	return data;
}
