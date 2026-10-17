[![build](https://github.com/jackburton79/libjgame/actions/workflows/c-cpp.yml/badge.svg)](https://github.com/jackburton79/libjgame/actions/workflows/c-cpp.yml)

# libjgame

A small C++17 library of building blocks for 2D games, built on top of
[SDL2](https://www.libsdl.org/). It is the common code shared by my own game
projects: windowing and software drawing, input, sound, timers and a handful of
support classes.

It is a static library (`libjgame.a`) with no separate public include
directory: each module is a folder of headers and sources, and the headers are
meant to be included directly.

## Modules

| Directory   | What it provides |
|-------------|------------------|
| `graphics/` | `GraphicsEngine` (window, renderer, video modes, screen scaling, fade, screenshots), `Bitmap` (software surface with pixel/line/rect/polygon drawing, dithered fills, colour key, alpha, palettes, masked and scaled blits), `Polygon`, and `Font`, a tiny built-in 3x5 pixel font that needs no font files. |
| `audio/`    | `SoundEngine`, one audio device that mixes a music stream, up to 8 overlapping one-shot samples and a raw ring buffer, with volume and fades; `AudioStream`, the interface for streamed music; `Synth`, a small synthesiser that turns a text "recipe" into sound effects, with no audio files and no audio device required. |
| `input/`    | `Events`, the per-frame event pump and quit handling; `Keyboard`, polled key state independent from SDL. |
| `timers/`   | `Timer`: named timers, one-shot and periodic callbacks, tick counting and frame-rate sync helpers. |
| `streams/`  | A `Stream` abstraction with file, memory and string implementations, plus `EncryptedStream`, a wrapper that decrypts another stream with a key. |
| `support/`  | Basic types (`SupportDefs.h`), reference counting (`Referenceable`, `Reference<T>`), `Path`, `Log`, `Bytes`, zlib decompression (`ZLibDecompressor`), `Rng`, a small deterministic and portable header-only random generator, and assorted utilities. |

## Requirements

- A C++17 compiler (GCC or Clang)
- SDL2 development files (`sdl2-config` must be in your `PATH`)
- zlib
- GNU Make

On Debian/Ubuntu:

```sh
sudo apt install build-essential libsdl2-dev zlib1g-dev
```

On macOS (Homebrew):

```sh
brew install sdl2
```

The library is built and tested on Linux and macOS by CI.

## Building

```sh
make              # builds lib/libjgame.a
make DEBUG=1      # debug build (-g -O0) instead of the default -O3
make clean
```

Warnings are treated as errors (`-Wall -Werror`).

## Tests

```sh
make tests        # builds the test programs in test/
make test         # builds and runs them all
```

`make test` runs every test with SDL's `dummy` video and audio drivers, so no
screen or sound card is needed.

## Using the library in your project

Add the library's module directories to the include path, then link against
`libjgame.a`, SDL2 and zlib:

```sh
g++ -std=c++17 \
    -I/path/to/libjgame \
    -I/path/to/libjgame/audio -I/path/to/libjgame/graphics \
    -I/path/to/libjgame/input -I/path/to/libjgame/streams \
    -I/path/to/libjgame/support -I/path/to/libjgame/timers \
    $(sdl2-config --cflags) \
    mygame.cpp -o mygame \
    -L/path/to/libjgame/lib -ljgame $(sdl2-config --libs) -lz
```

A minimal program:

```cpp
#include "Events.h"
#include "GraphicsEngine.h"
#include "Keyboard.h"
#include "Timer.h"

int main()
{
	GraphicsEngine::Initialize();
	Timer::Initialize();

	GraphicsEngine* gfx = GraphicsEngine::Get();
	gfx->SetVideoMode(320, 200, 32, GraphicsEngine::VIDEOMODE_PIXELATED);
	gfx->SetWindowSize(960, 600);

	while (!Events::QuitRequested()) {
		Events::Pump();
		if (Keyboard::WasKeyPressed(Keyboard::KEY_ESCAPE))
			Events::RequestQuit();

		Bitmap* screen = gfx->ScreenBitmap();
		screen->Clear(0);
		// ... draw the frame, e.g. with Font::DrawCentered(...)
		gfx->Update();
		Timer::Wait(16);
	}

	Timer::TearDown();
	GraphicsEngine::Destroy();
	return 0;
}
```

The programs in `test/` are also good examples of how each module is used
(for instance `FontTest.cpp`, `SynthTest.cpp` and `EventsTest.cpp`).

## License

Released under the [MIT License](LICENSE).
