# Configure paths for oaesis
# Christer Gustavsson 98-09-20

dnl AM_PATH_OAESIS([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND [, MODULES]]]])
dnl Test for oaesis, and define OAESIS_CFLAGS and OAESIS_LIBS

AC_DEFUN(AM_PATH_OAESIS,
[dnl 
dnl Get the cflags and libraries from the oaesis-config script
dnl
AC_ARG_WITH(oaesis-prefix,[  --with-oaesis-prefix=PFX   Prefix where oaesis is installed (optional)],
            oaesis_config_prefix="$withval", oaesis_config_prefix="")
AC_ARG_WITH(oaesis-exec-prefix,[  --with-oaesis-exec-prefix=PFX Exec prefix where oaesis is installed (optional)],
            oaesis_config_exec_prefix="$withval", oaesis_config_exec_prefix="")
AC_ARG_ENABLE(oaesistest, [  --disable-oaesistest       Do not try to compile and run a test oaesis program],
		    , enable_oaesistest=yes)

  if test x$oaesis_config_exec_prefix != x ; then
     oaesis_config_args="$oaesis_config_args --exec-prefix=$oaesis_config_exec_prefix"
     if test x${OAESIS_CONFIG+set} != xset ; then
        OAESIS_CONFIG=$oaesis_config_exec_prefix/bin/oaesis-config
     fi
  fi
  if test x$oaesis_config_prefix != x ; then
     oaesis_config_args="$oaesis_config_args --prefix=$oaesis_config_prefix"
     if test x${OAESIS_CONFIG+set} != xset ; then
        OAESIS_CONFIG=$oaesis_config_prefix/bin/oaesis-config
     fi
  fi

  AC_PATH_PROG(OAESIS_CONFIG, oaesis-config, no)
  min_oaesis_version=ifelse([$1], ,0.0.6,$1)
  AC_MSG_CHECKING(for oAESis - version >= $min_oaesis_version)
  no_oaesis=""
  if test "$OAESIS_CONFIG" = "no" ; then
    no_oaesis=yes
  else
    OAESIS_CFLAGS=`$OAESIS_CONFIG $oaesis_config_args --cflags`
    OAESIS_LIBS=`$OAESIS_CONFIG $oaesis_config_args --libs`
    oaesis_config_major_version=`$OAESIS_CONFIG $oaesis_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    oaesis_config_minor_version=`$OAESIS_CONFIG $oaesis_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
    oaesis_config_micro_version=`$OAESIS_CONFIG $oaesis_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
    if test "x$enable_oaesistest" = "xyes" ; then
      ac_save_CFLAGS="$CFLAGS"
      ac_save_LIBS="$LIBS"
      CFLAGS="$CFLAGS $OAESIS_CFLAGS"
      LIBS="$LIBS $OAESIS_LIBS"
     fi
  fi
  if test "x$no_oaesis" = x ; then
     AC_MSG_RESULT(yes)
     ifelse([$2], , :, [$2])     
  else
     AC_MSG_RESULT(no)
     if test "$OAESIS_CONFIG" = "no" ; then
       echo "*** The oaesis-config script installed by oaesis could not be found"
       echo "*** If oaesis was installed in PREFIX, make sure PREFIX/bin is in"
       echo "*** your path, or set the OAESIS_CONFIG environment variable to the"
       echo "*** full path to oaesis-config."
     else
        :
     fi
     OAESIS_CFLAGS=""
     OAESIS_LIBS=""
     ifelse([$3], , :, [$3])
  fi
  AC_SUBST(OAESIS_CFLAGS)
  AC_SUBST(OAESIS_LIBS)
])
