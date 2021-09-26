#include <SDL.h>
#include <math.h>
#include <stdio.h>
#include <stdarg.h>

#define W 		500
#define H 		500
#define NCTX		100

typedef uint8_t		uint8;
typedef uint32_t	uint32;
typedef unsigned int	uint;

typedef struct		Obj Obj;
typedef struct		Ctx Ctx;

enum {
	OLINE,
	ORECT,
	OCIRC,
};

struct Obj {
	uint	id;
	int	type;
	uint32	col;
	Obj	*link;
	Ctx	*ctx;
	union {
		struct Line {
			int x0, y0, x1, y1;
		} line;
		struct Rect {
			int tx, ty, bx, by;
		} rect;
		struct Circ {
			int cx, cy, r;
		} circ;
	};
};

/* wrap-around array of size NCTX, collisions handled using obj links */
struct Ctx {
	uint	cid;
	Obj	**o;
};

void	errorf(char *, ...);
void	*emalloc(size_t);
void	init();
void	draw();
void	put(uint32, int, int);
uint32	rgb(uint8, uint8, uint8);
void	putline(uint32, int, int, int, int);
void	putrect(uint32, int, int, int, int);
void	putcirc(uint32, int, int, int);
Ctx	*newctx(void);
void	drawctx(Ctx *);
uint	addobj(Ctx *, uint32, int);
