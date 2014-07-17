#ifndef __SB4SDL_H__
#define __SB4SDL_H__

#include "SDL/SDL.h"
#include <vector>

SDL_Surface *SurfaceFromImage(const class SB4Image *, std::vector<unsigned char> &scratch);
int SB4FlushSDLEvents();

#endif
