LINUX_PRGS = switchvt
MISC_PRGS = r2c_raw

if MINT_TARGET
  noinst_PROGRAMS = $(MISC_PRGS)
else
  noinst_PROGRAMS = $(LINUX_PRGS) $(MISC_PRGS)
endif

EXTRA_PROGRAMS = $(LINUX_PRGS) $(MISC_PRGS)

r2c_raw_SOURCES = r2c_raw.c

switchvt_SOURCES = switchvt.c

EXTRA_DIST = Makefile.purec
