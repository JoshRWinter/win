all:
	g++ -o driver -std=c++17 -Wall -Wno-unused-but-set-variable -g `pkg-config --cflags freetype2` driver.cpp ../win/*.cpp -lGL -lX11 -lfreetype -logg -lvorbis -lpulse -pthread
	./driver
