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

#include "debug.h"
#include "evnt.h"
#include "evnthndl.h"
#include "gemdefs.h"
#include "lib_global.h"
#include "mintdefs.h"
#include "mesagdef.h"
#include "resource.h"
#include "srv_calls.h"
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

/****************************************************************************
 * Local functions (use static!)                                            *
 ****************************************************************************/

static LONG waitforinput(LONG timeout,LONG *rhnd) {
  LONG l = timeout >> 15;
  LONG t = timeout & 0x7fffL;
  LONG fs;
  LONG rlhnd;
  
  DEBUG3 ("Fselect in waitforinput");
  while(l) {
    rlhnd = *rhnd;
    
    fs = Fselect(0x7fff,&rlhnd,NULL,0L);
    
    if(fs) {
      *rhnd = rlhnd;
      return fs;
    };
    
    l--;
  };
  
  rlhnd = *rhnd;
  
  fs = Fselect((WORD)t,&rlhnd,NULL,0L);
  *rhnd = rlhnd;
  return fs;
}

LONG eventselect(WORD events,LONG time,LONG *fhl) {
  if(events & MU_TIMER) {
    if(time <= 0) {
      DEBUG3 ("Fselect in eventselect");
      return Fselect(1,fhl,0L,0L);
    }
    else {	
      return waitforinput(time,fhl);
    };
  }
  else {
    DEBUG3 ("Fselect in eventselect");
    return Fselect(0,fhl,0L,0L);
  };			
}

/****************************************************************************
 * Public functions                                                         *
 ****************************************************************************/

/****************************************************************************
 * Evnt_waitclicks                                                          *
 *  Wait for mouse button clicks.                                           *
 ****************************************************************************/
WORD             /* Number of clicks that were counted.                     */
Evnt_waitclicks( /*                                                         */
WORD eventpipe,  /* Event message pipe.                                     */
WORD bstate,     /* Button state to wait for.                               */
WORD bmask,      /* Button mask.                                            */
WORD clicks,     /* Maximum number of clicks.                               */
WORD laststate)  /* Previous mouse button state.                            */
/****************************************************************************/
{
  WORD    clickcount = 0;
  EVNTREC er;
  WORD    extrawait = 1;
  
  while(clicks) {
    LONG fhl = (1L << eventpipe);
    
    if((extrawait = Fselect(clicktime,&fhl,0L,0L)) > 0) {
      Fread(eventpipe,sizeof(EVNTREC),&er);
      
      if(er.ap_event == APPEVNT_BUTTON) {
	if(((er.ap_value ^ laststate) & bmask) &&
	   ((bmask & er.ap_value) == bstate)) {
	  clicks--;
	  clickcount++;
	};
	
	laststate = (WORD)er.ap_value;
      };
    }
    else {
      break;
    };
	};
  
  if((clicks == 0) || extrawait) {
    (void)Fselect(clicktime,0L,0L,0L);
  };
  
  return clickcount;
}


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
  COMMSG    e;
  WORD      i;

  /*
  ** FIXME
  ** Call evnt_multi instead
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

/****************************************************************************
 * Evnt_do_multi                                                            *
 *  Implementation of evnt_multi.                                           *
 ****************************************************************************/
void                 /*                                                     */
Evnt_do_multi_old(       /*                                                     */
WORD     apid,       /* Application id.                                     */
WORD     eventpipe,  /* Event message pipe.                                 */
WORD     msgpipe,    /* AES message pipe.                                   */
EVENTIN  *ei,        /* Input parameters.                                   */
COMMSG   *buf,       /* Message buffer.                                     */
EVENTOUT *eo,        /* Output parameters.                                  */
WORD     level)      /* Number of times the function has been called by     */
                     /* itself.                                             */
/****************************************************************************/
{
  ULONG	starttime = 0 /*globals.time*/;

  WORD	lastbutton = 0 /*globals.mouse_button*/;
	
  LONG	fhevent,fhmsg;

  if(ei->events & MU_MESAG) {
    fhmsg = (1L << msgpipe);
  }
  else {
    fhmsg = 0L;
  };

  if(ei->events & (MU_M1 | MU_M2 | MU_BUTTON | MU_KEYBD)) {
    fhevent = (1L << eventpipe);
  }
  else {
    fhevent = 0L;
  };

  if(ei->events & (MU_M1 | MU_M2)) {
    RECTEVENT	re;
		
    re.apid = apid;
		
    if(ei->events & MU_M1) {
      re.flag1 = ei->m1flag;
      re.r1 = ei->m1r;
    }
    else {
      re.flag1 = -1;
    };
		
    if(ei->events & MU_M2) {
      re.flag2 = ei->m2flag;
      re.r2 = ei->m2r;
    }
    else {
      re.flag2 = -1;
    };

    Evhd_make_rectevent(&re);
  };

  if((level == 0) && (ei->events & MU_BUTTON) && (0x0100 & ei->bclicks) &&
     (ei->bmask & lastbutton) && (apid == Srv_click_owner())) {
    eo->mx = 0 /*globals.mouse_x*/;
    eo->my = 0 /*globals.mouse_y*/;
    eo->ks = (WORD)(Kbshift(-1) & 0x1f);
    eo->mb = 0 /*globals.mouse_button*/;

    if((ei->bclicks & 0xff) <= 0) {
      eo->mc = 0;
    }
    else {
      eo->mc = 1 + Evnt_waitclicks(eventpipe,ei->bmask & lastbutton,
                                   ei->bmask & lastbutton,
                                   ei->bclicks & 0xff,lastbutton);
    };

    eo->events = MU_BUTTON;

  }	
  else if((level == 0) && (ei->events & MU_BUTTON) &&
          ((!fhmsg) || (fhmsg && (Finstat(msgpipe) == 0))) &&
          (ei->bstate == (ei->bmask & lastbutton)) &&
          ((ei->bmask & lastbutton) || (ei->bclicks <= 1)) &&
          (apid == Srv_click_owner())) {

    eo->mx = 0 /*globals.mouse_x*/;
    eo->my = 0 /*globals.mouse_y*/;
    eo->ks = (WORD)(Kbshift(-1) & 0x1f);
    eo->mb = 0 /*globals.mouse_button*/;

    if(ei->bclicks <= 0) {
      eo->mc = 0;
    }
    else {
      eo->mc = 1 + Evnt_waitclicks(eventpipe,ei->bstate,ei->bmask,
                                   ei->bclicks,lastbutton);
    };

    eo->events = MU_BUTTON;
		
  }
  else {	
    while(1) {
      LONG fhl = (fhevent | fhmsg);
      LONG time = (((LONG)ei->hicount) << 16) + (LONG)ei->locount
        - (LONG)(0 /*globals.time*/ - starttime);

      if(eventselect(ei->events,time,&fhl) == 0) {
        eo->events = MU_TIMER;
				
        eo->mx = 0 /*globals.mouse_x*/;
        eo->my = 0 /*globals.mouse_y*/;
        eo->ks = (WORD)(Kbshift(-1) & 0x1f);
        eo->mb = 0 /*globals.mouse_button*/;

        break;
      };
		
      if(fhl & fhevent) {	
        EVNTREC	e;
				
        Fread(eventpipe,sizeof(EVNTREC),&e);

        if(e.ap_event == MO_RECT1) {
          EVNTREC_MOUSE	e2;
						
          Fread(eventpipe,sizeof(EVNTREC_MOUSE),&e2);

          if(ei->events & MU_M1) {						
            eo->events = MU_M1;

            eo->mx = e2.mx;
            eo->my = e2.my;
            eo->mb = e2.buttons;
            eo->ks = e2.kstate;
            break;
          };
        }
        else if(e.ap_event == MO_RECT2) {
          EVNTREC_MOUSE	e2;
						
          Fread(eventpipe,sizeof(EVNTREC_MOUSE),&e2);
					
          if(ei->events & MU_M2) {	
            eo->events = MU_M2;

            eo->mx = e2.mx;
            eo->my = e2.my;
            eo->mb = e2.buttons;
            eo->ks = e2.kstate;
            break;
          };
        }
        else if((e.ap_event == APPEVNT_KEYBOARD) &&
                (e.ap_value & 0xffff0000L) &&
                (ei->events & MU_KEYBD)) {
          eo->events = MU_KEYBD;
          eo->kc = (WORD)(e.ap_value >> 16);
          eo->ks = (WORD)(e.ap_value);
		
          /*
          eo->mx = globals.mouse_x;
          eo->my = globals.mouse_y;
          eo->mb = globals.mouse_button;
          */

          break;
        }
        else if((e.ap_event == APPEVNT_BUTTON) &&
                (ei->events & MU_BUTTON)) {
          if((0x0100 & ei->bclicks) && (ei->bmask & e.ap_value)) {
            lastbutton = (WORD)e.ap_value;

            /*
            eo->mx = globals.mouse_x;
            eo->my = globals.mouse_y;
            */
            eo->ks = (WORD)(Kbshift(-1) & 0x1f);
            eo->mb = (WORD)e.ap_value;/*globals.mouse_button*/

            if((ei->bclicks & 0xff) >= 1) {
              eo->mc = 1 + Evnt_waitclicks(eventpipe,ei->bmask & lastbutton,
                                           ei->bmask & lastbutton,
                                           (ei->bclicks & 0xff) - 1,lastbutton);
            }
            else {
              eo->mc = 1;
            };
						
            eo->events = MU_BUTTON;

            break;
          }
          else if((e.ap_value & ei->bmask) == ei->bstate) {
            lastbutton = (WORD)e.ap_value;
	
            /*
            eo->mx = globals.mouse_x;
            eo->my = globals.mouse_y;
            */
            eo->ks = (WORD)(Kbshift(-1) & 0x1f);
            eo->mb = (WORD)e.ap_value;/*globals.mouse_button*/

            if(ei->bclicks >= 1) {
              eo->mc = 1 + Evnt_waitclicks(eventpipe,ei->bstate,ei->bmask,
                                           ei->bclicks - 1,lastbutton);
            }
            else {
              eo->mc = 1;
            };

            eo->events = MU_BUTTON;
						
            break;
          };
					
          lastbutton = (WORD)e.ap_value;
        };
      }
      else if(fhl & fhmsg) {
        Fread(msgpipe,sizeof(COMMSG),buf);
	
        eo->events = MU_MESAG;
		
        if((buf->type == WM_REDRAW) && (buf->sid == -1)) {	
          RECT totsize;
			
          Wind_do_get (apid,
                       ((WORD *)buf)[3],
                       WF_CURRXYWH,
                       &totsize.x,
                       &totsize.y,
                       &totsize.width,
                       &totsize.height,
                       TRUE);
	
          buf->msg1 += totsize.x;
          buf->msg2 += totsize.y;
        };	
        /*				
        eo->mx = globals.mouse_x;
        eo->my = globals.mouse_y;
        */
        eo->ks = (WORD)(Kbshift(-1) & 0x1f);
        /*
        eo->mb = globals.mouse_button;
        */

        if(buf->type == AP_TERM) {
          DB_printf("Received AP_TERM");
        }

        break;
      };
    };
  };
	
  if(ei->events & (MU_M1 | MU_M2)) {
    Evhd_kill_rectevent(apid);
  };
}


/*
** Exported
**
** 1998-10-04 CG
** 1998-10-11 CG
** 1998-12-13 CG
** 1998-12-19 CG
** 1999-01-01 CG
** 1999-01-09 CG
** 1999-03-21 CG
** 1999-04-10 CG
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

  par.common.call = SRV_EVNT_MULTI;
  par.common.apid = apid;
  par.common.pid = getpid ();
  par.eventin = *eventin;

  /* Handle internal menu events */
  if (handle_menu_bar && (globals->menu != NULL)) {
    par.eventin.events |= MU_MENU_BAR;
    par.eventin.menu_bar = globals->menu_bar_rect;
  }

  Client_send_recv (&par,
                    sizeof (C_EVNT_MULTI),
                    &ret,
                    sizeof (R_EVNT_MULTI));

  /* Handle messages */
  if (ret.eventout.events & MU_MENU_BAR) {
    Evhd_handle_menu (apid);

    ret.eventout.events &= ~MU_MENU_BAR;
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
  }

  if (ret.eventout.events & MU_BUTTON) {
    Evhd_handle_button (apid,
                        ret.eventout.mb,
                        ret.eventout.mx,
                        ret.eventout.my);
  }

  if (eventin->events & MU_MESAG) {
    *msg = ret.msg;
  }

  *eventout = ret.eventout;
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
}


/*0x001a evnt_dclick*/
void	Evnt_dclick(AES_PB *apb) {
	if(apb->int_in[1] == EDC_SET) {
		clicktime = 320 - 60 * apb->int_in[0];
	};
	
	apb->int_out[0] = (320 - clicktime) / 60;
}
