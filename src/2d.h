#include "gfx.h"

typedef struct Prop Prop;
typedef struct Pctx Pctx;

struct Prop {
	double mass;
	double xv, yv;
};

struct Pctx {
	Prop *p;
	Ctx *ctx;
	int pmax;
};

void	addpobj(Pctx *, Obj *, double, double, double);
Pctx	*newpctx(void);
double	dist(Point *, Point *);
int	coll(Obj *, Obj *);
int	collcc(Obj *, Obj *);
