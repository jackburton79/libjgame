#include <iostream>

#include "Bitmap.h"
#include "GraphicsEngine.h"
#include "Log.h"

#include <SDL.h>

struct GraphicsEngine::RenderContext {
	RenderContext();
	~RenderContext();

	RenderContext(const RenderContext&) = delete;

	SDL_Window *SDLWindow;
	SDL_Renderer *SDLRenderer;
	SDL_Texture* SDLTexture;
};

GraphicsEngine::RenderContext::RenderContext()
	:
	SDLWindow(nullptr),
	SDLRenderer(nullptr),
	SDLTexture(nullptr)
{
}


GraphicsEngine::RenderContext::~RenderContext()
{
	if (SDLTexture != nullptr)
		SDL_DestroyTexture(SDLTexture);
	if (SDLRenderer != nullptr)
		SDL_DestroyRenderer(SDLRenderer);
	if (SDLWindow != nullptr)
		SDL_DestroyWindow(SDLWindow);
}


static GraphicsEngine *sGraphicsEngine = nullptr;

GraphicsEngine::GraphicsEngine()
	:
	fRenderContext(nullptr),
	fScreen(nullptr),
	fFlags(0),
	fOldDepth(0),
	fOldFlags(0)
{
	fRenderingOffset.x = fRenderingOffset.y = 0;
	fOldRect.w = fOldRect.h = fOldRect.x = fOldRect.y = 0;
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
		throw std::runtime_error("GraphicsEngine: SDL Error");
	SDL_ShowCursor(0);

	fRenderContext = new GraphicsEngine::RenderContext();
}


GraphicsEngine::~GraphicsEngine()
{
	fScreen->Release();
	delete fRenderContext;
	SDL_Quit();
}


/* static */
GraphicsEngine*
GraphicsEngine::Get()
{
	return sGraphicsEngine;
}


/* static */
bool
GraphicsEngine::Initialize()
{
	try {
		sGraphicsEngine = new GraphicsEngine();
	} catch (...) {
		sGraphicsEngine = nullptr;
		return false;
	}

	if (!GFX::InitializeGlobalPalettes()) {
		return false;
	}

	return true;
}


/* static */
void
GraphicsEngine::Destroy()
{
	delete sGraphicsEngine;
	sGraphicsEngine = nullptr;
}


void
GraphicsEngine::SetClipping(const GFX::rect* rect)
{
	if (rect != nullptr) {
		SDL_Rect sdlRect;
		GFXRectToSDLRect(rect, &sdlRect);
		SDL_SetClipRect(fScreen->Surface(), &sdlRect);
	}
	SDL_SetClipRect(fScreen->Surface(), nullptr);
}


bool
GraphicsEngine::BlitToScreen(const Bitmap* sourceBitmap,
					  const GFX::point& position)
{
	SDL_Rect sdlDestRect = {
		position.x,
		position.y,
		sourceBitmap->Width(),
		sourceBitmap->Height()
	};
	return SDL_BlitSurface(sourceBitmap->Surface(), nullptr,
					fScreen->Surface(), &sdlDestRect) == 0;
}


bool
GraphicsEngine::BlitToScreen(const Bitmap* source, GFX::rect *sourceRect,
		GFX::rect *destRect)
{
	SDL_Rect sdlSourceRect;
	SDL_Rect sdlDestRect;
	SDL_Rect* sdlSourceRectPtr = nullptr;
	SDL_Rect* sdlDestRectPtr = nullptr;
	if (sourceRect != nullptr) {
		GFXRectToSDLRect(sourceRect, &sdlSourceRect);
		sdlSourceRectPtr = &sdlSourceRect;
	}
	if (destRect != nullptr) {
		GFXRectToSDLRect(destRect, &sdlDestRect);
		sdlDestRectPtr = &sdlDestRect;
	}

	int result = SDL_BlitSurface(source->Surface(), sdlSourceRectPtr,
					fScreen->Surface(), sdlDestRectPtr);

	if (destRect != nullptr)
		SDLRectToGFXRect(&sdlDestRect, destRect);

	return result == 0;
}


bool
GraphicsEngine::BlitToScreenScaled(const Bitmap* source, GFX::rect *sourceRect,
		GFX::rect *destRect)
{
	SDL_Rect sdlSourceRect;
	SDL_Rect sdlDestRect;
	SDL_Rect* sdlSourceRectPtr = nullptr;
	SDL_Rect* sdlDestRectPtr = nullptr;
	if (sourceRect != NULL) {
		GFXRectToSDLRect(sourceRect, &sdlSourceRect);
		sdlSourceRectPtr = &sdlSourceRect;
	}
	if (destRect != nullptr) {
		GFXRectToSDLRect(destRect, &sdlDestRect);
		sdlDestRectPtr = &sdlDestRect;
	}

	int result = SDL_BlitScaled(source->Surface(), sdlSourceRectPtr,
					fScreen->Surface(), sdlDestRectPtr);

	if (destRect != nullptr)
		SDLRectToGFXRect(&sdlDestRect, destRect);

	return result == 0;
}


/*static*/
bool
GraphicsEngine::BlitBitmap(const Bitmap* source, GFX::rect *sourceRect,
		Bitmap *dest, GFX::rect *destRect)
{
	SDL_Rect sdlSourceRect;
	SDL_Rect sdlDestRect;
	SDL_Rect* sdlSourceRectPtr = nullptr;
	SDL_Rect* sdlDestRectPtr = nullptr;
	if (sourceRect != nullptr) {
		GFXRectToSDLRect(sourceRect, &sdlSourceRect);
		sdlSourceRectPtr = &sdlSourceRect;
	}
	if (destRect != nullptr) {
		GFXRectToSDLRect(destRect, &sdlDestRect);
		sdlDestRectPtr = &sdlDestRect;
	}

	int result = SDL_BlitSurface(source->Surface(), sdlSourceRectPtr,
					dest->Surface(), sdlDestRectPtr);

	if (destRect != nullptr)
		SDLRectToGFXRect(&sdlDestRect, destRect);

	return result == 0;
}


/*static*/
bool
GraphicsEngine::BlitBitmapWithMask(const Bitmap* bitmap,
		GFX::rect *source, Bitmap *destBitmap, GFX::rect *dest,
		const Bitmap* mask, GFX::rect *maskRect)
{
	uint32 xStart = 0;
	uint32 yStart = 0;
	if (dest->x < 0) {
		xStart -= dest->x;
		dest->x = 0;
	}
	if (dest->y < 0) {
		yStart -= dest->y;
		dest->y = 0;
	}

	if (!mask->Lock())
		return false;

	uint8* const maskBase = reinterpret_cast<uint8* const>(mask->Pixels());
	const uint32 maskPitch = mask->Pitch();
	const int maskWidth = (int)mask->Width();
	const int maskHeight = (int)mask->Height();

	for (uint32 y = yStart; y < bitmap->Height(); y++) {
		// The mask coordinates (maskRect + iteration offset) can fall outside
		// the mask bitmap, e.g. when an actor is positioned near or beyond the
		// area boundary. Clamp every mask access to [0, maskWidth) x
		// [0, maskHeight) and treat anything outside as fully masked, instead
		// of reading out of bounds (which segfaults).
		const int maskY = maskRect->y + (int)(y - yStart);
		const bool maskRowValid = maskY >= 0 && maskY < maskHeight;
		const uint8* maskPixels = maskRowValid
			? maskBase + (uint32)maskY * maskPitch : nullptr;
		for (uint32 x = xStart; x < bitmap->Width(); x++) {
			SDL_Rect sourceRect = {0, 0, 1, 1};
			SDL_Rect destRect = {0, 0, 1, 1};
			const int maskX = maskRect->x + (int)x;
			const uint8 maskValue =
				(maskRowValid && maskX >= 0 && maskX < maskWidth)
					? maskPixels[maskX] : (uint8)MASK_COMPLETELY;
			if (maskValue != MASK_COMPLETELY) {
				if (maskValue == MASK_SHADE) {
					if (y % 2 != 0 || x % 2 != 0)
						continue;
				}
				sourceRect.x = x;
				sourceRect.y = y;
				destRect.x = x - xStart + dest->x;
				destRect.y = y - yStart + dest->y;

				SDL_BlitSurface(bitmap->Surface(), &sourceRect,
						destBitmap->Surface(), &destRect);
			}
		}
	}
	mask->Unlock();

	// TODO: take into account the return value of the SDL_BlitSurface calls
	return true;
}


/* static */
bool
GraphicsEngine::BlitBitmapScaled(const Bitmap* bitmap, GFX::rect* sourceRect,
							Bitmap* surface, GFX::rect* destRect)
{
	SDL_Rect sdlSourceRect;
	SDL_Rect sdlDestRect;
	SDL_Rect* sdlSourceRectPtr = nullptr;
	SDL_Rect* sdlDestRectPtr = nullptr;
	if (sourceRect != nullptr) {
		GFXRectToSDLRect(sourceRect, &sdlSourceRect);
		sdlSourceRectPtr = &sdlSourceRect;
	}
	if (destRect != nullptr) {
		GFXRectToSDLRect(destRect, &sdlDestRect);
		sdlDestRectPtr = &sdlDestRect;
	}
	return SDL_SoftStretch(bitmap->Surface(), sdlSourceRectPtr,
			surface->Surface(), sdlDestRectPtr) == 0;
}


void
GraphicsEngine::SetVideoMode(uint16 width, uint16 height, uint16 depth,
		uint16 flags)
{
	if (fRenderContext->SDLWindow == nullptr) {
		int SDLWindowFlags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;
		if (flags & VIDEOMODE_FULLSCREEN)
			SDLWindowFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
		if (SDL_CreateWindowAndRenderer(width, height, SDLWindowFlags,
										&fRenderContext->SDLWindow,
										&fRenderContext->SDLRenderer) != 0) {
			throw std::runtime_error("Cannot Create Window");
		}
	}

	if (fRenderContext->SDLTexture != nullptr) {
		SDL_DestroyTexture(fRenderContext->SDLTexture);
		fRenderContext->SDLTexture = nullptr;
	}

	// TODO: add parameter to control this
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");  // make the scaled rendering look smoother.
	SDL_RenderSetLogicalSize(fRenderContext->SDLRenderer, width, height);

	// TODO: We don't respect the depth parameter
	SDL_Surface* surface = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);
	fRenderContext->SDLTexture = SDL_CreateTexture(fRenderContext->SDLRenderer,
						SDL_PIXELFORMAT_RGB888,
						SDL_TEXTUREACCESS_STREAMING,
						width, height);

	if (fScreen != nullptr)
		fScreen->Release();

	fScreen = new Bitmap(surface, true);
	fFlags = flags;

	// Center cursor in window
	SDL_WarpMouseInWindow(fRenderContext->SDLWindow,
						fScreen->Width() / 2, fScreen->Height() / 2);
}


void
GraphicsEngine::SaveCurrentMode()
{
	if (fScreen != nullptr) {
		SDL_Surface* surface = fScreen->Surface();
		fOldRect.x = fOldRect.y = 0;
		fOldRect.w = surface->w;
		fOldRect.h = surface->h;
		fOldDepth = surface->format->BitsPerPixel;
		fOldFlags = fFlags;
	}
}


void
GraphicsEngine::RestorePreviousMode()
{
	if (fOldDepth != 0) {
		SetVideoMode(fOldRect.w, fOldRect.h, fOldDepth, fOldFlags);
		fOldDepth = 0;
	}
}


GFX::rect
GraphicsEngine::ScreenFrame() const
{
	return fScreen->Frame();
}


void
GraphicsEngine::SetRenderingOffset(const GFX::point& point)
{
	fRenderingOffset = point;
}


void
GraphicsEngine::SetWindowCaption(const char* caption)
{
	if (fRenderContext != nullptr && fRenderContext->SDLWindow != nullptr)
		SDL_SetWindowTitle(fRenderContext->SDLWindow, caption);
}


Bitmap*
GraphicsEngine::ScreenBitmap()
{
	return fScreen;
}


void
GraphicsEngine::Update()
{
	SDL_UpdateTexture(fRenderContext->SDLTexture, nullptr,
			fScreen->Surface()->pixels,
			fScreen->Surface()->pitch);
	SDL_RenderClear(fRenderContext->SDLRenderer);
	SDL_Rect rect = { 0, 0, ScreenFrame().w, ScreenFrame().h };
	rect.x = fRenderingOffset.x;
	rect.y = fRenderingOffset.y;
	SDL_RenderCopy(fRenderContext->SDLRenderer, fRenderContext->SDLTexture, nullptr, &rect);
	SDL_RenderPresent(fRenderContext->SDLRenderer);
}


bool
GraphicsEngine::SaveScreenshot(const char* path)
{
	if (SDL_SaveBMP(fScreen->Surface(), path) != 0) {
		std::cerr << "GraphicsEngine::SaveScreenshot(" << path << "): "
			<< SDL_GetError() << std::endl;
		return false;
	}
	return true;
}


void
GraphicsEngine::SetFade(uint16 value)
{
	if (value <= 255)
		SDL_SetTextureColorMod(fRenderContext->SDLTexture, value, value, value);
}
