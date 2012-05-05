/*
** misc.h
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

#ifndef WORD
#define WORD short
#endif /* WORD */
#define LONG long

#ifndef TRUE
#define TRUE  1
#define FALSE 0
#endif /* TRUE */

#define HI_WORD(l) ((WORD)((LONG)l >> 16))
#define LO_WORD(l) ((WORD)((LONG)l & 0xffff))

#ifdef MINT_TARGET
#define PATH_SEPARATOR '\\'
#else
#define PATH_SEPARATOR '/'
#endif

/*
** Description
** Set current working directory
*/
int
misc_setpath(char * dir);
