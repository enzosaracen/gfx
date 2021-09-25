#include <SDL.h>
#include <math.h>
#include <stdio.h>

#define W 	500
#define H 	500
#define NCTX	50

typedef uint8_t		uint8;
typedef uint32_t	uint32;
typedef unsigned int	uint;

typedef union		Obj Obj;

struct Obj {
	uint id;
	union {
		struct Circ {
			double x, y, r;
		} circ;
		struct Rect {
			double x, y, l, w; 
		} rect;
	};
	Obj *link;
};

/* wrap-around array, collisions handled using obj links */
Obj *ctx[NCTX]; 
