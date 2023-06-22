#include <win/sound/CachingPcmSource.hpp>

namespace win
{

CachingPcmSource::CachingPcmSource(DecodingPcmSource &decoder)
	: writemark(0)
	, pcmlength(decoder.pcm_size())
	, pcmdata(new float[pcmlength])
	, mode(CachingPcmSourceMode::read_from_decoder)
	, decoder(decoder)
	, cache(decoder.channels(), pcmdata.get(), pcmlength)
{
}

int CachingPcmSource::channels()
{
	return cache.channels();
}

bool CachingPcmSource::empty()
{
	if (mode == CachingPcmSourceMode::read_from_decoder)
		return decoder.empty();
	else
		return cache.empty();
}

void CachingPcmSource::restart()
{
	if (mode == CachingPcmSourceMode::read_from_decoder)
	{
		if (!decoder.empty())
			win::bug("CachingPcmSource doesn't support restarting if not empty!");

		mode = CachingPcmSourceMode::read_from_cache;
	}
	else
	{
		cache.restart();
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
		return cache.read_samples(buf, samples);
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
