/*
 * SoundEngine.h
 *
 *  Created on: 07/giu/2012
 *      Author: JackBurton
 */

#pragma once

#include "AudioStream.h"
#include "SupportDefs.h"

#include <vector>

// One slot of SoundEngine's one-shot sound-effect pool - see PlaySample().
// The sound is converted when it is queued, so `data` is always interleaved
// stereo 16 bit at the mixer's rate and the audio callback only has to add it.
struct OneShotSound {
	std::vector<int16> data;
	uint32 position = 0;		// in samples (a stereo frame is two)
	bool active = false;
};


// The engine's music slot: one AudioStream, with a volume and a fade (see
// SoundEngine::PlayStream()). `converter` turns the stream's PCM into the
// mixer's format (really an SDL_AudioStream*; void* keeps SDL.h out of here).
struct StreamSlot {
	AudioStream* stream = nullptr;
	void* converter = nullptr;
	bool sourceEnded = false;	// the stream has nothing more to give
	float volume = 1.0f;		// the bus volume
	float gain = 1.0f;		// the fade's current level, 0-1
	float gainStep = 0.0f;		// per sample frame; 0 = no fade running
	float gainTarget = 1.0f;
	bool stopAtTarget = false;	// a fade out ends the stream
	bool finished = true;		// nothing (left) to play
	uint64 framesPlayed = 0;	// at the mixer's rate
};


class SoundBuffer {
public:
	SoundBuffer(bool stereo, bool bit16, uint16 sampleRate, uint32 bufferLen);
	~SoundBuffer();

	bool IsStereo() const;
	bool Is16Bit() const;
	uint16 SampleRate() const;
	uint8* Data();

	uint16 ConsumeSamples(uint8* destBuffer, uint16 numSamples);
	void AddSample(sint16 sample);

	uint32 AvailableData() const;

	// deleted
	SoundBuffer(const SoundBuffer&) = delete;
	SoundBuffer& operator=(const SoundBuffer&) = delete;

private:
	bool fStereo;
	bool f16Bit;
	uint16 fSampleRate;
	uint8* fData;
	uint32 fBufferLength;
	uint32 fBufferPos;
	uint32 fConsumedPos;
};


// Everything goes through one audio device, opened the first time something
// is played: the music stream, the one-shot samples and the raw buffer below
// are mixed together in its callback. (Opening a device per sound does not
// work on every platform; Haiku, for one, can't open several at once.)
class SoundEngine {
public:
	// The mixer's output format
	static const uint32 kMixSampleRate = 44100;

	static bool Initialize();
	static void Destroy();

	static SoundEngine* Get();

	bool InitBuffers(bool stereo, bool bit16, uint16 sampleRate, uint32 bufferLen);
	void DestroyBuffers();
	SoundBuffer* Buffer();

	bool Lock();
	void Unlock();

	void StartStopAudio();
	bool IsPlaying();

	// Plays `data` (already-decoded raw PCM, any channel count and rate) once.
	// Unlike the single continuous ring buffer above (built for MVE movie
	// audio, one stream at a time), this can overlap with the movie stream and
	// with other PlaySample() calls, up to kMaxOneShots at a time. If every
	// slot is busy, the one that has played longest is cut off rather than
	// dropping the new sound.
	void PlaySample(const uint8* data, uint32 dataSize, uint16 channels,
		uint16 bitsPerSample, uint32 sampleRate);

	// Streams `stream` (which the engine takes over, any channel count and
	// rate) as the music, after stopping the one already playing. `volume` is 0-1; a `fadeInMs` above 0
	// raises the volume from silence over that time. Returns false if the audio
	// audio device can't be opened (the stream is deleted).
	bool PlayStream(AudioStream* stream, float volume = 1.0f, uint32 fadeInMs = 0);
	// Ends the stream; with `fadeOutMs` above 0 it fades out first.
	void StopStream(uint32 fadeOutMs = 0);
	void SetStreamVolume(float volume);
	// Whether a stream is still playing (false once it ends or has faded out).
	bool IsStreamPlaying() const;
	// The stream that plays (or played last), NULL if none; the engine owns it
	// and deletes it when another one replaces it.
	AudioStream* Stream() const;
	// Milliseconds of the current stream played so far.
	uint32 StreamPositionMs() const;
	float StreamVolume() const;

	// deleted
	SoundEngine(const SoundEngine&) = delete;
	SoundEngine& operator=(const SoundEngine&) = delete;

private:
	SoundBuffer* fBuffer;
	void* fBufferConverter;		// the raw buffer's PCM to the mixer's format
	bool fBufferActive;
	bool fPlaying;
	uint32 fDevice;				// the SDL_AudioDeviceID, 0 until opened

	static const uint8 kMaxOneShots = 8;
	OneShotSound fOneShots[kMaxOneShots];
	StreamSlot fMusic;
	std::vector<int32> fMixScratch;		// preallocated: the callback must not allocate
	std::vector<float> fGainScratch;

	bool _EnsureDevice();
	void _LockDevice();
	void _UnlockDevice();

	void _Mix(int16* out, uint32 frames);
	void _MixStream(int32* mix, uint32 frames);
	void _MixBuffer(int32* mix, uint32 frames);

	SoundEngine();
	~SoundEngine();

	static void MixCallback(void* userData, uint8* stream, int len);
};
