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

#include <stdlib.h>

#include "debug.h"
#include "mesagdef.h"
#include "oconfig.h"
#include "srv_appl_info.h"
#include "srv_event.h"
#include "srv_get.h"
#include "vdi.h"


/* MOUSE_EVENT is used for storing mouse events */
typedef struct mouse_event {
  WORD x;
  WORD y;
  WORD buttons;
  WORD change;
} MOUSE_EVENT;

/* APPL_LIST is used for queueing applications that wait for better times */
typedef struct appl_list * APPL_LIST_REF;
#define APPL_LIST_REF_NIL ((APPL_LIST_REF)NULL)

typedef struct appl_list {
  WORD          is_waiting;
  COMM_HANDLE   handle;
  C_EVNT_MULTI  par;
  MOUSE_EVENT   mouse_buffer [MOUSE_BUFFER_SIZE];
  WORD          mouse_head;
  WORD          mouse_size;
  APPL_LIST_REF prev;
  APPL_LIST_REF next;
} APPL_LIST;

static APPL_LIST_REF waiting_appl = APPL_LIST_REF_NIL;
static APPL_LIST     apps [MAX_NUM_APPS];

static WORD x_last;
static WORD y_last;
static WORD buttons_new = 0;
static WORD buttons_last = 0;

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
  buttons_new = buttons;

  /* Wake up the server so that it will be able to distribute the event */
  Srv_wake ();
}


/*
** Exported
**
** 1998-12-07 CG
** 1998-12-13 CG
*/
void
srv_init_event_handler (WORD vdi_workstation_id) {
  void * old_button_vector;
  int    i;

  /* Reset buffers */
  for (i = 0; i < MAX_NUM_APPS; i++) {
    apps [i].is_waiting = FALSE;
    apps [i].mouse_head = 0;
    apps [i].mouse_size = 0;
  }

  /* Setup mouse button handler */
  Vdi_vex_butv (vdi_workstation_id, catch_mouse_buttons, &old_button_vector);
}


/*
** Description
** Queue the application to wait for events
**
** 1998-12-13 CG
*/
static
void
queue_appl (COMM_HANDLE    handle,
            C_EVNT_MULTI * par) {
  APPL_LIST_REF this_entry = &apps [par->common.apid];

  /* We got an empty list node => fill in some data and queue it */
  this_entry->is_waiting = TRUE;
  this_entry->handle = handle;
  this_entry->par = *par;
  this_entry->next = waiting_appl;
  this_entry->prev = APPL_LIST_REF_NIL;
  if (waiting_appl != APPL_LIST_REF_NIL) {
    waiting_appl->prev = this_entry;
  }
  waiting_appl = this_entry;
}


/*
** Description
** Dequeue an application that's done waiting for events
**
** 1998-12-13 CG
*/
static
void
dequeue_appl (APPL_LIST_REF this_entry) {
  this_entry->is_waiting = FALSE;

  if (this_entry->prev != APPL_LIST_REF_NIL) {
    this_entry->prev->next = this_entry->next;
  }

  if (this_entry->next != APPL_LIST_REF_NIL) {
    this_entry->next->prev = this_entry->prev;
  }

  this_entry->prev = this_entry->next = APPL_LIST_REF_NIL;
}


/*
** Description
** Check for waiting messages
**
** 1998-12-13 CG
*/
static
WORD
check_for_messages (C_EVNT_MULTI * par,
                    R_EVNT_MULTI * ret) {
  if (par->eventin.events & MU_MESAG) {
    /* Check for new messages */
    AP_INFO * ai = search_appl_info (par->common.apid);

    if (ai != NULL) {
      if (ai->message_size > 0) {
        DB_printf ("srv_event.c: srv_wait_for_event; got ai = 0x%x: message size = %d  apid = %d", ai, ai->message_size, ai->id);
        memcpy (&ret->msg,
                &ai->message_buffer [ai->message_head],
                MSG_LENGTH);
        ai->message_head =
          (ai->message_head + MSG_LENGTH) % MSG_BUFFER_SIZE;
        ai->message_size -= MSG_LENGTH;

        return MU_MESAG;
      }
    }
  }

  return 0;
}


/*
** Description
** Check for waiting mouse events
**
** 1998-12-13 CG
*/
static
WORD
check_mouse (C_EVNT_MULTI * par,
             R_EVNT_MULTI * ret) {
  if (par->eventin.events & MU_BUTTON) {
    if (apps [par->common.apid].mouse_size > 0) {
      apps [par->common.apid].mouse_size--;
      return MU_BUTTON;
    }
  }

  return 0;
}


/*
** Exported
**
** 1998-12-08 CG
** 1998-12-13 CG
*/
void
srv_wait_for_event (COMM_HANDLE    handle,
                    C_EVNT_MULTI * par) {
  R_EVNT_MULTI   ret;

  /* Are there any waiting messages? */
  ret.eventout.events = check_for_messages (par, &ret);

  /* FIXME Cheat for now and always return MU_TIMER */
  ret.eventout.events |= MU_TIMER;

  /* Look for waiting mouse events */
  ret.eventout.events |= check_mouse (par, &ret);

  /*
  ** If there were waiting events we return immediately and if not we
  ** queue the application in the waiting list.
  */
  if (ret.eventout.events == 0) {
    queue_appl (handle, par);
  } else {
    Srv_reply (handle, &ret, sizeof (R_EVNT_MULTI));
  }
}


/*
** Exported
**
** 1998-12-07 CG
** 1998-12-13 CG
*/
void
srv_handle_events (void) {
  /* Did the mouse buttons change? */
  if (buttons_last != buttons_new) {
    WORD click_owner = 0; /* FIXME lookup the real owner here */
    WORD index = (apps [click_owner].mouse_head +
                  apps [click_owner].mouse_size) % MOUSE_BUFFER_SIZE;

    apps [click_owner].mouse_buffer [index].x = x_last;
    apps [click_owner].mouse_buffer [index].y = y_last;
    apps [click_owner].mouse_buffer [index].buttons = buttons_new;
    apps [click_owner].mouse_buffer [index].change =
      buttons_new ^ buttons_last;
    apps [click_owner].mouse_size++;

    buttons_last = buttons_new;

    /* Wake the application if it's waiting */
    if (apps [click_owner].is_waiting) {
      R_EVNT_MULTI ret;
      
      ret.eventout.events = check_mouse (&apps [click_owner].par, &ret);

      if (ret.eventout.events != 0) {
        Srv_reply (apps [click_owner].handle, &ret, sizeof (R_EVNT_MULTI));

        /* The application is not waiting anymore */
        dequeue_appl (&apps [click_owner]);
      }
    }
  }
}


