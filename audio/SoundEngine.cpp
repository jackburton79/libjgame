/*
 * SoundEngine.cpp
 *
 *  Created on: 07/giu/2012
 *      Author: stefano
 */

#include "SoundEngine.h"

#include "Log.h"

#include "SDL.h"

#include <algorithm>
#include <iostream>

static SoundEngine* sSoundEngine = NULL;

SoundEngine::SoundEngine()
	:
	fBuffer(NULL),
	fPlaying(false)
{
	if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0)
		throw std::runtime_error("Error while initializing SDL Sound System");
}


SoundEngine::~SoundEngine()
{
	SDL_PauseAudio(1);
	SDL_CloseAudio();
	delete fBuffer;
}


/* static */
bool
SoundEngine::Initialize()
{
	std::cout << "Initializing Sound Engine... ";
	std::flush(std::cout);
	try {
		if (sSoundEngine == NULL)
			sSoundEngine = new SoundEngine();
	} catch (std::exception& e) {
		std::cerr << Log::Red << e.what() << std::endl;
		std::cout << "Failed!" << std::endl;
		return false;
	}
	std::cout << Log::Green << "OK!" << std::endl;
	std::cerr << Log::Normal;
	return true;
}


/* static */
void
SoundEngine::Destroy()
{
	delete sSoundEngine;
	sSoundEngine = NULL;
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
	std::cout << "InitBuffers(";
	std::cout << sampleRate << " KHz";
	std::cout << ", " << (stereo ? "STEREO" : "MONO");
	std::cout << ", " << (bit16 ? "16BIT" : "8BIT");
	std::cout << ", " << bufferLen << " bytes";
	std::cout << std::endl;

	fBuffer = new SoundBuffer(stereo, bit16, sampleRate, bufferLen);

	SDL_AudioSpec audioSpec;
	audioSpec.freq = sampleRate;
	audioSpec.format = AUDIO_S16;
	audioSpec.channels = stereo ? 2 : 1;
	audioSpec.samples = 4096;
	audioSpec.callback = SoundEngine::MixAudio;
	audioSpec.userdata = this;

	if (SDL_OpenAudio(&audioSpec, NULL) < 0 ) {
		std::cerr << Log::Red << "Unable to open audio: ";
		std::cerr << SDL_GetError() << std::endl;
		std::cerr << Log::Normal;
		delete fBuffer;
		fBuffer = NULL;
		return false;
	}

	return true;
}


void
SoundEngine::DestroyBuffers()
{
	SDL_CloseAudio();
	delete fBuffer;
	fBuffer = NULL;
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
	engine->Buffer()->ConsumeSamples(reinterpret_cast<uint8*>(stream), static_cast<uint32>(numBytes));
}


// SoundBuffer
SoundBuffer::SoundBuffer(bool stereo, bool bit16, uint16 sampleRate, uint32 bufferLen)
	:
	fStereo(stereo),
	f16Bit(bit16),
	fSampleRate(sampleRate),
	fData(NULL),
	fBufferLength(bufferLen),
	fBufferPos(0),
	fConsumedPos(0)
{
	fData = reinterpret_cast<uint8*>(calloc(1, bufferLen));
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
SoundBuffer::ConsumeSamples(uint8* destBuffer, uint32 numSamples)
{
	uint32 numRequested = (uint32)numSamples;
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

	return (uint16)numAvailable;
}


// return the number of bytes available
uint32
SoundBuffer::AvailableData() const
{
	if (fBufferPos < fConsumedPos)
		return (fBufferLength - fConsumedPos) + fBufferPos;

	return fBufferPos - fConsumedPos;
}
