/*
** srv_event.h
**
** Copyright 1998 Christer Gustavsson <cg@nocrew.org>
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
*/
void
srv_handle_events (void);

#endif /* _SRV_EVENT_H_ */
