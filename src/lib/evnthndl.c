/*
** evnthndl.c
**
** Copyright 1996 - 2000 Christer Gustavsson <cg@nocrew.org>
** Copyright 1996 Jan Paul Schmidt <Jan.P.Schmidt@mni.fh-giessen.de>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**  
** Read the file COPYING for more information.
*/

#define DEBUGLEVEL 0


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_BASEPAGE_H
#include <basepage.h>
#endif

#include <errno.h>

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#ifdef HAVE_IOCTL_H
#include <ioctl.h>
#endif

#ifdef HAVE_MINTBIND_H
#include <mintbind.h>
#endif

#ifdef HAVE_OSBIND_H
#include <osbind.h>
#endif

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "aesbind.h"
#include "appl.h"
#include "debug.h"
#include "evnt.h"
#include "evnthndl.h"
#include "lib_global.h"
#include "graf.h"
#include "mintdefs.h"
#include "mesagdef.h"
#include "lib_misc.h"
#include "objc.h"
#include "resource.h"
#include "types.h"
#include "wind.h"

#ifdef HAVE_SYSVARS_H
#include <sysvars.h>
#endif


#define EVHD_WAITTIME 200

#define EACCESS 36


/* Menu actions */
typedef enum {
  HM_NO,         /* No action          */
  HM_KILL,       /* Kill application   */
  HM_OPEN_ACC,   /* Open accessory     */
  HM_TOP_APPL,   /* Top application    */
  HM_MENU_MSG    /* Menu item selected */
}HM_ACTION;

typedef struct {
  HM_ACTION action;  /* Action to take                     */
  WORD      apid;    /* Application affected by the action */
  OBJECT    *tree;
  WORD      title;
  WORD      item;
  WORD      parent;
}HM_BUFFER;

/****************************************************************************
 * Module global variables                                                  *
 ****************************************************************************/

static struct {
  WORD       evid;
  WORD       lastervalid;
  EVNTREC    last_evntrec;
  WORD       mousefd;
  ULONG      lasttime;
}evntglbl = {
  -1,
  0,
  {-1,-1},
  -1,
  0UL
};

static struct {
  RECT   area;
}menu = {
  {0, 0, 0, 0}
};

static WORD iconify_x = 0;


/*
** Description
** Handle mouse button click on window arrow
**
** ToDo
** Fix repeat
**
** 1998-12-20 CG
** 1999-01-11 CG
** 1999-04-09 CG
** 1999-04-10 CG
** 1999-05-24 CG
*/
static
void
handle_arrow_click (WORD apid,
                    WORD win_id,
                    WORD object,
                    WORD msg) {
  COMMSG    mesag;
  WORD      owner;
  WORD      dummy;
        
  Wind_do_update (apid, BEG_MCTRL);
  
  mesag.type = WM_ARROWED;
  mesag.sid = 0;
  mesag.length = 0;
  mesag.msg0 = win_id;
  mesag.msg1 = msg;
  
  if(object) {
    Graf_do_mouse (apid, M_OFF, NULL);
    Wind_change (apid, win_id, object, SELECTED);
    Graf_do_mouse (apid, M_ON, NULL);
  }
  
  Wind_do_get (apid,
               win_id,
               WF_OWNER,
               &owner,
               &dummy,
               &dummy,
               &dummy,
               TRUE);
  
  Evnt_do_button (apid,
                  1,
                  LEFT_BUTTON,
                  0,
                  &dummy,
                  &dummy,
                  &dummy,
                  &dummy);
  
  if(object) {
    Graf_do_mouse (apid, M_OFF, NULL);
    Wind_change (apid, win_id, object,0);
    Graf_do_mouse (apid, M_ON, NULL);
  }
  
  Appl_do_write (apid, owner,16,&mesag);
  
  Wind_do_update (apid, END_MCTRL);
}


/*
** Description
** Handle click on window mover
*/
static
void
handle_mover_click (WORD apid,
                    WORD win_id,
                    WORD mouse_x,
                    WORD mouse_y)
{
  WORD      dummy;
  WORD      owner;
  WORD      top;
  WORD      last_x = mouse_x;
  WORD      last_y = mouse_y;
  EVNTREC   er;
  COMMSG    mesag;
  GLOBAL_COMMON * globals = get_global_common ();
  EVENTIN         ei =
  {
    MU_BUTTON | MU_TIMER,
    0,
    LEFT_BUTTON,
    0,
    0,
    {0,0,0,0},
    0,
    {0,0,0,0},
    200,
    0
  };

  EVENTOUT eo;
  COMMSG   buffer;

  Wind_do_update (apid, BEG_MCTRL);

  Wind_do_get(apid,
              win_id,
              WF_OWNER,
              &owner,
              &dummy,
              &top,
              &dummy,
              TRUE);
  
  Graf_do_mouse (apid, M_OFF, NULL);
  Wind_change (apid, win_id, W_NAME, SELECTED); 
  Graf_do_mouse (apid, M_ON, NULL);

  Evnt_do_multi(apid, &ei, &buffer, &eo, 0, DONT_HANDLE_MENU_BAR);

  if(eo.events & MU_BUTTON)
  {
    static COMMSG m;

    if(top == -1)
    {
      m.type = WM_BOTTOM;
    }
    else
    {
      m.type = WM_TOPPED;
    }
    
    m.sid = 0;
    m.length = 0;
    m.msg0 = win_id;
    
    Appl_do_write (apid, owner, 16, &m);
  }
  else
  {
    Graf_do_mouse (apid, FLAT_HAND, NULL);
        
    if (FALSE /*globals.realmove*/) { /* FIXME */
      mesag.type = WM_MOVED;
      mesag.sid = 0;
      mesag.length = 0;
      mesag.msg0 = win_id;
      
      Wind_do_get (apid,
                   win_id,
                   WF_CURRXYWH,
                   &mesag.msg1,
                   &mesag.msg2,
                   &mesag.msg3,
                   &mesag.msg4,
                   TRUE);
      
      do
      {
        if((last_x != mouse_x) || (last_y != mouse_y)) {
          WORD tmpy;
        
          mesag.msg1 += mouse_x - last_x;
          mesag.msg2 += mouse_y - last_y;
          
          last_x = mouse_x;
          last_y = mouse_y;
          
          tmpy = mesag.msg2;

          if(mesag.msg2 < (0/*globals.clheight*/ + 3)) { /* FIXME */
            mesag.msg2 = 0 /*globals.clheight*/ + 3;
          }
  
          Appl_do_write (apid, owner,16,&mesag);
          
          mesag.msg2 = tmpy;
        }
        
        /* FIXME
        while(1) {
          if(get_evntpacket(&er,(WORD)waittime) == 0) {
            er.ap_event = -1;
            break;
          };
          
          if(er.ap_event == APPEVNT_TIMER) {
            if((waittime -= (WORD)er.ap_value) <= 0) {
              break;
            };
          }
          else {
            update_local_mousevalues(&er);
          };
          
          if((er.ap_event == APPEVNT_BUTTON) &&
             !(er.ap_value & LEFT_BUTTON)) {
            break;
          };
        };
        */
      } while(!((er.ap_event == APPEVNT_BUTTON) &&
               !(er.ap_value & LEFT_BUTTON)));
    } else { /* No realtime moving */
      RECT bound;
      RECT winsize;
      
      bound.y = globals->clheight + 3;
      bound.height = 30000;
      bound.x = -15000;
      bound.width = 30000;
      
      Wind_do_update (apid, BEG_UPDATE);

      Wind_do_get (apid,
                   win_id,
                   WF_CURRXYWH,
                   &winsize.x,
                   &winsize.y,
                   &winsize.width,
                   &winsize.height,
                   TRUE);

      Graf_do_dragbox (apid,
                       winsize.width,
                       winsize.height,
                       winsize.x,
                       winsize.y,
                       &bound,
                       &mesag.msg1,
                       &mesag.msg2);
      
      mesag.type = WM_MOVED;
      mesag.sid = 0;
      mesag.length = 0;
      mesag.msg0 = win_id;

      /*
      if(mesag.msg2 < (globals.clheight + 3)) {
        mesag.msg2 = globals.clheight + 3;
      }
      */

      mesag.msg3 = winsize.width;
      mesag.msg4 = winsize.height;
      
      Appl_do_write (apid, owner, MSG_LENGTH,&mesag);
                
      Wind_do_update (apid, END_UPDATE);
    }
    
    Graf_do_mouse(apid, M_RESTORE,NULL);
  }
        
  Graf_do_mouse (apid, M_OFF, NULL);
  Wind_change (apid, win_id, W_NAME, 0);
  Graf_do_mouse (apid, M_ON, NULL);

  Wind_do_update (apid, END_MCTRL);
}


/*
** Description
** Handle window sizer click
*/
static
inline
void
handle_sizer_click (WORD apid,
                    WORD win_id,
                    WORD owner,
                    WORD mouse_x,
                    WORD mouse_y) {
  EVNTREC er;
  RECT    totsize;
  COMMSG  mesag;
  
  Wind_do_update (apid, BEG_MCTRL);
  Graf_do_mouse (apid, M_OFF, NULL);
  Wind_change (apid, win_id, W_SIZER, SELECTED);
  Graf_do_mouse (apid, M_ON, NULL);
  Wind_do_get (apid,
               win_id,
               WF_CURRXYWH,
               &totsize.x,
               &totsize.y,
               &totsize.width,
               &totsize.height,
               TRUE);
  
  if (FALSE /*globals.realsize*/) { /* FIXME */
    WORD    last_x;
    WORD    last_y;
    WORD    offsx = mouse_x - totsize.x - totsize.width;
    WORD    offsy = mouse_y - totsize.y - totsize.height;
    
    mesag.type = WM_SIZED;
    mesag.sid = 0;
    mesag.length = 0;
    mesag.msg0 = win_id;
    mesag.msg1 = totsize.x;
    mesag.msg2 = totsize.y;
    
    last_x = mouse_x;
    last_y = mouse_y;
    
    do {
      LONG  waittime = 200;
      
      if((last_x != mouse_x) || (last_y != mouse_y)) {
        mesag.msg3 = mouse_x - totsize.x - offsx;
        mesag.msg4 = mouse_y - totsize.y - offsy;
        
        last_x = mouse_x;
        last_y = mouse_y;
        
        Appl_do_write (apid, owner,16,&mesag);
      }
      
      /* FIXME
      while (TRUE) {
        if(get_evntpacket(&er,(WORD)waittime) == 0) {
          er.ap_event = -1;
          break;
        }
        
        if(er.ap_event == APPEVNT_TIMER) {
          if((waittime -= er.ap_value) <= 0) {
            break;
          }
        } else {
          update_local_mousevalues(&er);
        }
        
        if((er.ap_event == APPEVNT_BUTTON) &&
           !(er.ap_value & LEFT_BUTTON)) {
          break;
        }
      }
      */
    }while(!((er.ap_event == APPEVNT_BUTTON) &&
             !(er.ap_value & LEFT_BUTTON)));
  } else {
    Graf_do_rubberbox (apid,
                       totsize.x,
                       totsize.y,
                       100,
                       100,
                       &mesag.msg3,
                       &mesag.msg4);
    
    mesag.type = WM_SIZED;
    mesag.sid = 0;
    mesag.length = 0;
    mesag.msg0 = win_id;
    mesag.msg1 = totsize.x;
    mesag.msg2 = totsize.y;
    mouse_x = mesag.msg3 + totsize.x - 1;
    mouse_y = mesag.msg4 + totsize.y - 1;
    
    Appl_do_write (apid, owner, 16, &mesag);
  }
  
  Wind_do_update (apid, BEG_UPDATE);
  Graf_do_mouse (apid, M_OFF, NULL);
  Wind_change (apid, win_id, W_SIZER, 0);
  Graf_do_mouse (apid, M_ON, NULL);
  Wind_do_update (apid, END_UPDATE);
  Wind_do_update (apid, END_MCTRL);
}


/*
** Description
** Handle click on smaller window element
*/
static
inline
void
handle_smaller_click (WORD     apid,
		      WORD     win_id,
		      WORD     owner,
		      OBJECT * tree)
{
  COMMSG  mesag;

  Wind_do_update (apid, BEG_MCTRL);

  if(Graf_do_watchbox(apid, tree, WSMALLER, SELECTED, 0))
  {
    WORD            dummy;
    WORD            skeys;
    GLOBAL_COMMON * globals = get_global_common();
    
    Graf_do_mkstate(apid, &dummy, &dummy, &dummy, &skeys);
    
    if(skeys & K_CTRL)
    {
      mesag.type = WM_ALLICONIFY;
    }
    else
    {
      mesag.type = WM_ICONIFY;
    }
      
    mesag.sid = 0;
    mesag.length = 0;
    mesag.msg0 = win_id;
    mesag.msg3 = globals->icon_width;
    mesag.msg4 = globals->icon_height;
    mesag.msg1 = iconify_x;
    mesag.msg2 = globals->screen.height - mesag.msg4;
    
    iconify_x += mesag.msg3;
    if(iconify_x + mesag.msg3 > globals->screen.width)
    {
      iconify_x = 0;
    }
    
    DEBUG2("Sending iconify message to %d from %d", owner, apid);
    Appl_do_write(apid, owner, 16, &mesag);
  }

  Graf_do_mouse(apid, M_OFF, NULL);
  Wind_change(apid, win_id, W_SMALLER, 0);
  Graf_do_mouse(apid, M_ON, NULL);

  Wind_do_update (apid, END_MCTRL);
}


/*
** Description
** Handle click on window button element
*/
static
inline
void
handle_window_button_click(WORD     apid,
			   WORD     win_id,
			   WORD     owner,
			   OBJECT * tree,
			   WORD     obj,
			   WORD     log_obj,
			   WORD     message_type)
{
  COMMSG  mesag;

  Wind_do_update (apid, BEG_MCTRL);

  if(Graf_do_watchbox(apid, tree, obj, SELECTED, 0)) {
    mesag.type = message_type;
    mesag.sid = 0;
    mesag.length = 0;
    mesag.msg0 = win_id;
    Appl_do_write(apid, owner, 16, &mesag);
  }

  Graf_do_mouse(apid, M_OFF, NULL);
  Wind_change(apid, win_id, log_obj, 0);
  Graf_do_mouse(apid, M_ON, NULL);

  Wind_do_update (apid, END_MCTRL);
}


/*
** Description
** Handle mouse button click on window slider
**
** 1998-12-20 CG
** 1999-01-01 CG
** 1999-01-11 CG
** 1999-04-09 CG
** 1999-04-10 CG
** 1999-05-24 CG
*/
static
void
handle_slider_click (WORD     apid,
                     WORD     win_id,
                     WORD     elem,
                     WORD     owner,
                     OBJECT * tree,
                     WORD     mouse_x,
                     WORD     mouse_y)
{
  WORD last_x = mouse_x, last_y = mouse_y, new_x = last_x, new_y = last_y;
  WORD waittime = EVHD_WAITTIME;
  WORD bg = (elem == WVSLIDER)? WVSB : WHSB;
  WORD dx,dy;

  EVNTREC   er;
  COMMSG    mesag;
  WORD      xyarray[10];
  RECT      elemrect,bgrect;

  WORD *p_mousexy    = (elem == WHSLIDER)? &mouse_x        : &mouse_y;
  WORD *p_lastxy     = (elem == WHSLIDER)? &last_x         : &last_y;
  WORD *p_newxy      = (elem == WHSLIDER)? &new_x          : &new_y;
  WORD *p_dxy        = (elem == WHSLIDER)? &dx             : &dy;
  WORD *p_elemrectwh = (elem == WHSLIDER)? &elemrect.width : &elemrect.height;
  WORD *p_bgrectxy   = (elem == WHSLIDER)? &bgrect.x       : &bgrect.y;
  WORD *p_bgrectwh   = (elem == WHSLIDER)? &bgrect.width   : &bgrect.height;

  WORD widget = (elem == WHSLIDER) ? W_HELEV : W_VELEV;

  Graf_do_mouse (apid, FLAT_HAND, NULL);

  Objc_do_offset(tree,elem,(WORD *)&elemrect);
  elemrect.width = tree[elem].ob_width;
  elemrect.height = tree[elem].ob_height;

  dx = last_x - elemrect.x;
  dy = last_y - elemrect.y;

  Objc_do_offset(tree,bg,(WORD *)&bgrect);
  bgrect.width = tree[bg].ob_width;
  bgrect.height = tree[bg].ob_height;

  Graf_do_mouse (apid, M_OFF, NULL);
  Wind_change (apid, win_id, widget, SELECTED);
  Graf_do_mouse (apid, M_ON, NULL);

  if(0 /*globals.realslide*/) {
    /* FIXME
    while(1) {
      get_evntpacket(&er,0);
      update_local_mousevalues(&er);

      if((er.ap_event == APPEVNT_BUTTON) &&
         !(er.ap_value & LEFT_BUTTON)) {
        break;
      }
      
      if((*p_mousexy - *p_dxy + *p_elemrectwh) > (*p_bgrectxy + *p_bgrectwh)) {
        *p_newxy = *p_bgrectxy + *p_bgrectwh - *p_elemrectwh + *p_dxy;
      }
      else {
        if((*p_mousexy - *p_dxy) < *p_bgrectxy) {
          *p_newxy = *p_bgrectxy + *p_dxy;
        }
        else {
          *p_newxy = *p_mousexy;
        }
      }

      if(*p_newxy != *p_lastxy) {
        mesag.type = (elem == WHSLIDER)? WM_HSLID : WM_VSLID;         
        mesag.sid = 0;
        mesag.length = 0;
        mesag.msg0 = win_id;
        mesag.msg1 = (*p_bgrectwh == *p_elemrectwh) ? 0 :
          (WORD)((((LONG)*p_newxy - (LONG)*p_dxy -
                   (LONG)*p_bgrectxy) * (LONG)1000L) / 
                 ((LONG)*p_bgrectwh - (LONG)*p_elemrectwh));
        
        Appl_do_write (apid, owner,16,&mesag);

        while(1) {
          if(get_evntpacket(&er, waittime) == 0) {
            er.ap_event = -1;
            break;
          }
          
          if(er.ap_event == APPEVNT_TIMER) {
            if((waittime -= (WORD)er.ap_value) <= 0) {
              break;
            }
          }
          else {
            update_local_mousevalues(&er);
          }

          if((er.ap_event == APPEVNT_BUTTON) &&
             !(er.ap_value & LEFT_BUTTON)) {
            break;
          }

          *p_lastxy = *p_newxy;
        }
      }
    }
    */
  }
  else
  {
    Wind_do_update (apid, BEG_UPDATE);

    mesag.msg1 = Graf_do_slidebox (apid,
                                   tree,
                                   (elem == WHSLIDER) ? WHSB : WVSB,
                                   elem,
                                   (elem == WHSLIDER) ? 0 : 1);
    

    mesag.type = (elem == WHSLIDER)? WM_HSLID : WM_VSLID;      
    mesag.sid = 0;
    mesag.length = 0;
    mesag.msg0 = win_id;
    
    Appl_do_write (apid, owner, 16, &mesag);

    Wind_do_update (apid, END_UPDATE);
  }

  Graf_do_mouse (apid, M_OFF, NULL);
  Wind_change (apid, win_id, widget, 0);
  Graf_do_mouse (apid, M_ON, NULL);

  Graf_do_mouse (apid, M_RESTORE, NULL);  
}


/*
** Description
** Count number of clicks in a time set by evnt_dclick ()
**
** 1999-08-20 CG
*/
static
WORD
count_clicks (WORD apid,
	      WORD bclicks,
	      WORD bmask,
	      WORD bstate) {
  WORD           clicks_out = 0;
  static EVENTIN ei = {
    MU_BUTTON | MU_TIMER,
    0,
    0,
    0,
    0,
    {0,0,0,0},
    0,
    {0,0,0,0},
    200,
    0
  };

  EVENTOUT eo;
  COMMSG   buffer;

  while (bclicks > 0) {
    /* When this routine is entered the state will be true */
    ei.bmask = bmask;
    ei.bstate = (~bstate) & bmask;
    Evnt_do_multi (apid, &ei, &buffer, &eo, 0, DONT_HANDLE_MENU_BAR);
    
    if ((eo.events & MU_BUTTON) == 0) {
      break;
    }
    
    ei.bmask = bmask;
    ei.bstate = bstate;
    Evnt_do_multi (apid, &ei, &buffer, &eo, 0, DONT_HANDLE_MENU_BAR);
    
    if (eo.events & MU_BUTTON) {
      bclicks--;
      clicks_out++;
    } else {
      break;
    }
  }

  if (clicks_out > 0) {
    DEBUG2 ("evnthndl.c: count_clicks = %d", clicks_out);
  }

  return clicks_out;
}


/*
** Description
** Check user click against pattern
**
** FIXME: Count number of clicks and return them
**
** 1999-05-24 CG
** 1999-08-17 CG
** 1999-08-20 CG
*/
static
WORD
handle_user_click (WORD   apid,
		   WORD   mouse_button,
		   WORD   bclicks,
                   WORD   bmask,
                   WORD   bstate,
		   WORD * mc) {
  if (bclicks & 0x100) {
    if (mouse_button & bmask) {
      *mc = *mc + count_clicks (apid,
				(bclicks & 0xff) - *mc,
				bmask & mouse_button,
				mouse_button);
      return MU_BUTTON;
    } else {
      return 0;
    }
  } else if ((mouse_button & bmask) == bstate) {
    *mc = *mc + count_clicks (apid,
			      bclicks - *mc,
			      bmask,
			      bstate);
    return MU_BUTTON;
  } else {
    return 0;
  }
}


/*
** Description
** Handle mouse click on window element
*/
static
inline
WORD
handle_window_element_click(WORD   apid,
			    WORD   win_id,
			    WORD   owner,
			    WORD   top,
			    WORD   mouse_button,
			    WORD   mouse_x,
			    WORD   mouse_y,
			    WORD   bclicks,
			    WORD   bmask,
			    WORD   bstate,
			    WORD * mc)
{
  WORD     obj;
  OBJECT * tree;
  COMMSG   mesag;
  
  tree = Wind_get_rsrc (apid, win_id);
  obj = Objc_do_find (tree, 0, 9, mouse_x, mouse_y, 0);
  
  switch(obj)
  {
  case WCLOSER :
    handle_window_button_click(apid,
			       win_id,
			       owner,
			       tree,
			       WCLOSER,
			       W_CLOSER,
			       WM_CLOSED);
    return 0;
    
  case WSMALLER:
    handle_smaller_click(apid, win_id, owner, tree);
    return 0;
  
  case WFULLER:
    handle_window_button_click(apid,
			       win_id,
			       owner,
			       tree,
			       WFULLER,
			       W_FULLER,
			       WM_FULLED);
    return 0;
    
  case WSIZER:
    handle_sizer_click (apid, win_id, owner, mouse_x, mouse_y);
    return 0;
    
  case WAPP:            
  case WMOVER:
    handle_mover_click (apid, win_id, mouse_x, mouse_y);
    return 0;
    
  case WLEFT:
    handle_arrow_click (apid, win_id,W_LFARROW,WA_LFLINE);
    return 0;
    
  case WRIGHT:
    handle_arrow_click (apid, win_id,W_RTARROW,WA_RTLINE);
    return 0;
    
  case WUP:
    handle_arrow_click (apid, win_id,W_UPARROW,WA_UPLINE);
    return 0;
    
  case WDOWN:
    handle_arrow_click (apid, win_id,W_DNARROW,WA_DNLINE);
    return 0;
    
  case WHSB:
  {
    WORD xy[2];
    
    if(tree) {
      Objc_do_offset (tree, WHSLIDER, xy);
    }
    
    if(mouse_x > xy[0]) {
      handle_arrow_click (apid, win_id, 0, WA_RTPAGE);
    } else {
      handle_arrow_click (apid, win_id, 0, WA_LFPAGE);
    }
  }
  return 0;
  
  case WVSB:
  {
    WORD xy[2];
    
    if (tree) {
      Objc_do_offset (tree, WVSLIDER, xy);
    }
    
    if (mouse_y > xy[1]) {
      handle_arrow_click (apid, win_id, 0, WA_DNPAGE);
    } else {
      handle_arrow_click (apid, win_id, 0, WA_UPPAGE);
    }
  }
  return 0;
  
  case WVSLIDER:
  case WHSLIDER:
    handle_slider_click (apid, win_id, obj, owner, tree, mouse_x, mouse_y);
    return 0;
    
  default:
    if(win_id > 0) {
      COMMSG    m;
      
      if(top != -1) {
        /*
          Vdi_vq_mouse(globals.vid,&mouse_button,&mouse_x,&mouse_y);
        */
        
        if(mouse_button & LEFT_BUTTON) {
          /* FIXME: Replace with evnt_multi
          do {
            get_evntpacket(&er,0);
            update_local_mousevalues(&er);
          }while(!((er.ap_event == APPEVNT_BUTTON) &&
                   !(er.ap_value & LEFT_BUTTON)));
          */

          m.type = WM_TOPPED;
          m.sid = 0;
          m.length = 0;
          m.msg0 = win_id;
          
          Appl_do_write (apid, owner,16,&m);

          return 0;
        }
      }
    }
  }
  
  return handle_user_click (apid,
			    mouse_button,
			    bclicks,
			    bmask,
			    bstate,
			    mc);
}


/*
** Exported
*/
WORD
Evhd_handle_button (WORD   apid,
                    WORD   mouse_button,
                    WORD   mouse_x,
                    WORD   mouse_y,
		    WORD   bclicks,
                    WORD   bmask,
                    WORD   bstate,
		    WORD * mc,
                    WORD   handle_user_click_only)
{
  EVNTREC     er;

  if(!handle_user_click_only &&
     (mouse_button & LEFT_BUTTON))
  {
    WORD     win_id;
    WORD     owner;
    WORD     top;
    WORD     dummy;
    WORD     status;
    RECT     worksize;
    
    win_id = Wind_do_find (apid, mouse_x, mouse_y);

    Wind_do_get (apid,
                 win_id,
                 WF_OWNER,
                 &owner,
                 &status,
                 &top,
                 &dummy,
                 TRUE);

    Wind_do_get (apid,
                 win_id,
                 WF_WORKXYWH,
                 &worksize.x,
                 &worksize.y,
                 &worksize.width,
                 &worksize.height,
                 TRUE);

    if (status & (WIN_DESKTOP | WIN_DIALOG)) {
      /* Return the click to the user */
      return handle_user_click (apid,
				mouse_button,
				bclicks,
                                bmask,
                                bstate,
				mc);
    } else if (Misc_inside (&worksize, mouse_x, mouse_y)) {
      if((status & (WIN_UNTOPPABLE | WIN_ICONIFIED)) == WIN_UNTOPPABLE) {
        /* Return the click to the user */
        return handle_user_click (apid,
				  mouse_button,
				  bclicks,
                                  bmask,
                                  bstate,
				  mc);
      } else {
        COMMSG      m;
        
        if((status & WIN_ICONIFIED) &&
           (Evnt_do_button(apid,
                           1,
                           LEFT_BUTTON,
                           LEFT_BUTTON,
                           &dummy,
                           &dummy,
                           &dummy,
                           &dummy) >= 1))
        {
          m.type = WM_UNICONIFY;
          m.sid = 0;
          m.length = 0;
          m.msg0 = win_id;
          Wind_do_get (apid,
                       win_id,
                       WF_UNICONIFY,
                       &m.msg1,
                       &m.msg2,
                       &m.msg3,
                       &m.msg4,
                       TRUE);
          
          Appl_do_write (apid, owner,(WORD)sizeof(COMMSG),&m);
          
          return 0;
        } else if(top != -1) {
          /*
            Vdi_vq_mouse(globals.vid,&mouse_button,&mouse_x,&mouse_y);
          */
          
          /* FIXME
          if(mouse_button & LEFT_BUTTON) {
            do {
              get_evntpacket(&er,0);
              update_local_mousevalues(&er);
            } while(!((er.ap_event == APPEVNT_BUTTON) &&
                      !(er.ap_value & LEFT_BUTTON)));
          }
          */
          
          m.type = WM_TOPPED;
          m.sid = 0;
          m.length = 0;
          m.msg0 = win_id;
          
          Appl_do_write (apid, owner,16,&m);
          return 0;
        } else {
          return handle_user_click (apid,
				    mouse_button,
				    bclicks,
                                    bmask,
                                    bstate,
				    mc);
        }
      }
    } else { /* Click on a window element */
      return handle_window_element_click (apid,
                                          win_id,
                                          owner,
                                          top,
                                          mouse_button,
                                          mouse_x,
                                          mouse_y,
					  bclicks,
					  bmask,
					  bstate,
					  mc);
    }
  } else { /* not handle & LEFT_BUTTON */
    return handle_user_click (apid,
			      mouse_button,
			      bclicks,
                              bmask,
                              bstate,
			      mc);
  }
}


/*
** Description
** Update list of running applications and accessories
*/
static
void
update_appl_list (WORD   apid,
                  WORD * topped_application)
{
  GLOBAL_APPL * globals = get_globals (apid);
  WORD          more;
  char          name[20];
  WORD          type;
  WORD          ap_id;

  *topped_application = -1;

  /* Reset menu lists */
  globals->appl_menu.count = 0;
  globals->acc_menu.count = 0;

  more = Appl_do_search (apid, APP_FIRST, name, &type, &ap_id);
  while(TRUE)
  {
    if (type & (APP_APPLICATION | APP_ACCESSORY))
    {
      APPL_LIST * insert_menu;

      if(type & APP_APPLICATION)
      {
        insert_menu = &globals->appl_menu;

        /* For now the first application is the topped application */
        if(*topped_application == -1)
        {
          *topped_application = ap_id;
        }
      }
      else
      { /* Must be APP_ACCESSORY */
        insert_menu = &globals->acc_menu;
      }

      if (insert_menu->count == insert_menu->size) {
        insert_menu->size += 10;
        insert_menu->entries =
          (APPL_ENTRY *)realloc (insert_menu->entries,
                                 sizeof (APPL_ENTRY) * insert_menu->size);
      }

      strcpy(insert_menu->entries[insert_menu->count].name, name);
      insert_menu->entries[insert_menu->count].type = type;
      insert_menu->entries[insert_menu->count].ap_id = ap_id;
      insert_menu->count++;
    }

    if (more == 0) {
      break;
    }
    
    more = Appl_do_search (apid, APP_NEXT, name, &type, &ap_id);
  }
}


/*
** Description
** Update the application menu
**
** ToDo
** Fix separation of accessories and applications
*/
static
WORD
update_appl_menu(WORD apid)
{
  WORD          rwalk;
  WORD          topappl;
  GLOBAL_APPL * globals = get_globals (apid);
  int           i;

  update_appl_list(apid, &topappl);

  rwalk = PMENU_FIRST;

  for (i = 0; i < globals->appl_menu.count; i++, rwalk++) {
    strcpy (globals->common->pmenutad[rwalk].ob_spec.free_string,
            globals->appl_menu.entries[i].name);

    if (globals->appl_menu.entries[i].ap_id == topappl)
    {
      OB_STATE_SET(&globals->common->pmenutad[rwalk], CHECKED);
    }
    else
    {
      OB_STATE_CLEAR(&globals->common->pmenutad[rwalk], CHECKED);
    }
	
    OB_FLAGS_CLEAR(&globals->common->pmenutad[rwalk], HIDETREE);
    OB_FLAGS_CLEAR(&globals->common->pmenutad[rwalk], DISABLED);
  }

  if (globals->acc_menu.count > 0) {
    strcpy(globals->common->pmenutad[rwalk].ob_spec.free_string,
           "----------------------");
    OB_FLAGS_CLEAR(&globals->common->pmenutad[rwalk], HIDETREE);
    OB_STATE_CLEAR(&globals->common->pmenutad[rwalk], CHECKED);
    OB_STATE_SET(&globals->common->pmenutad[rwalk], DISABLED);
    rwalk++;
    
    for (i = 0; i < globals->acc_menu.count; i++, rwalk++) {
      strcpy (globals->common->pmenutad[rwalk].ob_spec.free_string,
              globals->acc_menu.entries[i].name);

      if (globals->acc_menu.entries[i].ap_id == topappl)
      {
        OB_STATE_SET(&globals->common->pmenutad[rwalk], CHECKED);
      }
      else
      {
        OB_STATE_CLEAR(&globals->common->pmenutad[rwalk], CHECKED);
      }
      
      OB_FLAGS_CLEAR(&globals->common->pmenutad[rwalk], HIDETREE);
      OB_STATE_CLEAR(&globals->common->pmenutad[rwalk], DISABLED);
    }
  }

  /* FIXME: Make pmenutad local to the application */
  OB_FLAGS_SET(&globals->common->pmenutad[rwalk], HIDETREE);
	
  globals->common->pmenutad[0].ob_height =
    globals->common->pmenutad[rwalk].ob_y;
	
  return topappl;
}


static WORD     get_matching_menu(OBJECT *t,WORD n) {
  WORD parent,start,i = 0;
        
  /*first we need to know which in order our title is*/
        
  parent = t[t[0].ob_head].ob_head;
  start = t[parent].ob_head;
        
  while(start != n) {
    /* we have failed to find the object! */
                
    if(start == parent)
      return -1;

    start = t[start].ob_next;
    i++;
  };
        
  /* now we shall find the i:th menubox! */
        
  parent = t[t[0].ob_head].ob_next;
        
  start = t[parent].ob_head;
        
  while(i) {
    start = t[start].ob_next;

    if(start == parent)
      return -1;
                        
    i--;
  };
        
  return start;
}


/*
** Description
** Handle drop down menu
**
** 1999-01-09 CG
** 1999-04-10 CG
** 1999-04-13 CG
** 1999-04-18 CG
** 1999-08-17 CG
*/
static
WORD
handle_drop_down (WORD        apid,
                  WORD        mx,
                  WORD        my,
                  HM_BUFFER * hm_buffer,
                  WORD        menubox,
                  WORD        title) {
  WORD     entry;
  WORD     deskbox;
  GLOBAL_APPL * globals = get_globals (apid);
  OBJECT * nmenu;
  
  deskbox = globals->menu[globals->menu[0].ob_tail].ob_head;

  if ((deskbox == menubox) && (my >= globals->common->pmenutad[0].ob_y)) {
    nmenu = globals->common->pmenutad;
    entry = Objc_do_find (nmenu, 0, 9, mx, my, 0);
  } else {
    nmenu = globals->menu;
    entry = Objc_do_find (nmenu, menubox, 9, mx, my, 0);
  }

  if (entry >= 0) {
    EVENTIN ei = {
      MU_BUTTON | MU_M1,
      0,
      LEFT_BUTTON,
      LEFT_BUTTON,
      MO_LEAVE,
      {0,0,1,1},
      0,
      {0,0,0,0},
      0,
      0
    };
    COMMSG        buffer;
    EVENTOUT      eo;

    Objc_area_needed (nmenu, entry, &ei.m1r);
    
    if (!(OB_STATE(&nmenu[entry]) & DISABLED) && (entry != 0))
    {
      /* select the entry and update it */

      OB_STATE_SET(&nmenu[entry], SELECTED);
      
      Graf_do_mouse (apid, M_OFF, NULL);
      Objc_do_draw (globals->vid, nmenu, 0, 9, &ei.m1r);
      Graf_do_mouse (apid, M_ON, NULL);
    }

    while(TRUE) {
      Evnt_do_multi (apid, &ei, &buffer, &eo, 0, DONT_HANDLE_MENU_BAR);

      if (eo.events & MU_M1) {
        if(!(OB_STATE(&nmenu[entry]) & DISABLED))
        {
          OB_STATE_CLEAR(&nmenu[entry], SELECTED);
          
          Graf_do_mouse (apid, M_OFF, NULL);
          Objc_do_draw (globals->vid, nmenu, 0, 9, &ei.m1r);
          Graf_do_mouse (apid, M_ON, NULL);
        }
        
        return 0;
      } else if (eo.events & MU_BUTTON) {
	WORD dummy;

	/* Wait for button to be released */
	Evnt_do_button (apid,
			1,
			LEFT_BUTTON,
			0,
			&dummy,
			&dummy,
			&dummy,
			&dummy);

        OB_STATE_CLEAR(&nmenu[entry], SELECTED);
                
        if (nmenu == globals->common->pmenutad)
	{
          APPL_ENTRY * appl_entry;
          WORD         new_entry = entry - PMENU_FIRST;

	  DEBUG2("new_entry = %d appl %d acc %d",
		 new_entry,
		 globals->appl_menu.count,
		 globals->acc_menu.count);
          if(new_entry < globals->appl_menu.count)
	  {
            appl_entry = &globals->appl_menu.entries[new_entry];
          }
	  else
	  {
            new_entry -= globals->appl_menu.count + 1;
            if(new_entry < globals->acc_menu.count)
	    {
              appl_entry = &globals->acc_menu.entries[new_entry];
            }
	    else
	    {
              appl_entry = NULL;
            }
          }
                                                
          if(appl_entry != NULL)
	  {
#if 0 /* FIXME */
	    int skeys;
               
	    Vdi_vq_key_s(evntglbl.evid,&skeys);
	    
	    if(skeys & K_CTRL)
	    {
	      hm_buffer->action = HM_KILL;
	      hm_buffer->apid = appl_entry->ap_id;
	    }
	    else
#endif
	    {
              if(appl_entry->type & APP_APPLICATION)
	      {
		DEBUG2("APPL top");
                hm_buffer->action = HM_TOP_APPL;
                hm_buffer->apid = appl_entry->ap_id;
              }
	      else
	      {
		DEBUG2("ACC open");
                hm_buffer->action = HM_OPEN_ACC;
                hm_buffer->apid = appl_entry->ap_id;
              }
            }
          } else {
            DEBUG0("%s: Line %d: handle_drop_down:\r\n"
                   "Couldn't find application to top!\r\n",
                   __FILE__,__LINE__);
          }
        }
	else
	{
	  DEBUG2("HN_MENU_MSG");
          hm_buffer->action = HM_MENU_MSG;
          hm_buffer->title = title;
          hm_buffer->item = entry;
          hm_buffer->tree = nmenu;
          hm_buffer->parent = menubox;
        }
        
        return 1;
      }
    }
  }

  return 0;
}


/*
** Description
** Handle selected menu title
*/
static
WORD
handle_selected_title (WORD        apid,
                       HM_BUFFER * hm_buffer,
                       WORD        mouse_x,
                       WORD        mouse_y) {
  WORD          box;
  WORD          title;
  WORD          deskbox;
  GLOBAL_APPL * globals = get_globals (apid);
  EVENTIN       ei = {
    MU_M1,
    0,
    0,
    0,
    MO_LEAVE,
    {0,0,0,0},
    0,
    {0,0,0,0},
    0,
    0
  };
  COMMSG        buffer;
  EVENTOUT      eo;

#ifdef REALMOVE
  WORD          dropwin;
#else
  char * menu_buffer;
  MFDB   mfdb_src;
  MFDB   mfdb_dst;
  int    pxy[8];
#endif
      
  title = Objc_do_find (globals->menu, 0, 9, mouse_x, mouse_y, 0);
  box = get_matching_menu (globals->menu, title);
  
  deskbox = globals->menu[globals->menu[0].ob_tail].ob_head;
          
  if (box >= 0) {
    RECT area;
    RECT titlearea;
            
    /* select the title and update it */
            
    OB_STATE_SET(&globals->menu[title], SELECTED);

    Objc_area_needed (globals->menu, title, &titlearea);

    Graf_do_mouse (apid, M_OFF, NULL);
    Objc_do_draw (globals->vid, globals->menu, 0, 9, &titlearea);
    Graf_do_mouse (apid, M_ON, NULL);

    Objc_area_needed (globals->menu, box, &area);
    
    if(box == deskbox) {
      WORD i;

      /* Get applications for menu */
      update_appl_menu (apid);

      Objc_do_offset (globals->menu, box, &globals->common->pmenutad[0].ob_x);
     
      globals->common->pmenutad[0].ob_y +=
        (globals->menu[globals->menu[box].ob_head].ob_height << 1);
      globals->common->pmenutad[0].ob_width = globals->menu[box].ob_width;

      for(i = PMENU_FIRST; i <= PMENU_LAST; i++) {
        globals->common->pmenutad[i].ob_width =
          globals->common->pmenutad[0].ob_width;
      }
      
      area.height =
        globals->common->pmenutad[0].ob_height +
        (globals->common->pmenutad[1].ob_height << 1) + 2;
    }

#ifdef REALMOVE
    dropwin = Wind_do_create (apid, 0, &area, WIN_MENU);
    Wind_do_open (apid,
                  dropwin,
                  &area);
#else
    /*
    ** FIXME: Control that we don't lock the system by trying to get the
    ** semaphores.
    */
    Wind_do_update (apid, BEG_MCTRL);
    Wind_do_update(apid, BEG_UPDATE);
    menu_buffer = malloc(((area.width + 15) * area.height * 
                          globals->common->num_planes) / 8);

    pxy[0] = area.x;
    pxy[1] = area.y;
    pxy[2] = area.x + area.width - 1;
    pxy[3] = area.y + area.height - 1;
    pxy[4] = 0;
    pxy[5] = 0;
    pxy[6] = area.width - 1;
    pxy[7] = area.height - 1;
    mfdb_src.fd_addr = NULL;
    mfdb_dst.fd_addr = menu_buffer; /* FIXME */
    mfdb_dst.fd_w = area.width;
    mfdb_dst.fd_h = area.height;
    mfdb_dst.fd_wdwidth = (area.width + 15) / 16;
    mfdb_dst.fd_stand = 0;
    mfdb_dst.fd_nplanes = globals->common->num_planes;
    mfdb_dst.fd_r1 = 0;
    mfdb_dst.fd_r2 = 0;
    mfdb_dst.fd_r3 = 0;
    Graf_do_mouse (apid, M_OFF, NULL);
    vro_cpyfm (globals->vid, S_ONLY, pxy, &mfdb_src, &mfdb_dst);
    Graf_do_mouse (apid, M_ON, NULL);
#endif

    OB_FLAGS_CLEAR(&globals->menu[box], HIDETREE);

    if (box == deskbox) {
      RECT clip = area;

      clip.height = globals->common->pmenutad[0].ob_y - area.y;
      Objc_do_draw (globals->vid, globals->menu, box, 9, &clip);
      
      clip.y = globals->common->pmenutad[0].ob_y;
      clip.height = globals->common->pmenutad[0].ob_height + 1;
      Objc_do_draw (globals->vid, globals->common->pmenutad, 0, 9, &clip);
    } else {
      Graf_do_mouse (apid, M_OFF, NULL);
      Objc_do_draw (globals->vid, globals->menu, box, 9, &area);
      Graf_do_mouse (apid, M_ON, NULL);
    }

    ei.m1r = titlearea;

    /* Wait for mouse to leave title rectangle */
    while(TRUE)
    {
      Evnt_do_multi (apid, &ei, &buffer, &eo, 0, DONT_HANDLE_MENU_BAR);

      if (eo.events & MU_M1) {
        WORD closebox = 0;
        
        if ((deskbox == box) &&
            (eo.my >= globals->common->pmenutad[0].ob_y))
	{
          if (!Misc_inside ((RECT *)&globals->common->pmenutad[0].ob_x,
                            eo.mx,
                            eo.my) ||
              (Objc_do_find (globals->common->pmenutad,
                             0,
                             9,
                             eo.mx,
                             eo.my,
                             0) < 0) ||
              handle_drop_down (apid, eo.mx, eo.my, hm_buffer, box, title)) {
            closebox = 1;
          }
        }
	else if ((Objc_do_find (globals->menu,
				box,
				9,
				eo.mx,
				eo.my,
				0) < 0) ||
		 handle_drop_down (apid,
				   eo.mx,
				   eo.my,
				   hm_buffer,
				   box,
				   title))
	{
          closebox = 1;
        }

        if(closebox)
        {
          OB_STATE_CLEAR(&globals->menu[title], SELECTED);
          Graf_do_mouse (apid, M_OFF, NULL);
          Objc_do_draw (globals->vid, globals->menu, 0, 9, &titlearea);
          Graf_do_mouse (apid, M_ON, NULL);
          
#ifdef REALMOVE
          Wind_do_close (apid, dropwin);
          Wind_do_delete (apid, dropwin);
#else
	  pxy[0] = 0;
	  pxy[1] = 0;
	  pxy[2] = area.width - 1;
	  pxy[3] = area.height - 1;
	  pxy[4] = area.x;
	  pxy[5] = area.y;
	  pxy[6] = area.x + area.width - 1;
	  pxy[7] = area.y + area.height - 1;
	  mfdb_dst.fd_addr = NULL;
	  mfdb_src.fd_addr = menu_buffer; /* FIXME */
	  mfdb_src.fd_w = area.width;
	  mfdb_src.fd_h = area.height;
	  mfdb_src.fd_wdwidth = (area.width + 15) / 16;
	  mfdb_src.fd_stand = 0;
	  mfdb_src.fd_nplanes = globals->common->num_planes;
	  mfdb_src.fd_r1 = 0;
	  mfdb_src.fd_r2 = 0;
	  mfdb_src.fd_r3 = 0;
	  Graf_do_mouse (apid, M_OFF, NULL);
	  vro_cpyfm (globals->vid, S_ONLY, pxy, &mfdb_src, &mfdb_dst);
	  Graf_do_mouse (apid, M_ON, NULL);
	  Wind_do_update (apid, END_UPDATE);
          Wind_do_update (apid, END_MCTRL);
	  free(menu_buffer);
#endif

          OB_FLAGS_SET(&globals->menu[box], HIDETREE);
          
          return 0;
        }
      }
    } 
  }
  
  return 0;
}


/*
** Exported
**
** 1998-12-20 CG
** 1999-01-09 CG
** 1999-03-17 CG
** 1999-04-18 CG
** 1999-05-24 CG
*/
void
Evhd_handle_menu (WORD apid,
                  WORD mouse_x,
                  WORD mouse_y) {
  WORD      menu_handled = 0;
  HM_BUFFER hm_buffer = {HM_NO,-1,NULL,-1,-1};

  handle_selected_title (apid,
                         &hm_buffer,
                         mouse_x,
                         mouse_y);

  switch(hm_buffer.action) {
  case HM_NO:
    break;

  case HM_KILL:
    Appl_do_control (apid, hm_buffer.apid, APC_KILL);
    break;

  case HM_OPEN_ACC:
    {
      COMMSG m;

      m.sid = 0;
      m.length = 0;
      m.msg0 = hm_buffer.apid;
      m.msg1 = hm_buffer.apid;
      m.type = AC_OPEN;
    
      Appl_do_write(hm_buffer.apid, m.msg0, 16, &m);
    }
    break;

  case HM_TOP_APPL:
    Appl_do_control (apid, hm_buffer.apid, APC_TOP);
    break;
    
  case HM_MENU_MSG:
    {
      MENUMSG m;
      
      m.type = MN_SELECTED;
      m.sid = 0;
      m.length = 0;
      m.title = hm_buffer.title;
      m.item = hm_buffer.item;
      m.tree = hm_buffer.tree;
      m.parent = hm_buffer.parent;
      
      Appl_do_write (apid, apid, 16, &m);
    }
    break;

  default:
    DEBUG0("Unknown action %d\r\n",hm_buffer.action);
  }
}
