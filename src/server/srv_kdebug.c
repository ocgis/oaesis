/*
** srv_kdebug.c
**
** Copyright 2000 Christer Gustavsson <cg@nocrew.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**  
** Read the file COPYING for more information.
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdarg.h>
#include <stdio.h>

#include "srv_kdebug.h"
#include "srv_trace.h"

/*
** Description
** Debug things inside of the kernel
*/
void
kdebug(char * fmt, ...)
{
  va_list arguments;
  char    s[128];
  
  va_start(arguments, fmt);
  vsprintf(s, fmt, arguments);
  va_end(arguments);

#ifdef MINT_TARGET
  kerinf->trace(s);
#else
  fprintf (stderr, "oaesis: %s\r\n", s);
#endif
}
