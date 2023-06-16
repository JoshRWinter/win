#include <cmath>
#include <cstring>

#include <win/sound/CachedPcmSource.hpp>

namespace win
{

CachedPcmSource::CachedPcmSource(int channel_count, const float *data, long length)
	: channel_count(channel_count)
	, bookmark(0)
	, pcmdata(data)
	, pcmdata_length(length)
{}

int CachedPcmSource::channels()
{
	return channel_count;
}

bool CachedPcmSource::empty()
{
	return bookmark == pcmdata_length;
}

void CachedPcmSource::restart()
{
	// really no reason for this, just wanted to assert some stuff
	if (bookmark != pcmdata_length)
		win::bug("CachedPcmSource does not support restarting if not empty!");

	bookmark = 0;
}

int CachedPcmSource::read_samples(float *buf, int samples)
{
	const long avail = pcmdata_length - bookmark;
	const long take = std::min((long)samples, avail);

	memcpy(buf, pcmdata + bookmark, take * sizeof(float));
	bookmark += take;

	return (int)take;
}

}
