#ifndef TEXTURE_H
#define TEXTURE_H

#include <exception>

class Texture{
public:
	virtual ~Texture(){};
	virtual int get_width()const=0;
	virtual int get_height()const=0;
	// caller will pass in pre allocated unsigned char bitmap[width()*height()*4]
	// bitmap format is rgba, one unsigned byte per channel, bottom left origin
	virtual void get_bitmap(unsigned char*)const=0;
};

#endif // TEXTURE_H
