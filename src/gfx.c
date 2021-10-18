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

void *ecalloc(size_t n, size_t s)
{
	void *p;

	p = calloc(n, s);
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
	int i, j, k, l;
	va_list arg;
	Ctx *ctx;
	Obj *o;

	va_start(arg, n);
	for(i = 0; i < n; i++) {
		ctx = va_arg(arg, Ctx *);
		for(j = 0; j < ctx->v->n; j++) {
			o = ctx->v->o[j];
			if(o == NULL || o->hide)
				continue;
			for(k = 0; k < o->pts->n; k++) {
				if(o->pts->y[k] >= H || o->pts->y[k] < 0 || o->pts->x[k] >= W || o->pts->x[k] < 0 || o->pts->rem[k])
					continue;
				for(l = 0; l < 4; l++)
						rast[((int)o->pts->y[k]*W + (int)o->pts->x[k])*4 + l] = o->col>>i*8 & 0xff;
			}
		}
	}
	if(SDL_LockSurface(scr) < 0)
		sdlerror("SDL_LockSurface");
	memcpy((uint8*)scr->pixels, rast, W*H*4);
	SDL_UnlockSurface(scr);
	SDL_UpdateRect(scr, 0, 0, 0, 0);
}

void addpts(Ptvec *p, int x, int y)
{
	int i;

	if(p->n >= p->max) {
		p->max += PTVECINC;
		p->x = erealloc(p->x, p->max*sizeof(double));
		p->y = erealloc(p->y, p->max*sizeof(double));
		p->rem = erealloc(p->rem, p->max*sizeof(int));
		for(i = 0; i < p->max; i++)
			p->rem[i] = 0;
	}
	p->x[p->n] = x;
	p->y[p->n++] = y;
}

void rempts(Ptvec *p, int i)
{
	if(i < p->n)
		p->rem[i] = 1;
}

void clrast(void)
{
	memset(rast, 0, W*H*4);
}

uint32 rgb(uint8 r, uint8 g, uint8 b)
{
	return SDL_MapRGB(scr->format, r, g, b);
}

void putline(Obj *o, int x0, int y0, int x1, int y1)
{
	int dx, dy, xi, yi, e, e2;

	dx = abs(x1 - x0);
	dy = -abs(y1 - y0);
	xi = x0 < x1 ? 1 : -1;
	yi = y0 < y1 ? 1 : -1;
	e = dx + dy;
	while(1) {
		addpts(o->pts, x0, y0);
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

void putrect(Obj *o, int x, int y, int w, int h)
{
	int nx, ny;

	nx = x + w;
	ny = y + h;
	putline(o, x, y, x, ny);
	putline(o, x, ny, nx, ny);
	putline(o, nx, ny, nx, y);
	putline(o, nx, y, x, y);
}

void putcirc(Obj *o, int cx, int cy, int r)
{
	int x, y, r2;

	r2 = r*r;
	addpts(o->pts, cx, cy+r);
	addpts(o->pts, cx, cy-r);
	addpts(o->pts, cx+r, cy);
	addpts(o->pts, cx-r, cy);

	x = 1;
	y = sqrt(r2 - 1) + 0.5;
	while(x < y) {
		addpts(o->pts, cx+x, cy+y);
		addpts(o->pts, cx+x, cy-y);
		addpts(o->pts, cx-x, cy+y);
		addpts(o->pts, cx-x, cy-y);
		addpts(o->pts, cx+y, cy+x);
		addpts(o->pts, cx+y, cy-x);
		addpts(o->pts, cx-y, cy+x);
		addpts(o->pts, cx-y, cy-x);
		x++;
		y = sqrt(r2 - x*x) + 0.5;
	}
	if(x == y) {
		addpts(o->pts, cx+x, cy+y);
		addpts(o->pts, cx+x, cy-y);
		addpts(o->pts, cx-x, cy+y);
		addpts(o->pts, cx-x, cy-y);
	}
}

void puttri(Obj *o, int x0, int y0, int x1, int y1, int x2, int y2)
{
	putline(o, x0, y0, x1, y1);
	putline(o, x1, y1, x2, y2);
	putline(o, x2, y2, x0, y0);
}

void addlist(Obj *o, Point *p)
{
	o->l[o->n].x = p->x;
	o->l[o->n++].y = p->y;
	if(o->n >= o->max) {
		o->max += LISTINC;
		o->l = erealloc(o->l, o->max*sizeof(Point));
	}
}

Ctx *newctx(void)
{
	Ctx *ctx;

	ctx = ecalloc(sizeof(Ctx), 1);
	ctx->v = emalloc(sizeof(Ovec));
	ctx->v->n = ctx->v->navail = 0;
	ctx->v->max = OVECINC;
	ctx->v->o = emalloc(OVECINC*sizeof(Obj*));
	ctx->v->avail = emalloc(OVECINC*sizeof(int));
	ctx->scale = 1;
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
	int i, j;
	double x, y, s;
	Obj *o;

	s = 1;
	x = y = 0;
	if(type & TSF) {
		s = sf/ctx->scale;
		ctx->scale = sf;
	}
	if(type & TRX) {
		ctx->xtr = xtr-ctx->xtr;
		x += ctx->xtr;
	}
	if(type & TRY) {
		ctx->ytr = ytr-ctx->ytr;
		y += ctx->ytr;
	}
	if(type & TVX) {
		ctx->vbx = vbx-ctx->vbx;
		x -= ctx->vbx;
	}
	if(type & TVY) {
		ctx->vby = vby-ctx->vby;
		y -= ctx->vby;
	}
	for(i = 0; i < ctx->v->n; i++) {
		o = ctx->v->o[i];
		if(o == NULL)
			continue;
		for(j = 0; j < o->pts->n; j++) {
			o->pts->x[j] += x;
			o->pts->y[j] += y;
			if(type & TSF) {
				o->pts->x[j] = s*(W/2-o->pts->x[j]) + W/2;
				o->pts->y[j] = s*(H/2-o->pts->y[j]) + H/2;
			}
		}
		adjobj(o, x, y, s);
	}
}

/* note that if transforming objects separate from context, future context transformations
 * will be disproportionate because transformations are done one-off here */
void trobj(Obj *o, int type, double sf, double xtr, double ytr, double vbx, double vby)
{
	int i;
	double x, y, s;

	s = 1;
	x = y = 0;
	if(type & TRX)
		x += xtr;
	if(type & TRY)
		y += ytr;
	if(type & TVX)
		x -= vbx;
	if(type & TVY)
		y -= vby;
	if(type & TSF)
		s = sf;
	for(i = 0; i < o->pts->n; i++) {
		o->pts->x[i] += x;
		o->pts->y[i] += y;
		if(type & TSF) {
			o->pts->x[i] = -s*(W/2-o->pts->x[i]) + W/2;
			o->pts->y[i] = -s*(H/2-o->pts->y[i]) + H/2;
		}
	}
	adjobj(o, x, y, s);
}

void adjobj(Obj *o, double x, double y, double s)
{
	#define tr(p) { \
		p->x += x; \
		p->y += y; \
		if(s != 1) { \
			p->x = s*(W/2-p->x) + W/2; \
			p->y = s*(H/2-p->y) + H/2; \
		} \
	}

	switch(o->type) {
	case ONONE:
		break;
	case OLINE:
		tr(o->p0);
		tr(o->p1);
		break;
	case ORECT:
		tr(o->rp);
		if(s != 1) {
			o->w *= s;
			o->h *= s;
		}
		break;
	case OCIRC:
		tr(o->cp);
		if(s != 1)
			o->r *= s;
		break;
	case OTRI:
		tr(o->t0);
		tr(o->t1);
		tr(o->t2);
		break;

	/* do nothing currently, probably should rework lists because already represented in pts,
	 * make one time operation when drawing obj to add points and then don't modify actual obj stuff */
	case OLIST:
		break;
	default:
		errorf("bad type in adjobj");
	}
}

void rotobj(Obj *o, double cx, double cy, double rad)
{
	int i;

	for(i = 0; i < o->pts->n; i++)
		rot(&o->pts->x[i], &o->pts->y[i], cx, cy, rad);
}

void rotctx(Ctx *ctx, double cx, double cy, double rad)
{
	int i, j;
	Obj *o;

	for(i = 0; i < ctx->v->n; i++) {
		o = ctx->v->o[i];
		if(o == NULL)
			continue;
		for(j = 0; j < o->pts->n; j++)
			rot(&o->pts->x[j], &o->pts->y[j], cx, cy, rad);
	}
}

void drawctx(Ctx *ctx)
{
	int i, j;
	Obj *o;

	for(i = 0; i < ctx->v->n; i++) {
		o = ctx->v->o[i];
		if(o == NULL)
			continue;
		o->pts->n = 0;
		switch(o->type) {
		case ONONE:
			break;
		case OLINE:
			putline(o, o->p0->x, o->p0->y, o->p1->x, o->p1->y);
			break;
		case ORECT:
			putrect(o, o->rp->x, o->rp->y, o->w, o->h);
			break;
		case OCIRC:
			putcirc(o, o->cp->x, o->cp->y, o->r);
			break;
		case OTRI:
			puttri(o, o->t0->x, o->t0->y, o->t1->x, o->t1->y, o->t2->x, o->t2->y);
			break;
		case OLIST:
			for(j = 0; j < o->n; j++)
				addpts(o->pts, o->l[j].x, o->l[j].y);
			break;
		default:
			errorf("bad obj type in drawctx");
		}
	}
}

void drawobj(Obj *o)
{
	int i;

	o->pts->n = 0;
	switch(o->type) {
	case OLINE:
		putline(o, o->p0->x, o->p0->y, o->p1->x, o->p1->y);
		break;
	case ORECT:
		putrect(o, o->rp->x, o->rp->y, o->w, o->h);
		break;
	case OCIRC:			
		putcirc(o, o->cp->x, o->cp->y, o->r);
		break;
	case OTRI:
		puttri(o, o->t0->x, o->t0->y, o->t1->x, o->t1->y, o->t2->x, o->t2->y);
		break;
	case OLIST:
		for(i = 0; i < o->n; i++)
			addpts(o->pts, o->l[i].x, o->l[i].y);
		break;
	default:
		errorf("bad obj type in drawobj");
	}
}

Obj *addobj(Ctx *ctx, uint32 col)
{
	Obj *o;

	o = emalloc(sizeof(Obj));
	o->ctx = ctx;
	o->col = col;
	o->pts = emalloc(sizeof(Ptvec));
	o->pts->n = 0;
	o->pts->max = PTVECINC;
	o->pts->x = emalloc(PTVECINC*sizeof(double));
	o->pts->y = emalloc(PTVECINC*sizeof(double));
	o->pts->rem = ecalloc(PTVECINC, sizeof(int));
	o->hide = 0;
	o->type = ONONE;
	if(ctx->v->navail > 0)
		o->i = ctx->v->avail[--ctx->v->navail];
	else {
		o->i = ctx->v->n++;
		if(ctx->v->n >= ctx->v->max) {
			ctx->v->max += OVECINC;
			erealloc(ctx->v->o, ctx->v->max*sizeof(Obj));
			erealloc(ctx->v->avail, ctx->v->max*sizeof(int));
		
		}
	}
	ctx->v->o[o->i] = o;
	return o;
}

void remobj(Obj *o)
{
	o->ctx->v->avail[o->ctx->v->navail++] = o->i;
	o->ctx->v->o[o->i] = NULL;
	free(o->pts->x);
	free(o->pts->y);
	free(o->pts);
	switch(o->type) {
	case ONONE:
		break;
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
	case OTRI:
		free(o->t0);
		free(o->t1);
		free(o->t2);
		break;
	case OLIST:
		free(o->l);
		break;
	default:
		errorf("bad obj type in remobj");
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
	o->max = LISTINC;
	o->l = emalloc(LISTINC*sizeof(Point));
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

int loopdelay(int tms, int st, int et, int *err)
{
	if(*err) {
		*err -= tms + (et-st);
		if(*err < 0)
			*err = 0;
		return 0;
	} else if(et-st > tms) {
		*err = et-st;
		return 0;
	} else
		return tms - (et-st);
}
