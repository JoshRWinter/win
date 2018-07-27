all:
	g++ -o driver -Wall -Wno-unused-but-set-variable -g `pkg-config --cflags freetype2` driver.cpp ../win/*.cpp -lGL -lX11 -lfreetype -logg -lvorbis -pthread
	./driver
