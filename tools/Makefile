CC = gcc
CFLAGS = -Wall -fomit-frame-pointer
LD = gcc
LDLIBS = -lc
TARGETS = cleanret.ttp depcheck.ttp r2c.ttp r2c_raw.ttp

CFILES = cleanret.c depcheck.c r2c.c r2c_raw.c

all: $(TARGETS)

.SUFFIXES: .c .ttp

.c.ttp:
	$(LINK.c) -o $@ $< $(LDLIBS)

clean:
	rm -f *.o

realclean:
	rm -f *.o $(TARGETS)


