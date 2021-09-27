#include "gfx.h"

#define R	25
#define D	100000
#define I	100
#define NGEN	100

Obj *gen[NGEN];

int main(void)
{
	int i, j, g;
	double sf;
	Ctx *ctx;
	uint32 w;
	Obj *p;

	init();
	ctx = newctx();
	w = rgb(0xff, 0xff, 0xff);
	srand(time(NULL));

	setcirc(addobj(ctx, w), W/2, H/2, R);
	for(i = 0; i < I; i++) {
		g = 0;
		for(j = 0; j < NCTX; j++) {
			p = ctx->o[j];
			while(p != NULL && g < NGEN-1) {
				if(p->type == OCIRC)
					gen[g++] = p;
				p = p->link;
			}
			if(g >= NGEN-1)
				break;
		}
		for(j = 0; j < g; j++) {
			setcirc(p = addobj(ctx, w), gen[j]->circ.cx + (rand()%D - D/2), gen[j]->circ.cy + (rand()%D - D/2), R);
			//setline(addobj(ctx, w), gen[j]->circ.cx, gen[j]->circ.cy, p->circ.cx, p->circ.cy);
		}
	}
	for(sf = 1; sf > 0; sf -= 0.01) {
		ctx->scale = sf;
		drawctx(ctx);
		clrast();
		SDL_Delay(30);
	}
	SDL_Delay(2000);

	SDL_Quit();
}
