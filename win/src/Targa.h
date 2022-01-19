#ifndef WIN_TEXTURE_H
#define WIN_TEXTURE_H

namespace win
{

class Targa
{
public:
	Targa();
	Targa(AssetRollStream);
	Targa(const Targa&) = delete;
	Targa(Targa&&) = default;

	void operator=(const Targa&) = delete;
	Targa &operator=(Targa&&) = default;

	int width() const;
	int height() const;
	int bpp() const;
	const unsigned char *data() const;;

private:
	void load_image_bytes(AssetRollStream&);

	unsigned short w, h;
	unsigned char bits;
	std::unique_ptr<unsigned char[]> bytes;
};

}

#endif
