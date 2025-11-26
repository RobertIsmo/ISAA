CC = gcc
CCFLAGS = -Wall -Wextra -Wpedantic -std=c99
DEV_CFLAGS = -g -O0
RELEASE_CFLAGS = -O2 -DNDEBUG

PREFIX = /usr/local

all: release

release: CCFLAGS += $(RELEASE_CFLAGS)
release: setup targets

dev: CCFLAGS += $(DEV_CFLAGS)
dev: setup targets

targets: build/isaa build/isaad

build/isaa: build/cli.o build/isaa.o
	$(CC) -o $@ $^

build/isaad: build/daemon.o build/isaa.o
	$(CC) -o $@ $^

build/%.o: src/%.c
	$(CC) $(CCFLAGS) -o $@ -c $<
build/%.o: lib/%.c
	$(CC) $(CCFLAGS) -o $@ -c $<

setup:
	mkdir -p build

clean:
	rm -fr build

install:
	echo "UNIMPLEMENTED"

uninstall:
	echo "UNIMPLEMENTED"

.PHONY: all clean install uninstall
