#include <win.h>

#ifdef WINPLAT_WINDOWS

namespace win
{

static std::string describe_dserr(HRESULT result)
{
	switch(result)
	{
	case DS_NO_VIRTUALIZATION: return "No virtualization";
	//case DS_INCOMPLETE: return "No virtualization";
	case DSERR_ACCESSDENIED: return "Access denied";
	case DSERR_ALLOCATED: return "Allocated";
	case DSERR_ALREADYINITIALIZED: return "Already initialized";
	case DSERR_BADFORMAT: return "Bad format";
	case DSERR_BADSENDBUFFERGUID: return "Bad send buffer GUID";
	case DSERR_BUFFERLOST: return "Buffer lost";
	case DSERR_BUFFERTOOSMALL: return "Buffer too small";
	case DSERR_CONTROLUNAVAIL: return "Control unavailable";
	case DSERR_DS8_REQUIRED: return "DS8 required";
	case DSERR_FXUNAVAILABLE: return "FX unavailable";
	case DSERR_GENERIC: return "Generic";
	case DSERR_INVALIDCALL: return "Invalid call";
	case DSERR_INVALIDPARAM: return "Invalid parameter";
	case DSERR_NOAGGREGATION: return "No aggregation";
	case DSERR_NODRIVER: return "No driver";
	case DSERR_NOINTERFACE: return "No interface";
	case DSERR_OBJECTNOTFOUND: return "Object not found";
	case DSERR_OTHERAPPHASPRIO: return "Other app has priority";
	case DSERR_OUTOFMEMORY: return "Out of memory";
	case DSERR_PRIOLEVELNEEDED: return "Priority level needed";
	case DSERR_SENDLOOP: return "Send loop";
	case DSERR_UNINITIALIZED: return "Uninitialized";
	case DSERR_UNSUPPORTED: return "Unsupported";
	default: return "Undetermined";
	}
}

SoundEngine::SoundEngine(Display &parent, AssetRoll &roll, SoundConfigFn fn)
	: soundbank(roll)
{
	next_id = 1;
	listener_x = 0.0f;
	listener_y = 0.0f;
	config_fn = fn;
	convbuffer = std::move(std::make_unique<std::int16_t[]>(CONVBUFFER_SAMPLES));

	if(DirectSoundCreate8(NULL, &context, NULL) != DS_OK)
		win::bug("Could not initialize DirectSound");

	if(context->SetCooperativeLevel(parent.window, DSSCL_PRIORITY) != DS_OK)
		win::bug("DirectSound: Could not set cooperation level");

	DSBUFFERDESC buffer;
	buffer.dwSize = sizeof(buffer);
	buffer.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME;
	buffer.dwBufferBytes = 0;
	buffer.dwReserved = 0;
	buffer.lpwfxFormat = NULL;
	buffer.guid3DAlgorithm = GUID_NULL;

	if(context->CreateSoundBuffer(&buffer, &primary, NULL) != DS_OK)
		win::bug("DirectSound: Could not create the primary sound buffer");

	WAVEFORMATEX format;
	format.wFormatTag = WAVE_FORMAT_PCM;
	format.nChannels = 2;
	format.nSamplesPerSec = 44100;
	format.nAvgBytesPerSec = 44100 * 4;
	format.nBlockAlign = 4;
	format.wBitsPerSample = 16;
	format.cbSize = 0;

	if(primary->SetFormat(&format) != DS_OK)
		win::bug("DirectSound: Could not set the primary buffer format");
}

SoundEngine::~SoundEngine()
{
	cleanup(true);
	primary->Release();
	context->Release();
}

unsigned long long SoundEngine::write_empty(ActiveSound &sound, unsigned long long bytes_left)
{
	void *buffer1 = NULL;
	void *buffer2 = NULL;
	DWORD size1 = 0;
	DWORD size2 = 0;

	if(sound.stream->Lock(sound.write_cursor, bytes_left, &buffer1, &size1, &buffer2, &size2, 0) != DS_OK)
		win::bug("DirectSound: couldn't lock stream buffer for empty write");

	memset(buffer1, 0, size1);
	memset(buffer2, 0, size2);

	if(sound.stream->Unlock(buffer1, size1, buffer2, size2) != DS_OK)
		win::bug("DirectSound: couldn't unlock stream buffer from empty write");

	sound.write_cursor = (sound.write_cursor + size1 + size2) % SOUND_BUFFER_BYTES;

	return (size1 + size2) / 2;
}

std::ifstream samplesfile("c:\\users\\josh\\desktop\\samples.bin", std::ifstream::binary);

void SoundEngine::write_to_stream(ActiveSound &sound)
{
	if(sound.stop)
		return;

	DWORD play_cursor = 0;
	if(sound.stream->GetCurrentPosition(&play_cursor, NULL) != DS_OK)
		win::bug("DirectSound: Couldn't determine play cursor position");

	const int bytes_left = sound.firstwrite ? MAX_WRITE_SAMPLES : sound.write_cursor <= play_cursor ? play_cursor - sound.write_cursor : ((SOUND_BUFFER_BYTES - sound.write_cursor) + play_cursor);
	const int want_to_write_samples = bytes_left / sizeof(std::int16_t);
	const int will_write_samples = std::min((unsigned long long)want_to_write_samples, MAX_WRITE_SAMPLES);

	if(bytes_left == 0)
		return;

	if(sound.sound->is_stream_completed() && !sound.looping)
	{
		sound.silent_samples_written += write_empty(sound, bytes_left);
		if(sound.silent_samples_written > SOUND_BUFFER_SAMPLES)
			sound.stop = true;
		return;
	}

#ifndef NDEBUG
	if(sound.write_cursor % 4 != 0)
		win::bug("DirectSound: write_cursor not divisible by 4");
	if(play_cursor % 4 != 0)
		win::bug("DirectSound: play_cursor not divisible by 4");
	if(bytes_left % 4 != 0)
		win::bug("DirectSound: bytes_left not divisible by 4");
	if(want_to_write_samples % 2 != 0)
		win::bug("DirectSound: want_to_write_samples not divisible by 2");
	if(will_write_samples % 2 != 0)
		win::bug("DirectSound: will_write_samples not divisible by 2");
#endif

	void *buffer1 = NULL;
	void *buffer2 = NULL;
	DWORD size1 = 0;
	DWORD size2 = 0;

	if(sound.sound->channels == 1)
	{
		const auto mono_write_samples = will_write_samples / 2;
		auto got = sound.sound->read(convbuffer.get(), mono_write_samples);

		if(sound.sound->is_stream_completed() && sound.looping)
		{
			const std::string name = sound.sound->name;
			soundbank.unload(*sound.sound);
			sound.sound = &soundbank.load(name.c_str());
		}

		if(got == 0)
			return;

		sound.firstwrite = false;

		channel_dupe(convbuffer.get() + got, convbuffer.get(), got);

		if(sound.stream->Lock(sound.write_cursor, got * sizeof(std::int16_t) * 2, &buffer1, &size1, &buffer2, &size2, 0) != DS_OK)
			win::bug("DirectSound: couldn't lock stream buffer");

		if(size1 + size2 != got * sizeof(std::int16_t) * 2)
			win::bug("DirectSound: sample request mismatch");

		memcpy(buffer1, convbuffer.get() + got, size1);
		memcpy(buffer2, convbuffer.get() + got + (size1 / sizeof(std::int16_t)), size2);

		if(sound.stream->Unlock(buffer1, size1, buffer2, size2) != DS_OK)
			win::bug("DirectSound: couldn't unlock stream buffer");

		sound.write_cursor = (sound.write_cursor + size1 + size2) % SOUND_BUFFER_BYTES;
	}
	else if(sound.sound->channels == 2)
	{
		auto got = sound.sound->read(convbuffer.get(), will_write_samples);

		if(sound.sound->is_stream_completed() && sound.looping)
		{
			const std::string name = sound.sound->name;
			soundbank.unload(*sound.sound);
			sound.sound = &soundbank.load(name.c_str());
		}

		if(got == 0)
			return;

		sound.firstwrite = false;

		if(sound.stream->Lock(sound.write_cursor, got * sizeof(std::int16_t), &buffer1, &size1, &buffer2, &size2, 0) != DS_OK)
			win::bug("DirectSound: couldn't lock stream buffer");

		if(size1 + size2 != got * sizeof(std::int16_t))
			win::bug("DirectSound: sample request mismatch");

		memcpy(buffer1, convbuffer.get(), size1);
		memcpy(buffer2, convbuffer.get() + (size1 / sizeof(std::int16_t)), size2);

		if(sound.stream->Unlock(buffer1, size1, buffer2, size2) != DS_OK)
			win::bug("DirectSound: couldn't unlock stream buffer");

		sound.write_cursor = (sound.write_cursor + size1 + size2) % SOUND_BUFFER_BYTES;
	}
	else
	{
		win::bug("DirectSound: unsupported channels");
	}
}

void SoundEngine::process()
{
	cleanup(false);
	for(auto &sound : sounds)
	{
		write_to_stream(sound);
	}
}

int SoundEngine::play(const char *name, bool loop)
{
	return play(name, true, loop, 0.0f, 0.0f);
}

int SoundEngine::play(const char *name, float x, float y, bool loop)
{
	return play(name, false, loop, x, y);
}

int SoundEngine::play(const char *name, bool ambient, bool looping, float x, float y)
{
	cleanup(false);
	if(sounds.size() >= MAX_SOUNDS)
		return -1;

	WAVEFORMATEX format;
	format.wFormatTag = WAVE_FORMAT_PCM;
	format.nSamplesPerSec = 44100;
	format.wBitsPerSample = 16;
	format.nChannels = 2;
	format.nBlockAlign = 4;
	format.nAvgBytesPerSec = 44100 * 4;
	format.cbSize = 0;

	DSBUFFERDESC buffer;
	buffer.dwSize = sizeof(buffer);
	buffer.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPAN | DSBCAPS_GETCURRENTPOSITION2;
	buffer.dwBufferBytes = SOUND_BUFFER_BYTES;
	buffer.dwReserved = 0;
	buffer.lpwfxFormat = &format;
	buffer.guid3DAlgorithm = GUID_NULL;

	IDirectSoundBuffer *tmp;
	if(context->CreateSoundBuffer(&buffer, &tmp, NULL) != DS_OK)
		win::bug("DirectSound: Could not create temp buffer");

	IDirectSoundBuffer8 *stream;
	tmp->QueryInterface(IID_IDirectSoundBuffer8, (void**)&stream);
	tmp->Release();

	auto &soundstream = soundbank.load(name);
	auto &sound = sounds.emplace_back(soundstream, stream, next_id++, ambient, looping, x, y);

	while(soundstream.channels.load() == -1);

	write_to_stream(sound);

	sound.stream->Play(0, 0, DSBPLAY_LOOPING);

	return sound.id;
}

void SoundEngine::pause(int)
{
}

void SoundEngine::resume(int)
{
}

void SoundEngine::stop(int)
{
}

void SoundEngine::source(int, float, float)
{
}

void SoundEngine::listener(float x, float y)
{
	listener_x = x;
	listener_y = y;
}

void SoundEngine::cleanup(bool all)
{
	for(auto sound = sounds.begin(); sound != sounds.end();)
	{
		if(all || sound->stop)
		{
			fprintf(stderr, "deleted sound %d\n", sound->id);
			soundbank.unload(*sound->sound);
			sound = sounds.erase(sound);
			continue;
		}

		++sound;
	}
}

}

#endif
