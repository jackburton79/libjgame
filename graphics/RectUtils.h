#ifndef __RECTUTILS_H
#define __RECTUTILS_H

#include "Bitmap.h"
#include "GraphicsDefs.h"


static inline GFX::rect
center_rect_in_rect(const GFX::rect& toCenter, const GFX::rect& windowRect)
{
	GFX::rect rect = toCenter;

	rect.x = (windowRect.w - toCenter.x) / 2;
	rect.y = (windowRect.h - toCenter.y) / 2;

	return rect;
}


#endif // __RECTUTILS_H
