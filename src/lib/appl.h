/*
** appl.h
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

#ifndef	__APPL__
#define	__APPL__

#include	"mesagdef.h"
#include	"types.h"


/* Internal appl_control() modes */
#define APC_TOPNEXT 0
#define APC_KILL    1


/*system calls*/

/****************************************************************************
 * Appl_do_init                                                             *
 *  Implementation of appl_init().                                          *
 ****************************************************************************/
WORD                    /* Application id, or -1.                           */
Appl_do_init(           /*                                                  */
GLOBAL_ARRAY * global); /* Global array.                                    */
/****************************************************************************/

void	Appl_init(AES_PB *apb);		/*0x000a*/
void	Appl_read(AES_PB *apb);		/*0x000b*/

/*
** Description
** Allocate and application id for process pid. The type is either
** APP_ACCESSORY or APP_APPLICATION
*/
WORD
Appl_do_reserve(WORD apid,
                WORD type,
                WORD pid);

/*
** Description
** Implementation of appl_write ()
**
** 1998-12-20 CG
*/
WORD
Appl_do_write (WORD   apid,
               WORD   addressee,
               WORD   length,
               void * m);

void
Appl_write (AES_PB *apb);	/*0x000c*/

/****************************************************************************
 * Appl_find                                                                *
 *  0x000d appl_find().                                                     *
 ****************************************************************************/
void              /*                                                        */
Appl_find(        /*                                                        */
AES_PB *apb);     /* AES parameter block.                                   */
/****************************************************************************/

/*
** Description
** Implementation of appl_search ()
**
** 1999-04-11 CG
*/
WORD
Appl_do_search (WORD   apid,
                WORD   mode,
                BYTE * name,
                WORD * type,
                WORD * ap_id);

/****************************************************************************
 * Appl_search                                                              *
 *  0x0012 appl_search().                                                   *
 ****************************************************************************/
void              /*                                                        */
Appl_search(      /*                                                        */
AES_PB *apb);     /* AES parameter block.                                   */
/****************************************************************************/

/****************************************************************************
 * Appl_do_exit                                                             *
 *  Implementation of appl_exit().                                          *
 ****************************************************************************/
WORD            /* 0 if error, or 1.                                        */
Appl_do_exit(   /*                                                          */
WORD apid);     /* Application id.                                          */
/****************************************************************************/

void	Appl_exit(AES_PB *apb);		/*0x0013*/

/****************************************************************************
 * Appl_getinfo                                                             *
 *   0x0082 appl_getinfo().                                                 *
 ****************************************************************************/
void              /*                                                        */
Appl_getinfo(     /*                                                        */
AES_PB *apb);     /* AES parameter block.                                   */
/****************************************************************************/

/*
** Description
** Library part of appl_control ()
**
** 1999-04-18 CG
*/
WORD
Appl_do_control (WORD apid,
                 WORD ap_id,
                 WORD mode);
#endif
