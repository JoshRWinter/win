#include <win/sound/pcmstream.hpp>

namespace win
{

PCMStream::PCMStream()
	: channel_count(-1)
	, writing_completed(false)
{
}

int PCMStream::read_samples(std::int16_t *dest, int len)
{
	return ringbuffer.read(dest, len);
}

int PCMStream::write_samples(const std::int16_t *source, int len)
{
	return ringbuffer.write(source, len);
}

int PCMStream::size() const
{
	return ringbuffer.size();
}

void PCMStream::complete_writing()
{
	writing_completed.store(true);
}

bool PCMStream::is_writing_completed() const
{
	return writing_completed.load();
}

void PCMStream::reset()
{
	writing_completed.store(false);
}

}
