/*
 * SoundEngine.cpp
 *
 *  Created on: 07/giu/2012
 *      Author: JackBurton
 */

#include "SoundEngine.h"

#include "Log.h"

#include "SDL.h"

#include <algorithm>
#include <iostream>

static SoundEngine* sSoundEngine = nullptr;

SoundEngine::SoundEngine()
	:
	fBuffer(nullptr),
	fPlaying(false)
{
	if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) {
		std::string error;
		error.append("SoundEngine: Error while initializing SDL Sound System: ");
		error.append(SDL_GetError());
		throw std::runtime_error(error);
	}
}


SoundEngine::~SoundEngine()
{
	SDL_PauseAudio(1);
	SDL_CloseAudio();
	delete fBuffer;

	for (uint8 i = 0; i < kMaxOneShots; i++) {
		if (fOneShots[i].device != 0)
			SDL_CloseAudioDevice(reinterpret_cast<SDL_AudioDeviceID>(fOneShots[i].device));
	}

	_CloseStreamDevice();
}


/* static */
bool
SoundEngine::Initialize()
{
	try {
		if (sSoundEngine == nullptr)
			sSoundEngine = new SoundEngine();
	} catch (std::exception& e) {
		return false;
	}
	return true;
}


/* static */
void
SoundEngine::Destroy()
{
	delete sSoundEngine;
	sSoundEngine = nullptr;
}


/* static */
SoundEngine*
SoundEngine::Get()
{
	return sSoundEngine;
}


bool
SoundEngine::InitBuffers(bool stereo, bool bit16, uint16 sampleRate, uint32 bufferLen)
{
#if 0
	std::cout << "InitBuffers(";
	std::cout << sampleRate << " KHz";
	std::cout << ", " << (stereo ? "STEREO" : "MONO");
	std::cout << ", " << (bit16 ? "16BIT" : "8BIT");
	std::cout << ", " << bufferLen << " bytes";
	std::cout << std::endl;
#endif
	fBuffer = new SoundBuffer(stereo, bit16, sampleRate, bufferLen);

	SDL_AudioSpec audioSpec;
	audioSpec.freq = sampleRate;
	audioSpec.format = AUDIO_S16;
	audioSpec.channels = stereo ? 2 : 1;
	audioSpec.samples = 4096;
	audioSpec.callback = SoundEngine::MixAudio;
	audioSpec.userdata = this;

	if (SDL_OpenAudio(&audioSpec, nullptr) < 0 ) {
		// TODO: Return SDL_Error() in some way
		delete fBuffer;
		fBuffer = nullptr;
		return false;
	}

	return true;
}


void
SoundEngine::DestroyBuffers()
{
	SDL_CloseAudio();
	delete fBuffer;
	fBuffer = nullptr;
}


SoundBuffer*
SoundEngine::Buffer()
{
	return fBuffer;
}


bool
SoundEngine::Lock()
{
	SDL_LockAudio();
	return true;
}


void
SoundEngine::Unlock()
{
	SDL_UnlockAudio();
}


void
SoundEngine::StartStopAudio()
{
	SDL_PauseAudio(0);
}


bool
SoundEngine::IsPlaying()
{
	return fPlaying;
}


/* static */
void
SoundEngine::MixAudio(void *castToThis, Uint8 *stream, int numBytes)
{
	SoundEngine* engine = reinterpret_cast<SoundEngine*>(castToThis);
	engine->Buffer()->ConsumeSamples(reinterpret_cast<uint8*>(stream),
									static_cast<uint16>(numBytes));
}


// SoundBuffer
SoundBuffer::SoundBuffer(bool stereo, bool bit16, uint16 sampleRate, uint32 bufferLen)
	:
	fStereo(stereo),
	f16Bit(bit16),
	fSampleRate(sampleRate),
	fData(nullptr),
	fBufferLength(bufferLen),
	fBufferPos(0),
	fConsumedPos(0)
{
	fData = reinterpret_cast<uint8*>(::calloc(1, bufferLen));
}


SoundBuffer::~SoundBuffer()
{
	free(fData);
}


bool
SoundBuffer::IsStereo() const
{
	return fStereo;
}


bool
SoundBuffer::Is16Bit() const
{
	return f16Bit;
}


uint16
SoundBuffer::SampleRate() const
{
	return fSampleRate;
}


uint8*
SoundBuffer::Data()
{
	return fData;
}


void
SoundBuffer::AddSample(sint16 sample)
{
	memcpy(fData + fBufferPos, &sample, sizeof(sample));
	fBufferPos += sizeof(sample);

	if (fBufferPos >= fBufferLength)
		fBufferPos = 0;

	if (fBufferPos == fConsumedPos)
		throw std::runtime_error("Buffer Overflow");
}


uint16
SoundBuffer::ConsumeSamples(uint8* destBuffer, uint16 numSamples)
{
	uint32 numRequested = static_cast<uint32>(numSamples);
	uint32 numAvailable = std::min(numRequested, AvailableData());
	if (numAvailable == 0)
		return 0;

	if (fConsumedPos > fBufferPos) {
		uint16 sizeBeforeEnd = std::min(numRequested, fBufferLength - fConsumedPos);
		memcpy(destBuffer, &fData[fConsumedPos], sizeBeforeEnd);
		if (numRequested > sizeBeforeEnd) {
			numRequested -= sizeBeforeEnd;
			memcpy(destBuffer + sizeBeforeEnd, fData, std::min(numRequested, fBufferPos));
		}
	} else
		memcpy(destBuffer, &fData[fConsumedPos], numAvailable);

	fConsumedPos += numAvailable;
	if (fConsumedPos >= fBufferLength)
		fConsumedPos = (fConsumedPos - fBufferLength);

	return static_cast<uint16>(numAvailable);
}


// return the number of bytes available
uint32
SoundBuffer::AvailableData() const
{
	if (fBufferPos < fConsumedPos)
		return (fBufferLength - fConsumedPos) + fBufferPos;

	return fBufferPos - fConsumedPos;
}


void
SoundEngine::PlaySample(const uint8* data, uint32 dataSize, uint16 channels,
	uint16 bitsPerSample, uint32 sampleRate)
{
	if (data == nullptr || dataSize == 0 || (bitsPerSample != 8 && bitsPerSample != 16))
		return;

	int slotIndex = -1;
	for (uint8 i = 0; i < kMaxOneShots; i++) {
		if (!fOneShots[i].active) {
			slotIndex = i;
			break;
		}
	}
	if (slotIndex == -1)
		slotIndex = 0; // pool full: steal the oldest slot

	OneShotSound& slot = fOneShots[slotIndex];

	const bool needsReopen = slot.device == 0 || slot.channels != channels
		|| slot.bitsPerSample != bitsPerSample || slot.sampleRate != sampleRate;

	if (needsReopen) {
		if (slot.device != 0)
			SDL_CloseAudioDevice(reinterpret_cast<SDL_AudioDeviceID>(slot.device));

		SDL_AudioSpec spec;
		SDL_zero(spec);
		spec.freq = (int)sampleRate;
		spec.format = bitsPerSample == 8 ? AUDIO_U8 : AUDIO_S16;
		spec.channels = (Uint8)channels;
		spec.samples = 2048;
		spec.callback = SoundEngine::MixOneShot;
		spec.userdata = &slot;

		SDL_AudioDeviceID device = SDL_OpenAudioDevice(nullptr, 0, &spec, nullptr, 0);
		if (device == 0) {
			// TODO: Return an error ?
			std::cerr << Log::Red << "SoundEngine::PlaySample(): Unable to open audio device: "
				<< SDL_GetError() << Log::Normal << std::endl;
			slot.device = 0;
			return;
		}
		slot.device = reinterpret_cast<uint32>(device);
		slot.channels = channels;
		slot.bitsPerSample = bitsPerSample;
		slot.sampleRate = sampleRate;
	} else {
		SDL_LockAudioDevice(reinterpret_cast<SDL_AudioDeviceID>(slot.device));
	}

	slot.data.assign(data, data + dataSize);
	slot.position = 0;
	slot.active = true;

	if (!needsReopen)
		SDL_UnlockAudioDevice(reinterpret_cast<SDL_AudioDeviceID>(slot.device));

	SDL_PauseAudioDevice(reinterpret_cast<SDL_AudioDeviceID>(slot.device), 0);
}


/* static */
void
SoundEngine::MixOneShot(void* userData, uint8* stream, int len)
{
	OneShotSound* slot = reinterpret_cast<OneShotSound*>(userData);
	if (!slot->active || slot->position >= slot->data.size()) {
		slot->active = false;
		memset(stream, 0, len);
		return;
	}

	uint32 remaining = (uint32)slot->data.size() - slot->position;
	uint32 toCopy = std::min((uint32)len, remaining);
	memcpy(stream, slot->data.data() + slot->position, toCopy);
	if ((uint32)len > toCopy)
		memset(stream + toCopy, 0, len - toCopy);

	slot->position += toCopy;
	if (slot->position >= slot->data.size())
		slot->active = false;
}


// The music stream: a device of its own (its format is the stream's), a
// callback that pulls PCM from the stream and applies the volume and the fade.

void
SoundEngine::_CloseStreamDevice()
{
	if (fMusic.device != 0) {
		// Closing the device waits for its callback, so the stream can go.
		SDL_CloseAudioDevice(reinterpret_cast<SDL_AudioDeviceID>(fMusic.device));
		fMusic.device = 0;
	}
	delete fMusic.stream;
	fMusic.stream = nullptr;
	fMusic.finished = true;
}


bool
SoundEngine::PlayStream(AudioStream* stream, float volume, uint32 fadeInMs)
{
	if (stream == nullptr)
		return false;

	const uint16 channels = stream->Channels();
	const uint32 sampleRate = stream->SampleRate();
	if (channels == 0 || channels > 2 || sampleRate == 0) {
		delete stream;
		return false;
	}

	const bool sameFormat = fMusic.device != 0 && fMusic.channels == channels
		&& fMusic.sampleRate == sampleRate;
	if (!sameFormat)
		_CloseStreamDevice();
	else
		SDL_LockAudioDevice(reinterpret_cast<SDL_AudioDeviceID>(fMusic.device));

	if (!sameFormat) {
		SDL_AudioSpec spec;
		SDL_zero(spec);
		spec.freq = (int)sampleRate;
		spec.format = AUDIO_S16;
		spec.channels = (Uint8)channels;
		spec.samples = 4096;
		spec.callback = SoundEngine::MixStream;
		spec.userdata = &fMusic;

		SDL_AudioDeviceID device = SDL_OpenAudioDevice(nullptr, 0, &spec, nullptr, 0);
		if (device == 0) {
			std::cerr << Log::Red << "SoundEngine::PlayStream(): Unable to open audio device: "
				<< SDL_GetError() << Log::Normal << std::endl;
			delete stream;
			return false;
		}
		fMusic.device = reinterpret_cast<uint32>(device);
		fMusic.channels = channels;
		fMusic.sampleRate = sampleRate;
	}

	AudioStream* previous = fMusic.stream;
	fMusic.stream = stream;
	fMusic.volume = volume;
	fMusic.framesPlayed = 0;
	fMusic.stopAtTarget = false;
	fMusic.finished = false;
	if (fadeInMs > 0) {
		fMusic.gain = 0.0f;
		fMusic.gainTarget = 1.0f;
		fMusic.gainStep = 1000.0f / ((float)fadeInMs * (float)sampleRate);
	} else {
		fMusic.gain = 1.0f;
		fMusic.gainTarget = 1.0f;
		fMusic.gainStep = 0.0f;
	}

	if (sameFormat)
		SDL_UnlockAudioDevice(reinterpret_cast<SDL_AudioDeviceID>(fMusic.device));
	delete previous;

	SDL_PauseAudioDevice(reinterpret_cast<SDL_AudioDeviceID>(fMusic.device), 0);
	return true;
}


void
SoundEngine::StopStream(uint32 fadeOutMs)
{
	if (fMusic.device == 0 || fMusic.finished)
		return;

	if (fadeOutMs == 0) {
		SDL_LockAudioDevice(reinterpret_cast<SDL_AudioDeviceID>(fMusic.device));
		fMusic.finished = true;
		SDL_UnlockAudioDevice(reinterpret_cast<SDL_AudioDeviceID>(fMusic.device));
		return;
	}

	SDL_LockAudioDevice(reinterpret_cast<SDL_AudioDeviceID>(fMusic.device));
	fMusic.gainTarget = 0.0f;
	fMusic.gainStep = fMusic.gain > 0.0f
		? fMusic.gain * 1000.0f / ((float)fadeOutMs * (float)fMusic.sampleRate)
		: 1.0f;
	fMusic.stopAtTarget = true;
	SDL_UnlockAudioDevice(reinterpret_cast<SDL_AudioDeviceID>(fMusic.device));
}


void
SoundEngine::SetStreamVolume(float volume)
{
	fMusic.volume = std::max(0.0f, std::min(1.0f, volume));
}


float
SoundEngine::StreamVolume() const
{
	return fMusic.volume;
}


bool
SoundEngine::IsStreamPlaying() const
{
	return fMusic.device != 0 && !fMusic.finished;
}


AudioStream*
SoundEngine::Stream() const
{
	return fMusic.stream;
}


uint32
SoundEngine::StreamPositionMs() const
{
	if (fMusic.sampleRate == 0)
		return 0;
	return (uint32)(fMusic.framesPlayed * 1000 / fMusic.sampleRate);
}


/* static */
void
SoundEngine::MixStream(void* userData, uint8* stream, int len)
{
	StreamSlot* slot = reinterpret_cast<StreamSlot*>(userData);
	memset(stream, 0, len);
	if (slot->finished || slot->stream == nullptr)
		return;

	// The stream may hand back less than asked for; ask again until the
	// buffer is full or it is over.
	size_t filled = 0;
	while (filled < (size_t)len) {
		const size_t got = slot->stream->Read(stream + filled, (size_t)len - filled);
		if (got == 0) {
			slot->finished = true;
			break;
		}
		filled += got;
	}

	const size_t frameBytes = sizeof(int16) * slot->channels;
	const size_t frames = filled / frameBytes;
	int16* samples = reinterpret_cast<int16*>(stream);
	for (size_t frame = 0; frame < frames; frame++) {
		if (slot->gainStep > 0.0f) {
			if (slot->gain < slot->gainTarget)
				slot->gain = std::min(slot->gain + slot->gainStep, slot->gainTarget);
			else
				slot->gain = std::max(slot->gain - slot->gainStep, slot->gainTarget);
			if (slot->gain == slot->gainTarget) {
				slot->gainStep = 0.0f;
				if (slot->stopAtTarget)
					slot->finished = true;
			}
		}
		const float factor = slot->gain * slot->volume;
		for (uint16 c = 0; c < slot->channels; c++) {
			int16& sample = samples[frame * slot->channels + c];
			sample = (int16)(sample * factor);
		}
		if (slot->finished && slot->stopAtTarget) {
			// silence from here on
			memset(stream + (frame + 1) * frameBytes, 0, len - (frame + 1) * frameBytes);
			slot->framesPlayed += frame + 1;
			return;
		}
	}
	slot->framesPlayed += frames;
}
