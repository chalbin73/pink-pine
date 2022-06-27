# Pink pine
This project is a simple program to generate audio noise.
It can generate Pink and White noise using Voss and Voss-McCartney algorithms

This program was made after seeing this [Tsoding Daily video](https://youtu.be/z3S60XTXdlw) on YouTube,
where he is trying to generate Pink Noise

Algorithms from [https://www.firstpr.com.au/dsp/pink-noise/#Voss](https://www.firstpr.com.au/dsp/pink-noise/#Voss)

# Usage

```console
$ pink-pine [-a algorithm] [-v volume] [-g generators_count] [-h]

```

Example :
Voss-McCartney algorithm
Volume = 55%
```console
$ pink-pine -a 2 -v 55
```

# Build
For now, it is only supported on Linux.

### Required

SDL2 Library

### Compiling

Simply run
```console
$ ./build.sh
```

This will produce an executable called `pink-pine`
