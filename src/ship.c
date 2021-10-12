#include "gfx.h"

#define NMAP	100
#define TICKMS	20
#define SBASE	8
#define SHT	25
#define BLHT	10
#define BLDUR	1000/TICKMS
#define YAW	0.05
#define SPEED	4
#define ROTSP	2.25
#define BLTSP	10
#define SCD	10
#define NBLT	BLDUR/SCD

typedef struct Bullet Bullet;

struct Bullet {
	Obj *o;
	int t;
	double ang;
};

uint32	w;
Bullet	blt[NBLT];
Ctx	*bltctx;

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
	rotobj(ship.o, ship.cp->x, ship.cp->y, r);
}

void thrust(int d)
{
	double xtr, ytr;

	xtr = ship.ctx->xtr + d*cos(ship.ang);
	ytr = ship.ctx->ytr + d*-sin(ship.ang);
	trctx(ship.ctx, TRX|TRY|TVX|TVY, 0, xtr, ytr, xtr, ytr);
	trctx(bltctx, TVX|TVY, 0, 0, 0, xtr, ytr);
}

void ginit(void)
{
	init();
	ship.ctx = newctx();
	bltctx = newctx();
	w = rgb(0xff, 0xff, 0xff);
	srand(time(NULL));

	ship.ang = PI/2;
	ship.cp = newpt(W/2, H/2 - SHT/2);
	settri(ship.o = addobj(ship.ctx, w), W/2-SBASE, H/2, W/2+SBASE, H/2, W/2, H/2-SHT);

	drawctx(ship.ctx);
	draw(1, ship.ctx);
}

int main(void)
{
	int i, j;
	int lt, ct, et, fps, cev, scd, iblt, nblt;

	ginit();
	et = fps = scd = iblt = nblt = 0;
	for(i = 0;;) {
		lt = SDL_GetTicks();
		input();
		if(scd > 0)
			scd--;
		for(j = 0; j < NBLT; j++) {
			if(blt[j].o != NULL) {
				if(++blt[j].t >= BLDUR) {
					remobj(blt[j].o);
					blt[j].o = NULL;
					if(iblt == -1)
						iblt = j;
					nblt--;
				} else
					trobj(blt[j].o, TRX|TRY, 0, BLTSP*cos(blt[j].ang), BLTSP*-sin(blt[j].ang), 0, 0);
			} else if(iblt == -1)
				iblt = j;
		}
		if(keyev[SDLK_a].state == KEYDOWN || keyev[SDLK_a].state == KEYHELD) {
			yaw(ROTSP);
			cev = 1;
		}
		if(keyev[SDLK_d].state == KEYDOWN || keyev[SDLK_d].state == KEYHELD) {
			yaw(-ROTSP);
			cev = 1;
		}
		if(keyev[SDLK_w].state == KEYDOWN || keyev[SDLK_w].state == KEYHELD) {
			thrust(SPEED);
			cev = 1;
		}
		if(keyev[SDLK_s].state == KEYDOWN || keyev[SDLK_s].state == KEYHELD) {
			thrust(-SPEED);
			cev = 1;
		}
		if(scd == 0 && (keyev[SDLK_SPACE].state == KEYDOWN || keyev[SDLK_SPACE].state == KEYHELD)) {
			if(iblt == -1)
				errorf("bullet count shouldn't be greater than NBLT");
			blt[iblt].ang = ship.ang;
			blt[iblt].t = 0;
			setline(blt[iblt].o = addobj(bltctx, w), ship.cp->x, ship.cp->y, ship.cp->x, ship.cp->y-SHT/2);
			rotp(blt[iblt].o->p1, blt[iblt].o->p0, blt[iblt].ang-PI/2);
			drawobj(blt[iblt].o);
			iblt = -1;
			nblt++;
			scd = SCD;
			cev = 1;
		}
		if(!et) {
			fps++;
			if(cev || nblt > 0)
				draw(2, ship.ctx, bltctx);
			clrast();
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
