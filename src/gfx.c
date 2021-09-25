#include "gfx.h"

SDL_Surface	*scr;
uint8		rast[W*H*4];

void error(char *s)
{
	fprintf(stderr, "%s: %s\n", s, SDL_GetError());
	SDL_Quit();
	exit(1);
}

void put(int x, int y, uint32 col)
{
	int i;

	for(i = 0; i < 4; i++)
		rast[(y*W + x)*4 + i] = col>>i*8 & 0xff;
}

void draw(void)
{
	if(SDL_LockSurface(scr) < 0)
		error("SDL_LockSurface");
	memcpy((uint8*)scr->pixels, rast, W*H*4);
	SDL_UnlockSurface(scr);
	SDL_UpdateRect(scr, 0, 0, 0, 0);
}

void init(void)
{
	int i;
	float z, p;

	if(SDL_Init(SDL_INIT_VIDEO) < 0)
		error("SDL_Init");

	scr = SDL_SetVideoMode(W, H, 32, 0);
	if(scr == NULL)
		error("SDL_SetVideoMode");
}
