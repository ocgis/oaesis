CFLAGS = -Wall
INCLUDES = -I../lib

bin_PROGRAMS = launcher.prg

launcher_prg_SOURCES = \
        launcher.c launch.c

launcher_prg_LDFLAGS = ../lib/liboaesis.la

launch.c launch.h: launch.rsc launch.hrd
	../../tools/r2c_raw launch.rsc

#noinst_LIBRARIES = liblauncher.a

#liblauncher_a_SOURCES = \
	launcher.c

#liblauncher_a_LDFLAGS =
