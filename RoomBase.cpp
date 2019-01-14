#include "Action.h"
#include "Actor.h"
#include "Animation.h"
#include "AreaResource.h"
#include "BackMap.h"
#include "BamResource.h"
#include "BCSResource.h"
#include "BmpResource.h"
#include "Bitmap.h"
#include "Container.h"
#include "Control.h"
#include "Core.h"
#include "CreResource.h"
#include "Door.h"
#include "Game.h"
#include "Graphics.h"
#include "GraphicsEngine.h"
#include "GUI.h"
#include "IDSResource.h"
#include "Label.h"
#include "MOSResource.h"
#include "Party.h"
#include "Polygon.h"
#include "RectUtils.h"
#include "Region.h"
#include "ResManager.h"
#include "RoomBase.h"
#include "Script.h"
#include "TextArea.h"
#include "TileCell.h"
#include "TisResource.h"
#include "Timer.h"
#include "TLKResource.h"
#include "WedResource.h"
#include "WMAPResource.h"

#include <algorithm>
#include <assert.h>
#include <iostream>
#include <stdexcept>

#include <SDL.h>


RoomBase::RoomBase()
	:
	Object(""),
	fDrawSearchMap(0),
	fDrawOverlays(true),
	fDrawPolygons(false),
	fDrawAnimations(true),
	fShowingConsole(false)
{
	fAreaOffset.x = fAreaOffset.y = 0;
}


RoomBase::~RoomBase()
{
}


/* virtual */
IE::rect
RoomBase::Frame() const
{
	return gfx_rect_to_rect(AreaRect());
}


GFX::rect
RoomBase::ViewPort() const
{
	return fScreenArea;
}


void
RoomBase::SetViewPort(GFX::rect rect)
{
	fScreenArea = rect;
}


IE::point
RoomBase::AreaOffset() const
{
	return fAreaOffset;
}


GFX::rect
RoomBase::VisibleArea() const
{
	return fMapArea;
}


void
RoomBase::SetAreaOffset(IE::point point)
{
	GFX::rect areaRect = AreaRect();
	fAreaOffset = point;
	if (fAreaOffset.x < 0)
		fAreaOffset.x = 0;
	else if (fAreaOffset.x + fScreenArea.w > areaRect.w)
		fAreaOffset.x = std::max(areaRect.w - fScreenArea.w, 0);
	if (fAreaOffset.y < 0)
		fAreaOffset.y = 0;
	else if (fAreaOffset.y + fScreenArea.h > areaRect.h)
		fAreaOffset.y = std::max(areaRect.h - fScreenArea.h, 0);

	fMapArea = offset_rect_to(fScreenArea,
			fAreaOffset.x, fAreaOffset.y);
}


void
RoomBase::SetRelativeAreaOffset(IE::point relativePoint)
{
	IE::point newOffset = fAreaOffset;
	newOffset.x += relativePoint.x;
	newOffset.y += relativePoint.y;
	SetAreaOffset(newOffset);
}


void
RoomBase::CenterArea(const IE::point& point)
{
	IE::point destPoint;
	destPoint.x = point.x - fScreenArea.w / 2;
	destPoint.y = point.y - fScreenArea.y / 2;
	SetAreaOffset(destPoint);
}


void
RoomBase::ConvertToArea(GFX::rect& rect)
{
	rect.x += fAreaOffset.x;
	rect.y += fAreaOffset.y;
}


void
RoomBase::ConvertToArea(IE::point& point)
{
	point.x += fAreaOffset.x;
	point.y += fAreaOffset.y;
}


void
RoomBase::ConvertFromArea(GFX::rect& rect)
{
	rect.x -= fAreaOffset.x;
	rect.y -= fAreaOffset.y;
}


void
RoomBase::ConvertFromArea(IE::point& point)
{
	point.x -= fAreaOffset.x;
	point.y -= fAreaOffset.y;
}


void
RoomBase::ConvertToScreen(GFX::rect& rect)
{
	rect.x += fScreenArea.x;
	rect.y += fScreenArea.y;
}


void
RoomBase::ConvertToScreen(IE::point& point)
{
	point.x += fScreenArea.x;
	point.y += fScreenArea.y;
}


/* static */
bool
RoomBase::IsPointPassable(const IE::point& point)
{
	/*uint8 state = RoomContainer::Get()->PointSearch(point);
	switch (state) {
		case 0:
		case 8:
		case 10:
		case 12:
		case 13:
			return false;
		default:
			return true;
	}*/
	return true;
}


void
RoomBase::ToggleOverlays()
{
	fDrawOverlays = !fDrawOverlays;
}


void
RoomBase::TogglePolygons()
{
	fDrawPolygons = !fDrawPolygons;
}


void
RoomBase::ToggleAnimations()
{
	fDrawAnimations = !fDrawAnimations;
}


void
RoomBase::ToggleSearchMap()
{
}


void
RoomBase::ToggleGUI()
{
	GUI* gui = GUI::Get();
	if (gui->IsWindowShown(0))
		gui->HideWindow(0);
	else
		gui->ShowWindow(0);

	if (gui->IsWindowShown(1))
		gui->HideWindow(1);
	else
		gui->ShowWindow(1);

	/*if (gui->IsWindowShown(3))
		gui->HideWindow(3);
	else
		gui->ShowWindow(3);*/
}


void
RoomBase::ToggleDayNight()
{
//	if (fWorldMap != NULL)
//		return;

/*	std::string wedName = fWed->Name();
	if (*wedName.rbegin() == 'N')
		wedName = fArea->Name();
	else
		wedName.append("N");

	std::vector<std::string> list;
	if (gResManager->GetResourceList(list, wedName.c_str(), RES_WED) > 0)
		_InitWed(wedName.c_str());*/
}


/* virtual */
void
RoomBase::VideoAreaChanged(uint16 width, uint16 height)
{
	GFX::rect rect(0, 0, width, height);
	SetViewPort(rect);
}


void
RoomBase::_UpdateCursor(int x, int y, int scrollByX, int scrollByY)
{
	if (scrollByX == 0 && scrollByY == 0) {
		// TODO: Handle other cursors
		GUI::Get()->SetArrowCursor(IE::CURSOR_HAND);
		return;
	}

	int cursorIndex = 0;
	if (scrollByX > 0) {
		if (scrollByY > 0)
			cursorIndex = IE::CURSOR_ARROW_SE;
		else if (scrollByY < 0)
			cursorIndex = IE::CURSOR_ARROW_NE;
		else
			cursorIndex = IE::CURSOR_ARROW_E;
	} else if (scrollByX < 0) {
		if (scrollByY > 0)
			cursorIndex = IE::CURSOR_ARROW_SW;
		else if (scrollByY < 0)
			cursorIndex = IE::CURSOR_ARROW_NW;
		else
			cursorIndex = IE::CURSOR_ARROW_W;
	} else {
		if (scrollByY > 0)
			cursorIndex = IE::CURSOR_ARROW_S;
		else if (scrollByY < 0)
			cursorIndex = IE::CURSOR_ARROW_N;
		else
			cursorIndex = IE::CURSOR_ARROW_E;
	}

	GUI::Get()->SetArrowCursor(cursorIndex);
}
