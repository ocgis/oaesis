SUBDIRS = dr-gem

INCLUDES = -I ../src/lib

noinst_PROGRAMS = formtest structtest testmenu

formtest_SOURCES = \
	formtest.c test2rsc.hrd test2rsc.rsc

structtest_SOURCES = \
	structtest.c

testmenu_SOURCES = \
	testmenu.c testmen.hrd testmen.rsc

if MINT_TARGET
formtest_LDADD = ../src/lib/liboaesis.a
testmenu_LDADD = ../src/lib/liboaesis.a
else # MINT_TARGET
formtest_LDADD = ../src/lib/liboaesis.la
testmenu_LDADD = ../src/lib/liboaesis.la
endif # MINT_TARGET

BUILT_SOURCES = test2rsc.c test2rsc.h testmen.c testmen.h

testmen.c testmen.h: testmen.rsc testmen.hrd
	../tools/r2c_raw testmen.rsc

test2rsc.c test2rsc.h: test2rsc.rsc test2rsc.hrd
	../tools/r2c_raw test2rsc.rsc
