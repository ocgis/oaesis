CFLAGS = -Wall @CFLAGS@
INCLUDES = -I../lib

bin_PROGRAMS = launcher.prg

launcher_prg_SOURCES = \
        boot.c boot.h launch.c launcher.c misc.c misc.h

EXTRA_launcher_prg_SOURCES = \
	launch.rsc launch.hrd

if USE_LIBTOOL
launcher_prg_LDADD = ../lib/liboaesis.la
else # USE_LIBTOOL
launcher_prg_LDADD = ../lib/liboaesis.a
endif # USE_LIBTOOL

launch.c launch.h: launch.rsc launch.hrd
	../../tools/r2c_raw launch.rsc
