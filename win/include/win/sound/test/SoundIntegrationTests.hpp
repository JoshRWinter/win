#ifndef WIN_USE_SOUND_INTEGRATION_TESTS_HPP
#define WIN_USE_SOUND_INTEGRATION_TESTS_HPP

#include <win/Win.hpp>

#ifndef WIN_USE_SOUND_INTEGRATION_TESTS
#error "Do not include this header in a normal build!"
#endif

#include <list>
#include <mutex>
#include <optional>

namespace win
{

// sound integration tests
namespace sit
{

enum class EventType
{
	soundcache_first_load,
	soundcache_partial_load,
	soundcache_full_load,
	soundcache_incomplete_unload,
	soundcache_duplicate_unload,
	soundcache_normal_unload,
	soundcache_resource_promoted,
	decoder_rehydrate,
	decoder_full_rehydrate,
	decoder_start,
	decoder_collect_channels,
	decoder_reset_partial,
	decoder_reset_full,
	decoder_writing_completed
};

inline const char *to_string(EventType type)
{
	switch (type)
	{
	case EventType::soundcache_first_load:
		return "soundcache_first_load";
	case EventType::soundcache_partial_load:
		return "soundcache_partial_load";
	case EventType::soundcache_full_load:
		return "soundcache_full_load";
	case EventType::soundcache_incomplete_unload:
		return "soundcache_incomplete_unload";
	case EventType::soundcache_duplicate_unload:
		return "soundcache_duplicate_unload";
	case EventType::soundcache_normal_unload:
		return "soundcache_normal_unload";
	case EventType::soundcache_resource_promoted:
		return "soundcache_resource_promoted";
	case EventType::decoder_rehydrate:
		return "decoder_rehydrate";
	case EventType::decoder_full_rehydrate:
		return "decoder_full_rehydrate";
	case EventType::decoder_start:
		return "decoder_start";
	case EventType::decoder_collect_channels:
		return "decoder_collect_channels";
	case EventType::decoder_reset_partial:
		return "decoder_reset_partial";
	case EventType::decoder_reset_full:
		return "decoder_reset_full";
	case EventType::decoder_writing_completed:
		return "decoder_writing_completed";
	default:
		win::bug("Invalid EventType");
	}
}

struct Event
{
	Event(EventType type, const std::string &name, int seek, const std::string &message)
		: type(type), name(name), seek(seek), message(message)
	{
		fprintf(stderr, "%s -- %s (@%d): \"%s\"\n", to_string(type), name.c_str(), seek, message.c_str());
	}

	EventType type;
	std::string name;
	int seek;
	std::string message;
};

inline std::list<Event> event_stream;
inline std::mutex event_stream_mutex;

inline void send_event(const Event &e)
{
	std::lock_guard<std::mutex> lock(event_stream_mutex);
	event_stream.push_back(e);
}

inline void reset_event_stream()
{
}

inline int pending_events()
{
	std::lock_guard<std::mutex> lock(event_stream_mutex);
	return (int)event_stream.size();
}

inline std::optional<Event> pop_event(EventType type)
{
	std::lock_guard<std::mutex> lock(event_stream_mutex);

	for (auto it = event_stream.begin(); it != event_stream.end(); ++it)
	{
		if (it->type == type)
		{
			Event copy = *it;
			event_stream.erase(it);
			return copy;
		}
	}

	return std::optional<Event>();
}

}

}

#endif
