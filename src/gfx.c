#include "gfx.h"

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

	if(x >= W || x < 0 || y >= H || y < 0)
		return;
	for(i = 0; i < 4; i++)
		rast[(y*W + x)*4 + i] = col>>i*8 & 0xff;
}

uint32 rgb(uint8 r, uint8 g, uint8 b)
{
	return SDL_MapRGB(scr->format, r, g, b);
}

void drawctx(Ctx *ctx)
{
	int i, x, y, cx, cy, r, r2;
	uint32 col;
	Obj *p;

	for(i = 0; i < NCTX; i++) {
		p = ctx->o[i];
		while(p != NULL) {
			cx = p->cx;
			cy = p->cy;
			col = p->col;
			switch(p->type) {
			case OCIRC:
				r = p->circ.r;
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
				draw();
				break;
			case ORECT:
			default:
				errorf("bad obj type");
			}
			p = p->link;
		}
	}
}

void init(void)
{
	if(SDL_Init(SDL_INIT_VIDEO) < 0)
		sdlerror("SDL_Init");

	scr = SDL_SetVideoMode(W, H, 32, 0);
	if(scr == NULL)
		sdlerror("SDL_SetVideoMode");
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
	return ctx;
}

uint circ(Ctx *ctx, uint32 col, doub cx, doub cy, doub r)
{
	Obj *circ;

	circ = emalloc(sizeof(Obj));
	circ->type = OCIRC;
	circ->ctx = ctx;
	circ->id = ctx->cid++;
	circ->col = col;
	circ->link = NULL;
	circ->cx = cx;
	circ->cy = cy;
	circ->circ.r = r;

	ctx->o[circ->id % NCTX] = circ;
	return circ->id;
}

int main(void)
{
	Ctx *ctx;

	init();
	ctx = newctx();
	circ(ctx, rgb(0xff, 0xff, 0xff), W/2, H/2, 100);
	drawctx(ctx);
	SDL_Delay(1000);
	SDL_Quit();
}
