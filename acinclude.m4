dnl Check for a named signal

AC_DEFUN([OAESIS_CHECK_SIGNAL],
[AC_CACHE_CHECK([for signal $1], ac_v_oaesis_check_signal_$1,
[
  AC_TRY_COMPILE(
  [#include <signal.h>],
  [int sig = $1; return sig;],
  ac_v_oaesis_check_signal_$1=yes,
  ac_v_oaesis_check_signal_$1=no)
])
  if test $ac_v_oaesis_check_signal_$1 == yes; then
    AC_DEFINE(HAVE_SIGNAL_$1)
  fi
])
