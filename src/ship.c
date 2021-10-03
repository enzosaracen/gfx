#include "gfx.h"

#define R	25
#define D	100000
#define I	100
#define NGEN	100
#define SBASE	8
#define SHT	25
#define YAW	0.05

Obj *gen[NGEN];

struct {
	Obj *base, *lt, *rt;
} ship;

void yaw(int d)
{
	Point *c;
	double r;

	r = d*YAW;
	c = ship.base->line.p0;
	rot(ship.rt->line.p0, c, r);
	rot(ship.rt->line.p1, c, r);
	rot(ship.lt->line.p1, c, r);
	rot(ship.base->line.p1, c, r);
}

int main(void)
{
	int i, j, g;
	Ctx *ctx;
	uint32 w;
	Obj *p;
	
	init();
	ctx = newctx();
	w = rgb(0xff, 0xff, 0xff);
	srand(time(NULL));

	setline(ship.base = addobj(ctx, w), W/2-SBASE, H/2, W/2+SBASE, H/2);
	setline(ship.lt = addobj(ctx, w), W/2-SBASE, H/2, W/2, H/2 - SHT);
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
			setcirc(p = addobj(ctx, w), gen[j]->circ.p->x + (rand()%D - D/2), gen[j]->circ.p->y + (rand()%D - D/2), R);
	}
	for(i = 0; i < 100; i++) {
		yaw(1);
		drawctx(ctx);
		clrast();
		SDL_Delay(5);
	}
	for(i = 0; i < 100; i++) {
		yaw(-1);
		drawctx(ctx);
		clrast();
		SDL_Delay(5);
	}
	SDL_Delay(2000);
	SDL_Quit();
}
