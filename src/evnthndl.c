/****************************************************************************

 Module
  evnthndl.c
  
 Description
  Event processing routines in oAESis.
  
 Author(s)
  cg (Christer Gustavsson <d2cg@dtek.chalmers.se>)
  jps (Jan Paul Schmidt <Jan.P.Schmidt@mni.fh-giessen.de>)

 Revision history
 
  960101 cg
   Added standard header.
   Mouse_gain_mctrl() and Mouse_release_mctrl() added.

  960129 cg
   Changed name form mouse.c to evnthndl.c.
   Public routines changed prefix from Mouse_ to Evhd_.
   
  960507 jps
   mouse arrow changement while window movement, sizing and sliding
   realtime slider.

  960623 cg
v   Fixed mover grabbing bug; if the mouse was moved during click on
   window mover the window was topped / bottomed instead of dragged.
   
 Copyright notice
  The copyright to the program code herein belongs to the authors. It may
  be freely duplicated and distributed without fee, but not charged for.
 
 ****************************************************************************/

/****************************************************************************
 * Used interfaces                                                          *
 ****************************************************************************/

#include <basepage.h>
#include <errno.h>
#include <fcntl.h>
#include <ioctl.h>
#include <mintbind.h>
#include <osbind.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#include "debug.h"
#include "evnt.h"
#include "evnthndl.h"
#include "gemdefs.h"
#include "global.h"
#include "graf.h"
#include "mintdefs.h"
#include "mesagdef.h"
#include "misc.h"
#include "mousedev.h"
#include "objc.h"
#include "resource.h"
#include "srv.h"
#include "types.h"
#include "vdi.h"
/*#include "wm/wm.h"*/

#include <sysvars.h>

/****************************************************************************
 * Macros                                                                   *
 ****************************************************************************/

#define EVHD_APID     (-2)
#define EVHD_WAITTIME 200
#define EVHD_PIPE     "u:\\pipe\\aesmsg"

#define MOUSE_SEM 0x6f4d4f55L  /*'oMOU'*/

#define EACCESS 36

#define MOUSE_LOCK  0x01010202
#define UPDATE_LOCK 0x02020101

/****************************************************************************
 * Typedefs of module global interest                                       *
 ****************************************************************************/

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
  REVENTLIST *eventlist;
  
  WORD       lastervalid;
  EVNTREC    last_evntrec;
  WORD       mousefd;
  ULONG      lasttime;
}evntglbl = {
  -1,
  0L,
  0,
  {-1,-1},
  -1,
  0UL
  };

static struct {
  RECT   area;
  OBJECT *tree;
  WORD   dropwin;
}menu = {
  {0, 0, 0, 0}
};

static WORD mouse_x;
static WORD mouse_y;
static WORD mouse_button;

static WORD iconify_x = 0;

/****************************************************************************
 * Local functions (use static!)                                            *
 ****************************************************************************/


static WORD time_message(EVNTREC *er) {
  ULONG	newtime;
  
  newtime = globals.time;
  
  if(newtime != evntglbl.lasttime) {
    er->ap_event = APPEVNT_TIMER;
    er->ap_value = newtime - evntglbl.lasttime;
    
    evntglbl.lasttime = newtime;
    
    return 1;
  };
  
  return 0;
}

static LONG get_evntpacket(EVNTREC *er,WORD maxwait) {
  LONG kb_shift;
  
  LONG stdinrfds,mrfds;
  LONG rhndl;
  
  
  if(evntglbl.lastervalid) {
    *er = evntglbl.last_evntrec;
    evntglbl.lastervalid = 0;
    return 1;
  };
  
  stdinrfds = 1L << (LONG)STDIN_FILENO;
  mrfds = 1L << (LONG)evntglbl.mousefd;
  rhndl = stdinrfds | mrfds;
  
  if(Fselect(maxwait,&rhndl,NULL,0L) == 0) {
    return 0;
  };
  
  if(rhndl & stdinrfds) {
    ULONG c = Fgetchar(STDIN_FILENO,0);
    
    kb_shift = Kbshift(-1);
    
    evntglbl.last_evntrec.ap_event = APPEVNT_KEYBOARD;
    evntglbl.last_evntrec.ap_value = ((c & 0xff) << 16) +
      ((c & 0x00ff0000L) << 8) + kb_shift;
  }
  
  if(rhndl & mrfds)  {
    Fread(evntglbl.mousefd, sizeof(EVNTREC), 
	  &evntglbl.last_evntrec);
  };
  
  if(time_message(er)) {
    evntglbl.lastervalid = 1;
  }
  else {
    *er = evntglbl.last_evntrec;
  };	
  
  return 1;
}

static void	update_local_mousevalues(EVNTREC *er) {
  switch((WORD)er->ap_event) {
  case	APPEVNT_BUTTON	:
    mouse_button = ((WORD *)&er->ap_value)[1];
    break;	
    
  case	APPEVNT_MOUSE	:
    mouse_x = ((WORD *)&er->ap_value)[1];
    mouse_y = ((WORD *)&er->ap_value)[0];
    break;
  };
}

static void globalize_mousevalues(void) {
  globals.mouse_x = mouse_x;
  globals.mouse_y = mouse_y;
  globals.mouse_button = mouse_button;
}

static void localize_mousevalues(void) {
  mouse_x = globals.mouse_x;
  mouse_y = globals.mouse_y;
  mouse_button = globals.mouse_button;
}

static void handle_arrow_click(WORD win_id,WORD object,WORD msg) {
  COMMSG    mesag;
  EVNTREC   er;
  WORD      owner;
  WORD      dummy;
	
  Evhd_wind_update(EVHD_APID,BEG_MCTRL);
  
  mesag.type = WM_ARROWED;
  mesag.sid = 0;
  mesag.length = 0;
  mesag.msg0 = win_id;
  mesag.msg1 = msg;
  
  if(object) {
    Srv_wind_change(win_id,object,SELECTED);
  };
  
  Srv_wind_get(win_id,WF_OWNER,&owner,&dummy,&dummy,&dummy);
  
  do {
    if(get_evntpacket(&er,globals.arrowrepeat) == 0) {
      Srv_appl_write(owner,16,&mesag);
    };
    
    update_local_mousevalues(&er);
  }while(!((er.ap_event == APPEVNT_BUTTON) &&
	   !(er.ap_value & LEFT_BUTTON)));
  
  if(object) {
    Srv_wind_change(win_id,object,0);
  };
  
  Srv_appl_write(owner,16,&mesag);
  
  Evhd_wind_update(EVHD_APID,END_MCTRL);
}

static void handle_mover_click(WORD win_id) {
  WORD      dummy;
  WORD      owner;
  WORD      top;
  WORD      timeleft = 100;
  WORD      last_x = mouse_x;
  WORD      last_y = mouse_y;
  EVNTREC   er;
  COMMSG    mesag;
  
  Srv_wind_get(win_id,WF_OWNER,&owner,&dummy,&top,&dummy);
  
  Srv_wind_change(win_id,W_NAME,SELECTED); 
  
  while(1) {
    if(get_evntpacket(&er,timeleft) == 0) {
      timeleft = 0;
      break;
    };
    
    if(er.ap_event == APPEVNT_TIMER) {
      timeleft -= (WORD)er.ap_value;
    };
    
    if(timeleft < 0) {
      timeleft = 0;
      break;
    };
    
    update_local_mousevalues(&er);
    
    if((er.ap_event == APPEVNT_BUTTON) &&
       !(LEFT_BUTTON & er.ap_value)) {
      break;
    };
  };
  
  if(timeleft) {
    static COMMSG	m;
  
    if(top == -1) {
      m.type = WM_BOTTOM;
    }
    else {
      m.type = WM_TOPPED;
    };
    
    m.sid = 0;
    m.length = 0;
    m.msg0 = win_id;
    
    Srv_appl_write(owner,16,&m);
  }
  else {
    Graf_do_mouse(FLAT_HAND,NULL);
  	
    if(globals.realmove) {
      mesag.type = WM_MOVED;
      mesag.sid = 0;
      mesag.length = 0;
      mesag.msg0 = win_id;
      
      Srv_wind_get(win_id,WF_CURRXYWH,
		   &mesag.msg1,&mesag.msg2,&mesag.msg3,&mesag.msg4);
      
      do {
        WORD	waittime = EVHD_WAITTIME;
        
        if((last_x != mouse_x) || (last_y != mouse_y)) {
	  WORD tmpy;
        
          mesag.msg1 += mouse_x - last_x;
          mesag.msg2 += mouse_y - last_y;
          
          last_x = mouse_x;
          last_y = mouse_y;
          
          tmpy = mesag.msg2;

          if(mesag.msg2 < (globals.clheight + 3)) {
	    mesag.msg2 = globals.clheight + 3;
          }
  
          Srv_appl_write(owner,16,&mesag);
          
          mesag.msg2 = tmpy;
        };
        
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
      }while(!((er.ap_event == APPEVNT_BUTTON) &&
               !(er.ap_value & LEFT_BUTTON)));
    }
    else {
      RECT bound;
      RECT winsize;
      
      bound.y = globals.clheight + 3;
      bound.height = 30000;
      bound.x = -15000;
      bound.width = 30000;
      
      Evhd_wind_update(Pgetpid(),BEG_UPDATE);

      Srv_wind_get(win_id,WF_CURRXYWH,
		   &winsize.x,&winsize.y,&winsize.width,&winsize.height);
			
      globals.mouse_button = mouse_button;
      
      Graf_do_dragbox(evntglbl.mousefd,
		      winsize.width,winsize.height,
		      winsize.x,winsize.y,
		      &bound,
		      &mesag.msg1,&mesag.msg2);
      
      Vdi_vq_mouse(globals.vid,&mouse_button,&mouse_x,&mouse_y);

      mesag.type = WM_MOVED;
      mesag.sid = 0;
      mesag.length = 0;
      mesag.msg0 = win_id;

      if(mesag.msg2 < (globals.clheight + 3)) {
       	mesag.msg2 = globals.clheight + 3;
      }

      mesag.msg3 = winsize.width;
      mesag.msg4 = winsize.height;
      
      Srv_appl_write(owner,MSG_LENGTH,&mesag);
			
      Evhd_wind_update(Pgetpid(),END_UPDATE);
    };
    
    Graf_do_mouse(M_RESTORE,NULL);
  };
	
  Srv_wind_change(win_id,W_NAME,0);

  globalize_mousevalues();
}

static void handle_slider_click(WORD win_id, WORD elem,WORD owner,
				OBJECT *tree) {
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

  WORD widget = (elem == WHSLIDER) ? W_HSLIDE : W_VSLIDE;

  Graf_do_mouse(FLAT_HAND, NULL);

  Objc_do_offset(tree,elem,(WORD *)&elemrect);
  elemrect.width = tree[elem].ob_width;
  elemrect.height = tree[elem].ob_height;

  dx = last_x - elemrect.x;
  dy = last_y - elemrect.y;

  Objc_do_offset(tree,bg,(WORD *)&bgrect);
  bgrect.width = tree[bg].ob_width;
  bgrect.height = tree[bg].ob_height;

  Srv_wind_change(win_id,widget,SELECTED);

  if(globals.realslide) {
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
	
	Srv_appl_write(owner,16,&mesag);

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
  }
  else {
    Evhd_wind_update(Pgetpid(),BEG_UPDATE);
    
    Vdi_vsl_type(evntglbl.evid,3);
    Vdi_vswr_mode(evntglbl.evid,MD_XOR);

    xyarray[0] = elemrect.x;
    xyarray[1] = elemrect.y;
    xyarray[2] = elemrect.x + elemrect.width - 1;
    xyarray[3] = xyarray[1];
    xyarray[4] = xyarray[2];
    xyarray[5] = elemrect.y + elemrect.height - 1;
    xyarray[6] = xyarray[0];
    xyarray[7] = xyarray[5];
    xyarray[8] = xyarray[0];
    xyarray[9] = xyarray[1];
    
    Vdi_v_hide_c(evntglbl.evid);
    Vdi_v_pline(evntglbl.evid,5,xyarray);
    Vdi_v_show_c(evntglbl.evid,1);
    
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
	Vdi_v_hide_c(evntglbl.evid);
	Vdi_v_pline(evntglbl.evid,5,xyarray);
	Vdi_v_show_c(evntglbl.evid,1);
	
	xyarray[0] += new_x - last_x;
	xyarray[1] += new_y - last_y;
	xyarray[2] += new_x - last_x;
	xyarray[3] = xyarray[1];
	xyarray[4] = xyarray[2];
	xyarray[5] += new_y - last_y;
	xyarray[6] = xyarray[0];
	xyarray[7] = xyarray[5];
	xyarray[8] = xyarray[0];
	xyarray[9] = xyarray[1];

	Vdi_v_hide_c(evntglbl.evid);
	Vdi_v_pline(evntglbl.evid,5,xyarray);
	Vdi_v_show_c(evntglbl.evid,1);
	
	*p_lastxy = *p_newxy;
      }
    }
    
    Vdi_v_hide_c(evntglbl.evid);
    Vdi_v_pline(evntglbl.evid,5,xyarray);
    Vdi_v_show_c(evntglbl.evid,1);

    mesag.type = (elem == WHSLIDER)? WM_HSLID : WM_VSLID;      
    mesag.sid = 0;
    mesag.length = 0;
    mesag.msg0 = win_id;
    mesag.msg1 = (*p_bgrectwh == *p_elemrectwh)? 0 :
      (WORD)((((LONG)*p_newxy - (LONG)*p_dxy -
	       (LONG)*p_bgrectxy) * (LONG)1000L) / 
	     ((LONG)*p_bgrectwh - (LONG)*p_elemrectwh));
    
    Srv_appl_write(owner,16,&mesag);

    Evhd_wind_update(Pgetpid(),END_UPDATE);
  }

  Srv_wind_change(win_id,widget,0);

  Graf_do_mouse(M_RESTORE, NULL);  
}


static void handle_button(WORD newbutton) {
  EVNTREC er;
  WORD    changed = newbutton ^ globals.mouse_button;
  
  localize_mousevalues();
  
  mouse_button = newbutton;
  
  if(globals.mouse_owner >= 0) {
    EVNTREC	e;
	  	      
    e.ap_event = APPEVNT_BUTTON;
    e.ap_value = mouse_button | (1L << 16);
    
    Srv_put_event((WORD)globals.mouse_owner,&e,sizeof(EVNTREC));
  }
  else {
    if((mouse_button & LEFT_BUTTON & changed)) {
      WORD   win_id;
      OBJECT *tree;
      WORD   owner;
      WORD   top;
      WORD   dummy;
      WORD   status;
      RECT   worksize;
      
      win_id = Srv_wind_find(mouse_x,mouse_y);
      tree = Srv_get_wm_info(win_id);
      Srv_wind_get(win_id,WF_OWNER,&owner,&status,&top,&dummy);      
      Srv_wind_get(win_id,WF_WORKXYWH,
		   &worksize.x,&worksize.y,&worksize.width,&worksize.height);
      
      if(status & WIN_DESKTOP) {
      	EVNTREC	e;

      	e.ap_event = APPEVNT_BUTTON;
      	e.ap_value = mouse_button | (1L << 16);
      	Srv_put_event(DESK_OWNER,&e,sizeof(EVNTREC));
      }
      else if((status & WIN_DIALOG)
	      || (status & WIN_MENU)) {
      	EVNTREC	e;
      
      	e.ap_event = APPEVNT_BUTTON;
      	e.ap_value = mouse_button | (1L << 16);
      	Srv_put_event(owner,&e,sizeof(EVNTREC));
    	}
    	else if(Misc_inside(&worksize,mouse_x,mouse_y)) {
	  if((status & (WIN_UNTOPPABLE | WIN_ICONIFIED)) == WIN_UNTOPPABLE) {
	    EVNTREC	e;
	    
	    e.ap_event = APPEVNT_BUTTON;
	    e.ap_value = mouse_button | (1L << 16);
	    Srv_put_event(owner,&e,sizeof(EVNTREC));
	  }
	  else {
	    COMMSG	m;
	    
	    if((status & WIN_ICONIFIED) &&
	       (Evnt_waitclicks(evntglbl.mousefd,LEFT_BUTTON,LEFT_BUTTON,1,
				LEFT_BUTTON) >= 1)) {
	      m.type = WM_UNICONIFY;
	      m.sid = 0;
	      m.length = 0;
	      m.msg0 = win_id;
	      Srv_wind_get(win_id,WF_UNICONIFY,&m.msg1,&m.msg2,
			   &m.msg3,&m.msg4);

	      Srv_appl_write(owner,(WORD)sizeof(COMMSG),&m);
	      
	      Vdi_vq_mouse(globals.vid,&mouse_button,&mouse_x,&mouse_y);
	    }
	    else if(top != -1) {
	      Vdi_vq_mouse(globals.vid,&mouse_button,&mouse_x,&mouse_y);
	      
	      if(mouse_button & LEFT_BUTTON) {
		do {
		  get_evntpacket(&er,0);
		  update_local_mousevalues(&er);
		}while(!((er.ap_event == APPEVNT_BUTTON) &&
			 !(er.ap_value & LEFT_BUTTON)));
    	      };
	        
	      m.type = WM_TOPPED;
	      m.sid = 0;
	      m.length = 0;
	      m.msg0 = win_id;

	      Srv_appl_write(owner,16,&m);
	    }
	    else {
	      er.ap_event = APPEVNT_BUTTON;
	      er.ap_value = (LONG)mouse_button | (1L << 16);
	      Srv_put_event(owner,&er,sizeof(EVNTREC));
	    };
	  };
    	}
    	else {
	  WORD   obj = Objc_do_find(tree,0,9,mouse_x,mouse_y,0);
	  COMMSG mesag;				

	  switch(obj) {		
	  case	WCLOSER	:
	    Srv_wind_change(win_id,W_CLOSER,SELECTED);
	    
	    do {
	      get_evntpacket(&er,0);
	      update_local_mousevalues(&er);
	    }while(!((er.ap_event == APPEVNT_BUTTON) &&
		     !(er.ap_value & LEFT_BUTTON)));
	    
	    
	    Srv_wind_change(win_id,W_CLOSER,0);
	    
	    mesag.type = WM_CLOSED;
	    mesag.sid = 0;
	    mesag.length = 0;
	    mesag.msg0 = win_id;
	    Srv_appl_write(owner,16,&mesag);
	    break;
	    
	  case WSMALLER:
	    {
	      WORD skeys;
	      
	      Srv_wind_change(win_id,W_SMALLER,SELECTED);

	      do {
		get_evntpacket(&er,0);
		update_local_mousevalues(&er);
	      }while(!((er.ap_event == APPEVNT_BUTTON) &&
		       !(er.ap_value & LEFT_BUTTON)));
	      
	      Srv_wind_change(win_id,W_SMALLER,0);
	      
	      Vdi_vq_key_s(evntglbl.evid,&skeys);
						
	      if(skeys & K_CTRL) {
		mesag.type = WM_ALLICONIFY;
	      }
	      else {
		mesag.type = WM_ICONIFY;
	      };
	      
	      mesag.sid = 0;
	      mesag.length = 0;
	      mesag.msg0 = win_id;
	      mesag.msg3 = globals.icon_width;
	      mesag.msg4 = globals.icon_height;
	      mesag.msg1 = iconify_x;
	      mesag.msg2 = globals.screen.height - mesag.msg4;
						
	      iconify_x += mesag.msg3;
	      if(iconify_x + mesag.msg3 > globals.screen.width) {
		iconify_x = 0;
	      };
						
	      Srv_appl_write(owner,16,&mesag);
	    };
	    break;
	    
	  case WFULLER:
	    Srv_wind_change(win_id,W_FULLER,SELECTED);
	    
	    do {
	      get_evntpacket(&er,0);
	      update_local_mousevalues(&er);
	    }while(!((er.ap_event == APPEVNT_BUTTON) &&
		     !(er.ap_value & LEFT_BUTTON)));
	    
	    Srv_wind_change(win_id,W_FULLER,0);
	    
	    mesag.type = WM_FULLED;
	    mesag.sid = 0;
	    mesag.length = 0;
	    mesag.msg0 = win_id;
	    Srv_appl_write(owner,16,&mesag);
	    
	    break;
		
	  case	WSIZER	:
	    {
	      RECT totsize;
	      
	      Srv_wind_change(win_id,W_SIZER,SELECTED);
	      Srv_wind_get(win_id,WF_CURRXYWH,&totsize.x,&totsize.y,
			   &totsize.width,&totsize.height);
	      
	      if(globals.realsize) {
		WORD	last_x;
		WORD	last_y;
		WORD	offsx = mouse_x - totsize.x - totsize.width;
		WORD	offsy = mouse_y - totsize.y - totsize.height;
		
		mesag.type = WM_SIZED;
		mesag.sid = 0;
		mesag.length = 0;
		mesag.msg0 = win_id;
		mesag.msg1 = totsize.x;
		mesag.msg2 = totsize.y;

		last_x = mouse_x;
		last_y = mouse_y;
		  
		do {
		  LONG	waittime = 200;
		  
		  if((last_x != mouse_x) || (last_y != mouse_y)) {
		    mesag.msg3 = mouse_x - totsize.x - offsx;
		    mesag.msg4 = mouse_y - totsize.y - offsy;
		    
		    last_x = mouse_x;
		    last_y = mouse_y;
		    
		    Srv_appl_write(owner,16,&mesag);
		  };
		  
		  while(1) {
		    if(get_evntpacket(&er,(WORD)waittime) == 0) {
		      er.ap_event = -1;
		      break;
		    };
		    
		    if(er.ap_event == APPEVNT_TIMER) {
		      if((waittime -= er.ap_value) <= 0) {
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
		}while(!((er.ap_event == APPEVNT_BUTTON) &&
			 !(er.ap_value & LEFT_BUTTON)));
	      }
	      else {
		globals.mouse_button = mouse_button;
		
		Graf_do_rubberbox(evntglbl.mousefd,
				  totsize.x,totsize.y,100,100,
				  &mesag.msg3,&mesag.msg4);
		
		Vdi_vq_mouse(globals.vid,&mouse_button,&mouse_x,&mouse_y);
		
		mesag.type = WM_SIZED;
		mesag.sid = 0;
		mesag.length = 0;
		mesag.msg0 = win_id;
		mesag.msg1 = totsize.x;
		mesag.msg2 = totsize.y;
		mouse_x = mesag.msg3 + totsize.x - 1;
		mouse_y = mesag.msg4 + totsize.y - 1;

		Srv_appl_write(owner,16,&mesag);
	      };

	      Evhd_wind_update(Pgetpid(),BEG_UPDATE);
					
	      Srv_wind_change(win_id,W_SIZER,0);
	      Evhd_wind_update(Pgetpid(),END_UPDATE);
	      break;
	    };
				
	  case WAPP:	        
	  case WMOVER:
	    handle_mover_click(win_id);
	    break;
	        
	  case WLEFT:
	    handle_arrow_click(win_id,W_LFARROW,WA_LFLINE);
	    break;
		
	  case WRIGHT:
	    handle_arrow_click(win_id,W_RTARROW,WA_RTLINE);
	    break;
		
	  case WUP:
	    handle_arrow_click(win_id,W_UPARROW,WA_UPLINE);
	    break;
		
	  case WDOWN:
	    handle_arrow_click(win_id,W_DNARROW,WA_DNLINE);
	    break;
	    
	  case WHSB:
	    {
	      WORD      xy[2];
	      
	      if(tree) {
		Objc_do_offset(tree,WHSLIDER,xy);
	      };
	
	      if(mouse_x > xy[0]) {
		handle_arrow_click(win_id,0,WA_RTPAGE);
	      }
	      else {
		handle_arrow_click(win_id,0,WA_LFPAGE);
	      };
	    };
	    break;
	    
	  case WVSB:
	    {
	      WORD xy[2];
	      
	      if(tree) {
		Objc_do_offset(tree,WVSLIDER,xy);
	      };
	      
	      if(mouse_y > xy[1]) {
		handle_arrow_click(win_id,0,WA_DNPAGE);
	      }
	      else {
		handle_arrow_click(win_id,0,WA_UPPAGE);
	      };
	    };
	    break;
	    
	  case WHSLIDER:
	    handle_slider_click(win_id,WHSLIDER,owner,tree);
	    break;
	        
	  case WVSLIDER:
	    handle_slider_click(win_id,WVSLIDER,owner,tree);
	    break;
	        
	  default:
	    if(win_id > 0) {
	      COMMSG	m;
	          
	      if(top != -1) {
		Vdi_vq_mouse(globals.vid,&mouse_button,&mouse_x,&mouse_y);

		if(mouse_button & LEFT_BUTTON) {
		  do {
		    get_evntpacket(&er,0);
		    update_local_mousevalues(&er);
		  }while(!((er.ap_event == APPEVNT_BUTTON) &&
			   !(er.ap_value & LEFT_BUTTON)));
		  
		  m.type = WM_TOPPED;
		  m.sid = 0;
		  m.length = 0;
		  m.msg0 = win_id;

		  Srv_appl_write(owner,16,&m);
		};
	      };
	    };
	  };
	};
    }
    else {
      WORD wid;
      WORD owner;
      WORD status;
      WORD dummy;
      RECT worksize;
	    
      wid = Srv_wind_find(mouse_x,mouse_y);
      Srv_wind_get(wid,WF_OWNER,&owner,&status,&dummy,&dummy);
      Srv_wind_get(wid,WF_WORKXYWH,
		   &worksize.x,&worksize.y,&worksize.width,&worksize.height);
 
 
      if(status & WIN_DESKTOP) {
	EVNTREC	e;
	      
	e.ap_event = APPEVNT_BUTTON;
	e.ap_value = mouse_button | (1L << 16);
	Srv_put_event(DESK_OWNER,&e,sizeof(EVNTREC));
      }
      else if(Misc_inside(&worksize,mouse_x,mouse_y)) {
	EVNTREC	e;
	      
	e.ap_event = APPEVNT_BUTTON;
	e.ap_value = mouse_button | (1L << 16);
	Srv_put_event(owner,&e,sizeof(EVNTREC));
      };
    };
  };

  globalize_mousevalues();
}

static void check_rectevents(WORD x,WORD y) {
  REVENTLIST	**rl;
  
  struct {
    EVNTREC       head;
    EVNTREC_MOUSE tail;
  }m;

  /* This speeds things up when there is nothing in the list */
  if(!evntglbl.eventlist) {
    return;
  };
    
  Psemaphore(SEM_LOCK,MOUSE_SEM,-1);
  
  rl = &evntglbl.eventlist;
  
  while(*rl) {
    if((globals.mouse_owner == -1) ||
       (globals.mouse_owner == (*rl)->event.apid)) {
      WORD    sendflag = 0;
      
      if((*rl)->event.flag1 != -1) {
	WORD insideflag = Misc_inside(&(*rl)->event.r1,x,y);
	    	
	if((insideflag && ((*rl)->event.flag1 == MO_ENTER)) ||
	   ((!insideflag) && ((*rl)->event.flag1 == MO_LEAVE))) {	
	  m.head.ap_event = MO_RECT1;
	  m.head.ap_value = (*rl)->event.apid;
	  m.tail.mx = x;
	  m.tail.my = y;
	  m.tail.buttons = globals.mouse_button;
	  /*m.tail.kstate = TODO <--*/
	  
	  sendflag = 1;	
	};
      }
      else if((*rl)->event.flag2 != -1) {
	WORD insideflag = Misc_inside(&(*rl)->event.r2,x,y);
	
	if((insideflag && ((*rl)->event.flag2 == MO_ENTER)) ||
	   ((!insideflag) && ((*rl)->event.flag2 == MO_LEAVE))) {
	  m.head.ap_event = MO_RECT2;
	  m.head.ap_value = (*rl)->event.apid;
	  m.tail.mx = x;
	  m.tail.my = y;
	  m.tail.buttons = globals.mouse_button;
	  /*m.tail.kstate = TODO <--*/
	  
	  sendflag = 1;
	};
      };

      if(sendflag) {
	REVENTLIST *tmp = *rl;

	Srv_put_event(tmp->event.apid,&m,sizeof(EVNTREC) +
		      sizeof(EVNTREC_MOUSE));
	
	/* Make sure that we only send one message => remove the entry*/
	
	*rl = (*rl)->next;

	Mfree(tmp);
      };
				    
      if(!(*rl)) {
	break;
      };
    };
	  
    rl = &(*rl)->next;
  };
  
  Psemaphore(SEM_UNLOCK,MOUSE_SEM,-1);
}

static void	getmenuxpos(WORD *x,WORD *width) {
  OBJECT	*t;
	
  WORD start;
	
  *width = 0;

  t = Srv_get_top_menu();
	
  if(t) {
    start = t[t[0].ob_head].ob_head;
	
    *x = t[0].ob_x + t[t[0].ob_head].ob_x + t[start].ob_x;
    *width = t[t[start].ob_tail].ob_x + t[t[start].ob_tail].ob_width;
  }
  else {
    DB_printf("%s: Line %d: getmenuxpos:\r\n"
	      "Couldn't find top menu.\r\n",__FILE__,__LINE__);
  };
}

static WORD	get_matching_menu(OBJECT *t,WORD n) {
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

static WORD handle_drop_down(HM_BUFFER *hm_buffer,WORD menubox,WORD title) {
  WORD     entry;
  WORD     deskbox = menu.tree[menu.tree[0].ob_tail].ob_head;
  
  OBJECT   *nmenu;
  
  if((deskbox == menubox) && (mouse_y >= globals.pmenutad[0].ob_y)) {
    nmenu = globals.pmenutad;
    entry = Objc_do_find(nmenu,0,9,mouse_x,mouse_y,0);
  }
  else {
    nmenu = menu.tree;
    entry = Objc_do_find(nmenu, menubox, 9, mouse_x, mouse_y,0);
  };

  if(entry >= 0) {
    RECT entryarea;
    
    Objc_area_needed(nmenu,entry,&entryarea);
    
    if(!(nmenu[entry].ob_state & DISABLED) && (entry != 0)) {
      /* select the entry and update it */

      nmenu[entry].ob_state |= SELECTED;
      
      Objc_do_draw(nmenu,0,9,&entryarea);
    };

    while(TRUE) {
      EVNTREC er;
			
      get_evntpacket(&er,0);
	
      switch((WORD)er.ap_event) {		
      case APPEVNT_KEYBOARD :
      case APPEVNT_TIMER :
	break;	
	      
      case APPEVNT_MOUSE	:
	mouse_y = (WORD)(er.ap_value >> 16);
	mouse_x = (WORD)er.ap_value;

	if(!Misc_inside(&entryarea,mouse_x,mouse_y)) {
	  if(!(nmenu[entry].ob_state & DISABLED)) {
	    nmenu[entry].ob_state &= ~SELECTED;
	    
	    Objc_do_draw(nmenu,0,9,&entryarea);
	  };
					
	  return 0;
	};
	break;
	    
      case APPEVNT_BUTTON	:
	{
	  WORD changebut = (WORD)(mouse_button ^ er.ap_value);

	  mouse_button = (WORD)er.ap_value;

	  if(changebut & LEFT_BUTTON & (!mouse_button)) {
	    nmenu[entry].ob_state &= ~SELECTED;
		
	    if(nmenu == globals.pmenutad) {
	      AP_LIST *mr;
	      WORD    walk = entry - PMENU_FIRST;
						
	      mr = globals.applmenu;
						
	      while(walk && mr) {
		mr = mr->mn_next;
		walk--;
	      };
							
	      if(walk) {
		walk--;
		mr = globals.accmenu;
		
		while(walk && mr) {
		  mr = mr->mn_next;
		  walk--;
		};
	      };
							
						
	      if(mr) {
		WORD skeys;
		
		Vdi_vq_key_s(evntglbl.evid,&skeys);

		if(skeys & K_CTRL) {
		  hm_buffer->action = HM_KILL;
		  hm_buffer->apid = mr->ai->id;
		}
		else {
		  if(mr->ai->type & APP_APPLICATION) {
		    hm_buffer->action = HM_TOP_APPL;
		    hm_buffer->apid = mr->ai->id;
		  }
		  else { /* Accessory */
		    hm_buffer->action = HM_OPEN_ACC;
		    hm_buffer->apid = mr->ai->id;
		  };
		};
	      }
	      else {
		DB_printf("%s: Line %d: handle_drop_down:\r\n"
			  "Couldn't find application to top!\r\n",
			  __FILE__,__LINE__);
	      };
	    }
	    else {
	      hm_buffer->action = HM_MENU_MSG;
	      hm_buffer->title = title;
	      hm_buffer->item = entry;
	      hm_buffer->tree = nmenu;
	      hm_buffer->parent = menubox;
	    };
	    
	    return 1;
	  };
	};
	break;
	
      default:
	DB_printf("%s: Line %d: handle_drop_down:\r\n"
		  "Unknown message.\r\n",__FILE__,__LINE__);
      };
    };	
  };
	
  return 0;
}

static WORD handle_selected_title(HM_BUFFER *hm_buffer) {
  EVNTREC er;
  WORD    box;
  WORD    title;
  WORD    deskbox;
  	
  title = Objc_do_find(menu.tree, 0, 9, mouse_x, mouse_y, 0);
  box = get_matching_menu(menu.tree, title);
  
  deskbox = menu.tree[menu.tree[0].ob_tail].ob_head;
	  
  if(box >= 0) {
    RECT area;
    RECT titlearea;
	    
    /* select the title and update it */
	    
    menu.tree[title].ob_state |= SELECTED;

    Objc_area_needed(menu.tree,title,&titlearea);
    Objc_do_draw(menu.tree,0,9,&titlearea);

    Objc_area_needed(menu.tree,box,&area);
    
    if(box == deskbox) {
      WORD i;
			
      Objc_do_offset(menu.tree,box,&globals.pmenutad[0].ob_x);
    
      globals.pmenutad[0].ob_y +=
	(menu.tree[menu.tree[box].ob_head].ob_height << 1);
      globals.pmenutad[0].ob_width = menu.tree[box].ob_width;

      for(i = PMENU_FIRST; i <= PMENU_LAST; i++) {
	globals.pmenutad[i].ob_width = globals.pmenutad[0].ob_width;
      }
		
      area.height = globals.pmenutad[0].ob_height + 
	(globals.pmenutad[1].ob_height << 1) + 2;
    };

    menu.dropwin = Srv_wind_create(0,0,&area,WIN_MENU);
    Srv_wind_open(menu.dropwin,&area);

    menu.tree[box].ob_flags &= ~HIDETREE;

    if(box == deskbox) {
      RECT clip = area;
      
      clip.height = globals.pmenutad[0].ob_y - area.y;
      Objc_do_draw(menu.tree,box,9,&clip);
      
      clip.y = globals.pmenutad[0].ob_y;
      clip.height = globals.pmenutad[0].ob_height + 1;
      Objc_do_draw(globals.pmenutad,0,9,&clip);
    }
    else {
      Objc_do_draw(menu.tree,box,9,&area);
    };

    /* Start to wait for messages and rect 1 */
    while(TRUE) {
      get_evntpacket(&er,0);
	
      switch((WORD)er.ap_event) {		
      case APPEVNT_BUTTON	:
      case APPEVNT_KEYBOARD :
      case APPEVNT_TIMER :
	break;	
	
      case APPEVNT_MOUSE	:
	mouse_x = (WORD)er.ap_value;
	mouse_y = (WORD)(er.ap_value >> 16);
	
	if(!Misc_inside(&titlearea,mouse_x,mouse_y)) {
	  WORD closebox = 0;
	      	
	  if((deskbox == box) && (mouse_y >= globals.pmenutad[0].ob_y)) {
	    if(!Misc_inside((RECT *)&globals.pmenutad[0].ob_x,
			    mouse_x,mouse_y) ||
	       (Objc_do_find(globals.pmenutad,0,9,mouse_x,mouse_y,0) < 0) ||
	       handle_drop_down(hm_buffer,box,title)) {
	      closebox = 1;
	    };
	  }
	  else if((Objc_do_find(menu.tree, box, 9, mouse_x, mouse_y,0)
		   < 0) || handle_drop_down(hm_buffer,box,title)) {
	    closebox = 1;
	  };
	  
	  if(closebox) {
	    menu.tree[title].ob_state &= ~SELECTED;
	    Objc_do_draw(menu.tree,0,9,&titlearea);
	    
	    Srv_wind_close(menu.dropwin);
	    Srv_wind_delete(menu.dropwin);
	    
	    menu.tree[box].ob_flags |= HIDETREE;
	    
	    return 0;
	  };
	};
	break;
	    
      default:
	DB_printf("%s: Line %d: handle_selected_title:\r\n"
		  "Unknown message.\r\n",__FILE__,__LINE__);
      };
    };	
  };
  
  return 0;
}

static void handle_menu(void) {
  WORD      menu_handled = 0;
  EVNTREC   er;
  HM_BUFFER hm_buffer = {HM_NO,-1,NULL,-1,-1};

  localize_mousevalues();
  
  menu.tree = Srv_get_top_menu();
  
  getmenuxpos(&menu.area.x,&menu.area.width);

  while(!menu_handled) {
    get_evntpacket(&er,0);

    switch((WORD)er.ap_event) {		
    case APPEVNT_BUTTON	:
    case APPEVNT_KEYBOARD :
    case APPEVNT_TIMER :
      break;	
      
    case APPEVNT_MOUSE	:
      mouse_x = ((WORD *)&er.ap_value)[1];
      mouse_y = ((WORD *)&er.ap_value)[0];

      if(Misc_inside(&menu.area,mouse_x,mouse_y)) {
	handle_selected_title(&hm_buffer);
      };
      
      if(mouse_y > menu.area.height) {
	globalize_mousevalues();
	
	menu_handled = 1;
      };
      break;
      
    default:
      DB_printf("%s: Line %d: handle_menu:\r\n"
		"Unknown message.\r\n",__FILE__,__LINE__);
    };
  };

  switch(hm_buffer.action) {
  case HM_NO:
    break;

  case HM_KILL:
    Srv_appl_control(hm_buffer.apid,APC_KILL);
    break;

  case HM_OPEN_ACC:
    {
      COMMSG m;

      m.sid = 0;
      m.length = 0;
      m.msg0 = hm_buffer.apid;
      m.msg1 = hm_buffer.apid;
      m.type = AC_OPEN;
    
      Srv_appl_write(m.msg0, 16, &m);
    };
    break;

  case HM_TOP_APPL:
    Srv_appl_control(hm_buffer.apid,APC_TOP);
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
      
      Srv_appl_write(TOP_MENU_OWNER, 16, &m);
    };
    break;

  default:
    DB_printf("Unknown action %d\r\n",hm_buffer.action);
  };
}

static WORD evnt_handler(LONG arg) {
  WORD	  work_in[] = {1,7,1,1,1,1,1,1,1,1,2};
  WORD	  work_out[57];
  EVNTREC	er;
  WORD    evhd_pipe;

  NOT_USED(arg);

  /* Create a pipe to take commands from */
  evhd_pipe = Fcreate(EVHD_PIPE,0);

  menu.area.height = globals.clheight + 3;

  evntglbl.mousefd = (WORD)Fopen(globals.mousename,0);
  
  if(evntglbl.mousefd < 0) {
    DB_printf("Couldn't open mouse pipe! Error %d",evntglbl.mousefd);
  };
  
  evntglbl.lasttime = globals.time;

  /* open up a vdi workstation to use in the process */
  
  evntglbl.evid = globals.vid;
  Vdi_v_opnvwk(work_in,&evntglbl.evid,work_out);
  
  while(1) {
    get_evntpacket(&er,0);

    switch((WORD)er.ap_event) {
    case APPEVNT_TIMER:
    	break;
    			
    case APPEVNT_BUTTON	:
      handle_button((WORD)er.ap_value);
      break;	
      
    case APPEVNT_MOUSE	:
      globals.mouse_x = (WORD)(er.ap_value & 0xffff);
      globals.mouse_y = (WORD)(er.ap_value >> 16);

	if(globals.mouse_owner >= 0) {
	  check_rectevents(globals.mouse_x,globals.mouse_y);

	  if(globals.mouse_mode) {
	    Srv_put_event((WORD)globals.mouse_owner,&er,sizeof(EVNTREC));
	  };
	}
	else {
	  if(globals.mouse_y < menu.area.height) {
	    handle_menu();
	  }
	  else {
	    check_rectevents(globals.mouse_x,globals.mouse_y);
	  };
	};
	break;
      
      case APPEVNT_KEYBOARD :
    	if((((er.ap_value & 0x00ff0000L) >> 16) == 0x09) &&
	   (er.ap_value & K_CTRL) && (er.ap_value & K_ALT)) {
	  Srv_appl_control(-1,APC_TOPNEXT);    		
    	}
    	else if(er.ap_value == 0x4a1f000cL) {
	  DB_printf("Killing oAESis");
    		
	  (void)Pkill(globals.srvpid,SIGQUIT);
	  (void)Pkill(globals.applpid,SIGQUIT);
	  (void)Pkill(globals.evntpid,SIGQUIT);
    	}
    	else if(er.ap_value == 0x1910000cL) {
	  DB_printf("Event handler OK.");
	  Srv_shake();
    	}
    	else if((er.ap_value & 0xc) == 0xc) {
	  DB_printf("Unknown command %lx",er.ap_value);
    	}
    	else {
	  Srv_put_event(TOP_APPL,&er,sizeof(EVNTREC));
	};
	break;
      
      default:
    	DB_printf("%s: Line %d:\r\nUnknown mouse event %ld!",
		  __FILE__,__LINE__,er.ap_event);
      };
  };
}

/****************************************************************************
 * Public functions                                                         *
 ****************************************************************************/

/****************************************************************************
 * Evhd_init_module                                                         *
 *  Initiate event processing module.                                       *
 ****************************************************************************/
void                   /*                                                   */
Evhd_init_module(void) /*                                                   */
/****************************************************************************/
{	
  /* Initialize internal mouse variables */
  
  Vdi_vq_mouse(globals.vid,&globals.mouse_button,
	       &globals.mouse_x,&globals.mouse_y);
  
  globals.evntpid = (WORD)Misc_fork(evnt_handler,0,"EvntHndl");

  Psemaphore(SEM_CREATE,MOUSE_LOCK,-1);
  Psemaphore(SEM_UNLOCK,MOUSE_LOCK,-1);
  Psemaphore(SEM_CREATE,UPDATE_LOCK,-1);
  Psemaphore(SEM_UNLOCK,UPDATE_LOCK,-1);
  Psemaphore(SEM_CREATE,MOUSE_SEM,-1);
  Psemaphore(SEM_UNLOCK,MOUSE_SEM,-1);
}

/****************************************************************************
 * Evhd_exit_module                                                         *
 *  Shutdown event processing module.                                       *
 ****************************************************************************/
void                   /*                                                   */
Evhd_exit_module(void) /*                                                   */
/****************************************************************************/
{
  Psemaphore(SEM_LOCK,MOUSE_SEM,-1);

  (void)Pkill(globals.evntpid,SIGKILL);
  
  Psemaphore(SEM_DESTROY,MOUSE_SEM,-1);
}

/****************************************************************************
 * Evhd_make_rectevent                                                      *
 *  Start reporting of mouse events.                                        *
 ****************************************************************************/
void                   /*                                                   */
Evhd_make_rectevent(   /*                                                   */
RECTEVENT *re)         /* Description of events that should be reported.    */
/****************************************************************************/
{
  REVENTLIST *rl = (REVENTLIST *)Mxalloc(sizeof(REVENTLIST),GLOBALMEM);
  
  Psemaphore(SEM_LOCK,MOUSE_SEM,-1);
  
  rl->event = *re;
  rl->next = evntglbl.eventlist;
  evntglbl.eventlist = rl;
  
  Psemaphore(SEM_UNLOCK,MOUSE_SEM,-1);
  
  check_rectevents(globals.mouse_x,globals.mouse_y);
}

/****************************************************************************
 * Evhd_kill_rectevent                                                      *
 *  End reporting of mouse events.                                          *
 ****************************************************************************/
void                   /*                                                   */
Evhd_kill_rectevent(   /*                                                   */
WORD apid)             /* Application id to end reporting to.               */
/****************************************************************************/
{
  REVENTLIST	**rl;
  
  Psemaphore(SEM_LOCK,MOUSE_SEM,-1);
  
  rl = &evntglbl.eventlist;
  
  while(*rl) {
    if((*rl)->event.apid == apid) {
      REVENTLIST	*tmp = *rl;
      
      *rl = tmp->next;
      
      Mfree(tmp);
      break;
    };
    
    rl = &(*rl)->next;
  };
  
  Psemaphore(SEM_UNLOCK,MOUSE_SEM,-1);
}


/****************************************************************************
 * Evhd_wind_update                                                         *
 *  Get / release update semaphore.                                         *
 ****************************************************************************/
WORD                   /*                                                   */
Evhd_wind_update(      /*                                                   */
WORD apid,             /* Application id.                                   */
WORD mode)             /* Mode.                                             */
/****************************************************************************/
{
  static WORD update_lock = 0,update_cnt = 0;
  static WORD mouse_lock,mouse_cnt = 0;

  WORD clnt_pid = Pgetpid();
  long timeout = (mode & 0x100) ? 0L : -1L; /* test for check-and-set mode */
  
  switch(mode & 0xff) {
  case BEG_UPDATE:	/* Grab the update lock */
  case BEG_UPDATE|0x100:
    if (update_lock==clnt_pid) {
      update_cnt++ ;
      break ;
    }

    if(Psemaphore(2,UPDATE_LOCK,timeout) == -EACCESS) {
      return 0;	/* screen locked by different process */
    }

    update_lock=clnt_pid;
    update_cnt=1 ;
    break;
  
  case END_UPDATE:
    if ((update_lock == clnt_pid) && (--update_cnt == 0)) {
      update_lock=FALSE;
      Psemaphore(3,UPDATE_LOCK,0);
    }
    break;
  
  case BEG_MCTRL:		/* Grab the mouse lock */
  case BEG_MCTRL|0x100:
    if (mouse_lock==clnt_pid) {
      mouse_cnt++ ;
      break ;
    }
     
    if (Psemaphore(2,MOUSE_LOCK,timeout) == -EACCESS) {
      return 0;	/* mouse locked by different process */
    }
    
    mouse_lock=clnt_pid;
    mouse_cnt=1 ;
    break;
    
  case END_MCTRL:
    if ((mouse_lock==clnt_pid)&&(--mouse_cnt==0)) {
      mouse_lock=FALSE;
      Psemaphore(3,MOUSE_LOCK,0);
    }
  }

  return 1;
}

