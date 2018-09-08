#include <iostream>
#include <fstream>
#include <memory>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "Atlas.h"

Atlas::Atlas(){
	canvas=NULL;
	canvas_width=-1;
	canvas_height=-1;
}

Atlas::~Atlas(){
	// iterate over bitmaps and free img data
	for(const Bitmap &b:bitmap_list)
		delete[] b.bmp;
	// canvas
	delete[] canvas;
}

void Atlas::add(const Texture &texture, int xpos, int ypos){
	if(canvas!=NULL){
		// already compiled, can't add new textures
		return;
	}

	const int width=texture.get_width();
	const int height=texture.get_height();

	// get the image data from <texture>
	unsigned char *imgdata=new unsigned char[width*height*4];
	texture.get_bitmap(imgdata);

	// add to internal store (std::vector<Bitmap>)
	bitmap_list.push_back(Bitmap(width,height,xpos,ypos,imgdata,bitmap_list.size()));
}

// generate the atlas bitmap
void Atlas::compile(){
	if(canvas!=NULL){
		// already compiled
		return;
	}

	// allocate canvas
	canvas_width=proposed_width();
	canvas_height=proposed_height();
	canvas=new unsigned char[canvas_width*canvas_height*4];
	memset(canvas,0,canvas_width*canvas_height*4);

	// write the bitmaps
	for(const Bitmap &bmp:bitmap_list)
		insert(bmp);
}

int Atlas::write(const char *output){
	if(canvas==NULL)
		compile();

	// construct header
	std::vector<unsigned short> header;

	// save the number of textures in the header
	header.push_back(bitmap_list.size());

	// save the dimension of the atlas bitmap in the header
	header.push_back(canvas_width);
	header.push_back(canvas_height);

	// need to process them in the order they were added,
	// which is not the current order
	for(int i=0;i<bitmap_list.size();++i){
		// find the bitmap with id <i>
		int index=-1;
		for(int j=0;j<bitmap_list.size();++j){
			if(bitmap_list[j].id==i){
				index=j;
				break;
			}
		}
		if(index==-1)
			return 0;

		// save the coordinates in the header
		header.push_back(bitmap_list[index].xpos);
		header.push_back(bitmap_list[index].ypos);
		header.push_back(bitmap_list[index].width);
		header.push_back(bitmap_list[index].height);
	}

	// write to file
	std::ofstream out(output, std::ofstream::binary);
	if(!out){
		std::cout<<"error: could not open \""<<output<<"\" for writing"<<std::endl;
		return 0;
	}

	// write magic
	unsigned char magic[]={'A','T','L','A','S'};
	out.write((char*)magic, sizeof(magic));

	// write headers
	out.write((char*)header.data(), header.size() * sizeof(unsigned short));

	// write bitmap
	out.write((char*)canvas, get_canvas_size());

	return (header.size() * sizeof(unsigned short)) + get_canvas_size() + sizeof(magic);
}

int Atlas::write_tga(const char *output){
	if(canvas==NULL)
		compile();

	// construct tga header
	unsigned short header[9]={
		0x0000,
		0x0002,
		0x0000,
		0x0000,
		0x0000,
		0x0000,
		(unsigned short)canvas_width,
		(unsigned short)canvas_height,
		0x0820
	};

	// open the output file in write mode
	std::ofstream out(output, std::ofstream::binary);
	if(!out){
		std::cout<<"couldn't open "<<output<<" for writing"<<std::endl;
		return 0;
	}

	// write the header
	out.write((char*)header,18);
	// write the bitmap data
	out.write((char*)canvas,canvas_width*canvas_height*4);
	return 18+(canvas_width*canvas_height*4);
}

// insert <b> into the canvas at [b->xpos, b->ypos]
void Atlas::insert(const Bitmap &b){
	if(canvas==NULL)
		return;

	// for each row in <b.bmp>
	for(int row=0;row<b.height;++row){
		const int dest_row_index=(row+b.ypos)*canvas_width;
		const int dest_col_index=b.xpos;
		const int src_row_index=row*b.width;

		// copy the row from source to dest
		if((dest_row_index + dest_col_index) * 4 < 0)
		{
			std::cerr << "destination out of bounds negative" << std::endl;
			std::abort();
		}
		else if(src_row_index * 4 < 0)
		{
			std::cerr << "source out of bounds negative" << std::endl;
			std::abort();
		}
		if(((dest_row_index + dest_col_index) * 4) + (b.width * 4) >= get_canvas_size())
		{
			std::cerr << "destination out of bounds" << std::endl;
			std::abort();
		}
		else if(src_row_index * 4 >= b.width * b.height * 4)
		{
			std::cerr << "src out of bounds" << std::endl;
			std::abort();
		}
		memcpy(canvas + ((dest_row_index+dest_col_index)*4), b.bmp + (src_row_index*4), b.width*4);
	}
}

// determine the width of the canvas based on its current state
int Atlas::proposed_width()const{
	int w=0;
	for(const Bitmap &b:bitmap_list){
		if(b.xpos==-1||b.ypos==-1) // <b> hasn't been evaluated yet
			continue;

		if(b.xpos+b.width>w)
			w=b.xpos+b.width;
	}

	return w;
}

// determine the height of the canvas based on its current state
int Atlas::proposed_height()const{
	int h=0;
	for(const Bitmap &b:bitmap_list){
		if(b.xpos==-1||b.ypos==-1) // <b> hasn't been evaluated yet
			continue;

		if(b.ypos+b.height>h)
			h=b.ypos+b.height;
	}

	return h;
}
