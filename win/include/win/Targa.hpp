#pragma once

#include <win/Win.hpp>
#include <win/Stream.hpp>

namespace win
{

class Targa
{
	WIN_NO_COPY(Targa);

public:
	explicit Targa(Stream);
	Targa(Targa&&) = default;

	Targa &operator=(Targa&&) = default;

	int width() const;
	int height() const;
	int bpp() const;
	const unsigned char *data() const;;

private:
	void load_image_bytes(Stream&);

	unsigned short w, h;
	unsigned char bits;
	std::unique_ptr<unsigned char[]> bytes;
};

}
