#ifndef WIN_TEXTURE_H
#define WIN_TEXTURE_H

namespace win
{

class texture
{
public:
	texture();
	texture(const texture&) = delete;
	texture(texture&&);
	texture(data);
	~texture();

	void operator=(const texture&) = delete;
	texture &operator=(texture&&);
	texture &operator=(data);
	operator unsigned() const;

	void nearest();
	void linear();
	void finalize();

private:
	void targa(data);
	unsigned object_;
};

}

#endif
