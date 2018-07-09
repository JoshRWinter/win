#include <thread>
#include <chrono>

#include "../win/win.h"

int main()
{
	win::system system;
	win::display display = system.make_display("window caption", 800, 600);

	win::event event;
	while((event = display.poll()) != win::event::CLOSE)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	return 0;
}
