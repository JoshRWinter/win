#include <win/sound/CachingPcmSource.hpp>

namespace win
{

CachingPcmSource::CachingPcmSource(DecodingPcmSource &decoder, CachedPcmSource &cached, float *pcmdata, long pcmlength)
	: writemark(0)
	, pcmlength(pcmlength)
	, pcmdata(pcmdata)
	, mode(CachingPcmSourceMode::read_from_decoder)
	, decoder(decoder)
	, cached(cached)
{
}

int CachingPcmSource::channels()
{
	return cached.channels();
}

bool CachingPcmSource::empty()
{
	if (mode == CachingPcmSourceMode::read_from_decoder)
		return decoder.empty();
	else
		return cached.empty();
}

void CachingPcmSource::restart()
{
	if (mode == CachingPcmSourceMode::read_from_decoder)
	{
		if (!decoder.empty())
			win::bug("CachingPcmSource doesn't support restarting if not empty!");

		mode = CachingPcmSourceMode::read_from_cache;
		decoder.stop();
	}
	else
	{
		cached.restart();
	}
}

int CachingPcmSource::read_samples(float *buf, int samples)
{
	if (mode == CachingPcmSourceMode::read_from_decoder)
	{
		const int got = decoder.read_samples(buf, samples);

		// squirrel away that data to the cache as well
		const long can_write = pcmlength - writemark;
		if (got > can_write)
			win::bug("CachingPcmSource: overwrite");

		memcpy(pcmdata.get() + writemark, buf, got * sizeof(float));
		writemark += got;

		return got;
	}
	else
	{
		return cached.read_samples(buf, samples);
	}
}

bool CachingPcmSource::is_complete() const
{
	return writemark == pcmlength;
}

long CachingPcmSource::pcm_length() const
{
	return pcmlength;
}

float *CachingPcmSource::release_pcm_data()
{
	return pcmdata.release();
}

}
