#include <win/win.hpp>

#ifndef WIN_USE_SOUND_INTEGRATION_TESTS
int main() { fprintf(stderr, "\033[31;1mSound integration tests not enabled.\033[0m\n"); return 0; }
#else

#include <win/sound/soundengine.hpp>

#include <win/sound/test/soundintegrationtests.hpp>

#if defined WINPLAT_WINDOWS
#include <windows.h>
static void platform_sleep(int millis)
{
	Sleep(millis);
}
#elif defined WINPLAT_LINUX
#include <unistd.h>
static void platform_sleep(int millis)
{
	usleep(millis * 1000);
}
#endif

#define sit_assert(exp) if (!(exp)) win::bug("\033[31;1mAssert failed: line " + std::to_string(__LINE__) + "\033[0m")

void smoke_reset_stream(win::AssetRoll &roll)
{
	{
		win::SoundEngine se(roll);
		se.play("6secstereo.ogg", 5, 1.0f, true);
		platform_sleep(6500);
	}

	std::optional<win::sit::Event> event;
	event = win::sit::pop_event(win::sit::EventType::soundcache_first_load);
	sit_assert(event.has_value());
	sit_assert(event->name == "6secstereo.ogg");
	sit_assert(event->seek == 0);
	sit_assert(event->message == "SoundCache: No cache");

	event = win::sit::pop_event(win::sit::EventType::decoder_start);
	sit_assert(event.has_value());
	sit_assert(event->name == "6secstereo.ogg");
	sit_assert(event->seek == 0);
	sit_assert(event->message == "PCMDecoder: decoder start");

	event = win::sit::pop_event(win::sit::EventType::decoder_collect_channels);
	sit_assert(event.has_value());
	sit_assert(event->name == "6secstereo.ogg");
	sit_assert(event->seek == 0);
	sit_assert(event->message == "PCMDecoder: collecting channels");

	event = win::sit::pop_event(win::sit::EventType::decoder_writing_completed);
	sit_assert(event.has_value());
	sit_assert(event->name == "6secstereo.ogg");
	sit_assert(event->seek == 0);
	sit_assert(event->message == "PCMDecoder: writing completed");

	event = win::sit::pop_event(win::sit::EventType::decoder_reset_partial);
	sit_assert(event.has_value());
	sit_assert(event->name == "6secstereo.ogg");
	sit_assert(event->seek == 0);
	sit_assert(event->message == "PCMDecoder: reset, partial rehydration");

	event = win::sit::pop_event(win::sit::EventType::soundcache_resource_promoted);
	sit_assert(event.has_value());
	sit_assert(event->name == "6secstereo.ogg");
	sit_assert(event->seek == 0);
	sit_assert(event->message == "SoundCache: resource promoted");

	sit_assert(win::sit::pending_events() == 0);

	{
		win::SoundEngine se(roll);
		se.play("1secmono.ogg", 5, 1.0f, true);
		platform_sleep(1500);
	}

	event = win::sit::pop_event(win::sit::EventType::soundcache_first_load);
	sit_assert(event.has_value());
	sit_assert(event->name == "1secmono.ogg");
	sit_assert(event->seek == 0);
	sit_assert(event->message == "SoundCache: No cache");

	event = win::sit::pop_event(win::sit::EventType::decoder_start);
	sit_assert(event.has_value());
	sit_assert(event->name == "1secmono.ogg");
	sit_assert(event->seek == 0);
	sit_assert(event->message == "PCMDecoder: decoder start");

	event = win::sit::pop_event(win::sit::EventType::decoder_collect_channels);
	sit_assert(event.has_value());
	sit_assert(event->name == "1secmono.ogg");
	sit_assert(event->seek == 0);
	sit_assert(event->message == "PCMDecoder: collecting channels");

	event = win::sit::pop_event(win::sit::EventType::decoder_writing_completed);
	sit_assert(event.has_value());
	sit_assert(event->name == "1secmono.ogg");
	sit_assert(event->seek == 0);
	sit_assert(event->message == "PCMDecoder: writing completed");

	event = win::sit::pop_event(win::sit::EventType::decoder_reset_full);
	sit_assert(event.has_value());
	sit_assert(event->name == "1secmono.ogg");
	sit_assert(event->seek == 0);
	sit_assert(event->message == "PCMDecoder: reset, full rehydration");

	event = win::sit::pop_event(win::sit::EventType::soundcache_resource_promoted);
	sit_assert(event.has_value());
	sit_assert(event->name == "1secmono.ogg");
	sit_assert(event->seek == 0);
	sit_assert(event->message == "SoundCache: resource promoted");

	sit_assert(win::sit::pending_events() == 0);
}

void smoke_unloads(win::AssetRoll &roll)
{
	{
		win::SoundEngine se(roll);
		se.stop(se.play("3secstereo.ogg", 5, 1.0f, false));
		se.save();
		se.play("3secstereo.ogg", 5, 1.0f, false);
		se.play("3secstereo.ogg", 5, 1.0f, false);

		platform_sleep(3500);
	}

	// first
	std::optional<win::sit::Event> event;
	event = win::sit::pop_event(win::sit::EventType::soundcache_first_load);
	sit_assert(event.has_value());
	sit_assert(event->name == "3secstereo.ogg");
	sit_assert(event->seek == 0);
	sit_assert(event->message == "SoundCache: No cache");

	// second
	event = win::sit::pop_event(win::sit::EventType::soundcache_first_load);
	sit_assert(event.has_value());
	sit_assert(event->name == "3secstereo.ogg");
	sit_assert(event->seek == 0);
	sit_assert(event->message == "SoundCache: No cache");

	// third
	event = win::sit::pop_event(win::sit::EventType::soundcache_first_load);
	sit_assert(event.has_value());
	sit_assert(event->name == "3secstereo.ogg");
	sit_assert(event->seek == 0);
	sit_assert(event->message == "SoundCache: No cache");

	// first
	event = win::sit::pop_event(win::sit::EventType::decoder_start);
	sit_assert(event.has_value());
	sit_assert(event->name == "3secstereo.ogg");
	sit_assert(event->seek == 0);
	sit_assert(event->message == "PCMDecoder: decoder start");

	// second
	event = win::sit::pop_event(win::sit::EventType::decoder_start);
	sit_assert(event.has_value());
	sit_assert(event->name == "3secstereo.ogg");
	sit_assert(event->seek == 0);
	sit_assert(event->message == "PCMDecoder: decoder start");

	// third
	event = win::sit::pop_event(win::sit::EventType::decoder_start);
	sit_assert(event.has_value());
	sit_assert(event->name == "3secstereo.ogg");
	sit_assert(event->seek == 0);
	sit_assert(event->message == "PCMDecoder: decoder start");

	// first
	event = win::sit::pop_event(win::sit::EventType::decoder_collect_channels);
	sit_assert(event.has_value());
	sit_assert(event->name == "3secstereo.ogg");
	sit_assert(event->seek == 0);
	sit_assert(event->message == "PCMDecoder: collecting channels");

	// second
	event = win::sit::pop_event(win::sit::EventType::decoder_collect_channels);
	sit_assert(event.has_value());
	sit_assert(event->name == "3secstereo.ogg");
	sit_assert(event->seek == 0);
	sit_assert(event->message == "PCMDecoder: collecting channels");

	// third
	event = win::sit::pop_event(win::sit::EventType::decoder_collect_channels);
	sit_assert(event.has_value());
	sit_assert(event->name == "3secstereo.ogg");
	sit_assert(event->seek == 0);
	sit_assert(event->message == "PCMDecoder: collecting channels");

	// second
	event = win::sit::pop_event(win::sit::EventType::decoder_writing_completed);
	sit_assert(event.has_value());
	sit_assert(event->name == "3secstereo.ogg");
	sit_assert(event->seek == 0);
	sit_assert(event->message == "PCMDecoder: writing completed");

	// third
	event = win::sit::pop_event(win::sit::EventType::decoder_writing_completed);
	sit_assert(event.has_value());
	sit_assert(event->name == "3secstereo.ogg");
	sit_assert(event->seek == 0);
	sit_assert(event->message == "PCMDecoder: writing completed");

	// first
	event = win::sit::pop_event(win::sit::EventType::soundcache_incomplete_unload);
	sit_assert(event.has_value());
	sit_assert(event->name == "3secstereo.ogg");
	sit_assert(event->seek == 0);
	sit_assert(event->message == "SoundCache: removing incomplete resource");

	// second
	event = win::sit::pop_event(win::sit::EventType::soundcache_resource_promoted);
	sit_assert(event.has_value());
	sit_assert(event->name == "3secstereo.ogg");
	sit_assert(event->seek == 0);
	sit_assert(event->message == "SoundCache: resource promoted");

	// third
	event = win::sit::pop_event(win::sit::EventType::soundcache_duplicate_unload);
	sit_assert(event.has_value());
	sit_assert(event->name == "3secstereo.ogg");
	sit_assert(event->seek == 0);
	sit_assert(event->message == "SoundCache: removing duplicate resource");

	sit_assert(win::sit::pending_events() == 0);
}

void smoke_partial_cache(win::AssetRoll &roll)
{
	{
	    win::SoundEngine se(roll);
	    se.play("6secmono.ogg", 5, 1.0f, false, 0);
	    platform_sleep(6500);
	    se.play("6secmono.ogg", 5, 1.0f, false, 0);
	    platform_sleep(6500);
	}

	std::optional<win::sit::Event> event;
	event = win::sit::pop_event(win::sit::EventType::soundcache_first_load);
	sit_assert(event.has_value());
	sit_assert(event->name == "6secmono.ogg");
	sit_assert(event->seek == 0);
	sit_assert(event->message == "SoundCache: No cache");

	event = win::sit::pop_event(win::sit::EventType::decoder_start);
	sit_assert(event.has_value());
	sit_assert(event->name == "6secmono.ogg");
	sit_assert(event->seek == 0);
	sit_assert(event->message == "PCMDecoder: decoder start");

	event = win::sit::pop_event(win::sit::EventType::decoder_collect_channels);
	sit_assert(event.has_value());
	sit_assert(event->name == "6secmono.ogg");
	sit_assert(event->seek == 0);
	sit_assert(event->message == "PCMDecoder: collecting channels");

	event = win::sit::pop_event(win::sit::EventType::decoder_writing_completed);
	sit_assert(event.has_value());
	sit_assert(event->name == "6secmono.ogg");
	sit_assert(event->seek == 0);
	sit_assert(event->message == "PCMDecoder: writing completed");

	event = win::sit::pop_event(win::sit::EventType::soundcache_resource_promoted);
	sit_assert(event.has_value());
	sit_assert(event->name == "6secmono.ogg");
	sit_assert(event->seek == 0);
	sit_assert(event->message == "SoundCache: resource promoted");

	// now comes the second sound

	event = win::sit::pop_event(win::sit::EventType::soundcache_partial_load);
	sit_assert(event.has_value());
	sit_assert(event->name == "6secmono.ogg");
	sit_assert(event->seek == 0);
	sit_assert(event->message == "SoundCache: Partial cache found");

	event = win::sit::pop_event(win::sit::EventType::decoder_rehydrate);
	sit_assert(event.has_value());
	sit_assert(event->name == "6secmono.ogg");
	sit_assert(event->seek == 0);
	sit_assert(event->message == "PCMDecoder: initial stream hydration, fill 220500");

	event = win::sit::pop_event(win::sit::EventType::decoder_start);
	sit_assert(event.has_value());
	sit_assert(event->name == "6secmono.ogg");
	sit_assert(event->seek == 0);
	sit_assert(event->message == "PCMDecoder: decoder start");

	event = win::sit::pop_event(win::sit::EventType::decoder_writing_completed);
	sit_assert(event.has_value());
	sit_assert(event->name == "6secmono.ogg");
	sit_assert(event->seek == 0);
	sit_assert(event->message == "PCMDecoder: writing completed");

	event = win::sit::pop_event(win::sit::EventType::soundcache_normal_unload);
	sit_assert(event.has_value());
	sit_assert(event->name == "6secmono.ogg");
	sit_assert(event->seek == 0);
	sit_assert(event->message == "SoundCache: normal unload");

	sit_assert(win::sit::pending_events() == 0);
}

void smoke_full_cache(win::AssetRoll &roll)
{
    {
	    win::SoundEngine se(roll);

		se.play("1secmono.ogg", 5, 1.0f, false, 2);
		platform_sleep(1500);
		se.play("1secmono.ogg", 5, 1.0f, false, 2);
		platform_sleep(1500);
		se.play("1secmono.ogg", 5, 1.0f, false, 0);
		platform_sleep(1500);
	}

	std::optional<win::sit::Event> event;
	event = win::sit::pop_event(win::sit::EventType::soundcache_first_load);
	sit_assert(event.has_value());
	sit_assert(event->name == "1secmono.ogg");
	sit_assert(event->seek == 2);
	sit_assert(event->message == "SoundCache: No cache");

	event = win::sit::pop_event(win::sit::EventType::decoder_start);
	sit_assert(event.has_value());
	sit_assert(event->name == "1secmono.ogg");
	sit_assert(event->seek == 2);
	sit_assert(event->message == "PCMDecoder: decoder start");

	event = win::sit::pop_event(win::sit::EventType::decoder_collect_channels);
	sit_assert(event.has_value());
	sit_assert(event->name == "1secmono.ogg");
	sit_assert(event->seek == 2);
	sit_assert(event->message == "PCMDecoder: collecting channels");

	event = win::sit::pop_event(win::sit::EventType::decoder_writing_completed);
	sit_assert(event.has_value());
	sit_assert(event->name == "1secmono.ogg");
	sit_assert(event->seek == 2);
	sit_assert(event->message == "PCMDecoder: writing completed");

	event = win::sit::pop_event(win::sit::EventType::soundcache_resource_promoted);
	sit_assert(event.has_value());
	sit_assert(event->name == "1secmono.ogg");
	sit_assert(event->seek == 2);
	sit_assert(event->message == "SoundCache: resource promoted");

	// now comes the second sound

	event = win::sit::pop_event(win::sit::EventType::soundcache_full_load);
	sit_assert(event.has_value());
	sit_assert(event->name == "1secmono.ogg");
	sit_assert(event->seek == 2);
	sit_assert(event->message == "SoundCache: Full cache found");

	event = win::sit::pop_event(win::sit::EventType::decoder_rehydrate);
	sit_assert(event.has_value());
	sit_assert(event->name == "1secmono.ogg");
	sit_assert(event->seek == 2);
	sit_assert(event->message == "PCMDecoder: initial stream hydration, fill 44098");

	event = win::sit::pop_event(win::sit::EventType::decoder_full_rehydrate);
	sit_assert(event.has_value());
	sit_assert(event->name == "1secmono.ogg");
	sit_assert(event->seek == 2);
	sit_assert(event->message == "PCMDecoder: full stream hydration");

	event = win::sit::pop_event(win::sit::EventType::soundcache_normal_unload);
	sit_assert(event.has_value());
	sit_assert(event->name == "1secmono.ogg");
	sit_assert(event->seek == 2);
	sit_assert(event->message == "SoundCache: normal unload");

	// now comes the third sound

	event = win::sit::pop_event(win::sit::EventType::soundcache_first_load);
	sit_assert(event.has_value());
	sit_assert(event->name == "1secmono.ogg");
	sit_assert(event->seek == 0);
	sit_assert(event->message == "SoundCache: No cache");

	event = win::sit::pop_event(win::sit::EventType::decoder_start);
	sit_assert(event.has_value());
	sit_assert(event->name == "1secmono.ogg");
	sit_assert(event->seek == 0);
	sit_assert(event->message == "PCMDecoder: decoder start");

	event = win::sit::pop_event(win::sit::EventType::decoder_collect_channels);
	sit_assert(event.has_value());
	sit_assert(event->name == "1secmono.ogg");
	sit_assert(event->seek == 0);
	sit_assert(event->message == "PCMDecoder: collecting channels");

	event = win::sit::pop_event(win::sit::EventType::decoder_writing_completed);
	sit_assert(event.has_value());
	sit_assert(event->name == "1secmono.ogg");
	sit_assert(event->seek == 0);
	sit_assert(event->message == "PCMDecoder: writing completed");

	event = win::sit::pop_event(win::sit::EventType::soundcache_resource_promoted);
	sit_assert(event.has_value());
	sit_assert(event->name == "1secmono.ogg");
	sit_assert(event->seek == 0);
	sit_assert(event->message == "SoundCache: resource promoted");

	sit_assert(win::sit::pending_events() == 0);
}

int main()
{
	//win::AssetRoll roll("/home/josh/programming/win/tests/assets/sit.roll");
	win::AssetRoll roll("c:/users/josh/desktop/win/tests/assets/sit.roll");

	smoke_full_cache(roll);
	smoke_partial_cache(roll);
	smoke_unloads(roll);
	smoke_reset_stream(roll);

	fprintf(stderr, "\033[32;1mall tests passed\033[0m\n");
}

#endif
