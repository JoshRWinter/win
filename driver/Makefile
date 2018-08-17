all:
	/home/josh/win/tools/roll /home/josh/win/driver/assets.roll `cat /home/josh/win/driver/roll.assets`
	g++ -o driver -std=c++17 -Wall -Wno-unused-but-set-variable -g `pkg-config --cflags freetype2` driver.cpp ../win/*.cpp -lGL -lX11 -lfreetype -logg -lvorbis -lpulse -pthread -lz
	./driver
