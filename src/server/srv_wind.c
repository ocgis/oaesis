/*
** srv_wind.c
**
** Copyright 1999 Christer Gustavsson <cg@nocrew.org>
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
#include "srv_global.h"
#include "srv_misc.h"
#include "srv_queue.h"
#include "srv_wind.h"
#include "types.h"

static WORD mouse_lock;
static WORD mouse_cnt = 0;

WINLIST * win_vis = NULL; /* FIXME: make static */

typedef struct lock_info_s * LOCK_INFO;
#define LOCK_INFO_NIL ((LOCK_INFO)NULL)

typedef struct lock_info_s {
  WORD        apid;
  COMM_HANDLE handle;
  LOCK_INFO   next;
} LOCK_INFO_S;

static LOCK_INFO free_lock_info = LOCK_INFO_NIL;

/*
** Description
** Allocate a new lock info structure
**
** 1999-02-06
*/
inline
LOCK_INFO
lialloc (COMM_HANDLE handle,
         WORD        apid) {
  LOCK_INFO li;

  if (free_lock_info == LOCK_INFO_NIL) {
    li = (LOCK_INFO)malloc (sizeof (LOCK_INFO_S));
  } else {
    li = free_lock_info;
    free_lock_info = free_lock_info->next;
  }

  li->apid = apid;
  li->handle = handle;

  return li;
}


/*
** Description
** Free an lock info structure
**
** 1999-02-06
*/
inline
void
lifree (LOCK_INFO     li,
        COMM_HANDLE * handle,
        WORD *        apid) {
  *handle = li->handle;
  *apid = li->apid;

  li->next = free_lock_info;
  free_lock_info = li;
}


/*
** Description
** Get lock
**
** 1999-02-06 CG
*/
inline
void
get_lock (COMM_HANDLE handle,
          WORD        apid,
          WORD *      lock,
          WORD *      lock_cnt,
          QUEUE       lock_q) {
  R_WIND_UPDATE ret;

  if (*lock_cnt > 0) {
    if (*lock == apid) {
      (*lock_cnt)++;
      
      /* Return OK */
      ret.common.retval = 1;
      Srv_reply (handle, &ret, sizeof (R_WIND_UPDATE));
    } else {
      /* Queue application */
      insert_last (lock_q, lialloc (handle, apid));
    }
  } else {
    *lock = apid;
    *lock_cnt = 1;
    
    /* Return OK */
    ret.common.retval = 1;
    Srv_reply (handle, &ret, sizeof (R_WIND_UPDATE));
  }
}


/*
** Description
** Try to return lock
**
** 1999-02-06 CG
*/
inline
void
return_lock (COMM_HANDLE handle,
             WORD        apid,
             WORD *      lock,
             WORD *      lock_cnt,
             QUEUE       lock_q) {
  R_WIND_UPDATE ret;

  if ((*lock_cnt > 0) && (*lock == apid)) {
    LOCK_INFO li;
    
    (*lock_cnt)--;

    /* Return ok */
    ret.common.retval = 1;
    Srv_reply (handle, &ret, sizeof (R_WIND_UPDATE));
    
    if (*lock_cnt == 0) {
      /* Give a queued application the lock if there is one */
      li = pop_first (lock_q);
      
      if (li != LOCK_INFO_NIL) {
        lifree (li, &handle, &apid);
        get_lock (handle, apid, lock, lock_cnt, lock_q);
      }
    }
  } else {
    /* Return error */
    ret.common.retval = 0;
    Srv_reply (handle, &ret, sizeof (R_WIND_UPDATE));
  }
}


/*
** Exported
**
** 1998-10-04 CG
** 1999-02-06 CG
*/
void
srv_wind_update (COMM_HANDLE     handle,
                 C_WIND_UPDATE * msg) {
  static WORD   update_lock = 0;
  static WORD   update_cnt = 0;
  static QUEUE  update_q = NULL;
  static QUEUE  mouse_q;

  if (update_q == NULL) {
    update_q = allocate_queue ();
    mouse_q = allocate_queue ();
  }

  switch(msg->mode) {
  case BEG_UPDATE | 0x100: /* FIXME */
  case BEG_UPDATE:	   /* Grab the update lock */
    get_lock (handle,
              msg->common.apid,
              &update_lock,
              &update_cnt,
              update_q);
    break;
  
  case END_UPDATE:
    return_lock (handle,
                 msg->common.apid,
                 &update_lock,
                 &update_cnt,
                 update_q);
    break;
  
  case BEG_MCTRL | 0x100:
  case BEG_MCTRL:		/* Grab the mouse lock */
    get_lock (handle,
              msg->common.apid,
              &mouse_lock,
              &mouse_cnt,
              mouse_q);
    break;
    
  case END_MCTRL:
    return_lock (handle,
                 msg->common.apid,
                 &mouse_lock,
                 &mouse_cnt,
                 mouse_q);
    break;
    
  default: /* FIXME */
    DB_printf ("srv_wind_update: Unhandled case!");
  }
}


/*
** Description
** Implementation of wind_get()
**
** 1998-12-07 CG
*/
void
srv_wind_get (C_WIND_GET * msg,
              R_WIND_GET * ret) {
  WINSTRUCT	*win;
  
  win = find_wind_description(msg->handle);
  
  if(win) {
    switch(msg->mode) {	
    case WF_KIND: /* 0x0001 */
      ret->common.retval = 1;
      ret->parm1 = win->elements;
      break;
      
    case WF_WORKXYWH	:	/*0x0004*/
      ret->common.retval = 1;
      ret->parm1 = win->worksize.x;
      ret->parm2 = win->worksize.y;
      ret->parm3 = win->worksize.width;
      ret->parm4 = win->worksize.height;
      break;
      
    case	WF_CURRXYWH	:	/*0x0005*/
      ret->common.retval = 1;
      ret->parm1 = win->totsize.x;
      ret->parm2 = win->totsize.y;
      ret->parm3 = win->totsize.width;
      ret->parm4 = win->totsize.height;
				break;
      
    case	WF_PREVXYWH	:	/*0x0006*/
      ret->common.retval = 1;
      ret->parm1 = win->lastsize.x;
      ret->parm2 = win->lastsize.y;
      ret->parm3 = win->lastsize.width;
      ret->parm4 = win->lastsize.height;
      break;
      
    case	WF_FULLXYWH	:	/*0x0007*/
      ret->common.retval = 1;
      ret->parm1 = win->maxsize.x;
      ret->parm2 = win->maxsize.y;
      ret->parm3 = win->maxsize.width;
      ret->parm4 = win->maxsize.height;
      break;
      
    case WF_HSLIDE: /*0x08*/
      ret->common.retval = 1;
      ret->parm1 = win->hslidepos;
      break;
      
    case WF_VSLIDE: /*0x09*/
      ret->common.retval = 1;
      ret->parm1 = win->vslidepos;
      break;
      
    case WF_TOP: /*0x000a*/
      if(win_vis) {
	ret->parm1 = win_vis->win->id;
	ret->parm2 = win_vis->win->owner;
	
	if(win_vis && win_vis->next) {
	  ret->parm3 = win_vis->next->win->id;
	}
	else {
	  ret->parm3 = -1;
	};
	
	ret->common.retval = 1;
      }
      else {
	ret->common.retval = 0;
      }
      break;
      
    case	WF_FIRSTXYWH:	/*0x000b*/
      win->rpos = win->rlist;
    case	WF_NEXTXYWH:	/*0x000c*/
      {
	RECT r;
	
	ret->common.retval = 1;
	
	while(win->rpos) {
	  if(Misc_intersect(&win->rpos->r,&win->worksize,&r)) {
	    break;
	  }
	  
	  win->rpos = win->rpos->next;
	}					
	
	if(!win->rpos) {
	  ret->parm1 = 0;
	  ret->parm2 = 0;
	  ret->parm3 = 0;
	  ret->parm4 = 0;
	} else {
	  win->rpos = win->rpos->next;
	  
	  ret->parm1 = r.x;
	  ret->parm2 = r.y;
	  ret->parm3 = r.width;
	  ret->parm4 = r.height;
	}
      }
      break;
      
    case WF_HSLSIZE: /*0x0f*/
      ret->common.retval = 1;
      ret->parm1 = win->hslidesize;
      break;				
      
    case WF_VSLSIZE: /*0x10*/
      ret->common.retval = 1;
      ret->parm1 = win->vslidesize;
      break;				
      
    case WF_SCREEN: /*0x11*/
      ret->common.retval = 1;
      ret->parm1 = 0;
      ret->parm2 = 0;
      ret->parm3 = 0;
      ret->parm4 = 0;
      break;
      
    case WF_OWNER: /*0x14*/
      if(win) {
	ret->common.retval = 1;
	ret->parm1 = win->owner;
	ret->parm2 = win->status;
	
	if(win->status & WIN_OPEN) {
	  WINLIST *wl = win_vis;
	  
	  
	  if(wl->win == win) {
	    ret->parm3 = -1;
	  } else if(wl) {
	    while(wl->next) {
	      if(wl->next->win == win) {
		ret->parm3 = wl->win->id;
		break;
	      }
	      
	      wl = wl->next;
	    }
	    
	    wl = wl->next;
	  }
	  
	  if(wl) {
	    wl = wl->next;
	    
	    if(wl) {
	      ret->parm4 = wl->win->id;
	    } else {
	      ret->parm4 = -1;
	    }
						}
	  else {
	    ret->parm4 = -1;
	  }							
	} else {
	  ret->parm3 = -1;
	  ret->parm4 = -1;
	}
      }
      else {
	ret->common.retval = 0;
      }
      break;
      
    case WF_ICONIFY: /* 0x1a */
      if(win->status & WIN_ICONIFIED) {
	ret->parm1 = 1;
      }
      else {
	ret->parm1 = 0;
      }
      
      ret->parm2 = globals.icon_width;
      ret->parm3 = globals.icon_height;
      
      ret->common.retval = 1;
      break;
      
    case WF_UNICONIFY: /* 0x1b */
      ret->common.retval = 1;
      ret->parm1 = win->origsize.x;
      ret->parm2 = win->origsize.y;
      ret->parm3 = win->origsize.width;
      ret->parm4 = win->origsize.height;
      break;
      
    case WF_WINX:
    case WF_WINXCFG:
      ret->common.retval = 0;
      break;
      
    default:
      DB_printf("%s: Line %d: srv_wind_get:\r\n"
		"Unknown mode %d  (0x%x) wind_get(%d,%d,...)",
		__FILE__,__LINE__,msg->mode,msg->mode,
		msg->handle,msg->mode);
      ret->common.retval = 0;
    }
  } else {
    ret->common.retval = 0;
  }
}


/*
** Exported
**
** 1998-12-20 CG
*/
void
srv_wind_find (C_WIND_FIND * par,
               R_WIND_FIND * ret) {
  WINLIST *l = win_vis;

  ret->common.retval = 0;

  while(l) {
    if (Misc_inside (&l->win->totsize, par->x, par->y)) {
      ret->common.retval = l->win->id;
      break;
    }
    
    l = l->next;
  }
}


/*
** Exported
**
** 1999-02-05 CG
*/
WORD
srv_click_owner (WORD mouse_x,
                 WORD mouse_y) {
  if (mouse_cnt > 0) {
    return mouse_lock;
  } else {
    WORD        status;
    WORD        owner;
    C_WIND_FIND c_wind_find;
    R_WIND_FIND r_wind_find;
    C_WIND_GET  c_wind_get;
    R_WIND_GET  r_wind_get;

    c_wind_find.x = mouse_x;
    c_wind_find.y = mouse_y;
    srv_wind_find (&c_wind_find, &r_wind_find);
    
    c_wind_get.handle = r_wind_find.common.retval;
    c_wind_get.mode = WF_OWNER;
    srv_wind_get (&c_wind_get, &r_wind_get);
    owner = r_wind_get.parm1;
    status = r_wind_get.parm2;
    
    if (status & WIN_DESKTOP) {
      AP_INFO *ai;
      
      ai = search_appl_info(DESK_OWNER);
      
      if(ai) {
	return ai->id;
      }
    } else {
      return owner;
    }
  }
  
  return 0;
}
