/*
 * GUI.cpp
 *
 *  Created on: 20/ott/2012
 *      Author: stefano
 */

#include "BackWindow.h"
#include "CHUIResource.h"
#include "Control.h"
#include "GraphicsEngine.h"
#include "GUI.h"
#include "ResManager.h"
#include "Room.h"


#include <algorithm>

GUI gGUI;
static GUI* sGUI = NULL;


GUI::GUI()
	:
	fResource(NULL)
{
	sGUI = &gGUI;
}


GUI::~GUI()
{
	gResManager->ReleaseResource(fResource);
	fActiveWindows.erase(fActiveWindows.begin(), fActiveWindows.end());
	//fActiveWindows.clear();
}


void
GUI::Load(const res_ref& name)
{
	gResManager->ReleaseResource(fResource);
	Clear();
	fResource = gResManager->GetCHUI(name);

	/*for (uint16 c = 0; c < fResource->CountWindows(); c++) {
		Window* window = fResource->GetWindow(c);
		if (window != NULL)
			fWindows.push_back(window);
	}*/
}


void
GUI::Draw()
{
	std::vector<Window*>::const_iterator i;
	for (i = fActiveWindows.begin(); i < fActiveWindows.end(); i++) {
		(*i)->Draw();
	}
}


void
GUI::MouseDown(int16 x, int16 y)
{
	IE::point point = { x, y };
	Window* window = _GetWindow(point);
	if (window != NULL)
		window->MouseDown(point);
	/*else
		Room::CurrentArea()->Clicked(x, y);*/
}


void
GUI::MouseUp(int16 x, int16 y)
{
	IE::point point = { x, y };
	Window* window = _GetWindow(point);
	if (window != NULL)
		window->MouseUp(point);
}


void
GUI::MouseMoved(int16 x, int16 y)
{
	IE::point point = { x, y };
	Window* window = _GetWindow(point);
	if (window != NULL)
		window->MouseMoved(point);
	/*else
		Room::CurrentArea()->MouseOver(x, y);*/
}


void
GUI::ShowWindow(uint16 id)
{
	Window* window = fResource->GetWindow(id);
	if (window != NULL) {
		fActiveWindows.push_back(window);
	}
}


void
GUI::HideWindow(uint16 id)
{
	std::vector<Window*>::iterator i;
	for (i = fActiveWindows.begin(); i != fActiveWindows.end(); i++) {
		if ((*i)->ID() == id) {
			fActiveWindows.erase(i);
			break;
		}
	}
}


bool
GUI::IsWindowShown(uint16 id) const
{
	std::vector<Window*>::const_iterator i;
	for (i = fActiveWindows.begin(); i != fActiveWindows.end(); i++) {
		if ((*i)->ID() == id)
			return true;
	}
	return false;
}


void
GUI::Clear()
{
	fActiveWindows.clear();
	fActiveWindows.erase(fActiveWindows.begin(), fActiveWindows.end());

	_AddBackgroundWindow();
}


Window*
GUI::GetWindow(uint16 id)
{
	std::vector<Window*>::const_iterator i;
	for (i = fActiveWindows.begin(); i != fActiveWindows.end(); i++) {
		if ((*i)->ID() == id)
			return *i;
	}

	return NULL;
}



void
GUI::ControlInvoked(uint32 controlID, uint16 windowID)
{
	Room* room = Room::Get();
	if (!strcmp(fResource->Name(), "GUIWMAP")) {
		switch (windowID) {
			case 0:
				switch (controlID) {
					case 1:
					{
						IE::point point;
						point.x = 0;
						point.y = -20;
						room->SetRelativeAreaOffset(point);
						break;
					}
					case 2:
					{
						IE::point point;
						point.x = 0;
						point.y = 20;
						room->SetRelativeAreaOffset(point);
						break;
					}
					case 8:
					{
						IE::point point;
						point.x = -20;
						point.y = -20;
						room->SetRelativeAreaOffset(point);
						break;
					}
					case 9:
					{
						IE::point point;
						point.x = 20;
						point.y = -20;
						room->SetRelativeAreaOffset(point);
						break;
					}
					case 10:
					{
						IE::point point;
						point.x = -20;
						point.y = 0;
						room->SetRelativeAreaOffset(point);
						break;
					}
					case 12:
					{
						IE::point point;
						point.x = 20;
						point.y = 0;
						room->SetRelativeAreaOffset(point);
						break;
					}
					case 13:
					{
						IE::point point;
						point.x = -20;
						point.y = 20;
						room->SetRelativeAreaOffset(point);
						break;
					}
					case 14:
					{
						IE::point point;
						point.x = 20;
						point.y = 20;
						room->SetRelativeAreaOffset(point);
						break;
					}
					default:
						std::cout << "window " << std::dec << windowID << ",";
						std::cout << "control " << controlID << std::endl;
						break;

				}
				break;
			default:
				std::cout << "window " << std::dec << windowID << ",";
				std::cout << "control " << controlID << std::endl;
				break;
			}
	} else if (!strcmp(fResource->Name(), "GUIMAP")) {
		switch (windowID) {
			case 2:
				switch (controlID) {
					case 1:
						room->LoadWorldMap();
						break;
					default:
						std::cout << "window " << std::dec << windowID << ",";
						std::cout << "control " << controlID << std::endl;
						break;
				}
				break;
			default:
				std::cout << "window " << std::dec << windowID << ",";
				std::cout << "control " << controlID << std::endl;
				break;
		}
	}
}


/* static */
GUI*
GUI::Default()
{
	return sGUI;
}


Window*
GUI::_GetWindow(IE::point pt)
{
	std::vector<Window*>::const_reverse_iterator i;
	for (i = fActiveWindows.rbegin(); i < fActiveWindows.rend(); i++) {
		Window* window = (*i);
		if (pt.x >= window->Position().x
				&& pt.x <= window->Position().x + window->Width()
				&& pt.y >= window->Position().y
				&& pt.y <= window->Position().y + window->Height()) {
			return window;
		}
	}

	return NULL;
}


void
GUI::_AddBackgroundWindow()
{
	BackWindow* backWindow = new BackWindow();
	//fWindows.push_back(backWindow);
	fActiveWindows.push_back(backWindow);
}
