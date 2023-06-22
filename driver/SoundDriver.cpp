#include <thread>

#include <win/sound/SoundEngine.hpp>

int main()
{
	win::AssetRoll roll("/home/josh/soundtesting/music.roll");
	//win::AssetRoll roll("c:\\users\\josh\\desktop\\music.roll");
	win::SoundEngine se(roll);

	se.play("platform_destroy.ogg", 5, 1.0f, 1.0f, 1.0f, false, true);
	std::this_thread::sleep_for(std::chrono::seconds(1));
	se.play("platform_destroy.ogg", 5, 1.0f, 1.0f, 1.0f, false, true);
	std::this_thread::sleep_for(std::chrono::seconds(1));
	se.play("platform_destroy.ogg", 5, 1.0f, 1.0f, 1.0f, true, true);
	std::this_thread::sleep_for(std::chrono::seconds(3));
	se.play("platform_destroy.ogg", 5, 1.0f, 1.0f, 1.0f, true, true);
	std::this_thread::sleep_for(std::chrono::seconds(5));

	return 0;
}
