#include <win/win.hpp>

#ifdef WINPLAT_WINDOWS

#include <mmdeviceapi.h>

#include <win/sound/soundengine_windows_directsound.hpp>

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
	: mixer(roll)
{
	if (DirectSoundCreate8(NULL, &context, NULL) != DS_OK)
		win::bug("DirectSound: Couldn't create context");

	DSBUFFERDESC primary_buffer;
	primary_buffer.dwSize = sizeof(primary_buffer);
	primary_buffer.dwFlags = DSBCAPS_PRIMARYBUFFER;
	primary_buffer.dwBufferBytes = 0;
	primary_buffer.dwReserved = 0;
	primary_buffer.lpwfxFormat = NULL;
	primary_buffer.guid3DAlgorithm = GUID_NULL;

	if (context->CreateSoundBuffer(&buffer, &primary, NULL) != DS_OK)
		win::bug("DirectSound: Couldn't create primary sound buffer");

	WAVEFORMATEX format;
	format.wFormatTag = WAVE_FORMAT_PCM;
	format.nChannels = 2;
	format.nSamplesPerSec = 44100;
	format.nAvgBytesPerSec = 44100 * 4;
	format.nBlockAlign = 4;
	format.wBitsPerSample = 16;
	format.cbSize = 0;

	if (primary->SetFormat(&format) != DS_OK)
		win::bug("DirectSound: Couldn't set primary buffer format");

	DSBUFFERDESC secondary_buffer;
	secondary_buffer.dwSize = sizeof(secondary_buffer);
	secondary_buffer.dwFlags = DSBCAPS_GETCURRENTPOSITION2;
	secondary_buffer.dwBufferBytes = write_size_stereo_samples * 3;
	secondary_buffer.dwReserved = 0;
	secondary_buffer.lpwfxFormat = &format;
	secondary_buffer.guid3DAlgorithm = GUID_NULL;

	IDirectSoundBuffer *tmp;
	if (context->CreateSoundBuffer(&secondary_buffer, &tmp, NULL) != DS_OK)
		win::bug("DirectSound: Couldn't create temporary buffer");
	if (tmp->QueryInterface(IID_IDirectSoundBuffer8, (void**)&secondary) != DS_OK)
		win::bug("DirectSound:: QueryInterface() failed");
	tmp->Release();

	loop_last_write = std::chrono::high_resolution_clock::now() - std::chrono::milliseconds(100); // set it to some time in the past, so the loop writes immediately
	loop_thread = std::move(std::thread(loop, std::ref(*this)));

	primary->Play(0, 0, 0);
}

SoundEngineWindowsDirectSound::~SoundEngineWindowsDirectSound()
{

}

std::uint32_t SoundEngineWindowsDirectSound::play(const SoundEnginePlayCommand&)
{
	return -1;
}

void SoundEngineWindowsDirectSound::save(const std::vector<SoundEnginePlaybackCommand>&, const std::vector<SoundEngineConfigCommand>&)
{
}

void SoundEngineWindowsDirectSound::loop(SoundEngineWindowsDirectSound &engine)
{
	while (true)
	{
		const auto now = std::chrono::high_resolution_clock::now();
		std::chrono::duration<float, std::micro> diff(now - engine.loop_last_write);
		if (diff.count() > write_freq_micro)
			break; // time to write
	}

	std::lock_guard<std::mutex> lock(engine.loop_lock);

	engine.write();
}

void SoundEngineWindowsDirectSound::write()
{
	DWORD play, write;
	primary->GetCurrentPosition(&play, &write);

	DWORD written_bytes;
	//const DWORD available_to_write_bytes = play < write ?
	//primary->Lock()
}

}

#endif
