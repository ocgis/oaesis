all: r2c_raw

include ../mincl.mint

r2c_raw: r2c_raw.c
	$(HOST_CC) $(CFLAGS) $< -o $@
