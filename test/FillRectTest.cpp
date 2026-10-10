// FillRect used to truncate the colour to 8 bits, which only works on 8 bit surfaces.
#include "Bitmap.h"

#include <SDL.h>

#include <cstdio>
#include <cstdlib>

int
main()
{
	Bitmap* bitmap = new Bitmap(8, 8, 32);
	bitmap->Clear(0);
	const uint32 color = bitmap->MapRGBColor(200, 100, 50);
	bitmap->FillRect(GFX::rect(2, 2, 3, 3), color);

	int failures = 0;
	if (bitmap->GetPixel(3, 3) != color) {
		std::printf("FAIL: pixel inside the rect is %08x, expected %08x\n",
			bitmap->GetPixel(3, 3), color);
		failures++;
	}
	if (bitmap->GetPixel(0, 0) != 0 || bitmap->GetPixel(5, 5) != 0) {
		std::printf("FAIL: FillRect painted outside its rect\n");
		failures++;
	}
	bitmap->Release();

	if (failures == 0)
		std::printf("FillRectTest: all good\n");
	return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
