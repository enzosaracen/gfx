#include "2d.h"

/* 2d physics layer atop gfx.c */

/* assumes obj has already been created under ctx, just adds properties */
void addpobj(Pctx *pc, Obj *o, double mass, double xv, double yv)
{
	if(pc->pmax < pc->ctx->v->max) {
		pc->pmax = pc->ctx->v->max;
		pc->p = erealloc(pc->p, pc->pmax*sizeof(Prop));
	}
	pc->p[o->i].mass = mass;
	pc->p[o->i].xv = xv;
	pc->p[o->i].yv = yv;
}

Pctx *newpctx(void)
{
	Pctx *pc;

	pc = emalloc(sizeof(Pctx));
	pc->ctx = newctx();
	pc->pmax = pc->ctx->v->max;
	pc->p = emalloc(pc->pmax*sizeof(Prop));
	return pc;
}

double dist(Point *p0, Point *p1)
{
	double dx, dy;

	dx = fabs(p1->x - p0->x);
	dy = fabs(p1->y - p0->y);
	return sqrt(dx*dx + dy*dy);
}

int coll(Obj *o0, Obj *o1)
{
	switch(o0->type) {
	case OCIRC:
		switch(o1->type) {
		case OCIRC:
			return collcc(o0, o1);
		/* temporary */
		case OLIST:
			return 0;
		default:
			errorf("unsupported");
		}
		break;
	/* temporary */
	case OLIST:
		return 0;
	default:
		errorf("unsupported");
	}
	return 0;
}

int collcc(Obj *o0, Obj *o1)
{
	return dist(o0->cp, o1->cp) <= o0->r + o1->r;
}

void update(Pctx *pc)
{
	int i, j, t;

	for(i = 0; i < pc->ctx->v->n; i++)
		trobj(pc->ctx->v->o[i], TRX|TRY, 0, pc->p[i].xv, pc->p[i].yv, 0, 0);

	for(i = 0; i < pc->ctx->v->n; i++) {
		for(j = i+1; j < pc->ctx->v->n; j++)
			if(coll(pc->ctx->v->o[i], pc->ctx->v->o[j])) {
				t = pc->p[i].xv;
				pc->p[i].xv = pc->p[j].xv;
				pc->p[j].xv = t;
				t = pc->p[i].yv;
				pc->p[i].yv = pc->p[i].yv;
				pc->p[j].yv = t;
			}
		/* temporary */
		if(pc->ctx->v->o[i]->type == OCIRC) {
			if(pc->ctx->v->o[i]->cp->x + pc->ctx->v->o[i]->r >= W || pc->ctx->v->o[i]->cp->x - pc->ctx->v->o[i]->r < 0)
				pc->p[i].xv *= -1;
			if(pc->ctx->v->o[i]->cp->y + pc->ctx->v->o[i]->r >= H || pc->ctx->v->o[i]->cp->y - pc->ctx->v->o[i]->r < 0)
				pc->p[i].yv *= -1;
		}
	}
}

void loop(Pctx *pc, int tms)
{
	int fps, cnt;
	int st, et, err, d;
	Obj *l;

	setlist(l = addobj(pc->ctx, rgb(0xff, 0xff, 0xff)));
	fps = err = cnt = 0;
	while(1) {
		st = SDL_GetTicks();
		input();
		addlist(l, pc->ctx->v->o[0]->cp);
		addlist(l, pc->ctx->v->o[1]->cp);
		update(pc);
		if(!err) {
			drawobj(l);
			draw(1, pc->ctx);
			clrast();
			fps++;
		}
		et = SDL_GetTicks();
		d = loopdelay(tms, st, et, &err);
		SDL_Delay(d);
		if(++cnt >= 1000/tms) {
			printf("fps: %d\n", fps);
			fps = cnt = 0;
		}
	}
}

int main(void)
{
	Pctx *pc;
	Obj *o;
	uint32 w;

	init();

	w = rgb(0xff, 0xff, 0xff);
	pc = newpctx();
	setcirc(o = addobj(pc->ctx, w), W/2-50, H/2, 10);
	addpobj(pc, o, 1, 1, 2);
	setcirc(o = addobj(pc->ctx, w), W/2+50, H/2, 10);
	addpobj(pc, o, 1, -1, 0);
	drawctx(pc->ctx);
	loop(pc, 5);
}
