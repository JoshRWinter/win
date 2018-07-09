#include "../win/win.h"

int main()
{
	win::system system;
	win::display display = system.make_display("window caption", 800, 600);

	return 0;
}
