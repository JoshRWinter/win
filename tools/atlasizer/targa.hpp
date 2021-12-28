#ifndef TARGA_HPP
#define TARGA_HPP

#include <fstream>
#include <memory>

class Targa
{
public:
	Targa(const std::string&);
	Targa(const Targa&) = delete;
	Targa(Targa&&) = default;

	void operator=(const Targa&) = delete;
	Targa &operator=(Targa&&) = default;

	int width() const;
	int height() const;
	int bpp() const;
	const unsigned char *data() const;
	unsigned char *release();

private:
	void load_image_bytes(std::ifstream&);

	std::string filename;
	unsigned short w, h;
	unsigned char bits;
	std::unique_ptr<unsigned char[]> bytes;
};

#endif
