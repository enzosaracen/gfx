#include "gfx.h"

SDL_Surface	*scr;
uint8		rast[W*H*4];
double		csf;
int		xtr, ytr;

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
	xtr = 0;
	ytr = 0;
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

	x += xtr;
	y += ytr;
	x = -csf*(W/2-x) + W/2;
	y = -csf*(H/2-y) + H/2;
	if(x >= W || x < 0 || y >= H || y < 0)
		return;
	for(i = 0; i < 4; i++)
		rast[(y*W + x)*4 + i] = col>>i*8 & 0xff;
}

void clrast(void)
{
	memset(rast, 0, W*H*4);
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

void putrect(uint32 col, int x, int y, int w, int h)
{
	int nx, ny;

	nx = x + w;
	ny = y + h;
	putline(col, x, y, x, ny);
	putline(col, x, ny, nx, ny);
	putline(col, nx, ny, nx, y);
	putline(col, nx, y, x, y);
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
	ctx->xtr = ctx->ytr = 0;
	return ctx;
}

void drawctx(Ctx *ctx)
{
	int i;
	Obj *p;

	csf = ctx->scale;
	xtr = ctx->xtr;
	ytr = ctx->ytr;
	for(i = 0; i < NCTX; i++) {
		p = ctx->o[i];
		while(p != NULL) {
			switch(p->type) {
			case OLINE:
				putline(p->col, p->line.x0, p->line.y0, p->line.x1, p->line.y1);
				break;
			case ORECT:
				putrect(p->col, p->rect.x, p->rect.y, p->rect.w, p->rect.h);
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
	xtr = ytr = 0;
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
	double dx, dy;

	o->type = OLINE;
	o->line.x0 = x0;
	o->line.y0 = y0;
	o->line.x1 = x1;
	o->line.y1 = y1;
	dx = x1 - x0;
	dy = -(y1 - y0);
	o->line.ao = fabs(atan(dy/dx));
	o->line.ir = sqrt(dx*dx + dy*dy);
	o->line.dx = dx;
	o->line.dy = dy;
}

void setrect(Obj *o, int x, int y, int w, int h)
{
	o->type = ORECT;
	o->rect.x = x;
	o->rect.y = y;
	o->rect.w = w;
	o->rect.h = h;
}

void setcirc(Obj *o, int cx, int cy, int r)
{
	o->type = OCIRC;
	o->circ.cx = cx;
	o->circ.cy = cy;
	o->circ.r = r;
}

void rotline(Obj *o, double rad)
{
	if(o->type != OLINE)
		errorf("bad type in rotline");
	o->line.x1 = o->line.x0 + o->line.ir*cos(rad+o->line.ao);
	o->line.y1 = o->line.y0 + o->line.ir*-sin(rad+o->line.ao);
}
