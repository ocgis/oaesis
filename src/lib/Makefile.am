INCLUDES = -I../comm -I../common
CFLAGS = -Wall @CFLAGS@
# Make a shared library using libtool

oaesisdir = @OSISINCLUDE@
oaesis_HEADERS = aesbind.h

BINDINGS_SRCS = \
	aesbind.c

TRAP_SRCS = \
	trap.c

LIB_SRCS = \
	appl.c cursors.c debug.c docalls.c evnt.c evnthndl.c form.c \
	fsel.c graf.c lib_comm.h lib_global.c lib_misc.c menu.c objc.c \
	resource.c rsrc.c scrp.c shel.c srv_put.c wind.c

if USE_LIBTOOL

lib_LTLIBRARIES = liboaesis.la

liboaesis_la_SOURCES = \
	$(BINDINGS_SRCS) $(LIB_SRCS)

# FIXME
EXTRA_liboaesis_la_SOURCES = \
	cursors.rsc resource.rsc cursors.hrd resource.hrd

else # USE_LIBTOOL

lib_LIBRARIES = liboaesis.a

noinst_LIBRARIES = liboaesis_client.a

liboaesis_a_SOURCES = \
	$(BINDINGS_SRCS) $(TRAP_SRCS)

liboaesis_client_a_SOURCES = \
	$(LIB_SRCS)

endif # USE_LIBTOOL

liboaesis_la_LDFLAGS = @LT_VERSION_INFO@

noinst_HEADERS = appl.h cursors.h evnt.h evnthndl.h form.h fsel.h graf.h lib_global.h lib_misc.h menu.h objc.h resource.h rsrc.h scrp.h shel.h wind.h

EXTRA_liboaesis_a_SOURCES = $(BINDINGS_SRCS) $(TRAP_SRCS) $(LIB_SRCS)

cursors.c cursors.h: cursors.rsc cursors.hrd
	../../tools/r2c_raw cursors.rsc

resource.c resource.h: resource.rsc resource.hrd
	../../tools/r2c_raw resource.rsc
