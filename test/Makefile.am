INCLUDES = -I ../src/lib

noinst_PROGRAMS = formtest structtest

formtest_SOURCES = \
	formtest.c test2rsc.rsc

structtest_SOURCES = \
	structtest.c

if MINT_TARGET
formtest_LDADD = ../src/lib/liboaesis.a
else # MINT_TARGET
formtest_LDADD = ../src/lib/liboaesis.la
endif # MINT_TARGET