#include <SDL.h>
#include <math.h>
#include <stdio.h>

#define W 	500
#define H 	500
#define NCTX	100

typedef uint8_t		uint8;
typedef uint32_t	uint32;
typedef unsigned int	uint;
typedef	double		doub;

typedef struct		Obj Obj;
typedef struct		Ctx Ctx;

enum {
	OCIRC,
	ORECT,
};

struct Obj {
	uint	id;
	doub	cx, cy;
	int	type;
	Obj	*link;
	Ctx	*ctx;
	uint32	col;
	union {
		struct Circ {
			doub r;
		} circ;
		struct Rect {
			doub l, w; 
		} rect;
	};
};

/* wrap-around array of size NCTX, collisions handled using obj links */
struct Ctx {
	uint cid;
	Obj **o;
};
