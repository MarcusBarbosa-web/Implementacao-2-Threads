CC = gcc
CFLAGS = -Wall -Wextra -g3 -fopenmp -pthread

all: src/output/mandelbrot

src/output/mandelbrot: src/mandelbrot.c
	mkdir -p src/output
	$(CC) $(CFLAGS) src/mandelbrot.c -o src/output/mandelbrot

clean:
	rm -rf src/output

test: src/output/mandelbrot
	cd src && ./output/mandelbrot 400 400 100 4

.PHONY: all clean test