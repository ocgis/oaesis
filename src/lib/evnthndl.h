#ifndef __EVHD__
#define __EVHD__

#include "types.h"

typedef struct {
	WORD	apid;
	BYTE	flag1;
	RECT	r1;
	BYTE	flag2;
	RECT	r2;
}RECTEVENT;

typedef struct reventlist {
	RECTEVENT	event;
	
	struct reventlist	*next;	
}REVENTLIST;

/*
** Description
** Handle button click
**
** 1998-12-20 CG
*/
void 
Evhd_handle_button (WORD apid,
                    WORD newbutton,
                    WORD x,
                    WORD y);

/****************************************************************************
 * Evhd_make_rectevent                                                      *
 *  Start reporting of mouse events.                                        *
 ****************************************************************************/
void                   /*                                                   */
Evhd_make_rectevent(   /*                                                   */
RECTEVENT *re);        /* Description of events that should be reported.    */
/****************************************************************************/

/****************************************************************************
 * Evhd_kill_rectevent                                                      *
 *  End reporting of mouse events.                                          *
 ****************************************************************************/
void                   /*                                                   */
Evhd_kill_rectevent(   /*                                                   */
WORD apid);            /* Application id to end reporting to.               */
/****************************************************************************/

/****************************************************************************
 * Evhd_init_module                                                         *
 *  Initiate event processing module.                                       *
 ****************************************************************************/
void                    /*                                                  */
Evhd_init_module(void); /*                                                  */
/****************************************************************************/

/****************************************************************************
 * Evhd_exit_module                                                         *
 *  Shutdown event processing module.                                       *
 ****************************************************************************/
void                    /*                                                  */
Evhd_exit_module(void); /*                                                  */
/****************************************************************************/

/****************************************************************************
 * Evhd_wind_update                                                         *
 *  Get / release update semaphore.                                         *
 ****************************************************************************/
WORD                   /*                                                   */
Evhd_wind_update(      /*                                                   */
WORD apid,             /* Application id.                                   */
WORD mode);            /* Mode.                                             */
/****************************************************************************/


/*
** Description
** Handle menu
**
** 1999-01-09 CG
*/
void
Evhd_handle_menu (WORD apid);

#endif
