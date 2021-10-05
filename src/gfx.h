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

typedef uint8_t		uint8;
typedef uint32_t	uint32;
typedef unsigned int	uint;

typedef struct		Point Point;
typedef struct		Obj Obj;
typedef struct		Ctx Ctx;
typedef struct		Kevs Kevs;

enum {
	OLINE,
	ORECT,
	OCIRC,
	OLIST,
};

enum {
	KEYINACTIVE,
	KEYHELD,
	KEYDOWN,
	KEYUP,
};

struct Point {
	double x, y;
};

struct Obj {
	uint	id;
	int	type;
	uint32	col;
	Obj	*link;
	Ctx	*ctx;
	union {
		struct {
			Point *p0, *p1;
		} line;
		struct {
			Point *p;
			int w, h;
		} rect;
		struct {
			Point *p;
			int r;
		} circ;
		struct {
			/* temporary solution, could do faster stuff with blits
			 * but dont know how to manage out-of-bound coords */
			int n;
			Point *a;
		} list;
	};
};

/* wrap-around array of size NCTX, collisions handled using obj links */
struct Ctx {
	uint	cid;
	Obj	**o;
	double	scale;
	int	xtr, ytr;
};

struct Kevs {
	int state;
	int ndown, nup;
};

Kevs	*keyev;

void	errorf(char *, ...);
void	*emalloc(size_t);
void	init();
void	draw();
void	put(uint32, int, int);
void	clrast(void);
uint32	rgb(uint8, uint8, uint8);
void	putline(uint32, int, int, int, int);
void	putrect(uint32, int, int, int, int);
void	putcirc(uint32, int, int, int);
void	addlist(Obj *, Point *);
Ctx	*newctx(void);
Point	*newpt(double, double);
void	drawctx(Ctx *);
Obj	*addobj(Ctx *, uint32);
void	setline(Obj *, double, double, double, double);
void	setrect(Obj *, double, double, int, int);
void	setcirc(Obj *, double, double, int);
void	setlist(Obj *);
void	rot(Point *, Point *, double);
void	input(void);
