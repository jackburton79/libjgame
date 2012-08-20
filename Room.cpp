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
#include "Graphics.h"
#include "GraphicsEngine.h"
#include "IDSResource.h"
#include "MOSResource.h"
#include "Polygon.h"
#include "RectUtils.h"
#include "ResManager.h"
#include "Room.h"
#include "Script.h"
#include "TileCell.h"
#include "TisResource.h"
#include "TLKResource.h"
#include "WedResource.h"
#include "WMAPResource.h"

#include <assert.h>
#include <iostream>

std::vector<MapOverlay*> *gOverlays = NULL;

static Room* sCurrentRoom = NULL;

Room::Room()
	:
	Object(""),
	fName(""),
	fWed(NULL),
	fArea(NULL),
	fBcs(NULL),
	fWorldMap(NULL),
	fWorldMapBackground(NULL),
	fWorldMapBitmap(NULL),
	fDrawOverlays(true),
	fDrawPolygons(false),
	fDrawAnimations(false)
{
	sCurrentRoom = this;
}


Room::~Room()
{
	// TODO: Delete various tilecells, overlays, animations
	_UnloadArea();
	_UnloadWorldMap();
}


res_ref
Room::AreaName() const
{
	return fName;
}


GFX::rect
Room::ViewPort() const
{
	return fVisibleArea;
}


bool
Room::LoadArea(const res_ref& areaName)
{
	_UnloadWorldMap();

	fName = areaName;

	std::cout << "Room::Load(" << areaName.CString() << ")" << std::endl;

	fArea = gResManager->GetARA(fName);
	if (fArea == NULL)
		return false;

	fWed = gResManager->GetWED(fName);
	if (fWed == NULL)
		return false;

	fBcs = gResManager->GetBCS(fArea->ScriptName());
	Script* roomScript = NULL;
	if (fBcs != NULL)
		roomScript = fBcs->GetScript();

	_LoadOverlays();
	_InitTileCells();
	_InitVariables();
	_InitAnimations();
	_LoadActors();
	_InitDoors();

	Core::Get()->EnteredArea(this, roomScript);

	delete roomScript;
	return true;
}


bool
Room::LoadArea(AreaEntry& area)
{
	MOSResource* mos = gResManager->GetMOS(area.LoadingScreenName());
	//Bitmap* loadingScreen = mos->Image();

	//GraphicsEngine::Get()->BlitToScreen(loadingScreen, NULL, NULL);
	//SDL_Delay(2000);
	//GraphicsEngine::DeleteBitmap(loadingScreen);

	bool result = LoadArea(area.Name());

	gResManager->ReleaseResource(mos);

	return result;
}


bool
Room::LoadWorldMap()
{
	// TODO: _UnloadActors/Doors() (if needed)
	if (fWorldMap != NULL)
		return true;

	_UnloadArea();

	Core::Get()->EnteredArea(this, NULL);

	fVisibleArea.x = fVisibleArea.y = 0;
	fName = "WORLDMAP";
	fWorldMap = gResManager->GetWMAP(fName);

	worldmap_entry entry = fWorldMap->WorldMapEntry();
	fWorldMapBackground = gResManager->GetMOS(entry.background_mos);
	fWorldMapBitmap = fWorldMapBackground->Image();
	for (uint32 i = 0; i < fWorldMap->CountAreaEntries(); i++) {
		AreaEntry& areaEntry = fWorldMap->AreaEntryAt(i);
		const Frame& iconFrame = areaEntry.Icon();
		IE::point position = areaEntry.Position();
		GFX::rect iconRect = { position.x - iconFrame.rect.w / 2,
					position.y - iconFrame.rect.h / 2,
					iconFrame.rect.w, iconFrame.rect.h };
		GraphicsEngine::Get()->BlitBitmap(iconFrame.bitmap, NULL, fWorldMapBitmap, &iconRect);

	}
	return true;
}


void
Room::SetViewPort(GFX::rect rect)
{
	uint16 areaWidth = 0;
	uint16 areaHeight = 0;

	if (fWed != NULL) {
		areaWidth = fOverlays[0]->Width() * TILE_WIDTH;
		areaHeight = fOverlays[0]->Height() * TILE_HEIGHT;
	} else if (fWorldMapBitmap != NULL ){
		areaWidth = fWorldMapBitmap->Width();
		areaHeight = fWorldMapBitmap->Height();
	}

	if (rect.x < 0)
		rect.x = 0;
	if (rect.y < 0)
		rect.y = 0;
	if (rect.x + rect.w > areaWidth)
		rect.x = areaWidth - rect.w;
	if (rect.y + rect.h > areaHeight)
		rect.y = areaHeight - rect.h;

	rect.x = std::max(rect.x, (sint16)0);
	rect.y = std::max(rect.y, (sint16)0);

	fVisibleArea = rect;
}


GFX::rect
Room::AreaRect() const
{
	GFX::rect rect;
	rect.x = rect.y = 0;
	rect.w = fOverlays[0]->Width() * TILE_WIDTH;
	rect.h = fOverlays[0]->Height() * TILE_HEIGHT;

	return rect;
}


void
Room::Draw(Bitmap *surface)
{
	if (fWorldMap != NULL) {
		GraphicsEngine::Get()->BlitToScreen(fWorldMapBitmap,
				&fVisibleArea, NULL);
		return;
	}
	_DrawBaseMap(fVisibleArea);

	if (fDrawAnimations)
		_DrawAnimations(fVisibleArea);

	if (true)
		_DrawActors(fVisibleArea);

	/*if (fDrawPolygons) {
		for (uint32 p = 0; p < fWed->CountPolygons(); p++) {
			Polygon* poly = fWed->PolygonAt(p);
			if (poly != NULL) {
				if (rects_intersect(offset_rect(poly->Frame(),
						-fVisibleArea.x, -fVisibleArea.y), fVisibleArea)) {
					Graphics::DrawPolygon(*poly, SDL_GetVideoSurface(),
							-fVisibleArea.x, -fVisibleArea.y);
				}
			}
		}
	}*/
}


void
Room::Clicked(uint16 x, uint16 y)
{
	x += fVisibleArea.x;
	y += fVisibleArea.y;

	if (fWorldMap != NULL) {
		res_ref newRoomName;
		for (uint32 i = 0; i < fWorldMap->CountAreaEntries(); i++) {
			AreaEntry& area = fWorldMap->AreaEntryAt(i);
			IE::point point = {x, y};
			if (rect_contains(area.Rect(), point)) {
				LoadArea(area);
				break;
			}
		}
	} else {
		const uint16 tileNum = TileNumberForPoint(x, y);
		fTileCells[tileNum]->Clicked();
	}
}


void
Room::MouseOver(uint16 x, uint16 y)
{
	const uint16 kBorderSize = 15;
	const uint16 kScrollingStep = 30;

	uint16 scrollByX = 0;
	uint16 scrollByY = 0;
	if (x < kBorderSize)
		scrollByX = -kScrollingStep;
	else if (x > fVisibleArea.w - kBorderSize)
		scrollByX = kScrollingStep;

	if (y < kBorderSize)
		scrollByY = -kScrollingStep;
	else if (y > fVisibleArea.h - kBorderSize)
		scrollByY = kScrollingStep;

	x += fVisibleArea.x;
	y += fVisibleArea.y;
	if (fWed != NULL) {
		const uint16 tileNum = TileNumberForPoint(x, y);
		fTileCells[tileNum]->MouseOver();
	} else if (fWorldMap != NULL) {
		for (uint32 i = 0; i < fWorldMap->CountAreaEntries(); i++) {
			AreaEntry& area = fWorldMap->AreaEntryAt(i);
			IE::point point = {x, y};
			if (rect_contains(area.Rect(), point)) {
				GraphicsEngine::Get()->StrokeRect(area.Rect(), 600);
				break;
			}
		}
	}

	GFX::rect rect = { fVisibleArea.x + scrollByX,
					fVisibleArea.y + scrollByY,
					fVisibleArea.w,
					fVisibleArea.h };
	SetViewPort(rect);
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
	// SDL_Flip(SDL_GetVideoSurface());
}


void
Room::TogglePolygons()
{
	fDrawPolygons = !fDrawPolygons;
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
	/*const bool wasDrawingOverlays = fDrawOverlays;

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
	fDrawOverlays = wasDrawingOverlays;*/
}

/* virtual */
void
Room::VideoAreaChanged(uint16 width, uint16 height)
{
	GFX::rect rect = {0, 0, width, height};
	SetViewPort(rect);
}


/* static */
Room*
Room::CurrentArea()
{
	return sCurrentRoom;
}


void
Room::_DrawBaseMap(GFX::rect area)
{
	MapOverlay *overlay = fOverlays[0];
	const uint16 overlayWidth = overlay->Width();
	const uint16 firstTileX = area.x / TILE_WIDTH;
	const uint16 firstTileY = area.y / TILE_HEIGHT;
	uint16 lastTileX = 1 + (area.x + area.w) / TILE_WIDTH;
	uint16 lastTileY = 1 + (area.y + area.h) / TILE_HEIGHT;

	lastTileX = std::min(lastTileX, overlayWidth);
	lastTileY = std::min(lastTileY, overlay->Height());

	GFX::rect tileRect = { 0, 0, TILE_WIDTH, TILE_HEIGHT };
	for (uint16 y = firstTileY; y < lastTileY; y++) {
		tileRect.y = y * TILE_HEIGHT;
		for (uint16 x = firstTileX; x < lastTileX; x++) {
			tileRect.x = x * TILE_WIDTH;
			const uint32 tileNum = y * overlayWidth + x;
			GFX::rect rect = offset_rect(tileRect, -area.x, -area.y);
			fTileCells[tileNum]->Draw(&rect, fDrawOverlays);
		}
	}
}


void
Room::_DrawAnimations(GFX::rect area)
{
	if (fAnimations.size() == 0)
		return;

	for (uint32 i = 0; i < fArea->CountAnimations(); i++) {
		if (fAnimations[i] != NULL) {
			Frame frame = fAnimations[i]->NextFrame();
			IE::point center = fAnimations[i]->Position();
			center = offset_point(center, -frame.rect.w / 2,
					-frame.rect.h / 2);
			Bitmap *animImage = frame.bitmap;
			if (animImage == NULL)
				continue;

			GFX::rect rect = { center.x, center.y,
					animImage->Width(), animImage->Height() };

			rect = offset_rect(rect, -frame.rect.x, -frame.rect.y);
			if (!rects_intersect(area, rect))
				continue;

			rect = offset_rect(rect, -area.x, -area.y);

			GraphicsEngine::Get()->BlitToScreen(animImage, NULL, &rect);
			GraphicsEngine::DeleteBitmap(frame.bitmap);
		}
	}
}


void
Room::_DrawActors(GFX::rect area)
{
	std::vector<Actor*>::iterator a;
	for (a = Actor::List().begin(); a != Actor::List().end(); a++) {
		try {
			(*a)->Draw(area, NULL);
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
		IE::variable var = fArea->VariableAt(n);
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


void
Room::_UnloadArea()
{
	if (fWed == NULL)
		return;

	for (uint32 c = 0; c < fAnimations.size(); c++)
		delete fAnimations[c];
	fAnimations.clear();

	for (uint32 c = 0; c < fTileCells.size(); c++)
		delete fTileCells[c];
	fTileCells.clear();

	for (uint32 c = 0; c < fOverlays.size(); c++)
		delete fOverlays[c];
	fOverlays.clear();

	std::vector<Actor*>::iterator i;

	Actor::List().erase(Actor::List().begin(), Actor::List().end());

	gResManager->ReleaseResource(fWed);
	fWed = NULL;
	gResManager->ReleaseResource(fArea);
	fArea = NULL;
	gResManager->ReleaseResource(fBcs);
	fBcs = NULL;

	gResManager->TryEmptyResourceCache();
}


void
Room::_UnloadWorldMap()
{
	if (fWorldMap == NULL)
		return;

	gResManager->ReleaseResource(fWorldMap);
	fWorldMap = NULL;
	gResManager->ReleaseResource(fWorldMapBackground);
	fWorldMapBackground = NULL;
	GraphicsEngine::DeleteBitmap(fWorldMapBitmap);
	fWorldMapBitmap = NULL;

	gResManager->TryEmptyResourceCache();
}
