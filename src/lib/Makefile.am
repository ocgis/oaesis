CFLAGS = -I../comm -I../common -Wall
# Make a shared library using libtool

oaesisdir = @OSISINCLUDE@
oaesis_HEADERS = aesbind.h

lib_LTLIBRARIES = liboaesis.la

liboaesis_la_SOURCES = \
	aesbind.c appl.c cursors.c debug.c docalls.c evnt.c evnthndl.c form.c \
	fsel.c graf.c lib_global.c lib_misc.c menu.c objc.c resource.c rsrc.c \
	scrp.c shel.c srv_calls.c srv_put.c vdi.c wind.c

EXTRA_liboaesis_la_SOURCES = \
	cursors.rsc resource.rsc cursors.hrd resource.hrd

noinst_HEADERS = appl.h cursors.h evnt.h evnthndl.h form.h fsel.h graf.h lib_global.h lib_misc.h menu.h objc.h resource.h rsrc.h scrp.h shel.h srv_calls.h wind.h

liboaesis_la_LDFLAGS = @LT_VERSION_INFO@

cursors.c cursors.h: cursors.rsc cursors.hrd
	../../tools/r2c_raw cursors.rsc
resource.c resource.h: resource.rsc resource.hrd
	../../tools/r2c_raw resource.rsc
