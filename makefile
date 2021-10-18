CC	= gcc
CFLAGS	= -Wall -Wextra -ggdb3 -lm $(shell sdl-config --cflags --libs)
OBJ	= src/gfx.o \
	  src/2d.o
	  # src/ship.o

HFILES	= src/gfx.h \
	  src/2d.h

%.o:	%.c $(HFILES)
	$(CC) -o $@ -c $< $(CFLAGS)

gfx:	$(OBJ) $(HFILES)
	$(CC) $(OBJ) -o $@ $(CFLAGS)

.PHONY:	clean

clean:
	rm -f gfx $(OBJ)
