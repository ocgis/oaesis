INCLUDES = -I../comm -I../common -I../lib

bin_PROGRAMS = oaesis

TRAP_SRCS = gcc.s

SERVER_SRCS = \
	boot.c debug.c main.c rlist.c srv.c srv_appl_info.c srv_comm.h \
	srv_event.c srv_get.c srv_global.c srv_misc.c srv_queue.c srv_wind.c \
        srv_appl_info.h srv_event.h srv_queue.h srv_wind.h

if MINT_TARGET
oaesis_SOURCES = \
	$(TRAP_SRCS) $(SERVER_SRCS)
oaesis_LDADD = ../lib/liboaesis_client.a

else # MINT_TARGET
oaesis_SOURCES = \
	$(SERVER_SRCS)
endif # MINT_TARGET

noinst_HEADERS = \
        boot.h lxgemdos.h mousedev.h rlist.h srv.h srv_global.h srv_misc.h

EXTRA_oaesis_SOURCES = \
	$(TRAP_SRCS) $(SERVER_SRCS)
