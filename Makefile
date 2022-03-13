CC = gcc
CFLAGS = -Wall
.PHONY: all clean run build
LIBS = -lm -lpthread

all: build

build: $(wildcard *.c)
	$(CC) $(CFLAGS) $^ -o tema1_par $(LIBS)

run:
	./tema1_par

clean:
	rm -rf tema1_par
