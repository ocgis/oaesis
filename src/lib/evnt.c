/*
** evnt.c
**
** Copyright 1996 - 2000 Christer Gustavsson <cg@nocrew.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**  
** Read the file COPYING for more information.
**
*/


#define DEBUGLEVEL 0

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <assert.h>

#ifdef HAVE_BASEPAGE_H
#include <basepage.h>
#endif

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#ifdef HAVE_MINTBIND_H
#include <mintbind.h>
#endif

#ifdef HAVE_OSBIND_H
#include <osbind.h>
#endif

#include <signal.h>
#include <stdio.h>

#ifdef HAVE_SUPPORT_H
#include <support.h>
#endif

#include <unistd.h>

#include "aesbind.h"
#include "debug.h"
#include "evnt.h"
#include "evnthndl.h"
#include "graf.h"
#include "lib_comm.h"
#include "lib_global.h"
#include "mintdefs.h"
#include "mesagdef.h"
#include "resource.h"
#include "srv_interface.h"
#include "srv_put.h"
#include "types.h"
#include "wind.h"

#ifdef HAVE_SYSVARS_H
#include <sysvars.h>
#endif

static WORD	clicktime = 200;


/*
** Description
** Implementation of evnt_keybd
*/
void
Evnt_keybd(AES_PB * apb)
{
  EVENTIN	ei;
  EVENTOUT	eo;
  COMMSG	buf;
  
  CHECK_APID(apb->global->apid);

  ei.events = MU_KEYBD;
  
  Evnt_do_multi(apb->global->apid, &ei, &buf, &eo, 0, DONT_HANDLE_MENU_BAR);
  
  apb->int_out[0] = eo.kc;
}


/*0x0015 evnt_button*/

/*
** Exported
**
** 1998-12-19 CG
** 1999-01-09 CG
*/
WORD
Evnt_do_button (WORD   apid,
                WORD   clicks,
                WORD   mask,
                WORD   state,
                WORD * mx,
                WORD * my,
                WORD * button,
                WORD * kstate) {
  EVENTIN	ei;
  
  EVENTOUT	eo;
  
  COMMSG	buf;
  
  ei.events = MU_BUTTON;
  ei.bclicks = clicks;
  ei.bmask   = mask;
  ei.bstate  = state;
  
  Evnt_do_multi (apid, &ei, &buf, &eo, 0, DONT_HANDLE_MENU_BAR);
  
  *mx = eo.mx;
  *my = eo.my;
  *button = eo.mb;
  *kstate = eo.ks;
  
  return eo.mc;
}


/*
** Description
** Implementation of evnt_button()
*/
void
Evnt_button(AES_PB *apb)
{
  CHECK_APID(apb->global->apid);

  apb->int_out[0] = Evnt_do_button(apb->global->apid,
                                   apb->int_in[0],
                                   apb->int_in[1],
                                   apb->int_in[2],
                                   &apb->int_out[1],
                                   &apb->int_out[2],
                                   &apb->int_out[3],
                                   &apb->int_out[4]);
}


/*0x0016 evnt_mouse*/
void
Evnt_mouse(AES_PB *apb)
{
  DEBUG0("evnt_mouse not implemented yet");
	
  apb->int_out[0] = 1;
}


/*
** Description
** Implementation of evnt_mesag()
*/
static
WORD
Evnt_do_mesag(WORD     apid,
              COMMSG * buf)
{
  EVENTIN	ei;
  
  EVENTOUT	eo;
    
  ei.events = MU_MESAG;

  Evnt_do_multi(apid, &ei, buf, &eo, 0, DONT_HANDLE_MENU_BAR);
  
  return 1;
}


/*
** Exported
*/
void
Evnt_mesag(AES_PB * apb)
{
  CHECK_APID(apb->global->apid);

  apb->int_out[0] = Evnt_do_mesag(apb->global->apid,
                                  (COMMSG *)apb->addr_in[0]);
}


/*0x0018 evnt_timer*/

void
Evnt_timer(AES_PB * apb)
{
  EVENTIN	ei;
  
  EVENTOUT	eo;
  
  COMMSG	buf;
  
  ei.events = MU_TIMER;
  ei.locount = apb->int_in[0];
  ei.hicount = apb->int_in[1];

  CHECK_APID(apb->global->apid);

  Evnt_do_multi(apb->global->apid, &ei, &buf, &eo, 0, DONT_HANDLE_MENU_BAR);  
}


/*0x0019 evnt_multi*/

/*
** Exported
*/
void
Evnt_do_multi (WORD       apid,
               EVENTIN  * eventin,
               COMMSG   * msg,
               EVENTOUT * eventout,
               WORD       level,
               WORD       handle_menu_bar)
{
  C_EVNT_MULTI  par;
  R_EVNT_MULTI  ret;
  GLOBAL_APPL * globals = get_globals (apid);
  WORD          events_out = 0;

  PUT_C_ALL(EVNT_MULTI,&par);

  par.eventin = *eventin;

  /* Handle internal menu events */
  if (handle_menu_bar && (globals->menu != NULL)) {
    par.eventin.events |= MU_MENU_BAR;
    par.eventin.menu_bar = globals->menu_bar_rect;
  }

  /* Loop until a user event has been found */
  while (events_out == 0)
  {
    DEBUG3 ("evnt.c: timeout = %d %d", par.eventin.hicount, par.eventin.locount);
    CLIENT_SEND_RECV(&par,
		     sizeof (C_EVNT_MULTI),
		     &ret,
		     sizeof (R_EVNT_MULTI));
    
    /* Handle messages */
    if (ret.eventout.events & MU_MENU_BAR)
    {
      Evhd_handle_menu (apid,
                        ret.eventout.mx,
                        ret.eventout.my);
    }
    
    if (ret.eventout.events & MU_MESAG)
    {
      if((ret.msg.type == WM_REDRAW) || (ret.msg.type == WM_EREDRAW))
      {
        /* Redraw window borders */
        Graf_do_mouse (apid, M_OFF, NULL);
        Wind_do_update (apid, BEG_UPDATE);
        Wind_redraw_elements (apid, ret.msg.msg0, (RECT *)&ret.msg.msg1, 0);
        Wind_do_update (apid, END_UPDATE);
        Graf_do_mouse (apid, M_ON, NULL);
      }
      else if(ret.msg.type == WM_AREDRAW)
      {
        /* Don't redraw elements, just window content */
        ret.msg.type = WM_REDRAW;
      }
      else if(ret.msg.type == WM_NEWTOP)
      {
        Wind_newtop(apid, ret.msg.msg0, WIND_NEWTOP);
      }
      else if(ret.msg.type == WM_UNTOPPED)
      {
        Wind_newtop(apid, ret.msg.msg0, WIND_UNTOPPED);
      }

      if (eventin->events & MU_MESAG)
      {
        *msg = ret.msg;
        events_out |= MU_MESAG;
      }
    }
    
    if (ret.eventout.events & MU_BUTTON)
    {
      DEBUG2 ("evnt.c: Evnt_do_multi: apid = %d bclicks = 0x%x bstate = 0x%x",
              apid, par.eventin.bclicks, par.eventin.bstate);
      events_out |= Evhd_handle_button (apid,
                                        ret.eventout.mb,
                                        ret.eventout.mx,
                                        ret.eventout.my,
					par.eventin.bclicks,
                                        par.eventin.bmask,
                                        par.eventin.bstate,
					&ret.eventout.mc,
                                        !handle_menu_bar);
    }

    if (ret.eventout.events & (MU_KEYBD | MU_M1 | MU_M2 | MU_TIMER)) {
      events_out |=
        ret.eventout.events & (MU_KEYBD | MU_M1 | MU_M2 | MU_TIMER);
    }
  } /* end while (events_out == 0) */

  if (events_out & MU_KEYBD) {
    DEBUG2 ("Got key to apid %d 0x%x events = 0x%x",
	    apid, ret.eventout.kc, events_out);
  }

  *eventout = ret.eventout;
  eventout->events = events_out;

  DEBUG3("Evnt_do_multi: events_ot = 0x%x", events_out);
}


/*
** Exported
*/
void
Evnt_multi (AES_PB *apb)
{
  CHECK_APID(apb->global->apid);

  Evnt_do_multi(apb->global->apid,
                (EVENTIN *)apb->int_in,
                (COMMSG *)apb->addr_in[0],
                (EVENTOUT *)apb->int_out,
                0,
                HANDLE_MENU_BAR);
  if (apb->int_out[0] & MU_KEYBD) {
    DEBUG2 ("Evnt_multi: key out = 0x%x", apb->int_out[5]);
  }
}


/*0x001a evnt_dclick*/
void	Evnt_dclick(AES_PB *apb) {
	if(apb->int_in[1] == EDC_SET) {
		clicktime = 320 - 60 * apb->int_in[0];
	};
	
	apb->int_out[0] = (320 - clicktime) / 60;
}
