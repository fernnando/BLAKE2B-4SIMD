PROG=mainBlake2
PREFIX?=/usr/local
MANDIR?=$(PREFIX)/man
NO_OPENMP?=0
NO_OPENMP_0=-fopenmp
NO_OPENMP_1=
CC?=gcc
CFLAGS?=-O3 -march=native -Werror=declaration-after-statement
CFLAGS+=-std=c99
CFLAGS+=$(NO_OPENMP_$(NO_OPENMP))
LIBS=
FILES=mainBlake2.c BLAKE2B-4SIMD.c
all: $(FILES)
	$(CC) $(FILES) $(CFLAGS) $(LIBS) -o $(PROG)

clean:
	rm -f $(PROG)

install:
	install -d $(DESTDIR)$(PREFIX)/bin
	install -d $(DESTDIR)$(MANDIR)/man1
	install -m 755 $(PROG) $(DESTDIR)$(PREFIX)/bin
	install -m 644 mainBlake2.1 $(DESTDIR)$(MANDIR)/man1/$(PROG).1
