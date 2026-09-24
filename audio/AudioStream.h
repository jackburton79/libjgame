/*
 * AudioStream.h
 *
 * A source of continuously streamed 16 bit PCM (music), pulled by the sound
 * engine's audio thread through SoundEngine::PlayStream().
 */

#pragma once

#include "SupportDefs.h"

#include <cstddef>

class AudioStream {
public:
	virtual ~AudioStream() {}

	virtual uint16 Channels() const = 0;
	virtual uint32 SampleRate() const = 0;

	// Writes up to `bytes` of interleaved little-endian 16 bit samples into
	// `buffer` and returns how many bytes it wrote; 0 means the stream is over.
	// Runs on the audio thread: it must not block for long or touch the game.
	virtual size_t Read(uint8* buffer, size_t bytes) = 0;
};
