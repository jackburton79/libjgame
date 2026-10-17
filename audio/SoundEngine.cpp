/*
 * SoundEngine.cpp
 *
 *  Created on: 07/giu/2012
 *      Author: JackBurton
 */

#include <jgame/SoundEngine.h>

#include <jgame/Log.h>

#include "SDL.h"

#include <algorithm>
#include <iostream>

namespace jgame {

static SoundEngine* sSoundEngine = nullptr;

static const uint32 kMixChannels = 2;
static const uint16 kMixSamples = 1024;		// frames per callback
static const size_t kChunkBytes = 4096;

typedef SDL_AudioStream ConverterType;


static SDL_AudioFormat
FormatForBits(uint16 bits)
{
	return bits == 8 ? AUDIO_U8 : AUDIO_S16LSB;
}


// Converts PCM from the given format to the mixer's: stereo, 16 bit, native
// endianness, SoundEngine::kMixSampleRate.
static ConverterType*
NewConverter(SDL_AudioFormat format, uint16 channels, uint32 sampleRate)
{
	return SDL_NewAudioStream(format, (Uint8)channels, (int)sampleRate,
		AUDIO_S16SYS, kMixChannels, (int)SoundEngine::kMixSampleRate);
}


SoundEngine::SoundEngine()
	:
	fBuffer(nullptr),
	fBufferConverter(nullptr),
	fBufferActive(false),
	fPlaying(false),
	fDevice(0)
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
	// Closing the device waits for its callback, so what it uses can go after
	if (fDevice != 0)
		SDL_CloseAudioDevice(static_cast<SDL_AudioDeviceID>(fDevice));
	fDevice = 0;

	if (fBufferConverter != nullptr)
		SDL_FreeAudioStream(static_cast<ConverterType*>(fBufferConverter));
	delete fBuffer;

	if (fMusic.converter != nullptr)
		SDL_FreeAudioStream(static_cast<ConverterType*>(fMusic.converter));
	delete fMusic.stream;
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


// The one audio device, opened on first use and kept for the engine's life.
// SDL converts to whatever the hardware wants, so the format asked for is
// always the one the callback gets.
bool
SoundEngine::_EnsureDevice()
{
	if (fDevice != 0)
		return true;

	SDL_AudioSpec spec;
	SDL_zero(spec);
	spec.freq = (int)kMixSampleRate;
	spec.format = AUDIO_S16SYS;
	spec.channels = (Uint8)kMixChannels;
	spec.samples = kMixSamples;
	spec.callback = SoundEngine::MixCallback;
	spec.userdata = this;

	SDL_AudioSpec obtained;
	SDL_AudioDeviceID device = SDL_OpenAudioDevice(nullptr, 0, &spec, &obtained, 0);
	if (device == 0) {
		std::cerr << Log::Red << "SoundEngine: Unable to open audio device: "
			<< SDL_GetError() << Log::Normal << std::endl;
		return false;
	}
	fMixScratch.assign((size_t)obtained.samples * obtained.channels, 0);
	fGainScratch.assign(obtained.samples, 0.0f);
	fDevice = static_cast<uint32>(device);
	SDL_PauseAudioDevice(device, 0);
	return true;
}


void
SoundEngine::_LockDevice()
{
	if (fDevice != 0)
		SDL_LockAudioDevice(static_cast<SDL_AudioDeviceID>(fDevice));
}


void
SoundEngine::_UnlockDevice()
{
	if (fDevice != 0)
		SDL_UnlockAudioDevice(static_cast<SDL_AudioDeviceID>(fDevice));
}


bool
SoundEngine::InitBuffers(bool stereo, bool bit16, uint16 sampleRate, uint32 bufferLen)
{
	if (!_EnsureDevice())
		return false;

	DestroyBuffers();

	// The buffer's samples are 16 bit whatever `bit16` says: that is what
	// the device it used to open got.
	ConverterType* converter = NewConverter(AUDIO_S16LSB, stereo ? 2 : 1, sampleRate);
	if (converter == nullptr)
		return false;

	SoundBuffer* buffer = new SoundBuffer(stereo, bit16, sampleRate, bufferLen);
	_LockDevice();
	fBuffer = buffer;
	fBufferConverter = converter;
	_UnlockDevice();
	return true;
}


void
SoundEngine::DestroyBuffers()
{
	_LockDevice();
	SoundBuffer* buffer = fBuffer;
	void* converter = fBufferConverter;
	fBuffer = nullptr;
	fBufferConverter = nullptr;
	fBufferActive = false;
	_UnlockDevice();

	if (converter != nullptr)
		SDL_FreeAudioStream(static_cast<ConverterType*>(converter));
	delete buffer;
}


SoundBuffer*
SoundEngine::Buffer()
{
	return fBuffer;
}


bool
SoundEngine::Lock()
{
	_LockDevice();
	return true;
}


void
SoundEngine::Unlock()
{
	_UnlockDevice();
}


void
SoundEngine::StartStopAudio()
{
	_LockDevice();
	fBufferActive = fBuffer != nullptr;
	_UnlockDevice();
}


bool
SoundEngine::IsPlaying()
{
	return fPlaying;
}


/* static */
void
SoundEngine::MixCallback(void* castToThis, Uint8* stream, int numBytes)
{
	SoundEngine* engine = reinterpret_cast<SoundEngine*>(castToThis);
	const uint32 frames = (uint32)numBytes / (sizeof(int16) * kMixChannels);
	engine->_Mix(reinterpret_cast<int16*>(stream), frames);
}


// Runs on the audio thread, with the device locked: adds up the sources into
// 32 bit accumulators and clips.
void
SoundEngine::_Mix(int16* out, uint32 frames)
{
	const uint32 samples = frames * kMixChannels;
	if (fMixScratch.size() < samples)
		fMixScratch.resize(samples);
	int32* mix = fMixScratch.data();
	std::fill(mix, mix + samples, 0);

	_MixStream(mix, frames);
	_MixBuffer(mix, frames);

	for (uint8 i = 0; i < kMaxOneShots; i++) {
		OneShotSound& slot = fOneShots[i];
		if (!slot.active)
			continue;
		const uint32 count = std::min(samples, (uint32)slot.data.size() - slot.position);
		for (uint32 s = 0; s < count; s++)
			mix[s] += slot.data[slot.position + s];
		slot.position += count;
		if (slot.position >= slot.data.size())
			slot.active = false;
	}

	for (uint32 s = 0; s < samples; s++)
		out[s] = (int16)std::max(-32768, std::min(32767, mix[s]));
}


// Pulls what the converter holds into `mix`, topping it up from `fill` first
// (which returns false when its source has nothing more). Returns the
// number of frames written.
template <typename Fill>
static uint32
PullConverted(ConverterType* converter, int32* mix, uint32 frames, float* gain,
	Fill fill)
{
	const int wantedBytes = (int)(frames * sizeof(int16) * kMixChannels);
	while (SDL_AudioStreamAvailable(converter) < wantedBytes) {
		if (!fill(converter))
			break;
	}

	int16 chunk[kChunkBytes / sizeof(int16)];
	uint32 done = 0;
	while (done < frames) {
		const uint32 batch = std::min<uint32>(frames - done,
			sizeof(chunk) / (sizeof(int16) * kMixChannels));
		const int got = SDL_AudioStreamGet(converter, chunk,
			(int)(batch * sizeof(int16) * kMixChannels));
		if (got <= 0)
			break;
		const uint32 gotFrames = (uint32)got / (sizeof(int16) * kMixChannels);
		for (uint32 f = 0; f < gotFrames; f++) {
			float g = gain != nullptr ? gain[f] : 1.0f;
			for (uint32 c = 0; c < kMixChannels; c++)
				mix[(done + f) * kMixChannels + c] += (int32)(chunk[f * kMixChannels + c] * g);
		}
		done += gotFrames;
	}
	return done;
}


// The music: pulls PCM from the AudioStream, converts it, applies the volume
// and the fade.
void
SoundEngine::_MixStream(int32* mix, uint32 frames)
{
	StreamSlot& slot = fMusic;
	if (slot.finished || slot.stream == nullptr || slot.converter == nullptr)
		return;
	ConverterType* converter = static_cast<ConverterType*>(slot.converter);

	// The stream may hand back less than asked for; ask again until the
	// converter has enough or the stream is over.
	auto fill = [&](ConverterType* c) {
		if (slot.sourceEnded)
			return false;
		uint8 buffer[kChunkBytes];
		const size_t got = slot.stream->Read(buffer, sizeof(buffer));
		if (got == 0) {
			slot.sourceEnded = true;
			SDL_AudioStreamFlush(c);
			return false;
		}
		SDL_AudioStreamPut(c, buffer, (int)got);
		return true;
	};

	// The fade changes per frame, so work out the gain of each frame first
	if (fGainScratch.size() < frames)
		fGainScratch.resize(frames);
	float* gains = fGainScratch.data();
	bool silentFromHere = false;
	uint32 silentFrame = frames;
	for (uint32 f = 0; f < frames; f++) {
		if (slot.gainStep > 0.0f) {
			if (slot.gain < slot.gainTarget)
				slot.gain = std::min(slot.gain + slot.gainStep, slot.gainTarget);
			else
				slot.gain = std::max(slot.gain - slot.gainStep, slot.gainTarget);
			if (slot.gain == slot.gainTarget) {
				slot.gainStep = 0.0f;
				if (slot.stopAtTarget) {
					silentFromHere = true;
					silentFrame = f + 1;
				}
			}
		}
		gains[f] = (silentFromHere && f >= silentFrame) ? 0.0f : slot.gain * slot.volume;
	}

	const uint32 done = PullConverted(converter, mix, frames, gains, fill);
	slot.framesPlayed += std::min(done, silentFrame);
	if (silentFromHere)
		slot.finished = true;
	else if (done < frames && slot.sourceEnded
			&& SDL_AudioStreamAvailable(converter) == 0)
		slot.finished = true;
}


// The raw ring buffer (movie audio)
void
SoundEngine::_MixBuffer(int32* mix, uint32 frames)
{
	if (!fBufferActive || fBuffer == nullptr || fBufferConverter == nullptr)
		return;

	auto fill = [&](ConverterType* c) {
		uint8 buffer[kChunkBytes];
		uint16 got = fBuffer->ConsumeSamples(buffer, sizeof(buffer));
		if (got == 0)
			return false;
		SDL_AudioStreamPut(c, buffer, got);
		return true;
	};
	PullConverted(static_cast<ConverterType*>(fBufferConverter), mix, frames, nullptr, fill);
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
	if (data == nullptr || dataSize == 0 || (bitsPerSample != 8 && bitsPerSample != 16)
			|| channels == 0 || sampleRate == 0)
		return;
	if (!_EnsureDevice())
		return;

	// Convert here, on the caller's thread, so the callback only adds
	ConverterType* converter = NewConverter(FormatForBits(bitsPerSample), channels, sampleRate);
	if (converter == nullptr) {
		std::cerr << Log::Red << "SoundEngine::PlaySample(): " << SDL_GetError()
			<< Log::Normal << std::endl;
		return;
	}
	std::vector<int16> converted;
	if (SDL_AudioStreamPut(converter, data, (int)dataSize) == 0
			&& SDL_AudioStreamFlush(converter) == 0) {
		const int bytes = SDL_AudioStreamAvailable(converter);
		if (bytes > 0) {
			converted.resize((size_t)bytes / sizeof(int16));
			const int got = SDL_AudioStreamGet(converter, converted.data(), bytes);
			converted.resize(got > 0 ? (size_t)got / sizeof(int16) : 0);
		}
	}
	SDL_FreeAudioStream(converter);
	if (converted.empty())
		return;

	_LockDevice();
	int slotIndex = -1;
	for (uint8 i = 0; i < kMaxOneShots; i++) {
		if (!fOneShots[i].active) {
			slotIndex = i;
			break;
		}
	}
	if (slotIndex == -1) {
		// Pool full: cut off the sound that has played the longest
		slotIndex = 0;
		for (uint8 i = 1; i < kMaxOneShots; i++) {
			if (fOneShots[i].position > fOneShots[slotIndex].position)
				slotIndex = i;
		}
	}
	OneShotSound& slot = fOneShots[slotIndex];
	slot.data.swap(converted);
	slot.position = 0;
	slot.active = true;
	_UnlockDevice();
}


// The music stream: converted to the mixer's format on the audio thread, with
// the volume and the fade applied.

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

	if (!_EnsureDevice()) {
		delete stream;
		return false;
	}

	ConverterType* converter = NewConverter(AUDIO_S16LSB, channels, sampleRate);
	if (converter == nullptr) {
		std::cerr << Log::Red << "SoundEngine::PlayStream(): " << SDL_GetError()
			<< Log::Normal << std::endl;
		delete stream;
		return false;
	}

	_LockDevice();
	AudioStream* previous = fMusic.stream;
	void* previousConverter = fMusic.converter;
	fMusic.stream = stream;
	fMusic.converter = converter;
	fMusic.sourceEnded = false;
	fMusic.volume = volume;
	fMusic.framesPlayed = 0;
	fMusic.stopAtTarget = false;
	fMusic.finished = false;
	if (fadeInMs > 0) {
		fMusic.gain = 0.0f;
		fMusic.gainTarget = 1.0f;
		fMusic.gainStep = 1000.0f / ((float)fadeInMs * (float)kMixSampleRate);
	} else {
		fMusic.gain = 1.0f;
		fMusic.gainTarget = 1.0f;
		fMusic.gainStep = 0.0f;
	}
	_UnlockDevice();

	if (previousConverter != nullptr)
		SDL_FreeAudioStream(static_cast<ConverterType*>(previousConverter));
	delete previous;
	return true;
}


void
SoundEngine::StopStream(uint32 fadeOutMs)
{
	if (fDevice == 0 || fMusic.finished)
		return;

	_LockDevice();
	if (fadeOutMs == 0) {
		fMusic.finished = true;
	} else {
		fMusic.gainTarget = 0.0f;
		fMusic.gainStep = fMusic.gain > 0.0f
			? fMusic.gain * 1000.0f / ((float)fadeOutMs * (float)kMixSampleRate)
			: 1.0f;
		fMusic.stopAtTarget = true;
	}
	_UnlockDevice();
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
	return fDevice != 0 && !fMusic.finished;
}


AudioStream*
SoundEngine::Stream() const
{
	return fMusic.stream;
}


uint32
SoundEngine::StreamPositionMs() const
{
	return (uint32)(fMusic.framesPlayed * 1000 / kMixSampleRate);
}

} // namespace jgame
