# ─── Interstellar Puzzle — Makefile ──────────────────────────────────────────

CC      = gcc
TARGET  = interstellar_puzzle

SRCS    = main.c source.c
OBJS    = $(SRCS:.c=.o)

SDL_CFLAGS := $(shell pkg-config --cflags sdl2 SDL2_image SDL2_ttf)
SDL_LIBS   := $(shell pkg-config --libs   sdl2 SDL2_image SDL2_ttf)

CFLAGS  = -Wall -Wextra -O2 -std=c99 $(SDL_CFLAGS)
LIBS    = $(SDL_LIBS) -lm

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LIBS)

%.o: %.c header.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

install-deps:
	sudo apt-get install -y libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev

.PHONY: all clean install-deps
