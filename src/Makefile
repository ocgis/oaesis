#
# Makefile for oAESis using GCC
#

CC = gcc
AS = gcc
LD = gcc
MODEL = -mshort
LDLIBS = -lc16


DEFS = -D__MINT__ 

MDEFS = -bm68k-atari-mint
CFLAGS = -Wall -fomit-frame-pointer $(MODEL) $(DEFS) $(MDEFS)
ASFLAGS = $(MODEL)

CFILES = appl.c boot.c debug.c docalls.c evnt.c evnthndl.c form.c fsel.c \
         global.c graf.c main.c menu.c misc.c mousedev.c objc.c \
         resource.c rlist.c rsrc.c scrp.c shel.c srv.c vdi.c wind.c

HFILES = appl.h boot.h debug.h evnt.h evnthndl.h form.h fsel.h gemdefs.h \
         global.h \
         graf.h lxgemdos.h mintdefs.h menu.h mesagdef.h misc.h mousedev.h \
         objc.h resource.h rlist.h rsrc.h scrp.h shel.h srv.h types.h \
         version.h vdi.h wind.h wmdefs.h
   
SFILES = gcc.s

OBJS = $(CFILES:%.c=%.o) $(SFILES:%.s=%.o)

all: oaesis.prg

oaesis.prg: $(OBJS)
	$(LINK.c) -o $@ $(OBJS) $(LDLIBS)

gcc.o: lxgemdos.h

#
# assembler source files
#
.SUFFIXES: .s .o0

.s.o:
	$(AS) $(ASFLAGS) -o $@ -c $<


gcc.o: gcc.s

tosify:
	cleanret -a $(CFILES) $(HFILES)

unixify:
	cleanret -u $(CFILES) $(HFILES)

EXTRAS= oaesis.prg $(ATRANS) $(ATRANSOBJ)

dep:
	rm -f depend
	for file in $(CFILES) ; \
	do\
	  gcc -MM $$file >> depend ;\
	done	

clean:
	rm -f $(OBJS) purec.o
	cd test; $(MAKE) clean

realclean:
	rm -f $(OBJS) purec.o $(EXTRAS) *.pdb
	cd test; $(MAKE) realclean

include depend
