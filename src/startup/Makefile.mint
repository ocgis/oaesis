INCLUDES = -I../common -I../launcher -I../lib -I../server -I../..

SERVER_CSRCS = \
	boot.c is_of_type.c main.c

SERVER_OBJS = $(SERVER_CSRCS:.c=.o)

OBJS = $(SERVER_OBJS)
CSRCS = $(SERVER_CSRCS)

EXTRA_LIBS = \
	../server/libserver.a ../launcher/liblauncher.a \
	../lib/liboaesis.a ../lib/liboaesis_client.a

all: oaesis

oaesis: $(SERVER_OBJS) $(EXTRA_LIBS)
	$(LD) -o $@  $(SERVER_OBJS) $(EXTRA_LIBS) \
	  -lovdisis

include ../../mincl.mint
