CC = gcc
AS = gcc
LD = gcc
GLOBAL_INCLUDES = -I/usr/local/include/osis
GLOBAL_DEFINES = -DHAVE_CONFIG_H -Wall -Wmissing-prototypes -Wunused -O2
AR = ar
RANLIB = ranlib

CFLAGS = $(INCLUDES) $(GLOBAL_INCLUDES) $(DEFINES) $(GLOBAL_DEFINES)

ifdef CSRCS
DEPS = $(CSRCS:.c=.P)
endif

ifdef DEPS
-include $(DEPS)
endif

subdirs:
	for subdir in $(SUBDIRS); do \
	  $(MAKE) -f Makefile.mint -C $$subdir;\
	done

clean:
	-$(RM) $(OBJS)
	for subdir in $(SUBDIRS); do \
	  $(MAKE) -f Makefile.mint clean -C $$subdir;\
	done

realclean:
	-$(RM) $(OBJS) $(DEPS)
	for subdir in $(SUBDIRS); do \
	  $(MAKE) -f Makefile.mint realclean -C $$subdir;\
	done

%.o: %.s
	$(CC) -c $<

%.o: %.c
	$(CC) $(CFLAGS) -Wp,-MD,$(*F).pp -c $<
	@-cp $(*F).pp $(*F).P; \
	tr ' ' '\012' < $(*F).pp \
	  | sed -e 's/^\\$$//' -e '/^$$/ d' -e '/:$$/ d' -e 's/$$/ :/' \
	    >> $(*F).P; \
	rm $(*F).pp
