/*
** srv_event.h
**
** Copyright 1998-1999 Christer Gustavsson <cg@nocrew.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**  
** Read the file COPYING for more information.
*/

#ifndef _SRV_EVENT_H_
#define _SRV_EVENT_H_

#include "srv_get.h"
#include "srv_interface.h"
#include "types.h"

/*
** Description
** The event handler is initialized by this function.
** All event vectors are set appropriately.
**
** 1998-12-07 CG
*/
void
srv_init_event_handler (WORD vdi_workstation_id);

/*
** Description
** Handle client event queries. If an requested event is queued it will
** be returned immediately. Otherwise the "client call" will be returned
** later by handle_events () when an event is available.
**
** 1998-12-08 CG
** 1998-12-13 CG
*/
void
srv_wait_for_event (COMM_HANDLE    handle,
                    C_EVNT_MULTI * par);

/*
** Description
** Handle waiting events collected from event vectors and distribute
** them to the applictions.
**
** 1998-12-07 CG
** 1999-08-05 CG
*/
void
srv_handle_events (WORD vdi_workstation_id);

/*
** Description
** Get mouse coordinates, mouse button state and shift key state
**
** 1998-12-23 CG
*/
void
srv_graf_mkstate (C_GRAF_MKSTATE * par,
                  R_GRAF_MKSTATE * ret);

/*
** Description
** Server part of graf_mouse ()
**
** 1999-01-03 CG
*/
void
srv_graf_mouse (WORD           vid,
                C_GRAF_MOUSE * par,
                R_GRAF_MOUSE * ret);

/*
** Description
** Wake an application if it's waiting for a message
**
** 1999-04-07 CG
*/
void
srv_wake_appl_if_waiting_for_msg (WORD id);

/* Interrupt handlers */
/*
** Description
** This procedure is installed with vex_butv to handle mouse button clicks.
*/
void
catch_mouse_buttons(int buttons);

/*
** Description
** This procedure is installed with vex_motv to handle mouse motion.
*/
void
catch_mouse_motion(int x,
		   int y);

/*
** Description
** This procedure is installed with vex_timv to handle timer clicks
*/
void
catch_timer_click(void);

#endif /* _SRV_EVENT_H_ */
