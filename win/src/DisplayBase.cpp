#include <win/DisplayBase.hpp>

namespace win
{

DisplayBase::DisplayBase()
{
	window_handler = default_window_handler;
	resize_handler = default_resize_handler;
	button_handler = default_button_handler;
	character_handler = default_character_handler;
	mouse_handler = default_mouse_handler;
}

void DisplayBase::register_window_handler(WindowHandler handler)
{
	window_handler = std::move(handler);
}

void DisplayBase::register_resize_handler(ResizeHandler handler)
{
	resize_handler = std::move(handler);
}

void DisplayBase::register_button_handler(ButtonHandler handler)
{
	button_handler = std::move(handler);
}

void DisplayBase::register_character_handler(CharacterHandler handler)
{
	character_handler = std::move(handler);
}

void DisplayBase::register_mouse_handler(MouseHandler handler)
{
	mouse_handler = std::move(handler);
}

}
