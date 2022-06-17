#include <string.h>

#include <win/soundstream.hpp>
#include <win/ogg.hpp>

namespace win
{

SoundStream::SoundStream(win::SoundCache *parent, Stream *oggstream, SoundStreamKind kind)
	: parent(parent)
    , kind(kind)
	, channel_count(0)
	, writing_completed(false)
{
	if (kind == SoundStreamKind::not_cached || kind == SoundStreamKind::partially_cached)
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

SoundStream::~SoundStream()
{
	if (kind == SoundStreamKind::not_cached || kind == SoundStreamKind::partially_cached)
    {
	    decoder_cancel.store(true);
	    decoder_thread.join();
	}
}

int SoundStream::read_samples(std::int16_t *dest, int len)
{
	return ringbuffer.read(dest, len);
}

int SoundStream::write_samples(const std::int16_t *source, int len)
{
	return ringbuffer.write(source, len);
}

void SoundStream::complete_writing()
{
	writing_completed.store(true);
}

bool SoundStream::is_writing_completed() const
{
	return writing_completed.load();
}

/*
int SoundStream::write_samples_impl(const std::int16_t *source, int len)
{
}
*/

}
