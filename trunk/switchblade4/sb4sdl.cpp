#include <stdlib.h>

#include <vector>
#include "sb4sdl.h"
#include "sb4image.h"

#define SURFACEBYTES 0x000000FF, 0x0000FF00, 0x00FF0000, 0x00000000


//SDL_CreateRGBSurfaceFrom(enc->frameHistory1, vidWidth, vidHeight, 32, vidWidth * 4, SURFACEBYTES);

SDL_Surface *SurfaceFromImage(const class SB4Image *img, std::vector<unsigned char> &scratch)
{
	const unsigned char *rows[3];
	const SB4ImagePlane *planes[3];
	unsigned int x, y, w, h, p;
	unsigned char *data;

	w = img->Width();
	h = img->Height();
	scratch.resize(w*h*3);
	data = &scratch[0];

	// Interlace planes
	for(p=0;p<3;p++)
		planes[p] = img->Plane(p);

	for(y=0;y<h;y++)
	{
		for(p=0;p<3;p++)
			rows[p] = planes[p]->Row(y);

		for(x=0;x<w;x++)
		{
			yuv2rgb(*rows[0]++, *rows[1]++, *rows[2]++, data, data+1, data+2);
			data += 3;
		}
	}

	return SDL_CreateRGBSurfaceFrom(&scratch[0], w, h, 24, w*3, SURFACEBYTES);
}

int SB4FlushSDLEvents()
{
	SDL_Event event;

	while(SDL_PollEvent(&event) )
	{
		if(event.type == SDL_QUIT)
			return 1;
	}
	return 0;
}
