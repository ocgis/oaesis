INCLUDES = -I../comm -I../common -I../.. -I../lib

SERVER_CSRCS = \
	boot.c srv_debug.c main.c rlist.c srv.c srv_appl.c srv_appl_info.c \
	srv_event.c srv_get.c srv_global.c srv_menu.c srv_misc.c srv_queue.c \
	srv_wind.c

SERVER_OBJS = $(SERVER_CSRCS:.c=.o)

TRAP_OBJS = gcc.o

OBJS = $(SERVER_OBJS) $(TRAP_OBJS)
CSRCS = $(SERVER_CSRCS)

EXTRA_LIBS = ../launcher/liblauncher.a ../lib/liboaesis.a \
	     ../lib/liboaesis_client.a

all: oaesis

oaesis: $(SERVER_OBJS) $(TRAP_OBJS) $(EXTRA_LIBS)
	$(LD) -o $@  $(SERVER_OBJS) $(TRAP_OBJS) $(EXTRA_LIBS) \
	  -lovdisis

srv_get.c:
	echo \#include \"srv_get_pmsg.c\" > srv_get.c

include ../../mincl.mint
