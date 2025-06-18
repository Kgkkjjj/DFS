CC=gcc
CFLAGS=-Wall -Wextra -O2 $(shell pkg-config --cflags libcurl)
LDFLAGS=$(shell pkg-config --libs libcurl) -pthread

all: cwebtester

cwebtester: cwebtester.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm -f cwebtester

.PHONY: all clean
