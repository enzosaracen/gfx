#include "gfx.h"

#define R	25
#define D	100000
#define I	100
#define NMAP	100
#define SBASE	8
#define SHT	25
#define YAW	0.05
#define TICKMS	10
#define SPEED	4
#define ROTSP	2.25

Obj *map[NMAP];

struct {
	Obj *b, *l, *r;
	Point *c;
	double ang;
} ship;

void yaw(int d)
{
	double r;

	r = d*YAW;
	ship.ang += r;
	rot(ship.r->p0, ship.c, r);
	rot(ship.r->p1, ship.c, r);
	rot(ship.l->p0, ship.c, r);
	rot(ship.l->p1, ship.c, r);
	rot(ship.b->p0, ship.c, r);
	rot(ship.b->p1, ship.c, r);
}

int main(void)
{
	int i, j, m;
	Ctx *shipctx, *mapctx;
	uint32 w;
	Obj *p, *seed;
	
	init();
	mapctx = newctx();
	shipctx = newctx();
	w = rgb(0xff, 0xff, 0xff);
	srand(time(NULL));

	ship.ang = PI/2;
	ship.c = newpt(W/2, H/2 - SHT/2);
	setline(ship.b = addobj(shipctx, w), W/2-SBASE, H/2, W/2+SBASE, H/2);
	setline(ship.l = addobj(shipctx, w), W/2-SBASE, H/2, W/2, H/2 - SHT);
	setline(ship.r = addobj(shipctx, w), W/2+SBASE, H/2, W/2, H/2 - SHT);

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

	mapctx->scale = 0.2;
	drawctx(mapctx);
	drawctx(shipctx);
	/* map drawing slows everything down, figure something out */
	while(1) {
		input();
		if(keyev[SDLK_a].state == KEYDOWN || keyev[SDLK_a].state == KEYHELD) {
			clrast();
			yaw(ROTSP);
			//drawctx(mapctx);
			drawctx(shipctx);
		}
		if(keyev[SDLK_d].state == KEYDOWN || keyev[SDLK_d].state == KEYHELD) {
			clrast();
			yaw(-ROTSP);
			//drawctx(mapctx);
			drawctx(shipctx);
		}
		if(keyev[SDLK_w].state == KEYDOWN || keyev[SDLK_w].state == KEYHELD) {
			shipctx->xtr += SPEED*cos(ship.ang);
			shipctx->ytr += SPEED*-sin(ship.ang);
			//vbx = shipctx->xtr;
			//vby = shipctx->ytr;
			clrast();
			//drawctx(mapctx);
			drawctx(shipctx);
		}
		if(keyev[SDLK_s].state == KEYDOWN || keyev[SDLK_s].state == KEYHELD) {
			shipctx->xtr -= SPEED*cos(ship.ang);
			shipctx->ytr -= SPEED*-sin(ship.ang);
			//vbx = shipctx->xtr;
			//vby = shipctx->ytr;
			clrast();
			//drawctx(mapctx);
			drawctx(shipctx);
		}
		SDL_Delay(TICKMS);
	}
}
