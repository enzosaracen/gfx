#include "gfx.h"

#define R	25
#define D	100000
#define I	100
#define NMAP	100
#define SBASE	8
#define SHT	25
#define YAW	0.05
#define TICKMS	20
#define SPEED	4
#define ROTSP	2.25

Obj *map[NMAP];

struct {
	Obj *o;
	Point *cp;
	double ang;
	Ctx *ctx;
} ship;

void yaw(int d)
{
	double r;

	r = d*YAW;
	ship.ang += r;
	rotctx(ship.ctx, ship.cp->x, ship.cp->y, r);
}

int main(void)
{
	int i, j, m;
	Ctx *mapctx, *ref;
	uint32 w;
	Obj *p, *seed;
	double xtr, ytr;
	int lt, ct, et, fps, cev;
	
	init();
	mapctx = newctx();
	ship.ctx = newctx();
	w = rgb(0xff, 0xff, 0xff);
	srand(time(NULL));

	ship.ang = PI/2;
	ship.cp = newpt(W/2, H/2 - SHT/2);
	settri(ship.o = addobj(ship.ctx, w), W/2-SBASE, H/2, W/2+SBASE, H/2, W/2, H/2-SHT);

	ref = newctx();
	setcirc(addobj(ref, w), W/2, H/2, R);
	setcirc(seed = addobj(mapctx, w), W/2, H/2, R);
	for(i = 0; i < I; i++) {
		m = 0;
		for(j = 0; j < NCTX; j++) {
			p = mapctx->o[j];
			while(p != NULL && m < NMAP-1) {
				if(p->type == OCIRC || p->type == OLINE)
					map[m++] = p;
				p = p->link;
			}
			if(m >= NMAP-1)
				break;
		}
		for(j = 0; j < m; j++)
			setcirc(p = addobj(mapctx, w), map[j]->cp->x + (rand()%D - D/2), map[j]->cp->y + (rand()%D - D/2), R);
	}
	remobj(seed);

	drawctx(mapctx);
	drawctx(ref);
	drawctx(ship.ctx);
	trctx(mapctx, TSF, 0.2, 0, 0, 0, 0);
	draw(3, ship.ctx, mapctx, ref);
	et = fps = 0;
	for(i = 0;;) {
		lt = SDL_GetTicks();
		input();
		if(keyev[SDLK_a].state == KEYDOWN || keyev[SDLK_a].state == KEYHELD) {
			yaw(ROTSP);
			cev = 1;
		}
		if(keyev[SDLK_d].state == KEYDOWN || keyev[SDLK_d].state == KEYHELD) {
			yaw(-ROTSP);
			cev = 1;
		}
		if(keyev[SDLK_w].state == KEYDOWN || keyev[SDLK_w].state == KEYHELD) {
			xtr = ship.ctx->xtr + SPEED*cos(ship.ang);
			ytr = ship.ctx->ytr + SPEED*-sin(ship.ang);
			trctx(ship.ctx, TRX|TRY|TVX|TVY, 0, xtr, ytr, xtr, ytr);
			trctx(ref, TVX|TVY, 0, 0, 0, xtr, ytr);
			trctx(mapctx, TVX|TVY, 0, 0, 0, xtr, ytr);
			cev = 1;
		}
		if(keyev[SDLK_s].state == KEYDOWN || keyev[SDLK_s].state == KEYHELD) {
			xtr = ship.ctx->xtr - SPEED*cos(ship.ang);
			ytr = ship.ctx->ytr - SPEED*-sin(ship.ang);
			trctx(ship.ctx, TRX|TRY|TVX|TVY, 0, xtr, ytr, xtr, ytr);
			trctx(ref, TVX|TVY, 0, 0, 0, xtr, ytr);
			trctx(mapctx, TVX|TVY, 0, 0, 0, xtr, ytr);
			cev = 1;
		}
		if(!et) {
			fps++;
			if(cev) {
				draw(3, ship.ctx, mapctx, ref);
				clrast();
			}
		}
		ct = SDL_GetTicks();
		if(et) {
			et -= TICKMS + (ct-lt);
			if(et < 0)
				et = 0;
		} else if(ct-lt > TICKMS)
			et = ct-lt;
		if(i == 1000/TICKMS - 1) {
			printf("fps: %d\n", fps);
			fps = 0;
		}
		i++;
		i %= 1000/TICKMS;
		cev = 0;
		if(ct-lt < TICKMS)
			SDL_Delay(TICKMS - (ct-lt));
	}
}
