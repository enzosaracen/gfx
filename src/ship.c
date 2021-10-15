#include "gfx.h"

#define TICKMS	20
#define SBASE	8
#define SBRAT	0.2
#define STRAT	0.2
#define NTRAIL	5
#define TRACD	10
#define STDEC	(SBASE+0.01)/NTRAIL
#define SHT	25
#define BLHT	10
#define BLDUR	1000/TICKMS
#define YAW	0.05
#define ROTSP	1.5
#define BLTSP	10
#define SCD	10
#define NBLT	BLDUR/SCD
#define NASTR	5
#define TASTR	2000/TICKMS
#define ASTRR	30
#define FIZDUR	1000/TICKMS
#define FIZVAR	10
#define FIZDEC	10
#define SHBW	SBASE*0.75
#define SHBH	SHT*0.75
#define VELMAX	5
#define VELINC	0.1
#define SBOX	ASTRR*2

typedef struct Bullet Bullet;
typedef struct Astr Astr;

struct Bullet {
	Obj *o;
	int t;
	double ang;
};

struct Astr {
	int t;
	Obj *o;
};

uint32	w;
Bullet	blt[NBLT];
Astr	astr[NASTR];
Ctx	*astrctx;
Ctx	*bltctx;

struct {
	Obj *o;
	Obj *b;
	Obj *t[NTRAIL];
	Point *cp;
	double ang;
	double vel;
	Ctx *ctx;
} ship;

void yaw(double d)
{
	double r;

	r = d*YAW;
	ship.ang += r;
	rotctx(ship.ctx, ship.cp->x, ship.cp->y, r);
}

void thrust(double d)
{
	double xtr, ytr;

	if(d != 0 && ship.vel+d <= VELMAX && ship.vel+d >= 0)
		ship.vel += d;
	else if(ship.vel+d < 0)
		ship.vel = 0;
	xtr = ship.ctx->xtr + ship.vel*cos(ship.ang);
	ytr = ship.ctx->ytr + ship.vel*-sin(ship.ang);
	trctx(ship.ctx, TRX|TRY|TVX|TVY, 0, xtr, ytr, xtr, ytr);
	trctx(bltctx, TVX|TVY, 0, 0, 0, xtr, ytr);
	trctx(astrctx, TVX|TVY, 0, 0, 0, xtr, ytr);
}

void ginit(void)
{
	int i;

	init();
	ship.ctx = newctx();
	astrctx = newctx();
	bltctx = newctx();
	w = rgb(0xff, 0xff, 0xff);
	srand(time(NULL));

	ship.ang = PI/2;
	ship.cp = newpt(W/2, H/2 - SHT/2);
	settri(ship.o = addobj(ship.ctx, w), W/2-SBASE, H/2, W/2+SBASE, H/2, W/2, H/2-SHT);
	setline(ship.b = addobj(ship.ctx, w), W/2-SBASE, H/2+SBRAT*SHT, W/2+SBASE, H/2+SBRAT*SHT);
	for(i = 0; i < NTRAIL; i++) {
		setline(ship.t[i] = addobj(ship.ctx, w), W/2-SBASE+(i+1)*STDEC, H/2+SBRAT*SHT+STRAT*SHT*(i+1), W/2+SBASE-(i+1)*STDEC, H/2+SBRAT*SHT+STRAT*SHT*(i+1));
		ship.t[i]->hide = 1;
	}
	drawctx(ship.ctx);
	draw(1, ship.ctx);
}

int main(void)
{
	int i, j, k, x, y;
	int lt, ct, et, fps, cev, scd, iblt, nblt, iastr, nastr, tastr, fiz, acl, pacl, itra, tracd;
	Point *ap, *bp;

	ginit();
	et = fps = scd = iblt = nblt = iastr = nastr = tastr = fiz = acl = pacl = itra = tracd = 0;
	for(i = 0;;) {
		lt = SDL_GetTicks();
		input();
		if(++tastr == TASTR) {
			tastr = 0;
			if(iastr != -1) {
				/* dont make astrr too large... */
				do { x = rand()%H; } while(x >= ship.cp->x-SBOX && x <= ship.cp->x+SBOX);
				do { y = rand()%H; } while(y >= ship.cp->y-SBOX && y <= ship.cp->y+SBOX);
				setcirc(astr[iastr].o = addobj(astrctx, w), x, y, ASTRR);
				drawobj(astr[iastr].o);
				trobj(astr[iastr].o, TVX|TVY, 0, 0, 0, astrctx->vbx, astrctx->vby);
				astr[iastr].t = -1;
				iastr = -1;
				nastr++;
				cev = 1;
			}
		}
		for(j = 0; j < NASTR; j++) {
			if(astr[j].o != NULL) {
				if(astr[j].t == -1) {
					ap = astr[j].o->cp;
					if(((ship.cp->y-SHBH/2 >= ap->y-ASTRR && ship.cp->y-SHBH/2 <= ap->y+ASTRR) || (ship.cp->y+SHBH/2 >= ap->y-ASTRR && ship.cp->y+SHBH/2 <= ap->y+ASTRR)) &&
						((ship.cp->x+SHBW/2 >= ap->x-ASTRR && ship.cp->x+SHBW/2 <= ap->x+ASTRR) || (ship.cp->x-SHBW/2 >= ap->x-ASTRR && ship.cp->x-SHBW/2 <= ap->x+ASTRR))) {
						printf("game over\n");
						SDL_Quit();
						return 0;
					}
					for(k = 0; k < NBLT; k++)
						if(blt[k].o != NULL) {
							bp = blt[k].o->p1;
							if(bp->x >= ap->x-ASTRR && bp->x <= ap->x+ASTRR && bp->y <= ap->y+ASTRR && bp->y >= ap->y-ASTRR) {
								astr[j].t = FIZDUR;
								fiz++;
								remobj(blt[k].o);
								blt[k].o = NULL;
								if(iblt == -1)
									iblt = k;
								nblt--;
							}
						}
				} else {
					if(astr[j].t == 0) {
						remobj(astr[j].o);
						astr[j].o = NULL;
						astr[j].t = -1;
						if(iastr == -1)
							iastr = j;
						nastr--;
						fiz--;
						cev = 1;
					} else {
						for(k = 0; k < astr[j].o->pts->n; k++) {
							astr[j].o->pts->x[k] += rand()%FIZVAR - FIZVAR/2;
							astr[j].o->pts->y[k] += rand()%FIZVAR - FIZVAR/2;
							if(rand()%FIZDEC == 0)
								rempts(astr[j].o->pts, k);
						}
						astr[j].t--;
					}
				}
			} else if(iastr == -1)
				iastr = j;
		}
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
			thrust(VELINC);
			acl = cev = 1;
			if(tracd == 0) {
				//rotobj(ship.t[itra], ship.cp->x, ship.cp->y, ship.ang-PI/2);
				ship.t[itra]->hide = 0;
				itra++;
				itra %= NTRAIL;
				tracd = TRACD;
			}
			tracd--;
		}
		if(keyev[SDLK_s].state == KEYDOWN || keyev[SDLK_s].state == KEYHELD) {
			thrust(-VELINC);
			cev = 1;
			acl = -1;
			ship.b->hide = 1;
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
			if(pacl == -1 && acl != -1) {
				ship.b->hide = 0;
			} else if(pacl == 1 && acl != 1) {
				for(j = 0; j < NTRAIL; j++)
					ship.t[j]->hide = 1;
				tracd = 0;
				itra = 0;
			}
			if(acl == 0)
				thrust(0);
			if(cev || nblt || fiz || ship.vel || pacl != 0)
				draw(3, ship.ctx, bltctx, astrctx);
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
		pacl = acl;
		cev = acl = 0;
		if(ct-lt < TICKMS)
			SDL_Delay(TICKMS - (ct-lt));
	}
}
