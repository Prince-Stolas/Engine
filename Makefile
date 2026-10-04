CC := cc
CFLAGS := $(shell sdl2-config --cflags)
LDLIBS := $(shell sdl2-config --libs)

.PHONY: all clean

all: engine

engine: main.c
	$(CC) $(CFLAGS) -o out *.c $(LDLIBS)

clean:
	rm -f out
