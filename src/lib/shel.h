#ifndef	__SHEL__
#define	__SHEL__

#include	"types.h"


void Shel_read(AES_PB *apb);	/*0x0078*/
void Shel_write(AES_PB *apb); /*0x0079*/


#define SHEL_FIND_ERROR 0
#define SHEL_FIND_OK    1

/*
** Description
** Implementation of shel_find()
**
** 1999-03-14 CG
*/
WORD
Shel_do_find (WORD   apid,
              BYTE * buf);

/****************************************************************************
 * Shel_find                                                                *
 *  0x007c shel_find().                                                     *
 ****************************************************************************/
void              /*                                                        */
Shel_find(        /*                                                        */
AES_PB *apb);     /* AES parameter block.                                   */
/****************************************************************************/

/****************************************************************************
 * Shel_envrn                                                               *
 *  0x007d shel_envrn().                                                    *
 ****************************************************************************/
void              /*                                                        */
Shel_envrn(       /*                                                        */
AES_PB *apb);     /* AES parameter block.                                   */
/****************************************************************************/

#endif
