INCLUDES = -I../comm -I../common -I../lib

noinst_LIBRARIES = libserver.a

TRAP_SRCS = gcc.s

SERVER_SRCS = \
	rlist.c srv.c srv_appl.c \
	srv_appl.h srv_appl_info.c srv_appl_info.h srv_call.c srv_call.h \
	srv_comm.h srv_debug.c srv_event.c srv_event.h srv_get.c srv_global.c \
	srv_malloc.h srv_menu.c srv_menu.h srv_misc.c srv_queue.c srv_queue.h \
	srv_trace.h srv_wind.c srv_wind.h

if MINT_TARGET
libserver_a_SOURCES = \
	$(TRAP_SRCS) $(SERVER_SRCS)

if LAUNCHER_AS_PRG
libserver_a_LIBADD = ../lib/liboaesis_client.a
else  # LAUNCHER_AS_PRG
libserver_a_LIBADD = \
	../lib/liboaesis_client.a ../launcher/liblauncher.a ../lib/liboaesis.a
endif # LAUNCHER_AS_PRG

else  # not MINT_TARGET
libserver_a_SOURCES = \
	$(SERVER_SRCS)

if LAUNCHER_AS_PRG
else  # LAUNCHER_AS_PRG
libserver_a_LIBADD = ../launcher/liblauncher.a ../lib/liboaesis.la
endif # LAUNCHER_AS_PRG

endif # MINT_TARGET

noinst_HEADERS = \
        lxgemdos.h mousedev.h rlist.h srv.h srv_global.h srv_misc.h

EXTRA_libserver_a_SOURCES = \
	$(TRAP_SRCS) $(SERVER_SRCS) purec.s srv_comm_device.c srv_comm_device.h

EXTRA_DIST = Makefile.mint
