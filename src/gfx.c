#include "gfx.h"

SDL_Surface	*scr;
uint8		rast[W*H*4];
double		csf;
int		xtr, ytr;
int		nkeys;
uint8		*prevks;
int		*keys;

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
	SDL_GetKeyState(&nkeys);
	keys = emalloc(nkeys*sizeof(*keys))
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

void addlist(Obj *o, Point *p)
{
	o->list.a[o->list.n].x = p->x;
	o->list.a[o->list.n++].y = p->y;
	if(o->list.n >= NLIST)
		errorf("list too large");
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

Point *newpt(double x, double y)
{
	Point *p;

	p = emalloc(sizeof(Point));
	p->x = x;
	p->y = y;
	return p;
}

void drawctx(Ctx *ctx)
{
	int i, j;
	Obj *p;

	csf = ctx->scale;
	xtr = ctx->xtr;
	ytr = ctx->ytr;
	for(i = 0; i < NCTX; i++) {
		p = ctx->o[i];
		while(p != NULL) {
			switch(p->type) {
			case OLINE:
				putline(p->col, p->line.p0->x, p->line.p0->y, p->line.p1->x, p->line.p1->y);
				break;
			case ORECT:
				putrect(p->col, p->rect.p->x, p->rect.p->y, p->rect.w, p->rect.h);
				break;
			case OCIRC:
				putcirc(p->col, p->circ.p->x, p->circ.p->y, p->circ.r);
				break;
			case OLIST:
				for(j = 0; j < p->list.n; j++)
					put(p->col, p->list.a[j].x, p->list.a[j].y);
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

void setline(Obj *o, double x0, double y0, double x1, double y1)
{
	o->type = OLINE;
	o->line.p0 = newpt(x0, y0);
	o->line.p1 = newpt(x1, y1);
}

void setrect(Obj *o, double x, double y, int w, int h)
{
	o->type = ORECT;
	o->rect.p = newpt(x, y);
	o->rect.w = w;
	o->rect.h = h;
}

void setcirc(Obj *o, double cx, double cy, int r)
{
	o->type = OCIRC;
	o->circ.p = newpt(cx, cy);
	o->circ.r = r;
}

void setlist(Obj *o)
{
	o->type = OLIST;
	o->list.n = 0;
	o->list.a = emalloc(NLIST*sizeof(*o->list.a));
}

void rot(Point *p, Point *c, double rad)
{
	double rx, ry;

	rx = p->x - c->x;
	ry = c->y - p->y;
	p->x = rx*cos(rad) - ry*sin(rad) + c->x;
	p->y = -(rx*sin(rad) + ry*cos(rad)) + c->y;
}

void input(void)
{
	int i;
	SDL_Event ev;
	uint8 *ks;

	while(SDL_PollEvent(&ev))
		switch(ev.type) {
		case SDL_QUIT:
			SQL_Quit();
			exit(0);
		case SDL_KEYDOWN:
			keys[]
		}
	
}
