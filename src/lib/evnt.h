#ifndef	__EVNT__
#define	__EVNT__
#include	"mesagdef.h"
#include	"types.h"

/****************************************************************************
 * Evnt_waitclicks                                                          *
 *  Wait for mouse button clicks.                                           *
 ****************************************************************************/
WORD             /* Number of clicks that were counted.                     */
Evnt_waitclicks( /*                                                         */
WORD eventpipe,  /* Event message pipe.                                     */
WORD bstate,     /* Button state to wait for.                               */
WORD bmask,      /* Button mask.                                            */
WORD clicks,     /* Maximum number of clicks.                               */
WORD laststate); /* Previous mouse button state.                            */
/****************************************************************************/

/* Description
** Implementation of evnt_button.                                          *
**
** 1998-12-19 CG
*/
WORD
Evnt_do_button(WORD   apid,
               WORD   clicks,
               WORD   mask,
               WORD   state,
               WORD * mx,
               WORD * my,
               WORD * button,
               WORD * kstate);

/*
** Description
** Implementation of evnt_multi.                                           *
**
** 1998-12-19 CG
*/
void
Evnt_do_multi(WORD       apid,
              EVENTIN  * ei,
              COMMSG   * buf,
              EVENTOUT * eo,
              WORD       level);


void	Evnt_keybd(AES_PB *apb);	/*0x0014*/
void	Evnt_button(AES_PB *apb);	/*0x0015*/
void	Evnt_mouse(AES_PB *apb);  /*0x0016*/
void	Evnt_mesag(AES_PB *apb);	/*0x0017*/
void	Evnt_timer(AES_PB *apb);	/*0x0018*/
void	Evnt_multi(AES_PB *apb);	/*0x0019*/
void	Evnt_dclick(AES_PB *apb);	/*0x001a*/

#endif
