/*
 * TextArea.cpp
 *
 *  Created on: 20/ott/2012
 *      Author: stefano
 */

#include "BamResource.h"
#include "Bitmap.h"
#include "GraphicsEngine.h"
#include "ResManager.h"
#include "Utils.h"
#include "TextArea.h"
#include "TextSupport.h"
#include "TLKResource.h"
#include "Utils.h"
#include "Window.h"


TextArea::TextLine::TextLine()
	:
	width(0),
	height(0)
{
}


TextArea::TextArea(IE::text_area* text)
	:
	Control(text),
	fBitmap(NULL),
	fChanged(true)
{
	fBitmap = new Bitmap(text->w, text->h, 8);

	GFX::Color foreground = { text->color1_r, text->color1_g, text->color1_b, text->color1_a };
	GFX::Color transparent = { text->color2_r, text->color2_g, text->color2_b, text->color2_a };
	GFX::Color background = { text->color3_r, text->color3_g, text->color3_b, text->color3_a };

	//GFX::Color transparent = palette.colors[0];
	GFX::Palette palette(foreground, background);
	fBitmap->SetPalette(palette);
	//fBitmap->SetColorKey(transparent.r, transparent.g, transparent.b, true);
}


TextArea::~TextArea()
{
	if (fBitmap != NULL)
		fBitmap->Release();
}


/* virtual */
void
TextArea::Draw()
{
	GFX::rect destRect(fControl->x, fControl->y, fControl->w, fControl->h);
	fWindow->ConvertToScreen(destRect);

	if (fChanged) {
		GFX::rect rect(0, 0, fBitmap->Width(), fBitmap->Height());
		std::string fontName = ((IE::text_area*)fControl)->font_bam.CString();
		fBitmap->Clear(0);
		uint32 flags = IE::LABEL_JUSTIFY_LEFT;
		std::vector<TextLine*>::const_iterator i;
		for (i = fLines.begin(); i != fLines.end(); i++) {
			const TextLine* line = *i;
			GFX::point where = { rect.x, rect.y };
			FontRoster::GetFont(fontName)->RenderString(line->text, flags, fBitmap, false, where);
			rect.y += line->height + 5;
		}
		fChanged = false;
	}
	GraphicsEngine::Get()->BlitToScreen(fBitmap, NULL, &destRect);
}


void
TextArea::AddText(const char* text)
{
	std::string fontName = ((IE::text_area*)fControl)->font_bam.CString();
	const Font* font = FontRoster::GetFont(fontName);

	std::string textString(text);
	while (!textString.empty()) {
		std::string textLine = font->TruncateString(textString, fControl->w);
		TextLine* newLine = new TextLine();
		newLine->text = textLine;
		newLine->width = font->StringWidth(textLine, &newLine->height);
		fLines.push_back(newLine);
	}
	fChanged = true;
}


void
TextArea::ClearText()
{
	for (std::vector<TextLine*>::const_iterator i = fLines.begin();
			i != fLines.end(); i++) {
		delete *i;
	}
	fLines.clear();
	fChanged = true;
}
