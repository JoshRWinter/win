#include <fstream>
#include <string.h>
#include <stdexcept>

#include "Targa.h"

Targa::Targa(const std::string &filename){
	// open the file
	std::ifstream file(filename, std::ifstream::binary);
	if(!file)
		throw std::runtime_error("Could not open file");

	// image properties
	bool compressed; // targa has optional RLE
	unsigned char bpp; // bits per pixel
	bool bottom_origin;

	// compressed?
	unsigned char image_type;
	file.seekg(2);
	file.read((char*)&image_type,1);
	if(file.gcount()!=1)
		throw std::runtime_error("Could not read image type field");
	compressed=(image_type>>3)&1;
	if(compressed)
		throw std::runtime_error("Compressed TARGAs are not supported");

	// width
	unsigned short w;
	file.seekg(12);
	file.read((char*)&w,2);
	if(file.gcount()!=2)
		throw std::runtime_error("Could not read width field");
	width=w;

	// height
	unsigned short h;
	file.read((char*)&h,2);
	if(file.gcount()!=2)
		throw std::runtime_error("Could not read height field");
	height=h;

	// bpp
	file.seekg(16);
	file.read((char*)&bpp,1);
	if(file.gcount()!=1)
		throw std::runtime_error("Could not read bpp field");
	if(bpp!=32&&bpp!=24)
		throw std::runtime_error("Only 32 or 24 BPP TARGAs are supported");

	// image descriptor
	unsigned char imdesc;
	file.read((char*)&imdesc,1);
	if(file.gcount()!=1)
		throw std::runtime_error("Could not read image descriptor field");
	bool bit4,bit5;
	bottom_origin=!((imdesc>>5)&1);

	// allocate and read
	const int datasize=width*height*4;
	data=new unsigned char[datasize];
	if(bpp==32){
		file.read((char*)data,datasize);
		int read;
		if((read = file.gcount())!=datasize)
			throw std::runtime_error("Could not read bitmap");
	}
	else{
		// account for 24 bit images (missing alpha channel)
		const int datasize24=width*height*3;
		int index=0; // into the bitmap (this->data)
		int total=0; // total bytes read from file
		while(total!=datasize24){
			file.read((char*)(data+index),3);
			if(file.gcount()!=3)
				throw std::runtime_error("Could not read bitmap row");
			data[index+3]=255;
			total+=3;
			index+=4;
		}
	}

	// correct pixel storage
	bgr_to_rgb();
}

Targa::~Targa(){
	delete[] data;
}

void Targa::get_bitmap(unsigned char *bmp)const{
	memcpy(bmp,data,width*height*4);
}

// TGAs store pixels [blue, green, red, alpha]
// atlas supports only [red, green, blue, alpha]
void Targa::bgr_to_rgb(){
	for(int i=0;i<width*height*4;i+=4){
		int tmp=data[i];
		data[i]=data[i+2];
		data[i+2]=tmp;
	}
}
