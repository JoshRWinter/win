#include <win/soundcache.hpp>

#ifdef WIN_USE_SOUND_INTEGRATION_TESTS
#include <win/test/soundintegrationtests.hpp>
#endif

namespace win
{

SoundCache::SoundCache(win::AssetRoll &roll)
	: roll(roll)
{}

SoundCache::~SoundCache()
{
	if (loaded_sounds.size() != 0)
		win::bug("Leftover sound streams");
}

Sound &SoundCache::load(const char *name, int seek)
{
	// see if we've got this one
    PCMResource *found = NULL;
    for (PCMResource &r : resources)
	{
		if (r.is_completed() && !strcmp(r.name(), name) && r.seek_start() == seek)
		{
			found = &r;
			break;
		}
	}

	if (found == NULL)
	{
#ifdef WIN_USE_SOUND_INTEGRATION_TESTS
		win::sit::send_event(win::sit::Event(win::sit::EventType::soundcache_first_load, name, seek, "SoundCache: No cache"));
#endif

		// no completed cache entry yet. make one
		PCMResource &resource = resources.add(name, seek);

		// need to hit the disk
		Stream s = roll[name];

		// the win::PCMDecoder will std::move the "s" into itself
		return loaded_sounds.add(resource, &s, seek);
	}
	else if (found->is_partial())
	{
#ifdef WIN_USE_SOUND_INTEGRATION_TESTS
		win::sit::send_event(win::sit::Event(win::sit::EventType::soundcache_partial_load, name, seek, "SoundCache: Partial cache found"));
#endif

		// partially cached
		// need to hit the disk
		Stream s = roll[name];

		// the win::PCMDecoder will std::move the "s" into itself
		return loaded_sounds.add(*found, &s, seek);
	}
	else
	{
#ifdef WIN_USE_SOUND_INTEGRATION_TESTS
		win::sit::send_event(win::sit::Event(win::sit::EventType::soundcache_full_load, name, seek, "SoundCache: Full cache found"));
#endif

    	return loaded_sounds.add(*found, (win::Stream*)NULL, seek);
	}
}

void SoundCache::unload(Sound &sound)
{
	PCMResource &resource = sound.source.resource();

	// does this resource already exist?
    PCMResource *found = NULL;
	for (PCMResource &r : resources)
	{
		if (!strcmp(r.name(), resource.name()) && r.seek_start() == resource.seek_start() && &r != &resource)
		{
			found = &r;
			break;
		}
	}

	if (!resource.is_completed())
	{
#ifdef WIN_USE_SOUND_INTEGRATION_TESTS
		win::sit::send_event(win::sit::Event(win::sit::EventType::soundcache_incomplete_unload, sound.source.resource().name(), sound.source.resource().seek_start(), "SoundCache: removing incomplete resource"));
#endif

		// the sound was given a resource but was stopped before the resource could fill up
		loaded_sounds.remove(sound);
		resources.remove(resource);
	}
	else if (found)
	{
#ifdef WIN_USE_SOUND_INTEGRATION_TESTS
		win::sit::send_event(win::sit::Event(win::sit::EventType::soundcache_duplicate_unload, sound.source.resource().name(), sound.source.resource().seek_start(), "SoundCache: removing duplicate resource"));
#endif

		// this sound's resource is a duplicate
		loaded_sounds.remove(sound);
		resources.remove(resource);
	}
	else
	{
#ifdef WIN_USE_SOUND_INTEGRATION_TESTS
		win::sit::send_event(win::sit::Event(win::sit::EventType::soundcache_normal_unload, sound.source.resource().name(), sound.source.resource().seek_start(), "SoundCache: normal unload"));
#endif

		loaded_sounds.remove(sound);
	}
}

}
