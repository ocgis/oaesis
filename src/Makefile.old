#
# Makefile for oAESis
#

# GCC specific definitions

CC = gcc
AS = gcc
LD = gcc
MODEL = -mshort
LDLIBS = -lc16
MDEFS = -bm68k-atari-mint
CFLAGS = -Wall -fomit-frame-pointer $(MODEL) $(DEFS) $(MDEFS)
ASFLAGS = $(MODEL)
SFILES = gcc.s

# end of GCC specific definitions

# Pure C specific definitions

#CC = pcc
#CFLAGS = -I=\\usr\\include -V
#SFILES = purec.s

#.c.o:
#	$(CC) $(CFLAGS) -O=$@ $<

#end if Pure C specific definitions

DEFS = -D__MINT__ 


CFILES = appl.c boot.c debug.c docalls.c evnt.c evnthndl.c form.c fsel.c \
         global.c graf.c main.c menu.c misc.c mousedev.c objc.c \
         resource.c rlist.c rsrc.c scrp.c shel.c srv.c srv_call.c vdi.c wind.c
# wm/wm.c

HFILES = appl.h boot.h debug.h evnt.h evnthndl.h form.h fsel.h gemdefs.h \
         global.h \
         graf.h lxgemdos.h mintdefs.h menu.h mesagdef.h misc.h mousedev.h \
         objc.h resource.h rlist.h rsrc.h scrp.h shel.h srv.h srv_call.h \
         types.h \
         version.h vdi.h wind.h
# wm/wm.h
   
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

.depend depend dep:
	gcc -MM $(CFILES) > .depend


clean:
	rm -f $(OBJS) purec.o

realclean:
	rm -f $(OBJS) purec.o $(EXTRAS) *.pdb

include .depend
