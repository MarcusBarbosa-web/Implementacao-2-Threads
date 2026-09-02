CC = gcc
CFLAGS = -Wall -Wextra -g3 -fopenmp -pthread

all: mandelbrot

mandelbrot: src/mandelbrot.c
	$(CC) $(CFLAGS) src/mandelbrot.c -o mandelbrot

clean:
	rm -f mandelbrot mandelbrot_*.pgm times.txt falha.txt

test: mandelbrot
	./mandelbrot 400 400 100 4

.PHONY: all clean test