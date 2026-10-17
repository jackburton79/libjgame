[![build](https://github.com/jackburton79/libjgame/actions/workflows/c-cpp.yml/badge.svg)](https://github.com/jackburton79/libjgame/actions/workflows/c-cpp.yml)

# libjgame

A small C++17 library of building blocks for 2D games, built on top of
[SDL2](https://www.libsdl.org/). It is the common code shared by my own game
projects: windowing and software drawing, input, sound, timers and a handful of
support classes.

It is a static library (`libjgame.a`). All the public headers are in
`include/jgame/` (one flat directory, no per-module subdirectories) and
everything is in the `jgame` namespace (`jgame::Bitmap`, `jgame::Timer`,
`jgame::GFX::Color`...).

```cpp
#include <jgame/jgame.h>      // everything...
#include <jgame/Bitmap.h>     // ...or just what you need

using namespace jgame;
```

## Modules

The sources are grouped in a directory per module; the headers named here are in
`include/jgame/`.

| Sources     | What it provides |
|-------------|------------------|
| `graphics/` | `GraphicsEngine` (window, renderer, video modes, screen scaling, fade, screenshots), `Bitmap` (software surface with pixel/line/rect/polygon drawing, dithered fills, colour key, alpha, palettes, masked and scaled blits), `Polygon`, and text: `Font`, the interface a font has to implement (so a game can use fonts of its own), `FontLoader`, where fonts come from, and a tiny built-in 3x5 pixel font that needs no font files (`Font::Builtin()`). |
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

### Installed (recommended)

```sh
make
sudo make install            # PREFIX=/usr/local by default; use PREFIX=... to change it
```

This installs `libjgame.a`, the headers (in `$PREFIX/include/jgame`) and a
`jgame.pc` file for pkg-config, which also brings in SDL2:

```sh
g++ -std=c++17 mygame.cpp $(pkg-config --cflags --libs jgame) -o mygame
```

If the prefix is not one pkg-config looks into, add `$PREFIX/lib/pkgconfig` to
`PKG_CONFIG_PATH`. `make uninstall` removes everything.

### Without installing

Point the include path at `include` and link the library, SDL2 and zlib:

```sh
g++ -std=c++17 -I/path/to/libjgame/include $(sdl2-config --cflags) \
    mygame.cpp -o mygame \
    -L/path/to/libjgame/lib -ljgame $(sdl2-config --libs) -lz
```

### A minimal program

[`examples/minimal.cpp`](examples/minimal.cpp) opens a window and shows some text:

```cpp
#include <jgame/jgame.h>

using namespace jgame;

int main()
{
	if (!GraphicsEngine::Initialize() || !Timer::Initialize())
		return 1;

	GraphicsEngine* gfx = GraphicsEngine::Get();
	gfx->SetVideoMode(320, 200, 32, GraphicsEngine::VIDEOMODE_PIXELATED);
	gfx->SetWindowSize(960, 600);

	Bitmap* screen = gfx->ScreenBitmap();
	while (!Events::QuitRequested()) {
		Events::Pump();
		if (Keyboard::WasKeyPressed(Keyboard::KEY_ESCAPE))
			Events::RequestQuit();

		screen->Clear(screen->MapRGBColor(20, 20, 60));
		Font::Builtin().DrawCentered(screen, 160, 90, "HELLO JGAME",
			screen->MapRGBColor(255, 255, 255), 4);
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
