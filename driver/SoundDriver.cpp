#include <thread>

#include <win/sound/SoundEngine.hpp>

int main()
{
	win::AssetRoll roll("/home/josh/soundtesting/music.roll");
	//win::AssetRoll roll("c:\\users\\josh\\desktop\\music.roll");
	win::SoundEngine se(roll);

	auto key = se.play("platform_destroy.ogg", 5, 1.0f, 1.0f, 1.0f);
	std::this_thread::sleep_for(std::chrono::seconds(20));

	return 0;
}
