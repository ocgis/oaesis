#ifndef	__FSEL__
#define	__FSEL__

/*
** Description
** Implementation of fsel_exinput ()
**
** 1998-12-20 CG
** 1999-05-24 CG
*/
WORD
Fsel_do_exinput (WORD   apid,
                 WORD * button,
                 BYTE * description,
                 BYTE * path,
                 BYTE * file);

/****************************************************************************
 * Fsel_input                                                               *
 *  0x005a fsel_input()                                                     *
 ****************************************************************************/
void              /*                                                        */
Fsel_input(       /*                                                        */
AES_PB *apb);     /* AES parameter block.                                   */
/****************************************************************************/

/****************************************************************************
 * Fsel_exinput                                                             *
 *  0x005b fsel_exinput()                                                   *
 ****************************************************************************/
void              /*                                                        */
Fsel_exinput(     /*                                                        */
AES_PB *apb);     /* AES parameter block.                                   */
/****************************************************************************/

#endif
