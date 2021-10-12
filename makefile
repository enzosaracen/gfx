CC	= gcc
CFLAGS	= -Wall -Wextra -ggdb3 -lm $(shell sdl-config --cflags --libs)
OBJ	= src/gfx.o \
	  src/ship.o

HFILES	= src/gfx.h

%.o:	%.c $(HFILES)
	$(CC) -o $@ -c $< $(CFLAGS)

gfx:	$(OBJ) $(HFILES)
	$(CC) $(OBJ) -o $@ $(CFLAGS)

.PHONY:	clean

clean:
	rm -f gfx $(OBJ)
