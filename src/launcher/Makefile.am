CFLAGS = -Wall @CFLAGS@
INCLUDES = -I../lib

SRCS = \
	launch.rsc launch.hrd boot.c boot.h launch.c launcher.c launcher.h \
	misc.c misc.h

if LAUNCHER_AS_PRG
bin_PROGRAMS = launcher.prg
else                             # LAUNCHER_AS_PRG
noinst_LIBRARIES = liblauncher.a
endif                            # LAUNCHER_AS_PRG

launcher_prg_SOURCES = $(SRCS)
liblauncher_a_SOURCES = $(SRCS)

if USE_LIBTOOL
launcher_prg_LDADD  = ../lib/liboaesis.la
else                             # USE_LIBTOOL
launcher_prg_LDADD  = ../lib/liboaesis.a
endif                            # USE_LIBTOOL

BUILT_SOURCES = launch.c launch.h

launch.c launch.h: launch.rsc launch.hrd
	../../tools/r2c_raw launch.rsc

EXTRA_DIST = Makefile.mint
