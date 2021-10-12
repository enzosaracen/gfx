#include <SDL.h>
#include <time.h>
#include <math.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#define W 		500
#define H 		500
#define NCTX		100
#define NLIST		1000
#define PI		M_PI
#define VECINC		50

typedef uint8_t		uint8;
typedef uint32_t	uint32;
typedef unsigned int	uint;

typedef struct		Point Point;
typedef struct		Obj Obj;
typedef struct		Ctx Ctx;
typedef struct		Kevs Kevs;
typedef struct		Pvec Pvec;

enum {
	OLINE,
	ORECT,
	OCIRC,
	OTRI,
	OLIST,
};

enum {
	KEYINACTIVE,
	KEYHELD,
	KEYDOWN,
	KEYUP,
};

enum {
	TSF = 1<<0,
	TRX = 1<<1,
	TRY = 1<<2,
	TVX = 1<<3,
	TVY = 1<<4,
};

struct Pvec {
	int n;
	double *x, *y;
	int max;
};

struct Point {
	double x, y;
};

struct Obj {
	uint	id;
	int	type;
	uint32	col;
	Obj	*link;
	Obj	*back;
	Ctx	*ctx;
	Pvec	*pts;
	union {
		struct {		/* line */
			Point *p0, *p1;
		};
		struct {		/* rectangle */
			Point *rp;
			int w, h;
		};
		struct {		/* circle */
			Point *cp;
			int r;
		};
		struct {		/* list */
			int n;
			Point *a;
		};
		struct {		/* triangle */
			Point *t0, *t1, *t2;
		};
	};
};

/* wrap-around array of size NCTX, collisions handled using obj links */
struct Ctx {
	uint		cid;
	Obj		**o;
	double		scale, xtr, ytr, vbx, vby, rot;
};

struct Kevs {
	int state;
	int ndown, nup;
};

Kevs	*keyev;

void	errorf(char *, ...);
void	*emalloc(size_t);
void	*erealloc(void *, size_t);
void	init();
void	draw(int, ...);
void	addpts(Pvec *, int, int);
void	clrast(void);
uint32	rgb(uint8, uint8, uint8);
void	putline(Obj *, int, int, int, int);
void	putrect(Obj *, int, int, int, int);
void	putcirc(Obj *, int, int, int);
void	puttri(Obj *, int, int, int, int, int, int);
void	addlist(Obj *, Point *);
Ctx	*newctx(void);
Point	*newpt(double, double);
void	trctx(Ctx *, int, double, double, double, double, double);
void	trobj(Obj *, int, double, double, double, double, double);
void	rotobj(Obj *, double, double, double);
void	drawctx(Ctx *);
void	drawobj(Obj *);
Obj	*addobj(Ctx *, uint32);
void	remobj(Obj *);
void	setline(Obj *, double, double, double, double);
void	setrect(Obj *, double, double, int, int);
void	setcirc(Obj *, double, double, int);
void	settri(Obj *, double, double, double, double, double, double);
void	setlist(Obj *);
void	rot(double *, double *, double, double, double);
void	rotp(Point *, Point *, double);
void	input(void);
