#ifndef WIN_SYSTEM_H
#define WIN_SYSTEM_H

namespace win
{

class display;

class system
{
	friend class display;

public:
	system();
	system(const system&) = delete;
	system(system&&);
	~system();

	void operator=(const system&) = delete;
	system &operator=(system&&);

	display make_display(const char*, int, int, int = 0, window_handle = 0);

	static system &get();
};

}

#endif
