#ifndef WIN_RESOURCE_H
#define WIN_RESOURCE_H

#include <vector>
#include <fstream>
#include <string>

namespace win
{

class resource
{
public:
	class error;

	resource(const char*);
	resource(const resource&) = delete;
	resource(resource&&);

	resource &operator=(const resource&) = delete;
	resource &operator=(resource&&);

	long long size() const;
	long long read(void*, long long);
	std::vector<unsigned char> read(long long = -1);

private:
	std::ifstream in_;
	std::string filename_;

public:
	class error : public std::runtime_error
	{
	public:
		error(const char *msg) : runtime_error(msg) {}
	};
};

}

#endif
