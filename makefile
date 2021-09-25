CC	= gcc
CFLAGS	= -Wall -Wextra -g
OBJ	= src/gfx.o

HFILES	= src/gfx.h

%.o:	%.c $(HFILES)
	$(CC) $(CFLAGS) -o $@ -c $<

gfx:	$(OBJ) $(HFILES)
	$(CC) $(CFLAGS) $(OBJ) -o $@

.PHONY:	clean

clean:
	rm -f gfx $(OBJ)
