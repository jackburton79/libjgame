#ifndef __GRAPHICS_ENGINE_H
#define __GRAPHICS_ENGINE_H

#include "SDL.h"
#include "Bitmap.h"
#include "IETypes.h"

#include <vector>

class Bitmap;
class Listener;
class GraphicsEngine {
public:
	GraphicsEngine();
	~GraphicsEngine();
	
	static GraphicsEngine* Get();
	static bool Initialize();
	static void Destroy();

	static Bitmap* CreateBitmap(uint16 width, uint16 height, uint16 depth);
	static void DeleteBitmap(Bitmap* sprite);

	enum MASK_VALUES {
		MASK_NO_MASK = 0,
		MASK_SHADE = 1,
		MASK_COMPLETELY = 255
	};

	static void BlitBitmap(const Bitmap* bitmap, GFX::rect* source, Bitmap* surface, GFX::rect* dest);
	static void BlitBitmapWithMask(const Bitmap* bitmap, GFX::rect* source,
								Bitmap* surface, GFX::rect* dest,
								const Bitmap* mask, GFX::rect* maskRect);
	static void FillRect(Bitmap* bitmap, GFX::rect* rect, uint8 pixelColor);

	void SetClipping(const GFX::rect* rect);
	void BlitToScreen(const Bitmap* bitmap, GFX::rect* source, GFX::rect* dest);
	void StrokeRect(const GFX::rect& rect, uint32 color);
	void FillRect(const GFX::rect& rect, uint32 color);

	void SetVideoMode(uint16 x, uint16 y, uint16 depth, uint16 flags);
	void SaveCurrentMode();
	void RestorePreviousMode();

	GFX::rect VideoArea() const;

	void SetWindowCaption(const char* caption);

	Bitmap* ScreenBitmap();

	void Flip();

	// Observer/Listener
	void AddListener(Listener* listener);
	void RemoveListener(Listener* listener);

private:
	Bitmap* fScreen;
	SDL_Rect fOldRect;
	uint16 fOldDepth;

	std::vector<Listener*> fListeners;
};

#endif
