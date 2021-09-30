#include "gfx.h"

#define PI	M_PI
#define R	25
#define D	100000
#define I	100
#define NGEN	100
#define SBASE	8
#define SHT	25

Obj *gen[NGEN];

int main(void)
{
	int i, j, g;
	double r;
	Ctx *ctx;
	uint32 w;
	Obj *p;
	struct {
		Obj *base;
		Obj *lt, *rt;
	} ship;


	init();
	ctx = newctx();
	w = rgb(0xff, 0xff, 0xff);
	srand(time(NULL));

	/* add type for object groups, maybe a property of obj that functions can check or something, use variadic function maybe */
	//setline(ship.base = addobj(ctx, w), W/2-SBASE, H/2, W/2+SBASE, H/2);
	//setline(ship.lt = addobj(ctx, w), W/2-SBASE, H/2, W/2, H/2 - SHT);
	setline(ship.rt = addobj(ctx, w), W/2+SBASE, H/2, W/2, H/2 - SHT);
	drawctx(ctx);
	SDL_Delay(1000);
	for(i = 0; i < I; i++) {
		g = 0;
		for(j = 0; j < NCTX; j++) {
			p = ctx->o[j];
			while(p != NULL && g < NGEN-1) {
				if(p->type == OCIRC || p->type == OLINE)
					gen[g++] = p;
				p = p->link;
			}
			if(g >= NGEN-1)
				break;
		}
		for(j = 0; j < g; j++)
			setcirc(p = addobj(ctx, w), gen[j]->circ.cx + (rand()%D - D/2), gen[j]->circ.cy + (rand()%D - D/2), R);
	}
	for(r = 0; r <= 2*PI; r += 0.01) {
		rotline(ship.rt, r);
		//rotline(ship.lt, r);
		//rotline(ship.base, r);
		drawctx(ctx);
		clrast();
		SDL_Delay(5);
	}
	SDL_Delay(2000);
	SDL_Quit();
}
