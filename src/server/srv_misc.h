/*
** srv_misc.h
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

#include	"types.h"

LONG srv_fork(WORD (*func)(LONG),LONG arg,BYTE *name);

#define max(a,b) ((a > b) ? a : b)
#define min(a,b) ((a < b) ? a : b)

#define INTS2LONG(a,b) (((((LONG)a)<<16)&0xffff0000L)|(((LONG)b)&0xffff))
#define MSW(a) ((unsigned short)((unsigned long)(a) >> 16))
#define LSW(a) ((unsigned short)((unsigned long)(a) & 0xffffUL))

WORD srv_get_cookie(LONG code,LONG *value);

/****************************************************************************
 *  srv_copy_area                                                          *
 *   Copy one area of the screen to another.
 ****************************************************************************/
void              /*                                                        */
srv_copy_area(   /*                                                        */
WORD vid,         /* VDI workstation id.                                    */
RECT *dst,        /* Where to the area is to be copied.                     */
RECT *src);       /* The original area.                                     */
/****************************************************************************/

/****************************************************************************
 *  srv_intersect                                                          *
 *   Get intersection of two rectangles.                                    *
 ****************************************************************************/
WORD              /* 0  Rectangles don't intersect.                         */
                  /* 1  Rectangles intersect but not completely.            */
                  /* 2  r2 is completely covered by r1.                     */
srv_intersect(   /*                                                        */
RECT *r1,         /* Rectangle r1.                                          */
RECT *r2,         /* Rectangle r2.                                          */
RECT *rinter);    /* Buffer where the intersecting part is returned.        */
/****************************************************************************/

/****************************************************************************
 * srv_inside                                                              *
 *  Check if coordinates is within rectangle.                               *
 ****************************************************************************/
WORD              /* 0  Outside of rectangle.                               */
                  /* 1  Inside rectangle.                                   */
srv_inside(      /*                                                        */
RECT *r,          /* Rectangle.                                             */
WORD x,           /* X coordinate.                                          */
WORD y);          /* Y coordinate.                                          */
/****************************************************************************/

/****************************************************************************
 * srv_setpath                                                             *
 *  Set current working directory. This one is stolen from the Mint-libs    *
 *  and modified because of the idiotic functionality of Dsetpath().        *
 ****************************************************************************/
WORD              /* 0 ok, or -1.                                           */
srv_setpath(     /*                                                        */
BYTE *dir);       /* New directory.                                         */
/****************************************************************************/

/****************************************************************************
 * srv_get_loadinfo                                                        *
 *  Get loading information.                                                *
 ****************************************************************************/
void                /*                                                      */
srv_get_loadinfo(  /*                                                      */
WORD pid,           /*                                                      */
WORD fnamelen,      /* Length of filename buffer.                           */
BYTE *cmdlin,       /* Command line buffer.                                 */
BYTE *fname);       /* File name buffer.                                    */
/****************************************************************************/


/*
** Description
** End thread
*/
void
srv_term (WORD retval);
