INCLUDES = -I../comm -I../common -I../lib

bin_PROGRAMS = oaesis

TRAP_SRCS = gcc.s

SERVER_SRCS = \
	boot.c srv_debug.c main.c rlist.c srv.c srv_appl.c srv_appl.h \
	srv_appl_info.c srv_appl_info.h srv_comm.h srv_event.c srv_event.h \
	srv_get.c srv_global.c srv_menu.c srv_menu.h srv_misc.c srv_queue.c \
	srv_queue.h srv_wind.c srv_wind.h

if MINT_TARGET
oaesis_SOURCES = \
	$(TRAP_SRCS) $(SERVER_SRCS)

if LAUNCHER_AS_PRG
oaesis_LDADD = ../lib/liboaesis_client.a
else  # LAUNCHER_AS_PRG
oaesis_LDADD = \
	../lib/liboaesis_client.a ../launcher/liblauncher.a ../lib/liboaesis.a
endif # LAUNCHER_AS_PRG

else  # MINT_TARGET
oaesis_SOURCES = \
	$(SERVER_SRCS)

if LAUNCHER_AS_PRG
else  # LAUNCHER_AS_PRG
oaesis_LDADD = ../launcher/liblauncher.a ../lib/liboaesis.la
endif # LAUNCHER_AS_PRG

endif # MINT_TARGET

noinst_HEADERS = \
        boot.h lxgemdos.h mousedev.h rlist.h srv.h srv_global.h srv_misc.h

EXTRA_oaesis_SOURCES = \
	$(TRAP_SRCS) $(SERVER_SRCS) purec.s

EXTRA_DIST = Makefile.mint
