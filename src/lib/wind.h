#ifndef	__WIND__
#define	__WIND__

/*#include "rlist.h"*/
#include "types.h"

# define REDRAW_ALL (-1)

/*window status codes*/
#define	WIN_OPEN       0x0001
#define	WIN_UNTOPPABLE 0x0002
#define	WIN_DESKTOP    0x0004
#define	WIN_TOPPED     0x0008
#define	WIN_DIALOG     0x0010
#define	WIN_MENU       0x0020
#define WIN_ICONIFIED  0x0040

/*
** Description
** Get the window elements resource for a window.
**
** 1998-12-20 CG
*/
OBJECT *
Wind_get_rsrc (WORD apid,
               WORD id);

/*
** Description
** Change state of window element
**
** 1998-12-20 CG
*/
WORD
Wind_change (WORD apid,
             WORD id,
             WORD object,
             WORD newstate);

/*
** Description
** Draw window elements of window <id> with a clipping rectangle <clip>.
** If <id> is REDRAW_ALL all windows of the application will be redrawn.
**
** 1998-10-11 CG
*/
void
Wind_redraw_elements (WORD   apid,
                      WORD   id,
                      RECT * clip,
                      WORD   start);

/****************************************************************************
 * Wind_do_create                                                           *
 *  Implementation of wind_create().                                        *
 ****************************************************************************/
WORD             /* 0 if error or 1 if ok.                                  */
Wind_do_create(  /*                                                         */
WORD   apid,     /* Owner of window.                                        */
WORD   elements, /* Elements of window.                                     */
RECT * maxsize,  /* Maximum size allowed.                                   */
WORD   status);  /* Status of window.                                       */
/****************************************************************************/

void Wind_create(AES_PB *apb); /*0x0064*/

/****************************************************************************
 * Wind_do_open                                                             *
 *  Implementation of wind_open().                                          *
 ****************************************************************************/
WORD           /* 0 if error or 1 if ok.                                    */
Wind_do_open(
WORD   apid,
WORD   id,     /* Identification number of window to open.                  */
RECT * size);  /* Initial size of window.                                   */
/****************************************************************************/

void Wind_open(AES_PB *apb);   /*0x0065*/

/*
** Description
** Implementation of wind_close ()
**
** 1998-12-20 CG
*/
WORD
Wind_do_close (WORD apid,
               WORD wid);

void
Wind_close (AES_PB *apb);  /*0x0066*/

/****************************************************************************
 * Wind_do_delete                                                           *
 *  Implementation of wind_delete().                                        *
 ****************************************************************************/
WORD             /* 0 if error or 1 if ok.                                  */
Wind_do_delete(  /*                                                         */
WORD apid,
WORD wid);       /* Identification number of window to close.               */
/****************************************************************************/

void Wind_delete(AES_PB *apb); /*0x0067*/

/*
** Description
** Lib part of wind_get
**
** 1998-10-04 CG
*/
WORD
Wind_do_get (WORD   apid,
             WORD   handle,
             WORD   mode,
             WORD * parm1,
             WORD * parm2,
             WORD * parm3,
             WORD * parm4,
             WORD   in_workarea);

void Wind_get(AES_PB *apb);    /*0x0068*/

/*
** Description
** Library part of wind_set ()
**
** 1998-12-25 CG
*/
WORD
Wind_do_set (WORD apid,
             WORD handle,
             WORD mode,
             WORD parm1,
             WORD parm2,
             WORD parm3,
             WORD parm4);

void
Wind_set(AES_PB *apb);    /*0x0069*/

/*
** Description
** Find window on known coordinates.                                       *
**
** 1998-12-20 CG
*/
WORD
Wind_do_find (WORD apid,
              WORD x,
              WORD y);

void
Wind_find (AES_PB *apb);   /*0x006a*/

/*
** Description
** Library part of wind_update
**
** 1998-12-23 CG
*/
WORD
Wind_do_update (WORD apid,
                WORD mode);

void
Wind_update (AES_PB *apb); /*0x006b*/

void Wind_calc(AES_PB *apb);   /*0x006c*/

/****************************************************************************
 * Wind_new                                                                 *
 *  0x006d wind_new().                                                      *
 ****************************************************************************/
void              /*                                                        */
Wind_new(         /*                                                        */
AES_PB *apb);     /* AES parameter block.                                   */
/****************************************************************************/

#endif
