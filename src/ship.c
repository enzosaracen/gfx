#include "gfx.h"

#define R	25
#define D	100000
#define I	100
#define NGEN	100
#define SBASE	8
#define SHT	25
#define YAW	0.05
#define TICKMS	5
#define SPEED	2

Obj *gen[NGEN];

struct {
	Obj *base, *lt, *rt;
	Point *cp;
	double r;
} ship;

void yaw(int d)
{
	double r;

	r = d*YAW;
	ship.r += r;
	rot(ship.rt->line.p0, ship.cp, r);
	rot(ship.rt->line.p1, ship.cp, r);
	rot(ship.lt->line.p0, ship.cp, r);
	rot(ship.lt->line.p1, ship.cp, r);
	rot(ship.base->line.p0, ship.cp, r);
	rot(ship.base->line.p1, ship.cp, r);
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

	ship.r = PI/2;
	ship.cp = newpt(W/2, H/2 - SHT/2);
	setline(ship.base = addobj(ctx, w), W/2-SBASE, H/2, W/2+SBASE, H/2);
	setline(ship.lt = addobj(ctx, w), W/2-SBASE, H/2, W/2, H/2 - SHT);
	setline(ship.rt = addobj(ctx, w), W/2+SBASE, H/2, W/2, H/2 - SHT);
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
	while(1) {
		input();
		if(keyev[SDLK_a].state == KEYDOWN || keyev[SDLK_a].state == KEYHELD)
			yaw(1);
		if(keyev[SDLK_d].state == KEYDOWN || keyev[SDLK_d].state == KEYHELD)
			yaw(-1);
		if(keyev[SDLK_w].state == KEYDOWN || keyev[SDLK_w].state == KEYHELD) {
			ctx->xtr += SPEED*cos(ship.r);
			ctx->ytr += SPEED*-sin(ship.r);
		}
		if(keyev[SDLK_s].state == KEYDOWN || keyev[SDLK_s].state == KEYHELD) {
			ctx->xtr -= SPEED*cos(ship.r);
			ctx->ytr -= SPEED*-sin(ship.r);
		}
		drawctx(ctx);
		clrast();
		SDL_Delay(TICKMS);
	}
}
