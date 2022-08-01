#include <win/soundcache.hpp>

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
		if (r.name() == name && r.seek_start() == seek)
		{
			found = &r;
			break;
		}
	}

	if (found == NULL)
	{
		fprintf(stderr, "SoundCache: No cached resource with name %s and seek %d\n", name, seek);

		// no cache entry yet. make one
		PCMResource &resource = resources.add(name, seek);

		// need to hit the disk
		Stream s = roll[name];

		// the win::PCMDecoder will std::move the "s" into itself
		return loaded_sounds.add(&resource, &s, seek);
	}
	else if (!found->is_completed())
	{
		fprintf(stderr, "SoundCache: cache present, but incomplete\n");
		// there is a cache entry, but it isn't done yet. don't use it, and don't make a new one

		// need to hit the disk
		Stream s = roll[name];

		// the win::PCMStream will std::move the "s" into itself
		return loaded_sounds.add((win::PCMResource*)NULL, &s, seek);
	}
	else if (found->is_partial())
	{
		fprintf(stderr, "SoundCache: partial cache present\n");
		// partially cached

		Stream s = roll[name];
		// the win::PCMStream will std::move the "s" into itself
		return loaded_sounds.add(found, &s, seek);
	}
	else
	{
		fprintf(stderr, "SoundCache: full cache present\n");
    	return loaded_sounds.add(found, (win::Stream*)NULL, seek);
	}
}

void SoundCache::unload(Sound &sound)
{
	PCMResource *const resource = sound.source.resource();

	if (resource == NULL)
	{
		// the sound was never given a resource
		fprintf(stderr, "SoundCache: unload - resource was null\n");
		loaded_sounds.remove(sound);
	}
	else if (!resource->is_completed())
	{
		// the sound was given a resource but was stopped before the resource could fill up
		fprintf(stderr, "SoundCache: unload - resource was incomplete\n");
		loaded_sounds.remove(sound);
		resources.remove(*resource);
	}
	else
	{
		fprintf(stderr, "SoundCache: unload - normal\n");
		loaded_sounds.remove(sound);
	}
}

}
