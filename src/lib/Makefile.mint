INCLUDES = -I../comm -I../common -I../.. -I.

BINDINGS_CSRCS = \
	aesbind.c

BINDINGS_OBJS = \
	$(BINDINGS_CSRCS:.c=.o)

TRAP_CSRCS = \
	trap.c

TRAP_OBJS = \
	$(TRAP_CSRCS:.c=.o)

LIB_CSRCS = \
	resource.c appl.c cursors.c lib_debug.c docalls.c evnt.c evnthndl.c \
	form.c fsel.c graf.c lib_global.c lib_misc.c menu.c objc.c rsrc.c \
	scrp.c shel.c srv_put.c wind.c

LIB_OBJS = \
	$(LIB_CSRCS:.c=.o)

OBJS = $(BINDINGS_OBJS) $(TRAP_OBJS) $(LIB_OBJS)
CSRCS = $(BINDINGS_CSRCS) $(TRAP_CSRCS) $(LIB_SRCS)

all: liboaesis.a liboaesis_client.a

liboaesis.a: $(BINDINGS_OBJS) $(TRAP_OBJS)
	$(AR) cru $@ $(BINDINGS_OBJS) $(TRAP_OBJS)
	-$(RANLIB) $@

liboaesis_client.a: $(LIB_OBJS)
	$(AR) cru $@ $(LIB_OBJS)
	-$(RANLIB) $@

cursors.c cursors.h: cursors.rsc cursors.hrd
	../../tools/r2c_raw cursors.rsc

resource.c resource.h: resource.rsc resource.hrd
	../../tools/r2c_raw resource.rsc

srv_put.c:
	echo \#include \"srv_put_pmsg.c\" > srv_put.c

include ../../mincl.mint
