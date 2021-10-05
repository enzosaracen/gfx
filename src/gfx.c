#include "gfx.h"

SDL_Surface	*scr;
uint8		rast[W*H*4];
double		csf;
double		xtr, ytr, ex, ey;
int		nkeys;

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
	xtr = ytr = ex = ey = 0;
	vbx = vby = 0;
	SDL_GetKeyState(&nkeys);
	keyev = emalloc(nkeys*sizeof(*keyev));
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
	ex += xtr - (int)xtr;
	y += ytr;
	ey += ytr - (int)ytr;
	x -= vbx;
	y -= vby;
	while(ex >= 1) {
		x++;
		ex--;
	}
	while(ey >= 1) {
		y++;
		ey--;
	}
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
	o->a[o->n].x = p->x;
	o->a[o->n++].y = p->y;
	if(o->n >= NLIST)
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
	ctx->xtr = ctx->ytr = ctx->ex = ctx->ey = 0;
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
	Obj *o;

	csf = ctx->scale;
	xtr = ctx->xtr;
	ytr = ctx->ytr;
	ex = ctx->ex;
	ey = ctx->ey;
	for(i = 0; i < NCTX; i++) {
		o = ctx->o[i];
		while(o != NULL) {
			switch(o->type) {
			case OLINE:
				putline(o->col, o->p0->x, o->p0->y, o->p1->x, o->p1->y);
				break;
			case ORECT:
				putrect(o->col, o->rp->x, o->rp->y, o->w, o->h);
				break;
			case OCIRC:
				putcirc(o->col, o->cp->x, o->cp->y, o->r);
				break;
			case OLIST:
				for(j = 0; j < o->n; j++)
					put(o->col, o->a[j].x, o->a[j].y);
				break;
			default:
				errorf("obj type unset");
			}
			o = o->link;
		}
	}
	draw();
	csf = 1;
	ctx->ex = ex;
	ctx->ey = ey;
	xtr = ytr = ex = ey = 0;
}

Obj *addobj(Ctx *ctx, uint32 col)
{
	Obj *o, *d;

	o = emalloc(sizeof(Obj));
	o->ctx = ctx;
	o->id = ctx->cid++;
	o->col = col;
	o->link = NULL;
	o->back = NULL;

	if((d = ctx->o[o->id % NCTX]) != NULL) {
		d->back = o;
		o->link = d;
	}
	ctx->o[o->id % NCTX] = o;
	return o;
}

void remobj(Obj *o)
{
	o->back->link = o->link;
	switch(o->type) {
	case OLINE:
		free(o->p0);
		free(o->p1);
		break;
	case ORECT:
		free(o->rp);
		break;
	case OCIRC:
		free(o->cp);
		break;
	case OLIST:
		free(o->a);
		break;
	}
	free(o);
}

void setline(Obj *o, double x0, double y0, double x1, double y1)
{
	o->type = OLINE;
	o->p0 = newpt(x0, y0);
	o->p1 = newpt(x1, y1);
}

void setrect(Obj *o, double x, double y, int w, int h)
{
	o->type = ORECT;
	o->rp = newpt(x, y);
	o->w = w;
	o->h = h;
}

void setcirc(Obj *o, double cx, double cy, int r)
{
	o->type = OCIRC;
	o->cp = newpt(cx, cy);
	o->r = r;
}

void setlist(Obj *o)
{
	o->type = OLIST;
	o->n = 0;
	o->a = emalloc(NLIST*sizeof(*o->a));
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

	for(i = 0; i < nkeys; i++) {
		if(keyev[i].state == KEYUP)
			keyev[i].state = KEYINACTIVE;
		if(keyev[i].state == KEYDOWN)
			keyev[i].state = KEYHELD;
		keyev[i].ndown = keyev[i].nup = 0;
	}
	while(SDL_PollEvent(&ev))
		switch(ev.type) {
		case SDL_QUIT:
			SDL_Quit();
			exit(0);
			break;
		case SDL_KEYDOWN:
			keyev[ev.key.keysym.sym].state = KEYDOWN;
			keyev[ev.key.keysym.sym].ndown++;
			break;
		case SDL_KEYUP:
			keyev[ev.key.keysym.sym].state = KEYUP;
			keyev[ev.key.keysym.sym].nup++;
			break;
		}
}
