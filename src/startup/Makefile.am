INCLUDES = -I../common -I../launcher -I../lib -I../server

bin_PROGRAMS = oaesis

TRAP_SRCS = gcc.s

SERVER_SRCS = \
	boot.c boot.h is_of_type.c is_of_type.h main.c

if MINT_TARGET
oaesis_SOURCES = \
	$(TRAP_SRCS) $(SERVER_SRCS)

if LAUNCHER_AS_PRG
oaesis_LDADD = ../lib/liboaesis_client.a ../server/libserver.a
else  # LAUNCHER_AS_PRG
oaesis_LDADD = \
	../lib/liboaesis_client.a ../launcher/liblauncher.a \
	../lib/liboaesis.a ../server/libserver.a
endif # LAUNCHER_AS_PRG

else  # MINT_TARGET
oaesis_SOURCES = \
	$(SERVER_SRCS)

if LAUNCHER_AS_PRG
else  # LAUNCHER_AS_PRG
oaesis_LDADD = ../launcher/liblauncher.a ../lib/liboaesis.la \
	../server/libserver.a
endif # LAUNCHER_AS_PRG

endif # MINT_TARGET

EXTRA_oaesis_SOURCES = \
	$(TRAP_SRCS) $(SERVER_SRCS) purec.s

EXTRA_DIST = Makefile.mint
