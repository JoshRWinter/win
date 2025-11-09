#pragma once

#include <win/Win.hpp>
#include <win/Stream.hpp>

namespace win
{

class MemoryStream : public StreamBase
{
public:
	explicit MemoryStream(const unsigned char *data, unsigned long long len, bool owned);
	~MemoryStream() override;

	unsigned long long size() const override;

	void read(void *dest, unsigned long long len) override;
	std::unique_ptr<unsigned char[]> read_all() override;
	std::string read_all_as_string() override;
	void seek(unsigned long long pos) override;
	unsigned long long tell() override;

private:
	bool owned;
	const unsigned char *data;
	unsigned long long position;
	unsigned long long data_len;
};

}
