/*
 * Synth.cpp
 */

#include "Synth.h"

#include "Rng.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <sstream>

namespace {

float
Oscillate(Synth::Wave wave, float phase, float duty, Rng& rng)
{
	switch (wave) {
		case Synth::WAVE_SQUARE:	return phase < duty ? 1.0f : -1.0f;
		case Synth::WAVE_SAW:		return 2.0f * phase - 1.0f;
		case Synth::WAVE_TRIANGLE:	return phase < 0.5f ? 4.0f * phase - 1.0f : 3.0f - 4.0f * phase;
		case Synth::WAVE_NOISE:		return rng.Range(-1.0f, 1.0f);
	}
	return 0;
}


std::vector<std::string>
Split(const std::string& text, char separator)
{
	std::vector<std::string> parts;
	std::string current;
	for (char c : text) {
		if (c == separator) {
			parts.push_back(current);
			current.clear();
		} else
			current += c;
	}
	parts.push_back(current);
	return parts;
}


bool
ParseNote(const std::string& text, uint32 sampleRate, Synth::Samples& out, std::string& error)
{
	std::istringstream in(text);
	std::vector<std::string> words;
	std::string word;
	while (in >> word)
		words.push_back(word);
	if (words.size() < 5 || words.size() > 6) {
		error = "a note is: wave from-Hz to-Hz seconds volume [decay], not '" + text + "'";
		return false;
	}

	Synth::Wave wave;
	if (words[0] == "square")
		wave = Synth::WAVE_SQUARE;
	else if (words[0] == "saw")
		wave = Synth::WAVE_SAW;
	else if (words[0] == "triangle")
		wave = Synth::WAVE_TRIANGLE;
	else if (words[0] == "noise")
		wave = Synth::WAVE_NOISE;
	else {
		error = "the wave should be square, saw, triangle or noise, not '" + words[0] + "'";
		return false;
	}

	float number[5] = { 0, 0, 0, 0, Synth::kDefaultDecay };
	for (size_t i = 1; i < words.size(); i++) {
		char* end = nullptr;
		number[i - 1] = std::strtof(words[i].c_str(), &end);
		if (*end != 0) {
			error = "'" + words[i] + "' is not a number (in '" + text + "')";
			return false;
		}
	}
	const float f0 = number[0], f1 = number[1], seconds = number[2], volume = number[3], decay = number[4];
	if (f0 < 0 || f0 > 8000 || f1 < 0 || f1 > 8000) {
		error = "the frequencies should be from 0 to 8000 Hz (in '" + text + "')";
		return false;
	}
	if (seconds <= 0 || seconds > 5) {
		error = "the length should be above 0 and at most 5 seconds (in '" + text + "')";
		return false;
	}
	if (volume < 0 || volume > 1) {
		error = "the volume should be from 0 to 1 (in '" + text + "')";
		return false;
	}
	if (decay < 0 || decay > 10) {
		error = "the decay should be from 0 to 10 (in '" + text + "')";
		return false;
	}
	out = Synth::Tone(wave, f0, f1, seconds, volume, decay, sampleRate);
	return true;
}

} // namespace


namespace Synth {

Samples
Tone(Wave wave, float fromHz, float toHz, float seconds, float volume, float decay,
	uint32 sampleRate)
{
	// The noise is the same every time
	Rng rng(12345);
	// A short ramp at the start, so the note doesn't click
	const float attackSamples = 80.0f * sampleRate / kDefaultSampleRate;
	int n = (int)(seconds * sampleRate);
	Samples out(n);
	float phase = 0;
	for (int i = 0; i < n; i++) {
		float t = (float)i / n;
		float freq = fromHz + (toHz - fromHz) * t;
		phase += freq / sampleRate;
		phase -= std::floor(phase);
		float attack = std::min(1.0f, i / attackSamples);
		float envelope = attack * std::pow(1.0f - t, decay);
		out[i] = (int16)(Oscillate(wave, phase, 0.5f, rng) * envelope * volume * 32000.0f);
	}
	return out;
}


Samples
Mix(const Samples& a, const Samples& b)
{
	Samples out(std::max(a.size(), b.size()));
	for (size_t i = 0; i < out.size(); i++) {
		int v = (i < a.size() ? a[i] : 0) + (i < b.size() ? b[i] : 0);
		out[i] = (int16)std::max(-32000, std::min(32000, v));
	}
	return out;
}


bool
Render(const std::string& recipe, Samples& out, std::string& error, uint32 sampleRate)
{
	if (recipe.find_first_not_of(" \t") == std::string::npos) {
		error = "the recipe is empty";
		return false;
	}
	Samples result;
	for (const std::string& step : Split(recipe, ';')) {
		Samples mixed;
		bool any = false;
		for (const std::string& note : Split(step, '+')) {
			Samples samples;
			if (!ParseNote(note, sampleRate, samples, error))
				return false;
			mixed = any ? Mix(mixed, samples) : samples;
			any = true;
		}
		result.insert(result.end(), mixed.begin(), mixed.end());
	}
	if (result.empty()) {
		error = "the recipe is empty";
		return false;
	}
	out = result;
	return true;
}

} // namespace Synth
