SUBDIRS = tools src

all: setup subdirs

setup:
	ln -sf config.h.mint config.h

include mincl.mint
