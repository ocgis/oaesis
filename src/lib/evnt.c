/****************************************************************************

 Module
  evnt.c
  
 Description
  Event handling routines in oAESis.
  
 Author(s)
 	cg (Christer Gustavsson <d2cg@dtek.chalmers.se>)
 	
 Revision history
 
  960127 cg
   Standard header added.

  960322 cg
	 Fixed "0x100" mode of evnt_multi().
 
 Copyright notice
  The copyright to the program code herein belongs to the authors. It may
  be freely duplicated and distributed without fee, but not charged for.
 
 ****************************************************************************/

#define DEBUGLEVEL 0

/****************************************************************************
 * Used interfaces                                                          *
 ****************************************************************************/

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

/****************************************************************************
 * Module global variables                                                  *
 ****************************************************************************/

static WORD	clicktime = 200;


/*0x0014 evnt_keybd*/

void	Evnt_keybd(AES_PB *apb) {
  EVNTREC	e;
  
  while(TRUE) {
    /*    Fread(apb->global->int_info->eventpipe,sizeof(EVNTREC),&e);*/
    
    if(e.ap_event == APPEVNT_KEYBOARD) {
      apb->int_out[0] = (WORD)(e.ap_value >> 16);
      break;
    };
  };
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


void	Evnt_button(AES_PB *apb) {
  /*	apb->int_out[0] = Evnt_do_button(apb->global->apid,
			apb->global->int_info->eventpipe,
			apb->int_in[0],apb->int_in[1],apb->int_in[2]
			,&apb->int_out[1],&apb->int_out[2],&apb->int_out[3]
			,&apb->int_out[4]);*/
}

/*0x0016 evnt_mouse*/
void	Evnt_mouse(AES_PB *apb) {
	DB_printf("evnt_mouse not implemented yet");
	
	apb->int_out[0] = 1;
}


/*
** Exported
**
** 1998-12-19 CG
*/
void
Evnt_mesag (AES_PB *apb) {
  /*
  ** FIXME
  ** Call evnt_multi instead
  COMMSG    e;
  WORD      i;
  SRV_APPL_INFO appl_info;
	
  Srv_get_appl_info(apb->global->apid,&appl_info);

  Fread(appl_info.msgpipe,MSG_LENGTH,&e);

  for(i = 0; i < 8; i++) {
    ((WORD *)apb->addr_in[0])[i] = ((WORD *)&e)[i];
  };
	
  if((e.type == WM_REDRAW) && (e.sid == -1)) {
    RECT totsize;
		
    Wind_do_get (apb->global->apid,
                 ((WORD *)apb->addr_in[0])[3],
                 WF_CURRXYWH,
                 &totsize.x,
                 &totsize.y,
                 &totsize.width,
                 &totsize.height,
                 TRUE);
	
    ((WORD *)apb->addr_in)[4] += totsize.x;
    ((WORD *)apb->addr_in)[5] += totsize.y;
  };			
  */

  DB_printf ("!!Implement Evnt_mesag in evnt.c");

  apb->int_out[0] = 1;
}

/*0x0018 evnt_timer*/

void	Evnt_timer(AES_PB *apb) {
	LONG	time = apb->int_in[1];
	
	time <<= 16;
	time += apb->int_in[0];
	time <<= 10;	/*approx * 1000 :-)*/

	usleep(time);
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
    if (ret.eventout.events & MU_MENU_BAR) {
      Evhd_handle_menu (apid,
                        ret.eventout.mx,
                        ret.eventout.my);
    }
    
    if (ret.eventout.events & MU_MESAG) {
      if (ret.msg.type == WM_REDRAW) {
        /* Redraw window borders */
        Graf_do_mouse (apid, M_OFF, NULL);
        Wind_do_update (apid, BEG_UPDATE);
        Wind_redraw_elements (apid, ret.msg.msg0, (RECT *)&ret.msg.msg1, 0);
        Wind_do_update (apid, END_UPDATE);
        Graf_do_mouse (apid, M_ON, NULL);
      }

      if (eventin->events & MU_MESAG) {
        *msg = ret.msg;
        events_out |= MU_MESAG;
      }
    }
    
    if (ret.eventout.events & MU_BUTTON) {
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
**
** 1998-12-19 CG
** 1999-01-09 CG
*/
void
Evnt_multi (AES_PB *apb) {
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
