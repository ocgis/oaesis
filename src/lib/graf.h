void	Graf_init_module(void);

/*
** Description
** Implementation of graf_dragbox ()
**
** 1998-12-20 CG
** 1998-12-25 CG
*/
WORD
Graf_do_dragbox (WORD     apid,
                 WORD     w,
                 WORD     h,
                 WORD     sx,
                 WORD     sy,
                 RECT   * bound,
                 WORD   * endx,
                 WORD   * endy);

/*
** Description
** Implementation of graf_rubberbox()
**
** 1998-12-20 CG
** 1998-12-25 CG
*/
WORD
Graf_do_rubberbox (WORD   apid,
                   WORD   bx,
                   WORD   by,
                   WORD   minw,
                   WORD   minh,
                   WORD * endw,
                   WORD * endh);

void Graf_do_handle(WORD *cwidth,WORD *cheight,WORD *width,WORD *height);

/****************************************************************************
 * Graf_do_grmobox                                                          *
 *  Implementation of graf_growbox() and graf_movebox().                    *
 ****************************************************************************/
WORD              /*                                                        */
Graf_do_grmobox(  /*                                                        */
RECT *r1,         /* Start rectangle.                                       */
RECT *r2);        /* End rectangle.                                         */
/****************************************************************************/

WORD Graf_do_mouse(WORD mode,MFORM *formptr);

/*
** Description
** Implementation of graf_watchbox().
**
** 1998-12-20 CG
*/
WORD
Graf_do_watchbox (WORD     apid,
                  OBJECT * tree,
                  WORD     obj,
                  WORD     instate,
                  WORD     outstate);

/*
** Description
** Implementation of graf_slidebox().
**
** 1998-12-20 CG
*/
WORD
Graf_do_slidebox (WORD     apid,
                  OBJECT * tree,
                  WORD     parent,
                  WORD     obj,
                  WORD     orient);

/****************************************************************************
 * Graf_rubberbox                                                           *
 *  0x0046 graf_rubberbox().                                                *
 ****************************************************************************/
void              /*                                                        */
Graf_rubberbox(   /*                                                        */
AES_PB *apb);     /* AES parameter block.                                   */
/****************************************************************************/

/****************************************************************************
 * Graf_dragbox                                                             *
 *  0x0047 graf_dragbox().                                                  *
 ****************************************************************************/
void              /*                                                        */
Graf_dragbox(     /*                                                        */
AES_PB *apb);     /* AES parameter block.                                   */
/****************************************************************************/

/****************************************************************************
 * Graf_movebox                                                             *
 *  0x0048 graf_movebox().                                                  *
 ****************************************************************************/
void              /*                                                        */
Graf_movebox(     /*                                                        */
AES_PB *apb);     /* AES parameter block.                                   */
/****************************************************************************/

/****************************************************************************
 * Graf_growbox                                                             *
 *  0x0049 graf_growbox().                                                  *
 ****************************************************************************/
void              /*                                                        */
Graf_growbox(     /*                                                        */
AES_PB *apb);     /* AES parameter block.                                   */
/****************************************************************************/

/****************************************************************************
 * Graf_shrinkbox                                                           *
 *  0x004a graf_shrinkbox().                                                *
 ****************************************************************************/
void              /*                                                        */
Graf_shrinkbox(   /*                                                        */
AES_PB *apb);     /* AES parameter block.                                   */
/****************************************************************************/

/****************************************************************************
 * Graf_watchbox                                                            *
 *  0x004b graf_watchbox().                                                 *
 ****************************************************************************/
void              /*                                                        */
Graf_watchbox(    /*                                                        */
AES_PB *apb);     /* AES parameter block.                                   */
/****************************************************************************/

/****************************************************************************
 * Graf_slidebox                                                            *
 *  0x004c graf_slidebox().                                                 *
 ****************************************************************************/
void              /*                                                        */
Graf_slidebox(    /*                                                        */
AES_PB *apb);     /* AES parameter block.                                   */
/****************************************************************************/

void
Graf_handle(AES_PB *apb);	/*0x004d*/

void
Graf_mouse(AES_PB *apb);	/*0x004e*/

/*
** Description
** Library part of graf_mkstate ()
**
** 1998-12-23 CG
*/
WORD
Graf_do_mkstate (WORD   apid,
                 WORD * mx,
                 WORD * my,
                 WORD * mb,
                 WORD * ks);

void
Graf_mkstate (AES_PB *apb);	/*0x004f*/
