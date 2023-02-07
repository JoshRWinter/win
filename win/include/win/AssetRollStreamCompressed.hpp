#pragma once

#include <win/Win.hpp>
#include <win/Stream.hpp>

namespace win
{

class AssetRollStreamCompressed : public StreamImpl
{
	WIN_NO_COPY_MOVE(AssetRollStreamCompressed);

public:
	AssetRollStreamCompressed(unsigned char*, unsigned long long);

	unsigned long long size() const override;
	void read(void*, unsigned long long) override;
	std::unique_ptr<unsigned char[]> read_all() override;
	std::string read_all_as_string() override;
	void seek(unsigned long long) override;
	unsigned long long tell() override;

private:
	std::unique_ptr<unsigned char[]> buffer;
	unsigned long long length;
	unsigned long long position;
};

}
