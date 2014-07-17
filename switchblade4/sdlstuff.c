#include <stdlib.h>

#include "SDL/SDL.h"
#include "libsb3/sb3.h"

static int vidWidth;
static int vidHeight;

static int windowWidth;
static int windowHeight;

static SDL_Surface *surf;
static SDL_Surface *feedbackSurfaces[6] = { NULL, NULL, NULL, NULL, NULL, NULL };

static unsigned char *pbar;

int rateGraph[32];
int rateAverages[32];

int graphFrame = 0;


void SDLProgressSetup(int width, int height)
{
	SDL_Init(SDL_INIT_VIDEO);

	memset(rateGraph, 0, sizeof(rateGraph));
	memset(rateAverages, 0, sizeof(rateAverages));

	vidWidth = width;
	vidHeight = height;

	windowWidth = vidWidth;
	windowHeight = vidHeight + 10;

	surf = SDL_SetVideoMode(windowWidth, windowHeight, 32, SDL_SWSURFACE);

	pbar = malloc(10*windowWidth*3);
	if(!surf || !pbar)
		exit(-1);

	memset(pbar, 192, 10*windowWidth*3);
}


#define SURFACEBYTES 0x000000FF, 0x0000FF00, 0x00FF0000, 0x00000000

int SDLProcessStats(sb3_encoder_t *enc, int frameNum, int frameMax, unsigned int bytes)
{
	SDL_Rect rect;
	SDL_Rect rect2;

	SDL_Event event;

	char frameDisplay[4096];

	int fillWidth;
	int i;
	int kbps, kibps, bps;

	fillWidth = frameNum * windowWidth / frameMax;

	for(i=0;i<10;i++)
	{
		memset(pbar + (windowWidth*3*i), 64, fillWidth * 3);
	}

	if(!feedbackSurfaces[0])
	{
		feedbackSurfaces[0] = SDL_CreateRGBSurfaceFrom(enc->frameHistory1, vidWidth, vidHeight, 32, vidWidth * 4, SURFACEBYTES);
		feedbackSurfaces[1] = SDL_CreateRGBSurfaceFrom(pbar, windowWidth, 10, 24, windowWidth * 3, SURFACEBYTES);
	}
	else
		feedbackSurfaces[0]->pixels = enc->frameHistory1;

	rect.x = rect.y = 0;
	rect.w = vidWidth;
	rect.h = vidHeight;

	SDL_BlitSurface(feedbackSurfaces[0], &rect, surf, &rect);

	// Copy the progress bar
	rect.w = rect2.w = windowWidth;
	rect.h = rect2.h = 10;

	rect2.x = 0;
	rect2.y = vidHeight;

	SDL_BlitSurface(feedbackSurfaces[1], &rect, surf, &rect2);

	SDL_UpdateRect(surf, 0, 0, 0, 0);


	bps = (bytes / (frameNum+1)) * 30 * 8;
	kbps = bps / 1000;
	kibps = bps / 1024;

	sprintf(frameDisplay, "avi2roq: Frame %i of %i   (%i kbps %i kibps %i bps)", frameNum + 1, frameMax, kbps, kibps, bps);

	SDL_WM_SetCaption(frameDisplay, frameDisplay);

	while(SDL_PollEvent(&event) )
	{
		if(event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_q && (event.key.keysym.mod & (KMOD_LSHIFT | KMOD_RSHIFT)) )
		{
			return 1;
		}
	}

	return 0;
}

void SDLShutdown()
{
	int i;

	printf("Killing SDL surfaces\n");
	for(i=0;i<4;i++)
		SDL_FreeSurface(feedbackSurfaces[i]);

	printf("Surfaces killed.  Quitting subsystem.\n");
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
	printf("Success.\n");
}
