#include <string.h>

#include <win/pcmstream.hpp>
#include <win/ogg.hpp>

namespace win
{

PCMStream::PCMStream(win::PCMStreamCache *parent, Stream *oggstream, PCMStreamCacheMode kind)
	: parent(parent)
	, kind(kind)
	, channel_count(0)
	, writing_completed(false)
{
	if (kind == PCMStreamCacheMode::not_cached || kind == PCMStreamCacheMode::partially_cached)
	{
		std::mutex mutex;
		std::condition_variable cvar;
		decoder_cancel.store(false);
		decoder_thread = std::move(std::thread(decodeogg, std::move(*oggstream), std::ref(*this), std::ref(cvar), std::ref(mutex)));

		{
			std::unique_lock<std::mutex> unique_lock(mutex);
			cvar.wait(unique_lock, [this]() { return channel_count != 0; });
		}
	}
}

PCMStream::~PCMStream()
{
	if (kind == PCMStreamCacheMode::not_cached || kind == PCMStreamCacheMode::partially_cached)
	{
		decoder_cancel.store(true);
		decoder_thread.join();
	}
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

/*
int SoundStream::write_samples_impl(const std::int16_t *source, int len)
{
}
*/

}
