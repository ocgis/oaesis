INCLUDES = -I../comm -I../common -I../.. -I../lib

SERVER_CSRCS = \
	srv_debug.c rlist.c srv.c srv_appl.c srv_appl_info.c srv_call.c \
        srv_event.c srv_global.c srv_kdebug.c srv_menu.c srv_misc.c \
        srv_queue.c srv_wind.c srv_comm_device.c

SERVER_OBJS = $(SERVER_CSRCS:.c=.o)

TRAP_OBJS = gcc.o

OBJS = $(SERVER_OBJS) $(TRAP_OBJS)
CSRCS = $(SERVER_CSRCS)

all: libserver.a

libserver.a: $(SERVER_OBJS) $(TRAP_OBJS)
	$(AR) cru $@ $(SERVER_OBJS) $(TRAP_OBJS)
	-$(RANLIB) $@

srv_get.c:
	echo \#include \"srv_get_pmsg.c\" > srv_get.c

include ../../mincl.mint
