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

#include "types.h"

/*
** Description
** The event handler is initialized by this function.
** All event vectors are set appropriately.
**
** 1998-12-07 CG
*/
void
init_event_handler (WORD vdi_workstation_id);


/*
** Description
** Handle waiting events collected from event vectors and distribute
** them to the applictions.
**
** 1998-12-07 CG
*/
void
handle_events (void);
#endif /* _SRV_EVENT_H_ */
