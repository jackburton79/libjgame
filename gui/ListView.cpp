/*
 * ListView.cpp
 *
 *  Created on: 13/set/2017
 *      Author: stefano
 */

#include "Bitmap.h"
#include "GraphicsEngine.h"
#include "ListView.h"
#include "ResManager.h"
#include "TextSupport.h"
#include "Window.h"

ListView::ListView(IE::control* control)
	:
	Control(control)
{
	std::cout << "ListView" << std::endl;
	fFontResource = gResManager->GetBAM("TOOLFONT");
	//fBitmap = new Bitmap(fControl->w, fControl->h, 8);
	fBitmap = new Bitmap(control->w, control->h, 16);
}


ListView::~ListView()
{
	fBitmap->Release();
}


void
ListView::Draw()
{
	GFX::rect itemRect = { (sint16)fControl->x, (sint16)fControl->y,
					50, 20 };
	for (StringList::iterator i = fList.begin();
			i != fList.end(); i++) {
		TextSupport::RenderString(*i, fFontResource, 0, fBitmap, itemRect);
		itemRect.y += 20;
	}

	GFX::rect destRect(fControl->x, fControl->y, fControl->w, fControl->h);
	fWindow->ConvertToScreen(destRect);
	GraphicsEngine::Get()->BlitToScreen(fBitmap, NULL, &destRect);
}


void
ListView::AttachedToWindow(Window* window)
{
	Control::AttachedToWindow(window);
}


void
ListView::MouseMoved(IE::point point, uint32 transit)
{
}


void
ListView::MouseDown(IE::point point)
{
}


void
ListView::MouseUp(IE::point point)
{
}


void
ListView::AddItem(std::string string)
{
	fList.push_back(string);
}
