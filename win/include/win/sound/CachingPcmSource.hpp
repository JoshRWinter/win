#pragma once

#include <memory>

#include <win/sound/DecodingPcmSource.hpp>
#include <win/sound/CachedPcmSource.hpp>

namespace win
{

enum class CachingPcmSourceMode
{
	read_from_decoder,
	read_from_cache
};

class CachingPcmSource : public PcmSource
{
	WIN_NO_COPY_MOVE(CachingPcmSource);

public:
	explicit CachingPcmSource(DecodingPcmSource &decoder, CachedPcmSource &cached, float *pcmdata, long pcmlength);

	int channels() override;
	bool empty() override;
	void restart() override;
	int read_samples(float *buf, int read_samples) override;

	bool is_complete() const;
	long pcm_length() const;
	float *release_pcm_data();

private:
	int writemark;
	long pcmlength;
	std::unique_ptr<float[]> pcmdata;
	CachingPcmSourceMode mode;
	DecodingPcmSource &decoder;
	CachedPcmSource &cached;
};

}
