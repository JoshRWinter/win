#ifndef WIN_TARGA_HPP
#define WIN_TARGA_HPP

#include <win/stream.hpp>

namespace win
{

class Targa
{
public:
	Targa();
	Targa(Stream);
	Targa(const Targa&) = delete;
	Targa(Targa&&) = default;

	void operator=(const Targa&) = delete;
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

#endif
