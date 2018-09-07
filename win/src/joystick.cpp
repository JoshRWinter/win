#include <win.h>

static void default_fn_event_button(win::button, bool)
{
}

static void default_fn_event_joystick(win::joystick_axis, int)
{
}

#if defined WINPLAT_LINUX

win::evdev_joystick::evdev_joystick()
{
	fd_ = -1;
	buffer_index_ = 0;
	memset(buffer_, 0, sizeof(buffer_));
	handler.key_button = default_fn_event_button;
	handler.js = default_fn_event_joystick;
}

win::evdev_joystick::evdev_joystick(evdev_joystick &&rhs)
{
	move(rhs);
}

win::evdev_joystick::~evdev_joystick()
{
	finalize();
}

win::evdev_joystick &win::evdev_joystick::operator=(evdev_joystick &&rhs)
{
	finalize();
	move(rhs);
	return *this;
}

void win::evdev_joystick::event_button(std::function<void(button, bool)> fn)
{
	handler.key_button = std::move(fn);
}

void win::evdev_joystick::event_joystick(fn_event_joystick fn)
{
	handler.js = std::move(fn);
}

void win::evdev_joystick::process()
{
	// try to find a joystick
	if(fd_ == -1 && !find())
		return;

	for(;;)
	{
		fd_set set;
		timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 0;

		FD_ZERO(&set);
		FD_SET(fd_, &set);

		const int result = select(fd_ + 1, &set, NULL, NULL, &tv);
		if(result == -1)
		{
			lost();
			return;
		}
		else if(!FD_ISSET(fd_, &set))
			return;

		const int want = INPUT_SIZE - buffer_index_;
		const int got = read(fd_, buffer_ + buffer_index_, want);

		if(got < 0)
		{
			lost();
			return;
		}

		if(got != want)
		{
			buffer_index_ += got;
			return;
		}

		// process the event
		input_event event;
		int index = 0;
		memcpy(&event.time.tv_sec, buffer_ + index, sizeof(event.time.tv_sec));
		index +=sizeof(event.time.tv_sec);
		memcpy(&event.time.tv_usec, buffer_ + index, sizeof(event.time.tv_usec));
		index += sizeof(event.time.tv_usec);
		memcpy(&event.type, buffer_ + index, sizeof(event.type));
		index += sizeof(event.type);
		memcpy(&event.code, buffer_ + index, sizeof(event.code));
		index += sizeof(event.code);
		memcpy(&event.value, buffer_ + index, sizeof(event.value));

		// dun care about sync events
		if(event.type == EV_SYN)
			continue;

		if(event.type == EV_ABS)
		{
			switch(event.code)
			{
				case ABS_RX:
					handler.js(joystick_axis::RIGHT_X, event.value);
					break;
				case ABS_RY:
					handler.js(joystick_axis::RIGHT_Y, event.value);
					break;
				case ABS_X:
					handler.js(joystick_axis::LEFT_X, event.value);
					break;
				case ABS_Y:
					handler.js(joystick_axis::LEFT_Y, event.value);
					break;
				case ABS_RZ:
					handler.js(joystick_axis::RIGHT_TRIGGER, event.value);
					break;
				case ABS_Z:
					handler.js(joystick_axis::LEFT_TRIGGER, event.value);
					break;
				case ABS_HAT0X:
					if(event.value != 0)
						handler.key_button(event.value < 0 ? win::button::JS_PAD_LEFT : win::button::JS_PAD_RIGHT, true);
					break;
				case ABS_HAT0Y:
					if(event.value != 0)
						handler.key_button(event.value < 0 ? win::button::JS_PAD_UP : win::button::JS_PAD_DOWN, true);
					break;
				default:
					fprintf(stderr, "\033[33;1mcode: %x\033[0m\n", event.code);
					break;
			}
		}
		else if(event.type == EV_KEY)
		{
			switch(event.code)
			{
				case BTN_X:
					handler.key_button(win::button::JS_X, event.value == 1);
					break;
				case BTN_Y:
					handler.key_button(win::button::JS_Y, event.value == 1);
					break;
				case BTN_B:
					handler.key_button(win::button::JS_B, event.value == 1);
					break;
				case BTN_A:
					handler.key_button(win::button::JS_A, event.value == 1);
					break;
				case BTN_TR:
					handler.key_button(win::button::JS_RBUMP, event.value == 1);
					break;
				case BTN_TL:
					handler.key_button(win::button::JS_LBUMP, event.value == 1);
					break;
				case BTN_THUMBR:
					handler.key_button(win::button::JS_RSTICK, event.value == 1);
					break;
				case BTN_THUMBL:
					handler.key_button(win::button::JS_LSTICK, event.value == 1);
					break;
				case BTN_SELECT:
					handler.key_button(win::button::JS_SELECT, event.value == 1);
					break;
				case BTN_START:
					handler.key_button(win::button::JS_START, event.value == 1);
					break;
				case BTN_MODE:
					handler.key_button(win::button::JS_GAMEPAD, event.value == 1);
					break;
				default:
					fprintf(stderr, "\033[33;1mcode: %x\033[0m\n", event.code);
					break;
			}
		}
	}
}

bool win::evdev_joystick::find()
{
	DIR *dir = opendir("/dev/input/by-id/");
	if(dir == NULL)
		bug("Could not open /dev/input/by-id/");

	dirent *ent;
	while((ent = readdir(dir)) != NULL)
	{
		const std::string fname = ent->d_name;

		if(fname.rfind("-event-joystick") != std::string::npos)
		{
			std::cerr << "Found " << fname.substr(0, fname.length() - 15) << std::endl;
			fd_ = open(("/dev/input/by-id/" + fname).c_str(), O_RDONLY);
			if(fd_ == -1)
				bug("Could not open input file: " + std::string(strerror(errno)));
			buffer_index_ = 0;
		}
	}

	closedir(dir);

	return false;
}

void win::evdev_joystick::lost()
{
	finalize();
	std::cerr << "Lost the joystick" << std::endl;
}

void win::evdev_joystick::move(evdev_joystick &rhs)
{
	fd_ = rhs.fd_;
	rhs.fd_ = -1;

	handler.key_button = std::move(rhs.handler.key_button);
	handler.js = std::move(rhs.handler.js);

	buffer_index_ = rhs.buffer_index_;

	memcpy(buffer_, rhs.buffer_, sizeof(buffer_));
}

void win::evdev_joystick::finalize()
{
	if(fd_ == -1)
		return;

	close(fd_);
	fd_ = -1;
}

#endif
