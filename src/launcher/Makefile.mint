INCLUDES = -I../comm -I../common -I../.. -I../lib

LAUNCHER_CSRCS = boot.c launch.c launcher.c misc.c

LAUNCHER_OBJS = $(LAUNCHER_CSRCS:.c=.o)

CSRCS = $(LAUNCHER_CSRCS)
OBJS = $(LAUNCHER_OBJS)

all: liblauncher.a

liblauncher.a: $(LAUNCHER_OBJS) ../lib/liboaesis.a
	$(AR) cru $@ $(LAUNCHER_OBJS)
	-$(RANLIB) $@

launch.c launch.h: launch.rsc launch.hrd
	../../tools/r2c_raw launch.rsc

include ../../mincl.mint
