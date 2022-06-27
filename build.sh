#!/bin/sh

set -xe

CFLAGS="-std=c11 -ggdb `pkg-config --cflags --libs sdl2 ncurses` -lm"
LIBS="`pkg-config --libs --static sdl2 ncurses` -lm"


gcc $CFLAGS -o pink-pine main.c $LIBS
