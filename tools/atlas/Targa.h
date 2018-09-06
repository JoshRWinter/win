#ifndef TARGA_H
#define TARGA_H

#include "Texture.h"

class TargaErrorCompressed:public TextureError{
public:
	virtual const char *what()const noexcept{return "compressed TARGAs are not supported";}
};

class TargaErrorBPP:public TextureError{
public:
	virtual const char *what()const noexcept{return "unsupported BPP";}
};

class Targa:public Texture{
public:
	Targa(const std::string&);
	virtual ~Targa();
	virtual int get_width()const{return width;}
	virtual int get_height()const{return height;}
	virtual void get_bitmap(unsigned char*)const;

private:
	void bgr_to_rgb();

	int width;
	int height;
	unsigned char *data; // img data
};

#endif // TARGA_H
