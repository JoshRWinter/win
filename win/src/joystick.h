#ifndef WIN_JOYSTICK_H
#define WIN_JOYSTICK_H

#include <functional>

#if defined WINPLAT_LINUX
#include <linux/input.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
constexpr int INPUT_SIZE = sizeof(timeval::tv_sec) + sizeof(timeval::tv_usec) + sizeof(input_event::type) + sizeof(input_event::code) + sizeof(input_event::value);

namespace win
{

class evdev_joystick
{
public:
	typedef std::function<void(button, bool)> fn_event_button;
	typedef std::function<void(joystick_axis, int)> fn_event_joystick;

	evdev_joystick();
	evdev_joystick(const evdev_joystick&) = delete;
	evdev_joystick(evdev_joystick&&);
	~evdev_joystick();

	void operator=(const evdev_joystick&) = delete;
	evdev_joystick &operator=(evdev_joystick&&);

	void event_button(std::function<void(button, bool)>);
	void event_joystick(fn_event_joystick);

	void process();

private:
	bool find();
	void lost();
	void move(evdev_joystick&);
	void finalize();

	int fd_;
	unsigned char buffer_[INPUT_SIZE];
	int buffer_index_;
	struct
	{
		fn_event_button key_button;
		fn_event_joystick js;
	}handler;
};

}
#endif

#endif
