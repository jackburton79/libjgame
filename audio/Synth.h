/*
 * Synth.h
 *
 * Sound effects made up from a few numbers instead of read from audio files:
 * a tiny synthesiser that turns notes, or a line of text describing them (a
 * recipe), into mono 16 bit samples, ready for SoundEngine::PlaySample().
 *
 * A recipe is steps joined by ';' that play one after the other. A step is
 * notes joined by '+' that play together. A note is
 *   <wave> <from Hz> <to Hz> <seconds> <volume> [decay]
 * where the wave is square, saw, triangle or noise, the pitch glides from the
 * first frequency to the second, the volume is 0-1 and the decay (default 1.5)
 * is how fast it fades out. For example:
 *   square 988 988 0.05 0.12 0.5 ; square 1318 1318 0.12 0.12
 *
 * The same input always gives the same samples. It needs neither SDL nor an
 * audio device.
 */

#pragma once

#include "SupportDefs.h"

#include <string>
#include <vector>

namespace Synth {

const uint32 kDefaultSampleRate = 22050;
const float kDefaultDecay = 1.5f;

enum Wave {
	WAVE_SQUARE,
	WAVE_SAW,
	WAVE_TRIANGLE,
	WAVE_NOISE
};

typedef std::vector<int16> Samples;

// A note gliding from `fromHz` to `toHz` with a decaying envelope
Samples Tone(Wave wave, float fromHz, float toHz, float seconds, float volume,
	float decay = kDefaultDecay, uint32 sampleRate = kDefaultSampleRate);

// The two played together, as long as the longer one; what is too loud is
// clipped
Samples Mix(const Samples& a, const Samples& b);

// The samples of a recipe. False (and `error` saying why) if it is not
// understood; `out` is then untouched.
bool Render(const std::string& recipe, Samples& out, std::string& error,
	uint32 sampleRate = kDefaultSampleRate);

} // namespace Synth
