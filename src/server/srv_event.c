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
#include "gemdefs.h"
#include "mesagdef.h"
#include "oconfig.h"
#include "srv_appl_info.h"
#include "srv_event.h"
#include "srv_get.h"
#include "srv_misc.h"
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

static WORD x_new = 0;
static WORD x_last = 0;
static WORD y_new = 0;
static WORD y_last = 0;
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
  buttons_new = buttons;

  /* Wake up the server so that it will be able to distribute the event */
  Srv_wake ();
}


/*
** Description
** This procedure is installed with vex_motv to handle mouse motion.
**
** 1998-12-13 CG
*/
static
void
catch_mouse_motion (int x,
                    int y) {
  WORD wake = ((x_new == x_last) && (y_new == y_last));
  x_new = x;
  y_new = y;

  /* Wake up the server so that it will be able to distribute the event */
  if (wake) {
    Srv_wake ();
  }
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
  void * old_motion_vector;
  int    i;

  /* Reset buffers */
  for (i = 0; i < MAX_NUM_APPS; i++) {
    apps [i].is_waiting = FALSE;
    apps [i].mouse_head = 0;
    apps [i].mouse_size = 0;
  }

  /* Setup mouse button handler */
  Vdi_vex_butv (vdi_workstation_id, catch_mouse_buttons, &old_button_vector);

  /* Setup mpuse motion handler */
  Vdi_vex_motv (vdi_workstation_id, catch_mouse_motion, &old_motion_vector);
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
** 1998-12-23 CG
*/
static
void
dequeue_appl (APPL_LIST_REF this_entry) {
  this_entry->is_waiting = FALSE;

  if (this_entry->prev != APPL_LIST_REF_NIL) {
    this_entry->prev->next = this_entry->next;
  } else if (this_entry == waiting_appl) {
    /* This should always be true if this_entry->prev == NULL */
    waiting_appl = this_entry->next;
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
        /*        DB_printf ("srv_event.c: srv_wait_for_event; got ai = 0x%x: message size = %d  apid = %d", ai, ai->message_size, ai->id);*/
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
check_mouse_buttons (C_EVNT_MULTI * par,
                     R_EVNT_MULTI * ret) {
  if (par->eventin.events & MU_BUTTON) {
    if (apps [par->common.apid].mouse_size > 0) {
      ret->eventout.mx =
        apps [par->common.apid].mouse_buffer [apps [par->common.apid].mouse_head].x;
      ret->eventout.my =
        apps [par->common.apid].mouse_buffer [apps [par->common.apid].mouse_head].y;
      ret->eventout.mb =
        apps [par->common.apid].mouse_buffer [apps [par->common.apid].mouse_head].buttons;
      ret->eventout.mc = 1; /* FIXME */
      apps [par->common.apid].mouse_head =
        (apps [par->common.apid].mouse_head + 1) % MOUSE_BUFFER_SIZE;
      apps [par->common.apid].mouse_size--;
      return MU_BUTTON;
    }
  }

  return 0;
}


/* FIXME include aesbind.h instead */
#define MO_ENTER 0
#define MO_LEAVE 1

/*
** Description
** Check if the mouse is inside / outside an area
**
** 1998-12-13 CG
** 1998-12-25 CG
*/
static
WORD
check_mouse_motion (WORD           x,
                    WORD           y,
                    C_EVNT_MULTI * par,
                    R_EVNT_MULTI * ret) {
  WORD retval = 0;

  if (par->eventin.events & MU_M1) {
    WORD inside = Misc_inside (&par->eventin.m1r, x, y);

    if ((inside && (par->eventin.m1flag == MO_ENTER)) ||
        ((!inside) && (par->eventin.m1flag == MO_LEAVE))) {
      retval |= MU_M1;
    }
  }

  if (par->eventin.events & MU_M2) {
    WORD inside = Misc_inside (&par->eventin.m2r, x, y);

    if ((inside && (par->eventin.m2flag == MO_ENTER)) ||
        ((!inside) && (par->eventin.m2flag == MO_LEAVE))) {
      retval |= MU_M2;
    }
  }

  /* Fill in mouse status if it hasn't been done already */
  if (!(ret->eventout.events & MU_BUTTON)) {
    ret->eventout.mx = x_last;
    ret->eventout.my = y_last;
    ret->eventout.mb = buttons_last;
    ret->eventout.ks = 0; /* FIXME
                          ** Return keystate once the keyboard handling is
                          ** implemented
                          */

  }

  return retval;
}


/*
** Exported
**
** 1998-12-08 CG
** 1998-12-13 CG
** 1998-12-23 CG
*/
void
srv_wait_for_event (COMM_HANDLE    handle,
                    C_EVNT_MULTI * par) {
  R_EVNT_MULTI   ret;

  /* Are there any waiting messages? */
  ret.eventout.events = check_for_messages (par, &ret);

  /* FIXME
  ** Cheat for now and always return MU_TIMER if the application
  ** sends MU_TIMER in
  */
  ret.eventout.events |= (MU_TIMER & par->eventin.events);

  /* Look for waiting mouse events */
  ret.eventout.events |= check_mouse_buttons (par, &ret);

  /* Check if inside or outside area */
  ret.eventout.events |= check_mouse_motion (x_new,
                                             y_new,
                                             par,
                                             &ret);
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
** Description
** Check if a mouse button has been pressed or released and report it to
** the right application
**
** 1998-12-13 CG
*/
static
void
handle_mouse_buttons (void) {
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
      
      ret.eventout.events = check_mouse_buttons (&apps [click_owner].par,
                                                 &ret);

      if (ret.eventout.events != 0) {
        Srv_reply (apps [click_owner].handle, &ret, sizeof (R_EVNT_MULTI));

        /* The application is not waiting anymore */
        dequeue_appl (&apps [click_owner]);
      }
    }
  }
}


/*
** Description
** Check if the mouse has been moved and now matches a rectangle for an
** waiting application
**
** 1998-12-13 CG
** 1998-12-23 CG
*/
static
void
handle_mouse_motion (void) {
  /* Did the mouse move? */
  if ((x_new != x_last) || (y_new != y_last)) {
    APPL_LIST_REF appl_walk = waiting_appl;

    while (appl_walk != APPL_LIST_REF_NIL) {
      APPL_LIST_REF this_appl = appl_walk;
      R_EVNT_MULTI  ret;

      appl_walk = appl_walk->next;

      ret.eventout.events = check_mouse_motion (x_new,
                                                y_new,
                                                &this_appl->par,
                                                &ret);
      if (ret.eventout.events != 0) {
        Srv_reply (this_appl->handle, &ret, sizeof (R_EVNT_MULTI));

        /* The application is not waiting anymore */
        dequeue_appl (this_appl);
      }
    }
    
    /* Update coordinates */
    x_last = x_new;
    y_last = y_new;
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
  handle_mouse_motion ();
  handle_mouse_buttons ();
}


/*
** Exported
**
** 1998-12-23 CG
*/
void
srv_graf_mkstate (C_GRAF_MKSTATE * par,
                  R_GRAF_MKSTATE * ret) {
  ret->mx = x_last;
  ret->my = y_last;
  ret->mb = buttons_last;
  ret->ks = 0; /* FIXME
               ** Return keystate once the keyboard handling is implemented
               */

  /* Return OK */
  ret->common.retval = 1;
}


/*
** Exported
**
** 1999-01-03 CG
*/
void
srv_graf_mouse (WORD           vid,
                C_GRAF_MOUSE * par,
                R_GRAF_MOUSE * ret) {
  switch (par->mode) {
  case M_ON :
    Vdi_v_show_c (vid, 0);
    break;
    
  case M_OFF :
    Vdi_v_hide_c (vid);
    break;
  }
}
