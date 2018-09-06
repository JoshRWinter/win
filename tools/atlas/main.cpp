#include <iostream>
#include <fstream>
#include <memory>
#include <vector>
#include <string>
#include <stdexcept>
#include <exception>

#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "Atlas.h"

using namespace std::string_literals;

struct input
{
	std::string filename;
	unsigned xpos;
	unsigned ypos;
};

static int go(int, char**);
static void calc_size(int,std::string&);
static void usage(const char*);
static const char *fileext(const char*);
static bool icompare(const char*,const char*);
static std::vector<input> collect_inputs(const char*);

int main(int argc,char **argv){
	// usage string
	if(argc < 3){
		usage(argv[0]);
		return 1;
	}

	try
	{
		return go(argc, argv);
	}
	catch(const std::exception &e)
	{
		std::cerr << "Fatal error: " << e.what() << std::endl;
		return 1;
	}
}

int go(int argc, char **argv){
	// collect the input files
	std::vector<input> inputs = collect_inputs(argv[2]);

	Atlas atlas;

	// iterate over input files
	for(const input &in : inputs){
		try{
			// construct Targa object
			Targa tga(in.filename); // might throw TextureError
			atlas.add(tga, in.xpos, in.ypos);
		}
		catch(const TextureError &te){
			std::cout<<"error on \""<<in.filename<<"\": "<<te.what()<<std::endl;
			std::cout<<"aborting..."<<std::endl;
			return 1;
		}
	}

	// write to output file
	const char *const output_ext=fileext(argv[1]);
	int bytes;
	if(icompare(output_ext,"tga"))
		// write atlas to tga file
		bytes=atlas.write_tga(argv[1]);
	else
		bytes=atlas.write(argv[1]);

	// display write info
	std::string formatted_size;
	calc_size(bytes,formatted_size);
	if(bytes>0)
		std::cout<<atlas.get_count()<<" image(s) successfully written to \""<<argv[1]<<"\" ("<<formatted_size<<")" << std::endl;
	else
		std::cout<<"error: could not open output file \""<<argv[1]<<"\" in write mode"<<std::endl;

	return 0;
}

static void calc_size(int bytes,std::string &size){
	char s[100];
	if(bytes>1048576)
		sprintf(s,"%.2f MB",(double)bytes/1048576);
	else
		sprintf(s,"%.2f KB",(double)bytes/1024);
	size=s;
}

static void usage(const char *name){
	std::cout<<"Usage: "<<name<<" output_file file1.tga [file2.tga ...]"<<std::endl;
}

static const char *fileext(const char *name){
	const int len=strlen(name);
	for(int i=len-1;i>=0;--i)
		if(name[i]=='.')
			return name+i+1;

	return name;
}

// portable strcasecmp
static bool icompare(const char *a,const char *b){
	int len=strlen(a);
	if(strlen(b)!=len)
		return false;

	for(int i=0;i<len;++i){
		if(tolower(a[i])!=tolower(b[i]))
			return false;
	}

	return true;
}

std::vector<input> collect_inputs(const char *fname)
{
	std::ifstream in(fname);
	if(!in)
		throw std::runtime_error("Couldn't open input file \""s + fname + "\"");

	std::vector<input> inputs;

	while(in.good())
	{
		std::string line;
		std::getline(in, line);

		if(line.length() == 0)
			continue;

		// line looks like
		// filename.tga, 20, 45"
		input inp;

		const auto rightquote = line.find("\"", 1);
		if(rightquote == std::string::npos)
			throw std::runtime_error("Parsing error: couldn't find right quote");

		inp.filename = line.substr(1, rightquote - 1);
		if(sscanf(line.data() + rightquote + 1, "%u %u", &inp.xpos, &inp.ypos) != 2)
			throw std::runtime_error("Parsing error: couldn't read x and y coords");

		inputs.push_back(inp);
	}

	return inputs;
}
