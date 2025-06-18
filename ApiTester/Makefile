CC=gcc
CFLAGS=-Wall -Wextra -O2 $(shell pkg-config --cflags libcurl)
LDFLAGS=$(shell pkg-config --libs libcurl) -pthread

all: apitester

apitester: apitester.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm -f apitester

.PHONY: all clean
