#include "Bitmap.h"
#include "Door.h"
#include "GraphicsEngine.h"
#include "ResManager.h"
#include "TileCell.h"
#include "TisResource.h"

#include "WedResource.h" // TODO: Remove once WedOverlay is moved


static SDL_Color sTransparentColor = { 0, 255, 0 };

TileCell::TileCell(uint32 number, MapOverlay** overlays, int numOverlays)
	:
	fNumber(number),
	fDoor(NULL),
	fOverlays(overlays),
	fNumOverlays(numOverlays)
{
}


void
_DrawOverlay(SDL_Surface *surface, SDL_Surface *cell,
		SDL_Rect rect, SDL_Color *color)
{
	if (color) {
		SDL_SetColorKey(cell, SDL_SRCCOLORKEY,
				SDL_MapRGB(cell->format, color->r, color->g, color->b));
	}

	SDL_BlitSurface(cell, NULL, surface, &rect);
}


void
TileCell::Draw(SDL_Surface *surface, SDL_Rect *rect, bool full)
{
	int maxOverlay = full ? fNumOverlays : 1;

	for (int i = maxOverlay - 1; i >= 0; i--) {
		// Check if this overlay needs to be drawn
	    if (i != 0 && (fOverlays[0]->TileMapForTileCell(fNumber)->Mask() & (1 << i)) == 0)
	    	continue;
	    MapOverlay *overlay = fOverlays[i];
		TileMap *map = overlay->TileMapForTileCell(fNumber);
		if (map == NULL)
			continue;

		int16 index = map->TileIndex();
		if (fDoor != NULL && !fDoor->Opened()) {
			int16 secondaryIndex = map->SecondaryTileIndex();
			if (secondaryIndex != -1)
				index = secondaryIndex;
			else
				printf("TileCell::Draw(): secondary index is -1. BUG?.\n");
		}

		TISResource *tis = gResManager->GetTIS(overlay->TileSet());
		Bitmap *cell = tis->TileAt(index);
		if (cell == NULL) {
			printf("NULL cell. BAD.\n");
			// TODO: Fix this. Shouldn't request an invalid cell
			cell = GraphicsEngine::CreateBitmap(64, 64, 8);
			//SDL_FillRect(cell, NULL, 3000);
		}
		gResManager->ReleaseResource(tis);
		SDL_Color *color = NULL;
		if (i == 0 && fOverlays[0]->TileMapForTileCell(fNumber)->Mask() != 0) {
			color = &sTransparentColor;
			//color = &cell->format->palette->colors[255];
		}

		_DrawOverlay(surface, cell->Surface(), *rect, color);

		GraphicsEngine::DeleteBitmap(cell);
	}
}

/*
void
TileCell::SetTileMap(TileMap *map, int overlayNum)
{
	fTileMap[overlayNum] = map;
}*/


Door *
TileCell::Door() const
{
	return fDoor;
}


void
TileCell::SetDoor(::Door *d)
{
	fDoor = d;
}


void
TileCell::Clicked()
{
	if (fDoor != NULL)
		fDoor->Toggle();
}


void
TileCell::MouseOver()
{

}


// TileMap
TileMap::TileMap()
	:
	fSecondaryIndex(-1)
{
}


void
TileMap::AddTileIndex(int16 index)
{
	fIndices.push_back(index);
}


int16
TileMap::TileIndex()
{
	int16 index = fIndices[fCurrentIndex];
	if (++fCurrentIndex >= fIndices.size())
		fCurrentIndex = 0;
	return index;
}


void
TileMap::SetSecondaryTileIndex(int16 index)
{
	fSecondaryIndex = index;
}


int16
TileMap::SecondaryTileIndex() const
{
	return fSecondaryIndex;
}


void
TileMap::SetMask(uint8 mask)
{
	fMask = mask;
}


uint8
TileMap::Mask() const
{
	return fMask;
}
