#ifndef TARGA_H
#define TARGA_H

#include "Texture.h"

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
