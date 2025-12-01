VERSION=$(shell git rev-parse --abbrev-ref HEAD)
CC = gcc
CCFLAGS = -Wall -Wextra -Wpedantic -std=c99 -fPIC -DISAA_VERSION=$(VERSION)
DEBUG_CFLAGS = -g -O0 -DISAA_DEBUG
RELEASE_CFLAGS = -O2 -DNDEBUG

BUILDDIR ?= build
PREFIX ?= /usr/local
DESTDIR ?=

default: release

test: CCFLAGS += $(DEBUG_CFLAGS)
test:$(BUILDDIR) $(BUILDDIR)/test_log
	./$(BUILDDIR)/test_log

release: CCFLAGS += $(RELEASE_CFLAGS)
release:$(BUILDDIR) targets

debug: CCFLAGS += $(DEBUG_CFLAGS)
debug:$(BUILDDIR) targets

lib: CCFLAGS += $(RELEASE_CFLAGS)
lib: lib-setup $(BUILDDIR)/libisaa.a $(BUILDDIR)/libisaa.so
lib-setup:$(BUILDDIR)
	mkdir -p $(BUILDDIR)/include
	cp lib/isaa.h $(BUILDDIR)/include/isaa.h

targets: $(BUILDDIR)/isaa $(BUILDDIR)/isaad

$(BUILDDIR)/isaa: $(BUILDDIR)/cli.o $(BUILDDIR)/isaa.o $(BUILDDIR)/parse.o $(BUILDDIR)/logger.o
	$(CC) $(CCFLAGS) -o $@ $^

$(BUILDDIR)/isaad: $(BUILDDIR)/daemon.o $(BUILDDIR)/isaa.o $(BUILDDIR)/parse.o $(BUILDDIR)/logger.o
	$(CC) $(CCFLAGS) -o $@ $^

$(BUILDDIR)/libisaa.a: $(BUILDDIR)/isaa.o $(BUILDDIR)/logger.o
	ar rcs $@ $^

$(BUILDDIR)/libisaa.so: $(BUILDDIR)/isaa.o $(BUILDDIR)/logger.o
	$(CC) $(CCFLAGS) -shared -o $@ $^

$(BUILDDIR)/test_log: $(BUILDDIR)/test_log.o $(BUILDDIR)/logger.o
	$(CC) $(CCFLAGS) -o $@ $^

$(BUILDDIR)/%.o: bin/%.c
	$(CC) $(CCFLAGS) -o $@ -c $^
$(BUILDDIR)/%.o: lib/%.c
	$(CC) $(CCFLAGS) -o $@ -c $^

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

clean:
	rm -fr $(BUILDDIR)

install: install-bin install-lib install-headers

install-bin:
	@if [ -f $(BUILDDIR)/isaa ]; then \
	    install -m755 -v $(BUILDDIR)/isaa $(DESTDIR)$(PREFIX)/bin/isaa; \
	else \
	    echo "Skipping isaa"; \
	fi
	@if [ -f $(BUILDDIR)/isaad ]; then \
	    install -m755 -v $(BUILDDIR)/isaad $(DESTDIR)$(PREFIX)/bin/isaad; \
	else \
	    echo "Skipping isaad"; \
	fi

install-lib:
	@if [ -f $(BUILDDIR)/libisaa.a ]; then \
	    install -m644 -v $(BUILDDIR)/libisaa.a $(DESTDIR)$(PREFIX)/lib/libisaa.a; \
	else \
	    echo "Skipping static lib"; \
	fi
	@if [ -f $(BUILDDIR)/libisaa.so ]; then \
	    install -m755 -v $(BUILDDIR)/libisaa.so $(DESTDIR)$(PREFIX)/lib/libisaa.so; \
	else \
	    echo "Skipping shared lib"; \
	fi

install-headers:
	@if [ -f $(BUILDDIR)/include/isaa.h ]; then \
	    install -m644 -v $(BUILDDIR)/include/isaa.h $(DESTDIR)$(PREFIX)/include/isaa.h; \
	else \
	    echo "Skipping header"; \
	fi

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/isaa
	rm -f $(DESTDIR)$(PREFIX)/bin/isaad
	rm -f $(DESTDIR)$(PREFIX)/lib/libisaa.a
	rm -f $(DESTDIR)$(PREFIX)/lib/libisaa.so
	rm -f $(DESTDIR)$(PREFIX)/include/isaa.h

.PHONY: default test release debug lib targets clean install uninstall \
        install-bin install-lib install-headers
