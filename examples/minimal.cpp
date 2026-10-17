// The smallest program that uses the library: a window with some text, until
// the window is closed or Escape is pressed.
//
//   g++ -std=c++17 examples/minimal.cpp $(pkg-config --cflags --libs jgame) -o minimal
#include <jgame/jgame.h>

using namespace jgame;

int
main()
{
	if (!GraphicsEngine::Initialize() || !Timer::Initialize())
		return 1;

	GraphicsEngine* gfx = GraphicsEngine::Get();
	gfx->SetWindowCaption("jgame");
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
