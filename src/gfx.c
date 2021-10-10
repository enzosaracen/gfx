#include "gfx.h"

int		nkeys;
SDL_Surface	*scr;
uint8		rast[W*H*4];

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

void *erealloc(void *p, size_t n)
{
	p = realloc(p, n);
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
	SDL_GetKeyState(&nkeys);
	keyev = emalloc(nkeys*sizeof(*keyev));
}

void draw(int n, ...)
{
	int i, j, k;
	va_list arg;
	Ctx *ctx;

	va_start(arg, n);
	for(i = 0; i < n; i++) {
		ctx = va_arg(arg, Ctx *);
		for(j = 0; j < ctx->pts->n; j++) {
			if(ctx->pts->y[j] >= H || ctx->pts->y[j] < 0 || ctx->pts->x[j] >= W || ctx->pts->x[j] < 0)
				continue;
			for(k = 0; k < 4; k++)
				rast[((int)ctx->pts->y[j]*W + (int)ctx->pts->x[j])*4 + k] = 0xffffff>>i*8 & 0xff;
		}
	}
	if(SDL_LockSurface(scr) < 0)
		sdlerror("SDL_LockSurface");
	memcpy((uint8*)scr->pixels, rast, W*H*4);
	SDL_UnlockSurface(scr);
	SDL_UpdateRect(scr, 0, 0, 0, 0);
}

void put(Ctx *ctx, uint32 col, int x, int y)
{
	addpts(ctx->pts, x, y);
}

void addpts(Pvec *p, int x, int y)
{
	if(p->n >= p->max-1) {
		p->max += VECINC;
		p->x = erealloc(p->x, p->max*sizeof(double));
		p->y = erealloc(p->y, p->max*sizeof(double));
	}
	p->x[p->n] = x;
	p->y[p->n++] = y;
}

void clrast(void)
{
	memset(rast, 0, W*H*4);
}

uint32 rgb(uint8 r, uint8 g, uint8 b)
{
	return SDL_MapRGB(scr->format, r, g, b);
}

void putline(Ctx *ctx, uint32 col, int x0, int y0, int x1, int y1)
{
	int dx, dy, xi, yi, e, e2;

	dx = abs(x1 - x0);
	dy = -abs(y1 - y0);
	xi = x0 < x1 ? 1 : -1;
	yi = y0 < y1 ? 1 : -1;
	e = dx + dy;
	while(1) {
		put(ctx, col, x0, y0);
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

void putrect(Ctx *ctx, uint32 col, int x, int y, int w, int h)
{
	int nx, ny;

	nx = x + w;
	ny = y + h;
	putline(ctx, col, x, y, x, ny);
	putline(ctx, col, x, ny, nx, ny);
	putline(ctx, col, nx, ny, nx, y);
	putline(ctx, col, nx, y, x, y);
}

void putcirc(Ctx *ctx, uint32 col, int cx, int cy, int r)
{
	int x, y, r2;

	r2 = r*r;
	put(ctx, col, cx, cy+r);
	put(ctx, col, cx, cy-r);
	put(ctx, col, cx+r, cy);
	put(ctx, col, cx-r, cy);

	x = 1;
	y = sqrt(r2 - 1) + 0.5;
	while(x < y) {
		put(ctx, col, cx+x, cy+y);
		put(ctx, col, cx+x, cy-y);
		put(ctx, col, cx-x, cy+y);
		put(ctx, col, cx-x, cy-y);
		put(ctx, col, cx+y, cy+x);
		put(ctx, col, cx+y, cy-x);
		put(ctx, col, cx-y, cy+x);
		put(ctx, col, cx-y, cy-x);
		x++;
		y = sqrt(r2 - x*x) + 0.5;
	}
	if(x == y) {
		put(ctx, col, cx+x, cy+y);
		put(ctx, col, cx+x, cy-y);
		put(ctx, col, cx-x, cy+y);
		put(ctx, col, cx-x, cy-y);
	}
}

void puttri(Ctx *ctx, uint32 col, int x0, int y0, int x1, int y1, int x2, int y2) {
	putline(ctx, col, x0, y0, x1, y1);
	putline(ctx, col, x1, y1, x2, y2);
	putline(ctx, col, x2, y2, x0, y0);
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

	ctx = calloc(sizeof(Ctx), 1);
	ctx->o = emalloc(NCTX*sizeof(Obj*));
	for(i = 0; i < NCTX; i++)
		ctx->o[i] = NULL;
	ctx->scale = 1;
	ctx->pts = emalloc(sizeof(Pvec));
	ctx->pts->n = 0;
	ctx->pts->max = VECINC;
	ctx->pts->x = emalloc(VECINC*sizeof(double));
	ctx->pts->y = emalloc(VECINC*sizeof(double));
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

void trctx(Ctx *ctx, int type, double sf, double xtr, double ytr, double vbx, double vby)
{
	int i;

	if(type & TSF)
		ctx->scale = sf/ctx->scale;
	if(type & TRX)
		ctx->xtr = xtr-ctx->xtr;
	if(type & TRY)
		ctx->ytr = ytr-ctx->ytr;
	if(type & TVX)
		ctx->vbx = vbx-ctx->vbx;
	if(type & TVY)
		ctx->vby = vby-ctx->vby;
	for(i = 0; i < ctx->pts->n; i++) {
		if(type & TSF) {
			ctx->pts->x[i] = -ctx->scale*(W/2-ctx->pts->x[i]) + W/2;
			ctx->pts->y[i] = -ctx->scale*(H/2-ctx->pts->y[i]) + H/2;
		}
		if(type & TRX)
			ctx->pts->x[i] += ctx->xtr;
		if(type & TVX)
			ctx->pts->x[i] -= ctx->vbx;
		if(type & TRY)
			ctx->pts->y[i] += ctx->ytr;
		if(type & TVY)
			ctx->pts->y[i] -= ctx->vby;
	}
}

void rotctx(Ctx *ctx, double cx, double cy, double rad)
{
	int i;

	for(i = 0; i < ctx->pts->n; i++)
		rot(&ctx->pts->x[i], &ctx->pts->y[i], cx, cy, rad);
}

void drawctx(Ctx *ctx)
{
	int i, j;
	Obj *o;

	ctx->pts->n = 0;
	for(i = 0; i < NCTX; i++) {
		for(o = ctx->o[i]; o != NULL; o = o->link) {
			switch(o->type) {
			case OLINE:
				putline(ctx, o->col, o->p0->x, o->p0->y, o->p1->x, o->p1->y);
				break;
			case ORECT:
				putrect(ctx, o->col, o->rp->x, o->rp->y, o->w, o->h);
				break;
			case OCIRC:
				putcirc(ctx, o->col, o->cp->x, o->cp->y, o->r);
				break;
			case OTRI:
				puttri(ctx, o->col, o->t0->x, o->t0->y, o->t1->x, o->t1->y, o->t2->x, o->t2->y);
				break;
			case OLIST:
				for(j = 0; j < o->n; j++)
					put(ctx, o->col, o->a[j].x, o->a[j].y);
				break;
			default:
				errorf("obj type unset");
			}
		}
	}
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

void settri(Obj *o, double x0, double y0, double x1, double y1, double x2, double y2)
{
	o->type = OTRI;
	o->t0 = newpt(x0, y0);
	o->t1 = newpt(x1, y1);
	o->t2 = newpt(x2, y2);
}

void setlist(Obj *o)
{
	o->type = OLIST;
	o->n = 0;
	o->a = emalloc(NLIST*sizeof(*o->a));
}

void rot(double *x, double *y, double cx, double cy, double rad)
{
	double rx, ry;

	rx = *x - cx;
	ry = cy - *y;
	*x = rx*cos(rad) - ry*sin(rad) + cx;
	*y = -(rx*sin(rad) + ry*cos(rad)) + cy;
}

void rotp(Point *p, Point *c, double rad)
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
