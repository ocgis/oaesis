/*
** srv_event.c
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

#include "debug.h"
#include "srv_event.h"
#include "srv_get.h"
#include "vdi.h"

/*
** Description
** This procedure is installed with vex_butv to handle mouse button clicks.
**
** 1998-12-06 CG
*/
static
void
catch_mouse_buttons (int buttons) {
  DB_printf ("srv.c: catch_mouse_buttons: buttons = %d", buttons);
  
  Srv_wake ();
}


/*
** Exported
**
** 1998-12-07 CG
*/
void
init_event_handler (WORD vdi_workstation_id) {
  void * old_button_vector;

  Vdi_vex_butv (vdi_workstation_id, catch_mouse_buttons, &old_button_vector);
}


/*
** Exported
**
** 1998-12-07 CG
*/
void
handle_events (void) {
  DB_printf ("srv_event.c: in handle_events");
}
