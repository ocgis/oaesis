/*
** misc.c
**
** Copyright 1999 Christer Gustavsson <cg@nocrew.org>
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

#include <mintbind.h>

/*
** Description
** Set current working directory
*/
int
misc_setpath(char * dir)
{
#ifdef MINT_TARGET
  int    drv, old;
  char * d;
  
  d = dir;
  old = Dgetdrv();
  if(*d && (*(d+1) == ':'))
  {
    drv = toupper(*d) - 'A';
    d += 2;
    (void)Dsetdrv(drv);
  }
  
  if(d[0] == '\0')
  {
    /* empty path means root directory */
    *d = '\\';
    *(d + 1) = '\0';
  }
  
  if(Dsetpath(d) < 0)
  {
    (void)Dsetdrv(old);
    return -1;
  }
  
  return 0;
#else /* MINT_TARGET */
  return chdir(dir);
#endif /* MINT_TARGET */
}
