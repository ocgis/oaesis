/*
** rsrc.h
**
** Copyright 1995 - 2001 Christer Gustavsson <cg@nocrew.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**  
** Read the file COPYING for more information.
**
*/


#ifndef	__RSRC__
#define	__RSRC__

#include "types.h"


/*
** Description
** Implementation of rsrc_rcfix ()
*/
WORD
Rsrc_do_rcfix (int      vid,
               RSHDR  * rshdr,
               int      swap_endian,
               int      is_internal);

/****************************************************************************
 *  Rsrc_duplicate                                                          *
 *   Create copy of resource tree. When the copy isn't needed anymore it    *
 *   should be freed using Rsrc_free_tree().                                *
 ****************************************************************************/
OBJECT *          /* New resource tree, or NULL.                            */
Rsrc_duplicate(   /*                                                        */
OBJECT *src);     /* Original resource tree.                                */
/****************************************************************************/

/****************************************************************************
 *  Rsrc_free_tree                                                          *
 *   Erase resource tree created with Rsrc_duplicate.                       *
 ****************************************************************************/
void              /*                                                        */
Rsrc_free_tree(   /*                                                        */
OBJECT *src);     /* Tree to erase.                                         */
/****************************************************************************/

/*
** Description
** Implementation of rsrc_gaddr()
*/
int
Rsrc_do_gaddr(RSHDR  *  rshdr,
              int       type,
              int       index,
              OBJECT ** addr,
              int       is_internal);

/****************************************************************************
 * System calls                                                             *
 ****************************************************************************/

void	Rsrc_load(AES_PB *apb);		/*0x006e*/
void	Rsrc_free(AES_PB *apb);		/*0x006f*/
void	Rsrc_gaddr(AES_PB *apb);	/*0x0070*/
void	Rsrc_saddr(AES_PB *apb);	/*0x0071*/
void	Rsrc_obfix(AES_PB *apb);	/*0x0072*/
void	Rsrc_rcfix(AES_PB *apb);	/*0x0073*/

#endif
