CFLAGS = -I../comm -I../common

bin_PROGRAMS = oaesis

oaesis_SOURCES = \
        boot.c debug.c \
        srv_global.c main.c \
#        resource.c \
        rlist.c \
        srv.c srv_get.c srv_misc.c \
        vdi.c

#        boot.h evnthndl.h \
#        global.h \
#        lxgemdos.h menu.h mesagdef.h misc.h mousedev.h \
#        oconfig.h \
#        objc.h resource.h rlist.h rsrc.h scrp.h shel.h \
#        version.h vdi.h wind.h

noinst_HEADERS = boot.h lxgemdos.h mousedev.h rlist.h srv.h srv_global.h srv_misc.h

EXTRA_oaesis_SOURCES = \
         mousedev.c
