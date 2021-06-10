#ifndef WIN_TEXTURE_H
#define WIN_TEXTURE_H

namespace win
{

enum TextureMode
{
	nearest,
	linear
};

class Texture
{
public:
	Texture();
	Texture(AssetRollStream, TextureMode);
	Texture(GLuint);
	Texture(const Texture&) = delete;
	Texture(Texture&&);
	~Texture();

	void operator=(const Texture&) = delete;
	Texture &operator=(Texture&&);

	unsigned get() const;

private:
	void finalize();
	void move(Texture&);
	void targa(AssetRollStream&);
	GLuint object;
};

}

#endif
