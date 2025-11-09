#include <win/Win.hpp>

#ifdef WINPLAT_WINDOWS

#include <mmdeviceapi.h>

#include <win/sound/SoundEngineWindowsDirectSound.hpp>

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

namespace win
{

SoundEngineWindowsDirectSound::SoundEngineWindowsDirectSound(AssetRoll &roll)
	: loop_cancel(false)
	, write_position(0)
	, mixer(roll)
{
	HRESULT r;

	if ((r = DirectSoundCreate8(NULL, &context, NULL)) != DS_OK)
		win::bug("DirectSound: Couldn't create context " + describe_dserr(r));

	if ((r = context->SetCooperativeLevel(GetDesktopWindow(), DSSCL_PRIORITY)) != DS_OK)
		win::bug("DirectSound: Couldn't set cooperative level " + describe_dserr(r));
	/*

	DSBUFFERDESC primary_buffer;
	primary_buffer.dwSize = sizeof(primary_buffer);
	primary_buffer.dwFlags = DSBCAPS_PRIMARYBUFFER;
	primary_buffer.dwBufferBytes = 0;
	primary_buffer.dwReserved = 0;
	primary_buffer.lpwfxFormat = NULL;
	primary_buffer.guid3DAlgorithm = GUID_NULL;

	if ((r = context->CreateSoundBuffer(&primary_buffer, &primary, NULL) != DS_OK))
		win::bug("DirectSound: Couldn't create primary buffer " + describe_dserr(r));
	 */

	WAVEFORMATEX format;
	format.wFormatTag = WAVE_FORMAT_PCM;
	format.nChannels = 2;
	format.nSamplesPerSec = 44100;
	format.nAvgBytesPerSec = 44100 * 4;
	format.nBlockAlign = 4;
	format.wBitsPerSample = 16;
	format.cbSize = 0;

	/*
	if ((r = primary->SetFormat(&format) != DS_OK))
		win::bug("DirectSound: Couldn't set primary buffer format " + describe_dserr(r));
	 */

	DSBUFFERDESC secondary_buffer;
	secondary_buffer.dwSize = sizeof(secondary_buffer);
	secondary_buffer.dwFlags = DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_GLOBALFOCUS;
	secondary_buffer.dwBufferBytes = buffer_len_bytes;
	secondary_buffer.dwReserved = 0;
	secondary_buffer.lpwfxFormat = &format;
	secondary_buffer.guid3DAlgorithm = GUID_NULL;

	IDirectSoundBuffer *tmp;
	if ((r = context->CreateSoundBuffer(&secondary_buffer, &tmp, NULL)) != DS_OK)
		win::bug("DirectSound: Couldn't create temporary buffer " + describe_dserr(r));
	if ((tmp->QueryInterface(IID_IDirectSoundBuffer8, (void**)&secondary)) != DS_OK)
		win::bug("DirectSound:: QueryInterface() failed " + describe_dserr(r));
	tmp->Release();

	unsigned char *buf;
	DWORD size;
	if ((r = secondary->Lock(0, buffer_len_bytes, (void**)&buf, &size, NULL, NULL, 0)) != DS_OK)
		win::bug("DirectSound: Couldn't lock buffer " + describe_dserr(r));

	memset(buf, 0, buffer_len_bytes);

	if ((r = secondary->Unlock(buf, size, NULL, 0)) != DS_OK)
		win::bug("DirectSound: Couldn't unlock buffer " + describe_dserr(r));

	if ((r = secondary->Play(0, 0, DSBPLAY_LOOPING)) != DS_OK)
		win::bug("no play " + describe_dserr(r));

	loop_thread = std::move(std::thread(loop, std::ref(*this), std::ref(loop_cancel)));
}

SoundEngineWindowsDirectSound::~SoundEngineWindowsDirectSound()
{
	loop_cancel = true;
	loop_thread.join();
	secondary->Release();
	context->Release();
}

std::uint32_t SoundEngineWindowsDirectSound::play(const SoundEnginePlayCommand &cmd)
{
	std::lock_guard<std::mutex> lock(loop_lock);
	return mixer.add(cmd.name, cmd.residency_priority, cmd.compression_priority, cmd.left, cmd.right, cmd.looping, cmd.cache, cmd.seek);
}

void SoundEngineWindowsDirectSound::save(const std::vector<SoundEnginePlaybackCommand> &playback, const std::vector<SoundEngineConfigCommand> &config)
{
	std::lock_guard<std::mutex> lock(loop_lock);

	for (const auto &cmd : playback)
	{
		if (cmd.stop)
			mixer.stop(cmd.key);
		else if (cmd.playing)
			mixer.resume(cmd.key);
		else
			mixer.pause(cmd.key);
	}

	for (const auto &cmd : config)
	{
		mixer.config(cmd.key, cmd.left, cmd.right);
	}
}

void SoundEngineWindowsDirectSound::loop(SoundEngineWindowsDirectSound &engine, std::atomic<bool> &cancel)
{
	auto last_write = std::chrono::high_resolution_clock::now();

	while (!cancel.load())
	{
		while (true)
		{
			const auto now = std::chrono::high_resolution_clock::now();
			std::chrono::duration<float, std::micro> diff(now - last_write);
			if (diff.count() > write_freq_micro)
				break; // time to write
		}

		std::lock_guard<std::mutex> lock(engine.loop_lock);
		while (!engine.write());
		last_write = std::chrono::high_resolution_clock::now();
	}
}

bool SoundEngineWindowsDirectSound::write()
{
	HRESULT r;

	DWORD play_position;
	DWORD safe_write_position;
	if ((r = secondary->GetCurrentPosition(&play_position, &safe_write_position)) != DS_OK)
		win::bug("DirectSound: Couldn't get buffer positions " + describe_dserr(r));

	// first, figure out if the write is in between the play and safe write
	if (
		(play_position < write_position && write_position < safe_write_position) ||
		(safe_write_position < play_position && play_position < write_position) ||
		(write_position < safe_write_position && safe_write_position < play_position)
	)
	{
		write_position = safe_write_position;
	}

	const DWORD available_to_write_bytes = write_position < play_position ? play_position - write_position : ((buffer_len_bytes - write_position) + play_position);
	const DWORD available_to_write_bytes_really = available_to_write_bytes - 4; // take a bit off the top, too scared to rear-end the play cursor
	if (available_to_write_bytes_really <= 0)
		return true;

	std::int16_t *buf1, *buf2;
	DWORD buf1_len, buf2_len;
	r = secondary->Lock(write_position, available_to_write_bytes_really, (void**)&buf1, &buf1_len, (void**)&buf2, &buf2_len, 0);

	if (r == DS_OK)
	{
		const int buf1_len_samples = (int)buf1_len / sizeof(std::int16_t);
		const int buf2_len_samples = (int)buf2_len / sizeof(std::int16_t);

		int samples_written = 0;

		// fill up buffer 1
		while (samples_written != buf1_len_samples)
			samples_written += mixer.mix_stereo(buf1 + samples_written, buf1_len_samples - samples_written);

		samples_written = 0;

		// fill up buffer 2
		while (samples_written != buf2_len_samples)
			samples_written += mixer.mix_stereo(buf2 + samples_written, buf2_len_samples - samples_written);

		if ((r = secondary->Unlock(buf1, buf1_len, buf2, buf2_len)) != DS_OK)
			win::bug("DirectSound: Can't unlock " + describe_dserr(r));

		write_position = (write_position + buf1_len + buf2_len) % buffer_len_bytes;
	}
	else if (r == DSERR_BUFFERLOST)
		win::bug("DirectSound: buffer lost");
	else if (r == DSERR_INVALIDCALL || r == DSERR_INVALIDPARAM)
	{
		fprintf(stderr, "invalid %s\n", describe_dserr(r).c_str());
		win::bug("bad");
		return false;
	}
	else
		win::bug("DirectSound: " + describe_dserr(r));

	return true;
}

}

#endif
