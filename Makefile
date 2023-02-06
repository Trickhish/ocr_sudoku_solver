# Makefile

CC = gcc
CPPFLAGS =
CFLAGS = -fsanitize=address -Wall -Wextra -O3 `pkg-config --cflags sdl2`
LDFLAGS = -g -fsanitize=address
LDLIBS = `pkg-config --libs sdl2 SDL2_image` -lm

all: treat

SRC = treat.c
OBJ = ${SRC:.c=.o}
EXE = ${SRC:.c=}

treat: treat.o

.PHONY: clean

clean:
	${RM} ${OBJ}
	${RM} ${EXE}

# END