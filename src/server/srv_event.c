/*
** srv_event.c
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

#include <stdlib.h>

#include "debug.h"
#include "gemdefs.h"
#include "mesagdef.h"
#include "oconfig.h"
#include "srv_appl_info.h"
#include "srv_event.h"
#include "srv_get.h"
#include "srv_misc.h"
#include "srv_wind.h"
#include "vdi.h"


/* MOUSE_EVENT is used for storing mouse events */
typedef struct mouse_event {
  WORD x;
  WORD y;
  WORD buttons;
  WORD change;
} MOUSE_EVENT;

/* KEY_EVENT is used for storing keyboard events */
typedef struct key_event {
  WORD state;
  WORD ascii;
  WORD scan;
} KEY_EVENT;

/* APPL_LIST is used for queueing applications that wait for better times */
typedef struct appl_list * APPL_LIST_REF;
#define APPL_LIST_REF_NIL ((APPL_LIST_REF)NULL)

typedef struct appl_list {
  WORD          is_waiting;
  COMM_HANDLE   handle;
  C_EVNT_MULTI  par;
  MOUSE_EVENT   mouse_last;
  MOUSE_EVENT   mouse_buffer [MOUSE_BUFFER_SIZE];
  WORD          mouse_head;
  WORD          mouse_size;
  KEY_EVENT     key_buffer [KEY_BUFFER_SIZE];
  WORD          key_head;
  WORD          key_size;
  ULONG         timer_event;
  APPL_LIST_REF prev;
  APPL_LIST_REF next;
} APPL_LIST;

static APPL_LIST_REF waiting_appl = APPL_LIST_REF_NIL;
static APPL_LIST     apps [MAX_NUM_APPS];

static WORD  x_new = 0;
static WORD  x_last = 0;
static WORD  y_new = 0;
static WORD  y_last = 0;
static WORD  buttons_new = 0;
static WORD  buttons_last = 0;

static KEY_EVENT key_buffer[KEY_BUFFER_SIZE];
static WORD key_buffer_head = 0;
static WORD key_buffer_tail = 0;

/*
** Timer wraparound is not yet handled, but will only be a problem if someone
** has a long uptime.
*/
#define MAX_TIMER_COUNT 0xffffffff
static ULONG timer_counter = 0;
static ULONG next_timer_event = MAX_TIMER_COUNT;
static WORD  timer_tick_ms; /* Number of milliseconds per tick */

/*
** Description
** This procedure is installed with vex_keyv to handle key presses.
**
** 1999-03-07 CG
** 1999-03-08 CG
*/
static
void
catch_keys (int state,
            int ascii,
            int scan) {
  /* Queue the keypress */
  key_buffer[key_buffer_tail].state = state;
  key_buffer[key_buffer_tail].ascii = ascii;
  key_buffer[key_buffer_tail].scan = scan;
  key_buffer_tail++;

  /* Wake up the server so that it will be able to distribute the event */
  Srv_wake ();
}


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
** Description
** This procedure is installed with vex_timv to handle timer clicks
**
** 1999-01-29 CG
*/
static
void
catch_timer_click (void) {
  /* Another 20 ms has passed */
  timer_counter += timer_tick_ms;

  /* Wake server so that it will wake the waiting application */
  if (timer_counter > next_timer_event) {
    /* Prevent multiple wake calls */
    /*    next_timer_event = MAX_TIMER_COUNT;*/
    Srv_wake ();
  }
}


/*
** Exported
**
** 1998-12-07 CG
** 1998-12-13 CG
** 1999-01-29 CG
** 1999-03-07 CG
** 1999-03-08 CG
*/
void
srv_init_event_handler (WORD vdi_workstation_id) {
  void * old_key_vector;
  void * old_button_vector;
  void * old_motion_vector;
  void * old_timer_vector;
  int    i;

  /* Reset buffers */
  for (i = 0; i < MAX_NUM_APPS; i++) {
    apps [i].is_waiting = FALSE;
    apps [i].mouse_head = 0;
    apps [i].mouse_size = 0;
    apps [i].key_head = 0;
    apps [i].key_size = 0;
  }

  /* Setup keyboard handler */
  Vdi_vex_keyv (vdi_workstation_id, catch_keys, &old_key_vector);

  /* Setup mouse button handler */
  Vdi_vex_butv (vdi_workstation_id, catch_mouse_buttons, &old_button_vector);

  /* Setup mouse motion handler */
  Vdi_vex_motv (vdi_workstation_id, catch_mouse_motion, &old_motion_vector);

  /* Setup timer handler */
  Vdi_vex_timv (vdi_workstation_id,
                catch_timer_click,
                &old_timer_vector,
                &timer_tick_ms);
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
** 1999-01-30 CG
*/
static
WORD
check_mouse_buttons (C_EVNT_MULTI * par,
                     R_EVNT_MULTI * ret) {
#define MOUSE_BUFFER_HEAD \
        apps[par->common.apid].mouse_buffer[apps [par->common.apid].mouse_head]

  WORD retval = 0;

  if (par->eventin.events & MU_BUTTON) {
    if (apps [par->common.apid].mouse_size > 0) {
      if ((MOUSE_BUFFER_HEAD.buttons & par->eventin.bmask) ==
          par->eventin.bstate) {
        /*        if (ret->eventout.mc == 0) {*/
          ret->eventout.mx = MOUSE_BUFFER_HEAD.x;
          ret->eventout.my = MOUSE_BUFFER_HEAD.y;
          /*        }*/
        
        ret->eventout.mb = MOUSE_BUFFER_HEAD.buttons;
        ret->eventout.mc = 1; /* FIXME */
      
        retval = MU_BUTTON;
      }

      /* Save the latest values */
      apps [par->common.apid].mouse_last = MOUSE_BUFFER_HEAD;

      /* Update mouse buffer head */
      apps [par->common.apid].mouse_head =
        (apps [par->common.apid].mouse_head + 1) % MOUSE_BUFFER_SIZE;
      apps [par->common.apid].mouse_size--;
    }
  }

  return retval;

#undef MOUSE_BUFFER_HEAD
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
** 1999-01-09 CG
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

  /*
  ** FIXME
  ** Check that the application owns the menu
  */
  if (par->eventin.events & MU_MENU_BAR) {
    if (Misc_inside (&par->eventin.menu_bar, x, y)) {
      retval |= MU_MENU_BAR;
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
** Description
** Check for waiting key events
**
** 1999-03-08 CG
*/
static
WORD
check_keys (C_EVNT_MULTI * par,
            R_EVNT_MULTI * ret) {
#define KEY_BUFFER_HEAD \
        apps[par->common.apid].key_buffer[apps [par->common.apid].key_head]

  WORD retval = 0;

  if (par->eventin.events & MU_KEYBD) {
    if (apps [par->common.apid].key_size > 0) {
      if (KEY_BUFFER_HEAD.scan & 0x80) {
        ret->eventout.ks = KEY_BUFFER_HEAD.state;
        ret->eventout.kc = KEY_BUFFER_HEAD.ascii;
        
        retval = MU_KEYBD;
      }

      /* Update key buffer head */
      apps [par->common.apid].key_head =
        (apps [par->common.apid].key_head + 1) % KEY_BUFFER_SIZE;
      apps [par->common.apid].key_size--;
    }
  }
  
  return retval;
  
#undef KEY_BUFFER_HEAD
}


/*
** Description
** Check for timer event
**
** 1999-01-29 CG
*/
static
WORD
check_timer (C_EVNT_MULTI * par,
             R_EVNT_MULTI * ret) {
  WORD retval = 0;

  if (par->eventin.events & MU_TIMER) {
    if (apps[par->common.apid].timer_event <= timer_counter) {
      return MU_TIMER;
    }
  }

  return retval;
}


/*
** Exported
**
** 1998-12-08 CG
** 1998-12-13 CG
** 1998-12-23 CG
** 1999-01-29 CG
** 1999-01-30 CG
** 1999-03-09 CG
*/
void
srv_wait_for_event (COMM_HANDLE    handle,
                    C_EVNT_MULTI * par) {
  R_EVNT_MULTI   ret;

  /* Are there any waiting messages? */
  ret.eventout.events = check_for_messages (par, &ret);

  /* Look for waiting mouse events */
  ret.eventout.mc = 0;
  ret.eventout.events |= check_mouse_buttons (par, &ret);

  /* Look for waiting key events */
  ret.eventout.events |= check_keys (par, &ret);

  /* Check if inside or outside area */
  ret.eventout.events |= check_mouse_motion (x_new,
                                             y_new,
                                             par,
                                             &ret);

  /* Check for timer event */
  if (par->eventin.events & MU_TIMER) {
    apps[par->common.apid].timer_event =
      timer_counter + (par->eventin.hicount << 16) + par->eventin.locount;
    
    if (apps[par->common.apid].timer_event < next_timer_event) {
      next_timer_event = apps[par->common.apid].timer_event;
    }
    
    ret.eventout.events |= check_timer (par, &ret);
  }

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
** 1999-02-05 CG
*/
static
void
handle_mouse_buttons (void) {
  /* Did the mouse buttons change? */
  if (buttons_last != buttons_new) {
    WORD click_owner = srv_click_owner (x_last, y_last);
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
** Check if a key has been pressed or released and report it to
** the right application
**
** 1999-03-08 CG
*/
static
void
handle_keys (void) {
  /* Has any key events occurred? */
  if (key_buffer_head != key_buffer_tail) {
    WORD topped_appl = get_top_appl ();
    WORD index = (apps [topped_appl].key_head +
                  apps [topped_appl].key_size) % KEY_BUFFER_SIZE;
    
    while (key_buffer_head != key_buffer_tail) {
      apps [topped_appl].key_buffer [index] = key_buffer [key_buffer_head];
      apps [topped_appl].key_size++;
      key_buffer_head = (key_buffer_head + 1) % KEY_BUFFER_SIZE;
    }

    /* Wake the application if it's waiting */
    if (apps [topped_appl].is_waiting) {
      R_EVNT_MULTI ret;
      
      ret.eventout.events = check_keys (&apps [topped_appl].par,
                                        &ret);

      if (ret.eventout.events != 0) {
        Srv_reply (apps [topped_appl].handle, &ret, sizeof (R_EVNT_MULTI));

        /* The application is not waiting anymore */
        dequeue_appl (&apps [topped_appl]);
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
** Description
** Check if any timer events have occured
**
** 1999-01-29 CG
*/
static
void
handle_timer (void) {
  /* Has any timer events at all occurred? */
  if (timer_counter > next_timer_event) {
    APPL_LIST_REF appl_walk = waiting_appl;

    next_timer_event = MAX_TIMER_COUNT;

    while (appl_walk != APPL_LIST_REF_NIL) {
      APPL_LIST_REF this_appl = appl_walk;
      R_EVNT_MULTI  ret;

      appl_walk = appl_walk->next;

      ret.eventout.events = check_timer (&this_appl->par,
                                         &ret);
      if (ret.eventout.events != 0) {
        Srv_reply (this_appl->handle, &ret, sizeof (R_EVNT_MULTI));

        /* The application is not waiting anymore */
        dequeue_appl (this_appl);
      } else {
        /* Update next timer event */
        if (this_appl->timer_event < next_timer_event) {
          next_timer_event = this_appl->timer_event;
        }
      }
    }
  }
}


/*
** Exported
**
** 1998-12-07 CG
** 1998-12-13 CG
** 1999-01-29 CG
** 1999-03-08 CG
*/
void
srv_handle_events (void) {
  handle_mouse_motion ();
  handle_mouse_buttons ();
  handle_keys ();
  handle_timer ();
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
