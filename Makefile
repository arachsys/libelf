PREFIX ?= /usr/local
INCDIR = $(PREFIX)/include
LIBDIR = $(PREFIX)/lib
DATADIR = $(PREFIX)/share
PKG_CONFIG = pkg-config

CFLAGS = -O2 -Wall $(shell $(PKG_CONFIG) --cflags zlib) \
  $(shell $(PKG_CONFIG) --cflags libzstd)
LDFLAGS =
LDLIBS = $(shell $(PKG_CONFIG) --libs zlib) \
  $(shell $(PKG_CONFIG) --libs libzstd)

MAJOR = 1
MINOR = 0.192

HEADERS = $(wildcard include/*.h src/*.h)
SOURCES = $(wildcard src/*.c)
LIBOBJS = $(patsubst %.c,%.o,$(SOURCES))
PICOBJS = $(patsubst %.c,%.os,$(SOURCES))

override CFLAGS += -DHAVE_CONFIG_H -Iinclude -Isrc

all: libelf.a libelf.so libelf.pc

clean:
	rm -f src/*.o src/*.os libelf.a libelf.so

libelf.a: $(LIBOBJS) Makefile
	$(AR) rcs $@ $(LIBOBJS)

libelf.so: $(PICOBJS) Makefile
	$(CC) $(LDFLAGS) -shared -Wl,-soname,libelf.so.$(MAJOR) \
	  -o $@ $(PICOBJS) $(LDLIBS)

%.o:: %.c $(HEADERS) Makefile
	$(CC) $(CFLAGS) -c -o $@ $<

%.os:: %.c $(HEADERS) Makefile
	$(CC) $(CFLAGS) -fPIC -c -o $@ $<

libelf.pc: libelf.pc.in Makefile
	sed \
		-e 's!@prefix[@]!$(PREFIX)!g' \
		-e 's!@libdir[@]!$(LIBDIR)!g' \
		-e 's!@includedir[@]!$(INCDIR)!g' \
		-e 's!@VERSION[@]!$(MINOR)!g' \
		$< >$@

install: install-headers install-static install-shared install-pc

install-headers: include/*.h
	mkdir -p $(DESTDIR)$(INCDIR)
	install -m 0644 include/*.h $(DESTDIR)$(INCDIR)

install-pc: libelf.pc
	mkdir -p $(DESTDIR)$(DATADIR)/pkgconfig
	install -m 0644 $^ $(DESTDIR)$(DATADIR)/pkgconfig

install-static: libelf.a install-headers
	mkdir -p $(DESTDIR)$(LIBDIR)
	install -m 0644 $< $(DESTDIR)$(LIBDIR)

install-shared: libelf.so install-headers
	mkdir -p $(DESTDIR)$(LIBDIR)
	install $< $(DESTDIR)$(LIBDIR)/libelf.so.$(MAJOR).$(MINOR)
	ln -f -s libelf.so.$(MAJOR).$(MINOR) $(DESTDIR)$(LIBDIR)/libelf.so.$(MAJOR)
	ln -f -s libelf.so.$(MAJOR).$(MINOR) $(DESTDIR)$(LIBDIR)/libelf.so

.PHONY: all clean install install-*
