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

#define DEBUGLEVEL 0

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>

#include "aesbind.h"
#include "debug.h"
#include "srv_comm.h"
#include "srv_global.h"
#include "srv_misc.h"
#include "srv_queue.h"
#include "srv_wind.h"
#include "types.h"

static WORD mouse_lock;
static WORD mouse_cnt = 0;

/* FIXME: make all of the variables below static */
WINLIST * win_vis = NULL;
WINLIST * win_list = 0L;
WINLIST * win_free = 0L;
WORD      win_next = 0;

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
*/
inline
LOCK_INFO
lialloc (COMM_HANDLE handle,
         WORD        apid) {
  LOCK_INFO li;

  if (free_lock_info == LOCK_INFO_NIL) {
    DEBUG3 ("srv_wind.c: lialloc: allocating new structure!");
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
      PUT_R_ALL(WIND_UPDATE, &ret, 1);
      SRV_REPLY(handle, &ret, sizeof (R_WIND_UPDATE));
    } else {
      /* Queue application */
      insert_last (lock_q, lialloc (handle, apid));
      DEBUG2 ("srv_wind.c: get_lock: queued appl %d", apid);
    }
  } else {
    DEBUG2 ("srv_wind.c: get_lock: appl %d got lock", apid);
    *lock = apid;
    *lock_cnt = 1;
    
    /* Return OK */
    PUT_R_ALL(WIND_UPDATE, &ret, 1);
    SRV_REPLY(handle, &ret, sizeof (R_WIND_UPDATE));
  }
}


/*
** Description
** Try to return lock
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

    DEBUG2 ("srv_wind.c: return_lock: appl %d", apid);
    /* Return ok */
    PUT_R_ALL(WIND_UPDATE, &ret, 1);
    SRV_REPLY(handle, &ret, sizeof (R_WIND_UPDATE));
    
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
    PUT_R_ALL(WIND_UPDATE, &ret, 0);
    SRV_REPLY(handle, &ret, sizeof (R_WIND_UPDATE));
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

  DEBUG2 ("srv_wind.c: srv_wind_update: mode = %d", msg->mode);

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
*/
void
srv_wind_get (C_WIND_GET * msg,
              R_WIND_GET * ret) {
  WINSTRUCT * win;
  WORD        retval;
  
  win = find_wind_description(msg->handle);
  
  if(win) {
    switch(msg->mode) {	
    case WF_KIND: /* 0x0001 */
      retval = 1;
      ret->parm1 = win->elements;
      break;
      
    case WF_WORKXYWH	:	/*0x0004*/
      retval = 1;
      ret->parm1 = win->worksize.x;
      ret->parm2 = win->worksize.y;
      ret->parm3 = win->worksize.width;
      ret->parm4 = win->worksize.height;
      break;
      
    case	WF_CURRXYWH	:	/*0x0005*/
      retval = 1;
      ret->parm1 = win->totsize.x;
      ret->parm2 = win->totsize.y;
      ret->parm3 = win->totsize.width;
      ret->parm4 = win->totsize.height;
				break;
      
    case	WF_PREVXYWH	:	/*0x0006*/
      retval = 1;
      ret->parm1 = win->lastsize.x;
      ret->parm2 = win->lastsize.y;
      ret->parm3 = win->lastsize.width;
      ret->parm4 = win->lastsize.height;
      break;
      
    case	WF_FULLXYWH	:	/*0x0007*/
      retval = 1;
      ret->parm1 = win->maxsize.x;
      ret->parm2 = win->maxsize.y;
      ret->parm3 = win->maxsize.width;
      ret->parm4 = win->maxsize.height;
      break;
      
    case WF_HSLIDE: /*0x08*/
      retval = 1;
      ret->parm1 = win->hslidepos;
      break;
      
    case WF_VSLIDE: /*0x09*/
      retval = 1;
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
	
	retval = 1;
      }
      else {
        retval = 0;
      }
      break;
      
    case	WF_FIRSTXYWH:	/*0x000b*/
      win->rpos = win->rlist;
    case	WF_NEXTXYWH:	/*0x000c*/
      {
	RECT r;
	
	retval = 1;
	
	while(win->rpos) {
	  if(srv_intersect(&win->rpos->r,&win->totsize,&r)) {
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
      retval = 1;
      ret->parm1 = win->hslidesize;
      break;				
      
    case WF_VSLSIZE: /*0x10*/
      retval = 1;
      ret->parm1 = win->vslidesize;
      break;				
      
    case WF_SCREEN: /*0x11*/
      retval = 1;
      ret->parm1 = 0;
      ret->parm2 = 0;
      ret->parm3 = 0;
      ret->parm4 = 0;
      break;
      
    case WF_OWNER: /*0x14*/
      if(win) {
	retval = 1;
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
      else
      {
	retval = 0;
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
      
      retval = 1;
      break;
      
    case WF_UNICONIFY: /* 0x1b */
      retval = 1;
      ret->parm1 = win->origsize.x;
      ret->parm2 = win->origsize.y;
      ret->parm3 = win->origsize.width;
      ret->parm4 = win->origsize.height;
      break;
      
    case WF_WINX:
    case WF_WINXCFG:
      retval = 0;
      break;
      
    default:
      DB_printf("%s: Line %d: srv_wind_get:\r\n"
		"Unknown mode %d  (0x%x) wind_get(%d,%d,...)",
		__FILE__,__LINE__,msg->mode,msg->mode,
		msg->handle,msg->mode);
      retval = 0;
    }
  } else {
    retval = 0;
  }

  PUT_R_ALL(WIND_GET, ret, retval);
}


/*
** Exported
*/
void
srv_wind_find (C_WIND_FIND * par,
               R_WIND_FIND * ret) {
  WINLIST * l = win_vis;
  WORD      retval = 0;

  while(l) {
    if (srv_inside (&l->win->totsize, par->x, par->y)) {
      retval = l->win->id;
      break;
    }
    
    l = l->next;
  }

  PUT_R_ALL(WIND_FIND, ret, retval);
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


/*
** Description
** Output debugging information about windowing functions
*/
void
srv_wind_debug(int mouse_x,
               int mouse_y)
{
  DB_printf("Mouse click owner: %d", srv_click_owner(mouse_x, mouse_y));
}


/* Description
** Create/fetch a free window structure/id
*/
WINSTRUCT *
winalloc(void)
{
  WINLIST * wl;
  
  if(win_free)
  {
    wl = win_free;
    win_free = win_free->next;
  }
  else
  {
    DEBUG2 ("srv.c: winalloc: Allocating memory");
    wl = (WINLIST *)malloc(sizeof(WINLIST));
    wl->win = (WINSTRUCT *)malloc(sizeof(WINSTRUCT));
    wl->win->id = win_next;
    win_next++;
  }
	
  wl->next = win_list;
  win_list = wl;
  
  return wl->win;
}


/*
** Description
** Initialize windows
*/
void
srv_init_windows(void)
{
  RECT          r;
  C_WIND_CREATE c_wind_create;
  R_WIND_CREATE r_wind_create;
  C_WIND_GET    c_wind_get;
  R_WIND_GET    r_wind_get;
  C_WIND_OPEN   c_wind_open;
  R_WIND_OPEN   r_wind_open;

  WINSTRUCT *   ws;
  WINLIST *     wl;

  /* Initialize desktop background */
  DEBUG3 ("srv.c: Initializing desktop window");
  ws = winalloc();
  ws->status = WIN_OPEN | WIN_DESKTOP | WIN_UNTOPPABLE;
  ws->owner = 0;
  
  
  ws->tree = NULL;
  
  ws->worksize.x = globals.screen.x;
  ws->worksize.y = globals.screen.y + globals.clheight + 3;
  ws->worksize.width = globals.screen.width;
  ws->worksize.height = globals.screen.height - globals.clheight - 3;
  
  ws->maxsize = ws->totsize = ws->worksize;
  
  wl = (WINLIST *)malloc(sizeof(WINLIST));
  
  wl->win = ws;
  
  wl->next = win_vis;
  win_vis = wl;

  wl->win->rlist = (RLIST *)malloc(sizeof(RLIST));
  wl->win->rlist->r.x = globals.screen.x;
  wl->win->rlist->r.y = globals.screen.y;
  wl->win->rlist->r.width = globals.screen.width;
  wl->win->rlist->r.height = globals.screen.height;
  wl->win->rlist->next = NULL;

  c_wind_get.handle = 0;
  c_wind_get.mode = WF_FULLXYWH;
  srv_wind_get (&c_wind_get, &r_wind_get);
  r = *((RECT *)&r_wind_get.parm1);

  r.height = r.y;
  r.y = 0;

  DEBUG3 ("srv.c: Creating menu bar window");
  c_wind_create.common.apid = 0;
  c_wind_create.elements = 0;
  c_wind_create.maxsize = r;
  c_wind_create.status = WIN_MENU;

  srv_wind_create (&c_wind_create, &r_wind_create);

  c_wind_open.id = r_wind_create.common.retval;
  c_wind_open.size = r;
  srv_wind_open (&c_wind_open,
                 &r_wind_open);
}
