#include "BmpResource.h"
#include "Graphics.h"
#include "Polygon.h"
#include "ResManager.h"
#include "Stream.h"
#include "TileCell.h"
#include "TisResource.h"
#include "WedResource.h"
#include "Utils.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>

#include <SDL.h>

using namespace std;

struct overlay {
	int16 width;
	int16 height;
	res_ref resource_ref;
	uint32 unk;
	uint32 tilemap_offset;
	uint32 tile_lookup_offset;
};


struct tilemap {
	int16 primary_tile_index;
	int16 primary_tile_count;
	int16 secondary_tile_index;
	int8 mask;
	int8 unk1;
	int8 unk2;
	int8 unk3;
};


WEDResource::WEDResource(uint8 *data, uint32 size, uint32 key)
	:
	Resource(data, size, key),
	fNumPolygons(0),
	fPolygons(NULL),
	fSearchMap(NULL),
	fHeightMap(NULL),
	fOverlays(NULL)
{
	fType = RES_WED;
	_Load();
}


WEDResource::WEDResource(const res_ref &name)
	:
	Resource(),
	fNumPolygons(0),
	fPolygons(NULL),
	fOverlays(NULL)
{
	fType = RES_WED;
}


WEDResource::~WEDResource()
{
	delete[] fPolygons;
	delete[] fOverlays;
}


void
WEDResource::_Load()
{
	char signature[5];
	ReadAt(0, signature, 4);
	signature[4] = '\0';
	if (strcmp(signature, "WED ")) {
		printf("WEDResource::_Load(): invalid signature %s\n",
				signature);
		throw -1;
	}
	char version[5];
	ReadAt(4, version, 4);
	version[4] = '\0';
	if (strcmp(version, "V1.3")) {
		printf("WEDResource::_Load(): invalid version %s\n",
				version);
		throw -1;
	}
	
	ReadAt(8, fNumOverlays);
	ReadAt(12, fNumDoors);
	ReadAt(16, fOverlaysOffset);
	ReadAt(20, f2ndHeaderOffset);

	_LoadPolygons();
	_LoadOverlays();
}


bool
WEDResource::Load(TArchive *archive, uint32 key)
{
	if (!Resource::Load(archive, key))
		return false;

	_Load();
	return true;
}


void
WEDResource::_DrawOverlay(SDL_Rect rect, SDL_Surface *surface,
		SDL_Surface *cell)
{
	SDL_BlitSurface(cell, NULL, surface, &rect);
}


void
WEDResource::_DrawTile(const int16 tileNum, SDL_Surface *surface,
		SDL_Rect tileRect, bool withOverlays)
{
    tilemap tileMap = OverlayAt(0)->TileMapFor(tileNum);
    int maxOverlay = withOverlays ? fNumOverlays : 1;
    // Draw the overlays from top to bottom:
    // this way we can draw the base overlay using
    // the correct colorkey, and the overlays (water, lava, etc)
    // are rendered correctly
    for (int i = maxOverlay - 1; i >= 0; i--) {
    	// Check if this overlay needs to be drawn
    	if (i != 0 && (tileMap.mask & (1 << i)) == 0)
    		continue;

    	MapOverlay *nextOverlay = OverlayAt(i);
   		if (nextOverlay == NULL)
   			break;

   		if (i != 0) {
   			printf("overlay %d mask: ", i);
   			int mask = nextOverlay->TileMapFor(tileNum).mask;
   			for (int z = 0; z < 7; z++) {
   				if ((mask & (1 << z)) != 0)
   					printf("%d", z);
   			}
   			printf("\n");
   		}
   		const int32 index = nextOverlay->TileIndexAt(tileNum);
   		TISResource *tis = gResManager->GetTIS(nextOverlay->TileSet());

   		SDL_Surface *cell = tis->TileCellAt(index);
   		if (i == 0) {
   			// Green is the colorkey
   			// TODO: only for BG, it seems
   			if (tileMap.mask != 0) {
   				uint32 color = SDL_MapRGB(cell->format, 0, 0, 255);
   				SDL_SetColorKey(cell, SDL_SRCCOLORKEY, color);
   			}
   			SDL_BlitSurface(cell, NULL, surface, &tileRect);
   		} else
   			_DrawOverlay(tileRect, surface, cell);

   		SDL_FreeSurface(cell);

   		gResManager->ReleaseResource(tis);
   	}
}


SDL_Surface *
WEDResource::GetAreaMap(bool withOverlays)
{
	// TODO: Change this, and create a function which
	// only draw the requested tile. Then the caller
	// needs to request the tiles it wants to draw.
	int32 numOverlays = CountOverlays();
	if (numOverlays <= 0)
		return NULL;

	MapOverlay *baseOverlay = OverlayAt(0);
	if (baseOverlay == NULL)
		return NULL;

	SDL_Surface *surface = SDL_CreateRGBSurface(
			SDL_SWSURFACE, baseOverlay->Width() * TILE_WIDTH,
			baseOverlay->Height() * TILE_HEIGHT, 16, 0, 0, 0, 0);

	SDL_Rect tileRect = { 0, 0, TILE_WIDTH, TILE_HEIGHT };
	const int16 width = baseOverlay->Width();
	const int16 height = baseOverlay->Height();
	for (int16 y = 0; y < height; y++) {
		tileRect.y = y * TILE_HEIGHT;
		for (int16 x = 0; x < width; x++) {
			tileRect.x = x * TILE_WIDTH;
			const int16 tileNum = y * width + x;
			_DrawTile(tileNum, surface, tileRect, withOverlays);
		}
	}

	/*for (int32 p = 0; p < CountPolygons(); p++) {
		Graphics::DrawPolygon(*PolygonAt(p), surface);
	}*/

	return surface;
}


int32
WEDResource::CountOverlays() const
{
	return fNumOverlays;
}


MapOverlay *
WEDResource::OverlayAt(int32 index)
{
	if (index < 0 || index >= fNumOverlays)
		return NULL;

	return &fOverlays[index];
}


Polygon *
WEDResource::PolygonAt(int32 index)
{
	return &fPolygons[index];
}


int32
WEDResource::CountPolygons() const
{
	return fNumPolygons;
}


void
WEDResource::_LoadOverlays()
{
	assert(fOverlays == NULL);

	fOverlays = new MapOverlay[fNumOverlays];
	for (int index = 0; index < fNumOverlays; index++) {
		Seek(fOverlaysOffset + index * sizeof(overlay), SEEK_SET);

		::overlay overlay;
		Read(overlay);
		MapOverlay &current = fOverlays[index];
		current.fTileSet = overlay.resource_ref;
		current.fWidth = overlay.width;
		current.fHeight = overlay.height;
		current.fTileMaps = new tilemap[overlay.width * overlay.height];

		int32 overlaySize = overlay.height * overlay.width;
		for (int32 x = 0; x < overlaySize; x++) {
			int32 offset = overlay.tilemap_offset + x * sizeof(tilemap);
			ReadAt(offset, current.fTileMaps[x]);
		}

		for (int32 x = 0; x < overlaySize; x++) {
			const int32 indexCount = current.fTileMaps[x].primary_tile_count;
			const int32 offset = overlay.tile_lookup_offset
					+ (current.fTileMaps[x].primary_tile_index) * sizeof(int16);

			int16 tisIndex;
			ReadAt(offset, tisIndex);
			for (int c = 0; c < indexCount; c++)
				current.fTilesIndexes[x + c] = tisIndex;
			x += indexCount - 1;
		}
	}
}


void
WEDResource::_LoadPolygons()
{
	assert(fPolygons == NULL);

	Seek(f2ndHeaderOffset, SEEK_SET);

	Read(fNumPolygons);

	fPolygons = new Polygon[fNumPolygons];

	int32 polygonsOffset;
	Read(polygonsOffset);

	int32 verticesOffset;
	int32 wallGroups;
	int32 indexTableOffset;

	Read(verticesOffset);
	Read(wallGroups);
	Read(indexTableOffset);

	for (int32 p = 0; p < fNumPolygons; p++) {
		Seek(polygonsOffset + p * sizeof(polygon), SEEK_SET);
		::polygon polygon;
		Read(polygon);
		fPolygons[p].SetFrame(polygon.x_min, polygon.x_max,
				polygon.y_min, polygon.y_max);
		Seek(polygon.vertex_index * sizeof(point) + verticesOffset, SEEK_SET);
		for (int i = 0; i < polygon.vertices_count; i++) {
			point vertex;
			Read(vertex);
			fPolygons[p].AddPoints(&vertex, 1);
		}
	}
}


/* static */
bool
WEDResource::_IsOverlayColor(const SDL_Color &color)
{
	return color.r == 0 and color.g == 0 and color.b == 255;
}


SDL_Color
WEDResource::_PixelSearchColor(int16 x, int16 y)
{
	/*const int32 searchMapX = x / fHAspect;
	const int32 searchMapY = y / fVAspect;
	const int32 pitch = fSearchMap->pitch;
	SDL_LockSurface(fSearchMap);
	const SDL_Color *colors = fSearchMap->format->palette->colors;
	uint8 *pixels = (Uint8*)(fSearchMap->pixels);
	const int32 pixelIndex = searchMapY * pitch + searchMapX;
	const SDL_Color color = colors[pixels[pixelIndex]];
	SDL_UnlockSurface(fSearchMap);

	//printf("pixel: %d\n", pixels[pixelIndex]);

	return color;*/
	SDL_Color color;
	return color;
}


int16
WEDResource::_PointHeight(int16 x, int16 y)
{
	/*const int32 mapX = x / fHAspect;
	const int32 mapY = y / fVAspect;
	const int32 pitch = fHeightMap->pitch;
	SDL_LockSurface(fHeightMap);
	const SDL_Color *colors = fHeightMap->format->palette->colors;
	uint8 *pixels = (Uint8*)(fHeightMap->pixels);
	const int32 pixelIndex = mapY * pitch + mapX;
	const SDL_Color color = colors[pixels[pixelIndex]];
	SDL_UnlockSurface(fHeightMap);

	//uint32 pix = SDL_MapRGB(color.r, color.g, color.b);
	if (color.r == 128 and color.g == 128 and color.b == 128)
		return 0;
*/
	return 1;
}


// MapOverlay
MapOverlay::MapOverlay()
	:
	fTileMaps(NULL)
{
}


MapOverlay::~MapOverlay()
{
	delete[] fTileMaps;
}


res_ref
MapOverlay::TileSet() const
{
	return fTileSet;
}


int16
MapOverlay::Width() const
{
	return fWidth;
}


int16
MapOverlay::Height() const
{
	return fHeight;
}


tilemap
MapOverlay::TileMapFor(int32 i)
{
	return fTileMaps[i];
}


int16
MapOverlay::TileIndexAt(int16 i)
{
	return fTilesIndexes[i];
}


void
MapOverlay::PrintTileMaps()
{
	for (int32 x = 0; x < fHeight * fWidth; x++) {
		printf("tilemap[%d]:\n", x);
		printf("\tp_index: %d\n", fTileMaps[x].primary_tile_index);
		printf("\tp_count: %d\n", fTileMaps[x].primary_tile_count);
		printf("\ts_index: %d\n", fTileMaps[x].secondary_tile_index);
		printf("\tmask: %d\n", fTileMaps[x].mask);
	}
}


