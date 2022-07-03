#!/bin/sh

set -xe

CFLAGS="-std=c11 -ggdb `pkg-config --cflags --libs sdl2` -lm"
LIBS="`pkg-config --libs --static sdl2` -lm"


gcc $CFLAGS -o pink-pine main.c $LIBS
