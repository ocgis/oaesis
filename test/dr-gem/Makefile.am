INCLUDES = -I ../../src/lib

noinst_PROGRAMS = \
	example.prg example1.prg example2.prg \
	example3.prg

example_prg_SOURCES = example.c
example1_prg_SOURCES = example1.c
example2_prg_SOURCES = example2.c
example3_prg_SOURCES = example3.c

if MINT_TARGET
example_prg_LDADD = ../../src/lib/liboaesis.a
example1_prg_LDADD = ../../src/lib/liboaesis.a
example2_prg_LDADD = ../../src/lib/liboaesis.a
example3_prg_LDADD = ../../src/lib/liboaesis.a
else # MINT_TARGET
example_prg_LDADD = ../../src/lib/liboaesis.la
example1_prg_LDADD = ../../src/lib/liboaesis.la
example2_prg_LDADD = ../../src/lib/liboaesis.la
example3_prg_LDADD = ../../src/lib/liboaesis.la
endif # MINT_TARGET

