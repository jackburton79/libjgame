#pragma once

#include <jgame/Bitmap.h>
#include <jgame/GraphicsDefs.h>

struct SDL_Window;
struct SDL_Renderer;

namespace jgame {
class GraphicsEngine {
public:
	struct RenderContext;

	static GraphicsEngine* Get();
	static bool Initialize();
	static void Destroy();

	enum MASK_VALUES {
		MASK_NO_MASK = 0,
		MASK_SHADE = 1,
		MASK_COMPLETELY = 255
	};

	static bool BlitBitmap(const Bitmap* sourceBitmap, GFX::rect* sourceRect,
						   Bitmap* destBitmap, GFX::rect* destRect);
	static bool BlitBitmapWithMask(const Bitmap* sourceBitmap, GFX::rect* sourceRect,
								Bitmap* destBitmap, GFX::rect* destRect,
								const Bitmap* mask, GFX::rect* maskRect);
	static bool BlitBitmapScaled(const Bitmap* sourceBitmap, GFX::rect* sourceRect,
							Bitmap* destBitmap, GFX::rect* destRect);

	enum BLITTING_FLAGS {
		BLITTING_NORMAL = 0,
		BLITTING_SCALED = 1
	};
	bool BlitToScreen(const Bitmap* sourceBitmap,
						  const GFX::point& position);
	bool BlitToScreen(const Bitmap* sourceBitmap,
					  GFX::rect* sourceRect,
					  GFX::rect* destRect);
	bool BlitToScreenScaled(const Bitmap* sourceBitmap,
					  GFX::rect* sourceRect,
					  GFX::rect* destRect);

	void SetClipping(const GFX::rect* rect);

	enum VIDEOMODE_FLAGS {
		VIDEOMODE_WINDOWED = 0,
		VIDEOMODE_FULLSCREEN = 1,
		// Scale the screen to the window without smoothing (for pixel art)
		VIDEOMODE_PIXELATED = 2
	};
	void SetVideoMode(uint16 x, uint16 y, uint16 depth, uint16 flags);
	// The window can have a size of its own: the screen is scaled to fit it,
	// keeping its proportions. A game can then draw at a low resolution and
	// show it bigger. Call it after SetVideoMode().
	void SetWindowSize(uint16 width, uint16 height);
	// The SDL window and renderer the engine draws on (null before SetVideoMode())
	SDL_Window* Window() const;
	SDL_Renderer* Renderer() const;
	void SaveCurrentMode();
	void RestorePreviousMode();

	GFX::rect ScreenFrame() const;
	void SetRenderingOffset(const GFX::point& point);

	void SetWindowCaption(const char* caption);

	Bitmap* ScreenBitmap();
	void Update();

	bool SaveScreenshot(const char* path);

	void SetFade(uint16 value);

private:
	RenderContext* fRenderContext;

	Bitmap* fScreen;
	uint16 fFlags;
	GFX::point fRenderingOffset;

	GFX::rect fOldRect;
	uint16 fOldDepth;
	uint16 fOldFlags;

	GraphicsEngine();
	~GraphicsEngine();
};

} // namespace jgame
