#ifndef	__APPL__
#define	__APPL__

#include	"mesagdef.h"
#include	"types.h"


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

#endif
