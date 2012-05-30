#include "Actor.h"
#include "Animation.h"
#include "AreaResource.h"
#include "BamResource.h"
#include "BCSResource.h"
#include "BmpResource.h"
#include "Bitmap.h"
#include "Core.h"
#include "CreResource.h"
#include "Door.h"
#include "GraphicsEngine.h"
#include "IDSResource.h"
#include "RectUtils.h"
#include "ResManager.h"
#include "Room.h"
#include "Script.h"
#include "TileCell.h"
#include "TisResource.h"
#include "TLKResource.h"
#include "WedResource.h"

#include <assert.h>
#include <iostream>

std::vector<MapOverlay*> *gOverlays = NULL;

Room::Room()
	:
	fWed(NULL),
	fArea(NULL),
	fBcs(NULL),
	fLightMap(NULL),
	fSearchMap(NULL),
	fHeightMap(NULL),
	fDrawOverlays(false),
	fDrawPolygons(false),
	fDrawLightMap(false),
	fDrawSearchMap(false),
	fDrawHeightMap(false),
	fDrawAnimations(false)
{
}


Room::~Room()
{
	SDL_FreeSurface(fLightMap);
	SDL_FreeSurface(fSearchMap);
	SDL_FreeSurface(fHeightMap);

	// TODO: Delete various tilecells, overlays, etc.
	gResManager->ReleaseResource(fWed);
	gResManager->ReleaseResource(fArea);
	gResManager->ReleaseResource(fBcs);
}


SDL_Rect
Room::ViewPort() const
{
	return fVisibleArea;
}


bool
Room::Load(const char *resName)
{
	std::cout << "Room::Load(" << resName << ")" << std::endl;

	fArea = gResManager->GetARA(resName);
	fName = fArea->WedName();
	fWed = gResManager->GetWED(fName);

	fBcs = gResManager->GetBCS(fArea->ScriptName());
	if (fBcs != NULL)
		Core::Get()->AddScript(fBcs->GetScript());

	_LoadOverlays();
	_InitTileCells();

	BMPResource *bmp = gResManager->GetBMP(
			ResourceManager::HeightMapName(fName).c_str());
	fHeightMap = bmp->Image();
	gResManager->ReleaseResource(bmp);

	bmp = gResManager->GetBMP(
			ResourceManager::LightMapName(fName).c_str());
	fLightMap = bmp->Image();

	gResManager->ReleaseResource(bmp);

	bmp = gResManager->GetBMP(
			ResourceManager::SearchMapName(fName).c_str());
	fSearchMap = bmp->Image();
	gResManager->ReleaseResource(bmp);

	_InitVariables();
	_InitAnimations();
	_LoadActors();
	_InitDoors();

	return true;
}


void
Room::SetViewPort(SDL_Rect rect)
{
	const uint16 areaWidth = fOverlays[0]->Width() * TILE_WIDTH;
	const uint16 areaHeigth = fOverlays[0]->Height() * TILE_HEIGHT;

	if (rect.x < 0)
		rect.x = 0;
	if (rect.y < 0)
		rect.y = 0;
	if (rect.x + rect.w > areaWidth)
		rect.x = areaWidth - rect.w;
	if (rect.y + rect.h > areaHeigth)
		rect.y = areaHeigth - rect.h;

	rect.x = std::max(rect.x, (Sint16)0);
	rect.y = std::max(rect.y, (Sint16)0);

	fVisibleArea = rect;
}


void
Room::Draw(SDL_Surface *surface)
{
	// TODO: Check scripts

	_DrawBaseMap(surface, fVisibleArea);

	if (fDrawAnimations)
		_DrawAnimations(surface, fVisibleArea);

	if (true)
		_DrawActors(surface, fVisibleArea);

	if (fDrawLightMap)
		_DrawLightMap(surface);
	if (fDrawHeightMap)
		_DrawHeightMap(surface, fVisibleArea);
	if (fDrawSearchMap)
		_DrawSearchMap(surface, fVisibleArea);

	if (fDrawPolygons) {
		// TODO:
	}

	SDL_Flip(surface);
}


void
Room::Clicked(uint16 x, uint16 y)
{
	x += fVisibleArea.x;
	y += fVisibleArea.y;

	const uint16 tileNum = TileNumberForPoint(x, y);
	fTileCells[tileNum]->Clicked();
}


void
Room::MouseOver(uint16 x, uint16 y)
{
	x += fVisibleArea.x;
	y += fVisibleArea.y;

	const uint16 tileNum = TileNumberForPoint(x, y);
	fTileCells[tileNum]->MouseOver();
}


uint16
Room::TileNumberForPoint(uint16 x, uint16 y)
{
	const uint16 overlayWidth = fOverlays[0]->Width();
	const uint16 tileX = x / TILE_WIDTH;
	const uint16 tileY = y / TILE_HEIGHT;

	return tileY * overlayWidth + tileX;
}


void
Room::ToggleOverlays()
{
	fDrawOverlays = !fDrawOverlays;
	SDL_Flip(SDL_GetVideoSurface());
}


void
Room::TogglePolygons()
{
	fDrawPolygons = !fDrawPolygons;
}


void
Room::ToggleLightMap()
{
	fDrawLightMap = !fDrawLightMap;
}


void
Room::ToggleSearchMap()
{
	fDrawSearchMap = !fDrawSearchMap;
}


void
Room::ToggleHeightMap()
{
	fDrawHeightMap = !fDrawHeightMap;
}


void
Room::ToggleAnimations()
{
	fDrawAnimations = !fDrawAnimations;
}


void
Room::DumpOverlays(const char* path)
{
	// TODO: Code duplication with _DrawBaseMap().
	// Make it safe to be called from here.
	const bool wasDrawingOverlays = fDrawOverlays;

	for (uint32 overlayNum = 0; overlayNum < fOverlays.size(); overlayNum++) {
		MapOverlay *overlay = fOverlays[overlayNum];
		if (overlay->Width() == 0 || overlay->Height() == 0)
			continue;

		MapOverlay** overlays = &overlay;

		char fileName[32];
		snprintf(fileName, 32, "overlay%d.bmp", overlayNum);
		TPath destPath(path, fileName);
		SDL_Surface *surface = SDL_CreateRGBSurface(SDL_SWSURFACE,
				overlay->Width() * TILE_WIDTH, overlay->Height() * TILE_HEIGHT, 24, 0, 0, 0, 0);

		SDL_Rect area;
		area.x = area.y = 0;
		area.w = surface->w;
		area.h = surface->h;

		const uint16 firstTileX = area.x / TILE_WIDTH;
		const uint16 firstTileY = area.y / TILE_HEIGHT;
		uint16 lastTileX = 1 + (area.x + area.w) / TILE_WIDTH;
		uint16 lastTileY = 1 + (area.y + area.h) / TILE_HEIGHT;

		lastTileX = std::min(lastTileX, overlay->Width());
		lastTileY = std::min(lastTileY, overlay->Height());

		SDL_Rect tileRect = { 0, 0, TILE_WIDTH, TILE_HEIGHT };
		for (uint16 y = firstTileY; y < lastTileY; y++) {
			tileRect.y = y * TILE_HEIGHT;
			for (uint16 x = firstTileX; x < lastTileX; x++) {
				tileRect.x = x * TILE_WIDTH;
				const uint32 tileNum = y * overlay->Width() + x;
				SDL_Rect rect = offset_rect(tileRect, -area.x, -area.y);
				TileCell cell(tileNum, overlays, 1);
				cell.Draw(surface, &rect, false);
			}
		}
		SDL_SaveBMP(surface, destPath.Path());
		SDL_FreeSurface(surface);
	}
	fDrawOverlays = wasDrawingOverlays;
}


void
Room::_DrawBaseMap(SDL_Surface *surface, SDL_Rect area)
{
	MapOverlay *overlay = fOverlays[0];
	const uint16 overlayWidth = overlay->Width();
	const uint16 firstTileX = area.x / TILE_WIDTH;
	const uint16 firstTileY = area.y / TILE_HEIGHT;
	uint16 lastTileX = 1 + (area.x + area.w) / TILE_WIDTH;
	uint16 lastTileY = 1 + (area.y + area.h) / TILE_HEIGHT;

	lastTileX = std::min(lastTileX, overlayWidth);
	lastTileY = std::min(lastTileY, overlay->Height());

	SDL_Rect tileRect = { 0, 0, TILE_WIDTH, TILE_HEIGHT };
	for (uint16 y = firstTileY; y < lastTileY; y++) {
		tileRect.y = y * TILE_HEIGHT;
		for (uint16 x = firstTileX; x < lastTileX; x++) {
			tileRect.x = x * TILE_WIDTH;
			const uint32 tileNum = y * overlayWidth + x;
			SDL_Rect rect = offset_rect(tileRect, -area.x, -area.y);
			fTileCells[tileNum]->Draw(surface, &rect, fDrawOverlays);
		}
	}
}


void
Room::_DrawLightMap(SDL_Surface *surface)
{
	SDL_Rect destRect = {
		0, surface->h - fLightMap->h,
		fLightMap->w, fLightMap->h
	};
	SDL_BlitSurface(fLightMap, NULL, surface, &destRect);
}


void
Room::_DrawSearchMap(SDL_Surface *surface, SDL_Rect area)
{
	SDL_Rect destRect = {
		fLightMap->w, surface->h - fSearchMap->h,
		fSearchMap->w + fLightMap->w, fSearchMap->h
	};
	SDL_BlitSurface(fSearchMap, NULL, surface, &destRect);
}


void
Room::_DrawHeightMap(SDL_Surface *surface, SDL_Rect area)
{
	SDL_Rect destRect = {
		fLightMap->w + fSearchMap->w, surface->h - fHeightMap->h,
		fSearchMap->w + fLightMap->w + fHeightMap->w, fHeightMap->h
	};
	SDL_BlitSurface(fHeightMap, NULL, surface, &destRect);

	/*MapOverlay *overlay = fWed->OverlayAt(0);
	SDL_Rect fill = {
		destRect.x + (area.x / (overlay->Width() / fHeightMap->w)),
		destRect.y + (area.y / (overlay->Height() / fHeightMap->h)),
		(area.w / (overlay->Width() / fHeightMap->w)),
		(area.h / (overlay->Height() / fHeightMap->h))
	};
	SDL_FillRect(surface, &fill, 45);*/
}


void
Room::_DrawAnimations(SDL_Surface *surface, SDL_Rect area)
{
	if (fAnimations.size() == 0)
		return;

	for (uint32 i = 0; i < fArea->CountAnimations(); i++) {
		if (fAnimations[i] != NULL) {
			Frame frame = fAnimations[i]->NextFrame();
			point center = fAnimations[i]->fCenter;
			center = offset_point(center, -frame.rect.w / 2,
					-frame.rect.h / 2);
			SDL_Surface *animImage = frame.bitmap->Surface();
			if (animImage == NULL)
				continue;

			SDL_Rect rect = { center.x, center.y,
					animImage->w, animImage->h };

			rect = offset_rect(rect, -frame.rect.x, -frame.rect.y);

			if (!rects_intersect(area, rect))
				continue;

			rect = offset_rect(rect, -area.x, -area.y);

			SDL_BlitSurface(animImage, NULL, surface, &rect);

			GraphicsEngine::DeleteBitmap(frame.bitmap);
		}
	}
}


void
Room::_DrawActors(SDL_Surface *surface, SDL_Rect area)
{
	std::vector<Actor*>::iterator a;
	for (a = Actor::List().begin(); a != Actor::List().end(); a++) {
		try {
			(*a)->Draw(surface, area);
		} catch (...) {
			continue;
		}
	}
}


void
Room::_LoadOverlays()
{
	uint32 numOverlays = fWed->CountOverlays();
	for (uint32 i = 0; i < numOverlays; i++) {
		MapOverlay *overlay = fWed->GetOverlay(i);
		fOverlays.push_back(overlay);
	}
}


void
Room::_InitTileCells()
{
	uint32 numTiles = fOverlays[0]->Size();
	for (uint16 i = 0; i < numTiles; i++) {
		fTileCells.push_back(new TileCell(i, fOverlays.data(), fOverlays.size()));
	}
}


void
Room::_InitVariables()
{
	uint32 numVars = fArea->CountVariables();
	for (uint32 n = 0; n < numVars; n++) {
		variable var = fArea->VariableAt(n);
		Core::Get()->SetVariable(var.name, var.value);
	}
}


void
Room::_InitAnimations()
{
	for (uint32 i = 0; i < fArea->CountAnimations(); i++)
		fAnimations.push_back(new Animation(fArea->AnimationAt(i)));
}


void
Room::_LoadActors()
{
	for (uint16 i = 0; i < fArea->CountActors(); i++) {
		Actor::Add(new Actor(*fArea->ActorAt(i)));
	}
}


void
Room::_InitDoors()
{
	assert(fTileCells.size() > 0);

	uint32 numDoors = fWed->CountDoors();
	for (uint32 c = 0; c < numDoors; c++) {
		Door *door = new Door(fArea->DoorAt(c));
		fWed->GetDoorTiles(door, c);
		Door::Add(door);

		for (uint32 i = 0; i < door->fTilesOpen.size(); i++) {
			fTileCells[door->fTilesOpen[i]]->SetDoor(door);
		}
	}
}
