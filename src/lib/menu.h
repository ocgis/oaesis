/*
** menu.h
**
** Copyright 1996 - 2000 Christer Gustavsson <cg@nocrew.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**  
** Read the file COPYING for more information.
**
*/
#ifndef __MENU__
#define __MENU__

#include "types.h"


/*
** Description
** 0x001e menu_bar () library code
**
** 1999-01-09 CG
*/
WORD
Menu_do_bar (WORD     apid,
             OBJECT * tree,
             WORD     mode);

/****************************************************************************
 *  Menu_bar                                                                *
 *   0x001e menu_bar() library code.                                        *
 ****************************************************************************/
void              /*                                                        */
Menu_bar(         /*                                                        */
AES_PB *apb);     /* Pointer to AES parameter block.                        */
/****************************************************************************/

/****************************************************************************
 *  Menu_icheck                                                             *
 *   0x001f menu_icheck().                                                  *
 ****************************************************************************/
void              /*                                                        */
Menu_icheck(      /*                                                        */
AES_PB *apb);     /* Pointer to AES parameter block.                        */
/****************************************************************************/

/****************************************************************************
 *  Menu_ienable                                                            *
 *   0x0020 menu_ienable().                                                 *
 ****************************************************************************/
void              /*                                                        */
Menu_ienable(     /*                                                        */
AES_PB *apb);     /* Pointer to AES parameter block.                        */
/****************************************************************************/

/****************************************************************************
 *  Menu_tnormal                                                            *
 *   0x0021 menu_tnormal().                                                 *
 ****************************************************************************/
void              /*                                                        */
Menu_tnormal(     /*                                                        */
AES_PB *apb);     /* Pointer to AES parameter block.                        */
/****************************************************************************/

/****************************************************************************
 *  Menu_text                                                               *
 *   0x0022 menu_text().                                                    *
 ****************************************************************************/
void              /*                                                        */
Menu_text(        /*                                                        */
AES_PB *apb);     /* Pointer to AES parameter block.                        */
/****************************************************************************/

/****************************************************************************
 *  Menu_register                                                           *
 *   0x0023 menu_register().                                                *
 ****************************************************************************/
void              /*                                                        */
Menu_register(    /*                                                        */
AES_PB *apb);     /* Pointer to AES parameter block.                        */
/****************************************************************************/


/*
** Description
** 0x0024 menu_popup
*/
void
Menu_popup(AES_PB * apb);

#endif
