// The synthesiser: what a recipe renders to, and that wrong recipes are
// refused with a message that says what is wrong.
#include <jgame/Synth.h>

#include <algorithm>
#include <cstdio>
#include <cstdlib>

using namespace jgame;

static int sFailures = 0;

#define CHECK(cond, msg) \
	do { \
		if (!(cond)) { \
			std::printf("FAIL: %s\\n", msg); \
			sFailures++; \
		} \
	} while (0)

static void
Refuses(const std::string& recipe, const char* mustMention)
{
	Synth::Samples out(3, 7);
	std::string error;
	if (Synth::Render(recipe, out, error)) {
		std::printf("FAIL: the recipe '%s' was accepted (expected a message about '%s')\n",
			recipe.c_str(), mustMention);
		sFailures++;
		return;
	}
	if (error.find(mustMention) == std::string::npos) {
		std::printf("FAIL: the message '%s' does not mention '%s'\n", error.c_str(), mustMention);
		sFailures++;
	}
	if (out != Synth::Samples(3, 7)) {
		std::printf("FAIL: the refused recipe '%s' changed the output\n", recipe.c_str());
		sFailures++;
	}
}


static int
Peak(const Synth::Samples& samples)
{
	int peak = 0;
	for (int16 v : samples)
		peak = std::max(peak, std::abs((int)v));
	return peak;
}


int
main()
{
	const size_t rate = Synth::kDefaultSampleRate;
	Synth::Samples one, two, mixed, same, silent;
	std::string error;
	CHECK(Synth::Render("square 440 440 0.1 0.2", one, error), "a note was refused");
	CHECK(one.size() == (size_t)(0.1f * rate), "a 0.1 s note has the wrong length");
	CHECK(Peak(one) > 1000 && Peak(one) <= 32000, "the note is silent or clipped");
	CHECK(one[0] == 0, "the note starts with a click");

	// ';' plays one after the other, '+' together
	CHECK(Synth::Render("square 440 440 0.1 0.2 ; saw 220 110 0.05 0.2", two, error)
		&& two.size() == one.size() + (size_t)(0.05f * rate), "two steps in a row have the wrong length");
	CHECK(Synth::Render("square 440 440 0.1 0.2 + saw 220 110 0.3 0.2", mixed, error)
		&& mixed.size() == (size_t)(0.3f * rate), "two notes together don't last as long as the longer one");
	CHECK(two.size() >= one.size() && Synth::Samples(two.begin(), two.begin() + one.size()) == one,
		"the first step changed when another followed");

	// The recipe and the functions give the same
	CHECK(Synth::Tone(Synth::WAVE_SQUARE, 440, 440, 0.1f, 0.2f) == one, "Tone() and the recipe differ");
	CHECK(Synth::Mix(Synth::Tone(Synth::WAVE_SQUARE, 440, 440, 0.1f, 0.2f),
		Synth::Tone(Synth::WAVE_SAW, 220, 110, 0.3f, 0.2f)) == mixed, "Mix() and the recipe differ");

	// The default decay is 1.5, and the same recipe always sounds the same
	CHECK(Synth::Render("square 440 440 0.1 0.2 1.5", same, error) && same == one, "the default decay is not 1.5");
	Synth::Samples noise, noiseAgain;
	Synth::Render("noise 0 0 0.1 0.2", noise, error);
	Synth::Render("noise 0 0 0.1 0.2", noiseAgain, error);
	CHECK(noise == noiseAgain && Peak(noise) > 1000, "noise is not the same every time");
	CHECK(Synth::Render("square 440 440 0.1 0", silent, error) && Peak(silent) == 0, "volume 0 makes a sound");

	// Layers never wrap around: they are clipped
	Synth::Samples loud;
	CHECK(Synth::Render("square 440 440 0.1 1 + square 440 440 0.1 1 + square 440 440 0.1 1", loud, error)
		&& Peak(loud) == 32000, "layers that are too loud are not clipped");

	// Another rate: as long in seconds, so more samples
	Synth::Samples fast;
	CHECK(Synth::Render("triangle 440 440 0.1 0.2", fast, error, 44100) && fast.size() == 4410,
		"the length doesn't follow the sample rate");

	// A square wave at 441 Hz and 44100 Hz turns every 50 samples
	const Synth::Samples wave = Synth::Tone(Synth::WAVE_SQUARE, 441, 441, 0.5f, 1.0f, 0.0f, 44100);
	int turns = 0;
	for (size_t i = 1; i < wave.size(); i++)
		if ((wave[i] > 0) != (wave[i - 1] > 0))
			turns++;
	CHECK(turns >= 438 && turns <= 444, "a 441 Hz note doesn't have 441 cycles a second");

	Refuses("", "empty");
	Refuses("   ", "empty");
	Refuses("square 440 440 0.1", "a note is");
	Refuses("square 440 440 0.1 0.2 1 7", "a note is");
	Refuses("sine 440 440 0.1 0.2", "sine");
	Refuses("square high 440 0.1 0.2", "high");
	Refuses("square 440 440 0.1 loud", "loud");
	Refuses("square -5 440 0.1 0.2", "frequencies");
	Refuses("square 440 99999 0.1 0.2", "frequencies");
	Refuses("square 440 440 0 0.2", "length");
	Refuses("square 440 440 60 0.2", "length");
	Refuses("square 440 440 0.1 2", "volume");
	Refuses("square 440 440 0.1 0.2 99", "decay");
	Refuses("square 440 440 0.1 0.2 ; ", "a note is");
	Refuses("square 440 440 0.1 0.2 + ", "a note is");

	if (sFailures == 0)
		std::printf("SynthTest: all good\n");
	return sFailures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
