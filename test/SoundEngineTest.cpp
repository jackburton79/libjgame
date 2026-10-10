// SoundEngine must use a single audio device: some platforms (Haiku) crash or
// refuse when a program opens several. The "dummy" SDL driver allows only one
// device at a time, so this fails if the engine opens a second one.
#include "AudioStream.h"
#include "SoundEngine.h"

#include <SDL.h>

#include <cstdio>
#include <cstdlib>
#include <vector>

static int sFailures = 0;

#define CHECK(cond, msg) \
	do { \
		if (!(cond)) { \
			std::printf("FAIL: %s\n", msg); \
			sFailures++; \
		} \
	} while (0)


// A mono 11025 Hz stream (so it needs converting) of `seconds` of a tone
class ToneStream : public AudioStream {
public:
	ToneStream(float seconds) : fRemaining((size_t)(seconds * 11025) * 2) {}
	uint16 Channels() const { return 1; }
	uint32 SampleRate() const { return 11025; }
	size_t Read(uint8* buffer, size_t bytes)
	{
		size_t n = bytes < fRemaining ? bytes : fRemaining;
		int16* out = reinterpret_cast<int16*>(buffer);
		for (size_t i = 0; i < n / 2; i++)
			out[i] = (int16)((fPhase++ / 20) % 2 ? 8000 : -8000);
		fRemaining -= n;
		return n;
	}
private:
	size_t fRemaining;
	uint32 fPhase = 0;
};


int
main()
{
	SDL_setenv("SDL_AUDIODRIVER", "dummy", 1);
	if (!SoundEngine::Initialize()) {
		std::printf("SoundEngine::Initialize() failed\n");
		return EXIT_FAILURE;
	}
	SoundEngine* engine = SoundEngine::Get();

	// Music and more one-shots than the pool has slots, in several formats
	CHECK(engine->PlayStream(new ToneStream(0.3f), 0.5f), "PlayStream");
	std::vector<uint8> mono8(2000, 200);
	std::vector<int16> stereo16(4000, 1000);
	for (int i = 0; i < 20; i++) {
		engine->PlaySample(mono8.data(), (uint32)mono8.size(), 1, 8, 8000);
		engine->PlaySample(reinterpret_cast<const uint8*>(stereo16.data()),
			(uint32)(stereo16.size() * 2), 2, 16, 22050);
	}
	CHECK(engine->IsStreamPlaying(), "the stream is playing");

	// The legacy ring buffer shares the device too
	CHECK(engine->InitBuffers(true, true, 22050, 1 << 16), "InitBuffers");
	engine->StartStopAudio();

	// Time passes on the audio thread, and the finite stream ends by itself
	uint32 position = 0;
	for (int i = 0; i < 100 && engine->IsStreamPlaying(); i++) {
		SDL_Delay(20);
		position = engine->StreamPositionMs();
	}
	CHECK(!engine->IsStreamPlaying(), "the finite stream ends");
	CHECK(position >= 250 && position <= 450, "stream position is about its length");

	// Replacing a stream and fading out
	CHECK(engine->PlayStream(new ToneStream(10.0f), 1.0f, 50), "second PlayStream");
	SDL_Delay(100);
	CHECK(engine->IsStreamPlaying(), "second stream plays");
	engine->StopStream(100);
	for (int i = 0; i < 50 && engine->IsStreamPlaying(); i++)
		SDL_Delay(20);
	CHECK(!engine->IsStreamPlaying(), "fade out ends the stream");

	engine->DestroyBuffers();
	SoundEngine::Destroy();
	if (sFailures == 0)
		std::printf("SoundEngineTest: all good\n");
	return sFailures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
