#include "gfx.h"

SDL_Surface	*scr;
uint8		rast[W*H*4];
double		csf;

void sdlerror(char *s)
{
	fprintf(stderr, "%s: %s\n", s, SDL_GetError());
	SDL_Quit();
	exit(1);
}

void errorf(char *fmt, ...)
{
	va_list arg;

	fprintf(stderr, "error: ");
	va_start(arg, fmt);
	vfprintf(stderr, fmt, arg);
	va_end(arg);
	fprintf(stderr, "\n");
	SDL_Quit();
	exit(1);
}

void *emalloc(size_t n)
{
	void *p;

	p = malloc(n);
	if(p == NULL)
		errorf("out of memory");
	return p;
}

void init(void)
{
	if(SDL_Init(SDL_INIT_VIDEO) < 0)
		sdlerror("SDL_Init");

	scr = SDL_SetVideoMode(W, H, 32, 0);
	if(scr == NULL)
		sdlerror("SDL_SetVideoMode");
	SDL_WM_SetCaption("gfx", "");
	csf = 1;
}

void draw(void)
{
	if(SDL_LockSurface(scr) < 0)
		sdlerror("SDL_LockSurface");
	memcpy((uint8*)scr->pixels, rast, W*H*4);
	SDL_UnlockSurface(scr);
	SDL_UpdateRect(scr, 0, 0, 0, 0);
}

void put(uint32 col, int x, int y)
{
	int i;

	x *= csf;
	y *= csf;
	if(x >= W || x < 0 || y >= H || y < 0)
		return;
	for(i = 0; i < 4; i++)
		rast[(y*W + x)*4 + i] = col>>i*8 & 0xff;
}

uint32 rgb(uint8 r, uint8 g, uint8 b)
{
	return SDL_MapRGB(scr->format, r, g, b);
}

void putline(uint32 col, int x0, int y0, int x1, int y1)
{
	int dx, dy, xi, yi, e, e2;

	dx = abs(x1 - x0);
	dy = -abs(y1 - y0);
	xi = x0 < x1 ? 1 : -1;
	yi = y0 < y1 ? 1 : -1;
	e = dx + dy;
	while(1) {
		put(col, x0, y0);
		if(x0 == x1 && y0 == y1)
			break;
		e2 = 2*e;
		if(e2 >= dy) {
			e += dy;
			x0 += xi;
		}
		if(e2 <= dx) {
			e += dx;
			y0 += yi;
		}
	}
}

void putrect(uint32 col, int tx, int ty, int bx, int by)
{
	putline(col, tx, ty, tx, by);
	putline(col, tx, by, bx, by);
	putline(col, bx, by, bx, ty);
	putline(col, bx, ty, tx, ty);
}

void putcirc(uint32 col, int cx, int cy, int r)
{
	int x, y, r2;

	r2 = r*r;
	put(col, cx, cy+r);
	put(col, cx, cy-r);
	put(col, cx+r, cy);
	put(col, cx-r, cy);

	x = 1;
	y = sqrt(r2 - 1) + 0.5;
	while(x < y) {
		put(col, cx+x, cy+y);
		put(col, cx+x, cy-y);
		put(col, cx-x, cy+y);
		put(col, cx-x, cy-y);
		put(col, cx+y, cy+x);
		put(col, cx+y, cy-x);
		put(col, cx-y, cy+x);
		put(col, cx-y, cy-x);
		x++;
		y = sqrt(r2 - x*x) + 0.5;
	}
	if(x == y) {
		put(col, cx+x, cy+y);
		put(col, cx+x, cy-y);
		put(col, cx-x, cy+y);
		put(col, cx-x, cy-y);
	}
}

Ctx *newctx(void)
{
	int i;
	Ctx *ctx;

	ctx = emalloc(sizeof(Ctx));
	ctx->o = emalloc(NCTX*sizeof(Obj*));
	for(i = 0; i < NCTX; i++)
		ctx->o[i] = NULL;
	ctx->cid = 0;
	ctx->scale = 1;
	return ctx;
}

void drawctx(Ctx *ctx)
{
	int i;
	Obj *p;

	csf = ctx->scale;
	for(i = 0; i < NCTX; i++) {
		p = ctx->o[i];
		while(p != NULL) {
			switch(p->type) {
			case OLINE:
				putline(p->col, p->line.x0, p->line.y0, p->line.x1, p->line.y1);
				break;
			case ORECT:
				putrect(p->col, p->rect.tx, p->rect.ty, p->rect.bx, p->rect.by);
				break;
			case OCIRC:
				putcirc(p->col, p->circ.cx, p->circ.cy, p->circ.r);
				break;
			default:
				errorf("obj type unset");
			}
			p = p->link;
		}
	}
	draw();
	csf = 1;
}

Obj *addobj(Ctx *ctx, uint32 col)
{
	Obj *o, *d;

	o = emalloc(sizeof(Obj));
	o->ctx = ctx;
	o->id = ctx->cid++;
	o->col = col;
	o->link = NULL;

	if((d = ctx->o[o->id % NCTX]) != NULL)
		o->link = d;
	ctx->o[o->id % NCTX] = o;
	return o;
}

void setline(Obj *o, int x0, int y0, int x1, int y1)
{
	o->type = OLINE;
	o->line.x0 = x0;
	o->line.y0 = y0;
	o->line.x1 = x1;
	o->line.y1 = y1;
}

void setrect(Obj *o, int tx, int ty, int bx, int by)
{
	o->type = ORECT;
	o->rect.tx = tx;
	o->rect.ty = ty;
	o->rect.bx = bx;
	o->rect.by = by;
}

void setcirc(Obj *o, int cx, int cy, int r)
{
	o->type = OCIRC;
	o->circ.cx = cx;
	o->circ.cy = cy;
	o->circ.r = r;
}

void ctxtrs(Ctx *ctx, int ox, int oy)
{
	int i;
	Obj *p;

	for(i = 0; i < NCTX; i++) {
		p = ctx->o[i];
		while(p != NULL) {
			switch(p->type) {
			case OLINE:
				p->line.x0 += ox;
				p->line.x1 += ox;
				p->line.y0 += oy;
				p->line.y1 += oy;
				break;
			case ORECT:
				p->rect.tx += ox;
				p->rect.bx += ox;
				p->rect.ty += oy;
				p->rect.by += oy;
				break;
			case OCIRC:
				p->circ.cx += ox;
				p->circ.cy += oy;
				break;
			default:
				errorf("obj type unset");
			}
			p = p->link;
		}
	}
}

void clrast(void)
{
	memset(rast, 0, W*H*4);
}
