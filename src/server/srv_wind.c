/*
** srv_wind.c
**
** Copyright 1999-2000 Christer Gustavsson <cg@nocrew.org>
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
#include "rlist.h"
#include "srv_appl.h"
#include "srv_comm.h"
#include "srv_global.h"
#include "srv_malloc.h"
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

static OBJC_COLORWORD top_colour[20] = {
  {BLACK,BLACK,1,7,LWHITE},
  {BLACK,BLACK,1,7,LWHITE},
  {BLACK,BLACK,1,7,LWHITE},
  {BLACK,BLACK,0,7,LYELLOW},
  {BLACK,BLACK,1,7,LWHITE},
  {BLACK,BLACK,0,7,WHITE},
  {BLACK,BLACK,1,7,LWHITE},
  {BLACK,BLACK,1,7,LWHITE},
  {BLACK,BLACK,1,7,LWHITE},
  {BLACK,BLACK,1,7,LWHITE},
  {BLACK,BLACK,1,7,LWHITE},
  {BLACK,BLACK,1,7,LWHITE},
  {BLACK,BLACK,1,7,LWHITE},
  {BLACK,BLACK,1,7,LWHITE},
  {BLACK,BLACK,1,7,LWHITE},
  {BLACK,BLACK,1,7,LWHITE},
  {BLACK,BLACK,1,7,LWHITE},
  {BLACK,BLACK,1,7,LWHITE},
  {BLACK,BLACK,1,7,LWHITE},
  {BLACK,BLACK,1,7,LWHITE}
};

static OBJC_COLORWORD untop_colour[20] = {
  {BLACK,BLACK,1,7,LWHITE},
  {BLACK,BLACK,1,7,LWHITE},
  {BLACK,BLACK,1,7,LWHITE},
  {BLACK,BLACK,1,7,LWHITE},
  {BLACK,BLACK,1,7,LWHITE},
  {BLACK,BLACK,0,7,WHITE},
  {BLACK,BLACK,1,7,LWHITE},
  {BLACK,BLACK,1,7,LWHITE},
  {BLACK,BLACK,1,7,LWHITE},
  {BLACK,BLACK,1,7,LWHITE},
  {BLACK,BLACK,1,7,LWHITE},
  {BLACK,BLACK,1,7,LWHITE},
  {BLACK,BLACK,1,7,LWHITE},
  {BLACK,BLACK,1,7,LWHITE},
  {BLACK,BLACK,1,7,LWHITE},
  {BLACK,BLACK,1,7,LWHITE},
  {BLACK,BLACK,1,7,LWHITE},
  {BLACK,BLACK,1,7,LWHITE},
  {BLACK,BLACK,1,7,LWHITE},
  {BLACK,BLACK,1,7,LWHITE}
};

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
static
inline
LOCK_INFO
lialloc(COMM_HANDLE handle,
        WORD        apid)
{
  LOCK_INFO li;

  if (free_lock_info == LOCK_INFO_NIL) {
    DEBUG3 ("srv_wind.c: lialloc: allocating new structure!");
    li = (LOCK_INFO)MALLOC(sizeof (LOCK_INFO_S));
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
static
inline
void
lifree(LOCK_INFO     li,
       COMM_HANDLE * handle,
       WORD *        apid)
{
  *handle = li->handle;
  *apid = li->apid;

  li->next = free_lock_info;
  free_lock_info = li;
}


/*
** Description
** Get lock
*/
static
inline
void
get_lock(COMM_HANDLE handle,
         WORD        apid,
         WORD *      lock,
         WORD *      lock_cnt,
         QUEUE       lock_q)
{
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
static
inline
void
return_lock(COMM_HANDLE handle,
            WORD        apid,
            WORD *      lock,
            WORD *      lock_cnt,
            QUEUE       lock_q)
{
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
** Description
** Find the window structure of the window with identification number id
*/
WINSTRUCT *
find_wind_description(WORD id)
{
  WINLIST * wl;
  
  wl = win_list;
  
  while(wl)
  {
    if(wl->win->id == id)
    {
      return wl->win;
    }
    
    wl = wl->next;
  }
  
  return NULL;
}


/*
** Description
** Change window slider position and size
*/
static
WORD
changeslider (WINSTRUCT * win,
              WORD        redraw,
              WORD        which,
              WORD        position,
              WORD        size)
{
  if(which & VSLIDE) {
    if(position != -1) {
      if(position > 1000) {
        win->vslidepos = 1000;
      } else if(position < 1) {
        win->vslidepos = 1;
      } else {
        win->vslidepos = position;
      }
    }
		
    if(size != -1) {
      if(size > 1000) {
        win->vslidesize = 1000;
      } else if(size < 1) {
        win->vslidesize = 1;
      } else {
        win->vslidesize = size;
      }
    }
  }
  
  if(which & HSLIDE) {
    if(position != -1) {
      if(position > 1000) {
        win->hslidepos = 1000;
      } else if (position < 1) {
        win->hslidepos = 1;
      } else {
        win->hslidepos = position;
      }
    }
    
    if(size != -1) {
      if(size > 1000) {
        win->hslidesize = 1000;
      } else if(size < 1) {
        win->hslidesize = 1;
      } else {
        win->hslidesize = size;
      }
    }
  }

  return 1;
}


/*
** Description
** setwinsize sets the size and position of window win to size
*/
static
void
setwinsize (WINSTRUCT * win,
            RECT      * size)
{
  WORD	dx,dy,dw,dh;

  win->lastsize = win->totsize;
	
  win->totsize = * size;
	
  dx = size->x - win->lastsize.x;
  dy = size->y - win->lastsize.y;
  dw = size->width - win->lastsize.width;
  dh = size->height - win->lastsize.height;
	
  win->worksize.x += dx;
  win->worksize.y += dy;
  win->worksize.width += dw;
  win->worksize.height += dh;

  /*
  if(win->tree) {
    win->tree[0].ob_x = win->totsize.x;
    win->tree[0].ob_y = win->totsize.y;
    win->tree[0].ob_width = win->totsize.width;
    win->tree[0].ob_height = win->totsize.height;
	
    win->tree[WMOVER].ob_width += dw;
		
    win->tree[WFULLER].ob_x += dw;

    win->tree[WSMALLER].ob_x += dw;
	
    win->tree[WDOWN].ob_x += dw;
    win->tree[WDOWN].ob_y += dh;	
	
    win->tree[WSIZER].ob_x += dw;
    win->tree[WSIZER].ob_y += dh;	
	
    win->tree[WRIGHT].ob_x += dw;
    win->tree[WRIGHT].ob_y += dh;	
	
    win->tree[WLEFT].ob_y += dh;	
	
    win->tree[WVSB].ob_x += dw;
    win->tree[WVSB].ob_height += dh;	
	
    win->tree[WHSB].ob_y += dh;
    win->tree[WHSB].ob_width += dw;	
	
    win->tree[WINFO].ob_width += dw;
		
    win->tree[WUP].ob_x += dw;
		
    win->tree[TFILLOUT].ob_width += dw;
	
    win->tree[RFILLOUT].ob_height += dh;
    win->tree[RFILLOUT].ob_x += dw;
	
    win->tree[BFILLOUT].ob_width += dw;
    win->tree[BFILLOUT].ob_y += dy;
	
    win->tree[SFILLOUT].ob_x += dw;
    win->tree[SFILLOUT].ob_y += dh;

    win->tree[WAPP].ob_width = win->tree[WMOVER].ob_width;
		
    changeslider(win,0,HSLIDE | VSLIDE,-1,-1);
  }
  */
}


/*
** Description
** Get the owner of the top desktop
*/
static
WORD
get_deskbg_owner(void)
{
  AP_INFO * ai;
  
  ai = search_appl_info (DESK_OWNER);
  
  if(ai != AP_INFO_REF_NIL)
  {
    return ai->id;
  }
  else
  {
    return -1;
  }
}


/*
** Description
** Draw the elements of the window win that intersects with the rectangle
**  r
*/
static
void
draw_wind_elements(WINSTRUCT * win,
                   RECT *      r,
                   WORD start)
{
  RLIST	*rl = win->rlist;

  if((win->id != 0) && win->tree)
  {	
    while(rl) {		
      RECT	r2;
		
      if(srv_intersect(&rl->r,r,&r2)) {
        /*
          Objc_do_draw(win->tree,start,3,&r2);
          */
      }
			
      rl = rl->next;
    }
  }
}


/*
** Description
** Change the size of a window
*/
static
WORD
changewinsize (WINSTRUCT * win,
               RECT *      size,
               WORD        vid,
               WORD        drawall) {	
  WORD dx = size->x - win->totsize.x;
  WORD dy = size->y - win->totsize.y;
  WORD dw = size->width - win->totsize.width;
  WORD dh = size->height - win->totsize.height;
  RECT oldtotsize = win->totsize;
  RECT oldworksize = win->worksize;
  
  setwinsize(win,size);
  
  if (win->status & WIN_OPEN) {
    if(dx || dy) { /* pos (and maybe size) is to be changed */
      REDRAWSTRUCT	m;
      
      WINLIST	*wl = win_vis;
      
      while(wl) {
        if(wl->win == win) {
          wl = wl->next;
          break;
        }
        
        wl = wl->next;
      }
      
      if(wl) {
	RLIST	*rlwalk;
	RLIST	*rlournew = NULL;
	RLIST	*rlourold = win->rlist;
	RLIST	*old_rlist = Rlist_duplicate(win->rlist);
	RLIST	*rlmove = NULL;
	RLIST *rlmove1 = NULL;
	WINLIST *wlwalk = wl;
	
	win->rlist = 0L;
        
	/*grab the rectangles we need from our old list*/
	
	Rlist_rectinter(&rlournew,size,&rlourold);
	
	while(wlwalk) {
	  /*get the new rectangles we need*/
	  
	  Rlist_rectinter(&rlournew,size,&wlwalk->win->rlist);
	  
	  wlwalk = wlwalk->next;
	}
	
	Rlist_insert(&win->rlist,&rlournew);
	
	if (drawall) {
	  C_APPL_WRITE c_appl_write;
	  R_APPL_WRITE r_appl_write;
          
	  m.type = WM_REDRAW;
	  
	  if (FALSE /*globals.realmove*/) {
	    m.sid = -1;
	  } else {
	    m.sid = 0;
	  }
	  
	  m.length = 0;
	  m.wid = win->id;
	  
	  m.area = *size;
	  
          /* FIXME
	  draw_wind_elements(win,&m.area,0);
	  */

	  if (FALSE /*globals.realmove*/) { /* FIXME */
	    m.area.x = 0;
	    m.area.y = 0;
	  }
	  
	  c_appl_write.addressee = win->owner;
	  c_appl_write.length = MSG_LENGTH;
          c_appl_write.is_reference = TRUE;
	  c_appl_write.msg.ref = &m;
	  srv_appl_write (&c_appl_write, &r_appl_write);
	} else {			
	  /* figure out which rectangles that are moveable*/
	  if(dw || dh) {
	    Rlist_rectinter (&rlmove1, &win->worksize, &win->rlist);
	  } else {
	    rlmove1 = win->rlist;
	    win->rlist = NULL;
	  }
	  
	  rlwalk = old_rlist;
	  
	  while (rlwalk) {
	    rlwalk->r.x += dx;
	    rlwalk->r.y += dy;
	    
	    Rlist_rectinter(&rlmove,&rlwalk->r,&rlmove1);
	    
	    rlwalk = rlwalk->next;
	  }
	  
	  /* move the rectangles that are moveable */
	  Rlist_sort(&rlmove,dx,dy);
	  
	  rlwalk = rlmove;
	  
	  v_hide_c (vid);
	  
          /*
          ** FIXME
          ** All drawing should be done by the client. Implement a way of
          ** returning which rectangles are moveable to the client.
          */
	  while (rlwalk) {
	    MFDB mfdbd, mfdbs;
	    int  koordl[8];
	    
	    mfdbd.fd_addr = 0L;
	    mfdbs.fd_addr = 0L;
	    
	    koordl[4] = rlwalk->r.x;
	    koordl[5] = rlwalk->r.y + rlwalk->r.height - 1;
	    koordl[6] = rlwalk->r.x + rlwalk->r.width - 1;
	    koordl[7] = rlwalk->r.y;
	    koordl[0] = koordl[4] - dx;
	    koordl[1] = koordl[5] - dy;
	    koordl[2] = koordl[6] - dx;
	    koordl[3] = koordl[7] - dy;
	    
	    vro_cpyfm (vid, S_ONLY, koordl, &mfdbs, &mfdbd);
	    
	    rlwalk = rlwalk->next;
	  }
	  
	  v_show_c (vid, 1);
	  
	  /* update rectangles that are not moveable */
	  
	  m.type = WM_REDRAW;
	  
	  if (FALSE /*globals.realmove*/) { /* FIXME */
	    m.sid = -1;
	  } else {
	    m.sid = 0;
	  }
	  
	  m.length = 0;
	  m.wid = win->id;
	  
	  Rlist_insert(&win->rlist,&rlmove1);
	  
	  rlwalk = win->rlist;
	  
	  while (rlwalk) {
	    C_APPL_WRITE c_appl_write;
            R_APPL_WRITE r_appl_write;

	    m.area.x = rlwalk->r.x;
	    m.area.y = rlwalk->r.y;
	    
	    m.area.width = rlwalk->r.width;
	    m.area.height = rlwalk->r.height;
	    
            /* FIXME
	    draw_wind_elemfast(win,&m.area,0);
	    */

	    if (FALSE /*globals.realmove*/) { /* FIXME */
	      m.area.x -= size->x;
	      m.area.y -= size->y;
	    }
	    
	    c_appl_write.addressee = win->owner;
	    c_appl_write.length = MSG_LENGTH;
            c_appl_write.is_reference = TRUE;
	    c_appl_write.msg.ref = &m;
	    srv_appl_write (&c_appl_write, &r_appl_write);
	    
	    rlwalk = rlwalk->next;
	  }
	  
	  Rlist_insert(&win->rlist,&rlmove);
	}
	
	Rlist_erase (&old_rlist);
	
	wlwalk = wl;
	
	while (wlwalk) {
	  RLIST	*rltheirnew = NULL;
	  
	  /*give away the rectangles we don't need*/
	  Rlist_rectinter(&rltheirnew,&wlwalk->win->totsize,&rlourold);
	  
	  /*update the new rectangles*/
	  
	  rlwalk = rltheirnew;
	  
	  if (rltheirnew) {
	    C_APPL_WRITE c_appl_write;
            R_APPL_WRITE r_appl_write;

	    m.type = WM_REDRAW;
	    
	    if (FALSE /*globals.realmove*/) { /* FIXME */
	      m.sid = -1;
	    } else {
	      m.sid = 0;
	    }
	    
	    m.length = 0;
	    m.wid = wlwalk->win->id;
	    
	    m.area = oldtotsize;
	    
	    if (FALSE /*globals.realmove*/) { /* FIXME */
	      m.area.x -= wlwalk->win->totsize.x;
	      m.area.y -= wlwalk->win->totsize.y;
	    }
	    
	    c_appl_write.addressee = wlwalk->win->owner;
	    c_appl_write.length = MSG_LENGTH;
            c_appl_write.is_reference = TRUE;
	    c_appl_write.msg.ref = &m;
	    srv_appl_write (&c_appl_write, &r_appl_write);
	  }
	  
	  Rlist_insert(&wlwalk->win->rlist,&rltheirnew);
	  
	  wlwalk->win->rpos = wlwalk->win->rlist;
	  
	  wlwalk = wlwalk->next;
	}
	
	win->rpos = win->rlist;
      }
    } else if (dw || dh) /*only size changed*/ {
      RECT	rt;
      RECT	dn;
      
      REDRAWSTRUCT	m;
      
      RLIST	*rl = 0L;
      RLIST	*rl2;
      RLIST	*rltop = 0L;
      
      WINLIST	*wl = win_vis;
      
      
      rt.x = size->x + size->width;
      
      if(dw < 0) {
	rt.width = -dw;
      } else {
	rt.x -= dw;
	rt.width = dw;
      }
      
      rt.y = size->y;
      rt.height = size->height;
      
      dn.y = size->y + size->height;
      
      if(dh < 0) {
	dn.height = -dh;
	dn.width = oldtotsize.width;
      } else {
	dn.y -= dh;
	dn.height = dh;
	dn.width = size->width;
      }
      
      dn.x = size->x;
      
      if(dw < 0) {
	Rlist_rectinter(&rl,&rt,&win->rlist);
      }
      
      if(dh < 0) {
	Rlist_rectinter(&rl,&dn,&win->rlist);
      }
      
      /* Find our window */
      
      while(wl) {
	if(wl->win == win) {
	  wl = wl->next;
	  break;
	}
	
	wl = wl->next;
      }
      
      while(wl) {
	RLIST	*rd = 0;
	
	if(dw < 0) {
	  Rlist_rectinter(&rd,&wl->win->totsize,&rl);
	} else if(dw > 0) {
	  Rlist_rectinter(&rltop,&rt,&wl->win->rlist);
	}
	
	if(dh < 0) {
	  Rlist_rectinter(&rd,&wl->win->totsize,&rl);
	} else if(dh > 0) {
	  Rlist_rectinter(&rltop,&dn,&wl->win->rlist);
	}
	
	rl2 = rd;
	
	m.type = WM_REDRAW;
	
	if(FALSE /*globals.realmove*/) { /* FIXME */
	  m.sid = -1;
	} else {
	  m.sid = 0;
	}
	
	m.length = 0;

	if (rd) {
	  C_APPL_WRITE c_appl_write;
          R_APPL_WRITE r_appl_write;
          
	  m.wid = wl->win->id;
	  
	  m.area = oldtotsize;
	  
	  if (FALSE /*globals.realmove*/) { /* FIXME */
	    m.area.x -= wl->win->totsize.x;
	    m.area.y -= wl->win->totsize.y;
	  }

	  c_appl_write.addressee = wl->win->owner;
	  c_appl_write.length = MSG_LENGTH;
          c_appl_write.is_reference = TRUE;
	  c_appl_write.msg.ref = &m;
	  srv_appl_write (&c_appl_write, &r_appl_write);
	}
	
	Rlist_insert(&wl->win->rlist,&rd);
				
	wl->win->rpos = wl->win->rlist;
	
	wl = wl->next;
      }
      
      rl2 = 0;
      
      Rlist_rectinter(&rl2,&oldworksize,&win->rlist);
      
      Rlist_rectinter(&rltop,&win->worksize,&win->rlist);
      
      Rlist_insert(&win->rlist,&rl2);
      
      rl2 = rltop;
      rltop = 0L;
      
      Rlist_insert(&rltop,&rl2);
      
      m.wid = win->id;
      
      rl2 = rltop;
      
      while (rl2) {
	C_APPL_WRITE c_appl_write;
        R_APPL_WRITE r_appl_write;

	m.area.x = rl2->r.x;
	m.area.y = rl2->r.y;
	
	if (FALSE /*globals.realmove*/) { /* FIXME */
	  m.area.x -= size->x;
	  m.area.y -= size->y;
	}
	
	m.area.width = rl2->r.width;
	m.area.height = rl2->r.height;
	
	c_appl_write.addressee = win->owner;
	c_appl_write.length = MSG_LENGTH;
        c_appl_write.is_reference = TRUE;
	c_appl_write.msg.ref = &m;
	srv_appl_write (&c_appl_write, &r_appl_write);
	
	rl2 = rl2->next;
      }
      
      Rlist_insert(&win->rlist,&rltop);
      
      rl2 = win->rlist;
      
      while(rl2) {
	/* FIXME
        draw_wind_elemfast(win,&rl2->r,0);
	*/

	rl2 = rl2->next;
      }
      
      win->rpos = win->rlist;
    }
  }
  
  return 1;
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
      
    case WF_DCOLOR: /*0x13*/
      *(OBJC_COLORWORD *)&ret->parm2 = top_colour[msg->parm1];
      *(OBJC_COLORWORD *)&ret->parm3 = untop_colour[msg->parm1];
      
      retval = 0;
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
** Description
** Implementation of wind_delete()
*/
void
srv_wind_delete (C_WIND_DELETE * msg,
                 R_WIND_DELETE * ret)
{
  WINLIST ** wl;
  WORD       retval;

  wl = &win_list;
  
  while(*wl) {		
    if((*wl)->win->id == msg->id)
      break;
    
    wl = &(*wl)->next;
  }
  
  if(!(*wl)) {
    /* No such window found! */
    retval = 0;
  } else {
    /* We found the window */
    if((*wl)->win->status & WIN_OPEN) {
      C_WIND_CLOSE c_wind_close;
      R_WIND_CLOSE r_wind_close;

      c_wind_close.id = msg->id;
      
      srv_wind_close (&c_wind_close,
                      &r_wind_close);
    }

    /* FIXME : Remove
    delwinelem((*wl)->win->tree);
    */

    if(msg->id == (win_next - 1)) {
      WORD	found;
      WINLIST	*wt = *wl;
      
      *wl = (*wl)->next;
      
      FREE(wt->win);
      FREE(wt);
      win_next--;
      
      do {
        WINLIST 	**wlwalk = &win_list;
        
        found = 0;
        
        while(*wlwalk) {
          if((*wlwalk)->win->id == (win_next - 1))
	  {
	    found = 1;
	    break;
	  }
          
          wlwalk = &(*wlwalk)->next;
        }
        
        if(!found) {
          wlwalk = &win_free;
          
          while(*wlwalk) {
            if((*wlwalk)->win->id == (win_next - 1)) {
              WINLIST	*wltemp = *wlwalk;
              
              *wlwalk = (*wlwalk)->next;
              
              FREE(wltemp->win);
              FREE(wltemp);
              
              win_next--;
              
              break;
            }
            
            wlwalk = &(*wlwalk)->next;
          }
        }
      }while(!found && win_next);
    } else {
      WINLIST	*wt = *wl;
      
      *wl = (*wl)->next;
      
      wt->next = win_free;
      win_free = wt;
    }
    
    retval = 1;
  }

  PUT_R_ALL(WIND_DELETE, ret, retval);
}


/*
** Description
** Implementation of wind_open ()
*/
void
srv_wind_open (C_WIND_OPEN * msg,
               R_WIND_OPEN * ret)
{
  WINLIST      *wl,*wp;
  WINSTRUCT    *ws,*oldtop = NULL;
  RLIST	       *rl = 0L;
  REDRAWSTRUCT m;
  WORD         owner;
  WORD         wastopped = 0;
  WORD         retval;

  retval = 1;

  ws = find_wind_description (msg->id);
  
  if(ws) {
    if(!(ws->status & WIN_OPEN)) {
      DEBUG2 ("srv.c: srv_wind_open: Allocating memory");
      wl = (WINLIST *)MALLOC(sizeof(WINLIST));
      
      wl->win = ws;
      
      setwinsize (wl->win, &msg->size);
      
      if (win_vis) {
	if(win_vis->win->status & WIN_DIALOG) {
	  wl->next = win_vis->next;
	  win_vis->next = wl;
	  wl->win->status &= ~WIN_TOPPED;
	} else {					
	  oldtop = win_vis->win;
	  wl->next = win_vis;
	  win_vis = wl;
	  
	  if (!(wl->win->status & WIN_MENU)) {
	    wl->win->status |= WIN_TOPPED;
	    oldtop->status &= ~WIN_TOPPED;
	    wastopped = 1;
	  }
	}
      } else {	
	wl->next = 0L;
	win_vis = wl;
	wl->win->status |= WIN_TOPPED;
      }
      
      wp = wl->next;
      
      while (wp != NULL) {
	RLIST	*rd = 0L;
	
	Rlist_rectinter(&rl,&wl->win->totsize,&wp->win->rlist);
	
	Rlist_insert(&rd,&wp->win->rlist);
	
        wp->win->rlist = rd;
	wp = wp->next;
      }
      
      wl->win->rlist = 0L;
      Rlist_insert(&wl->win->rlist,&rl);
      
      wl->win->status |= WIN_OPEN;
      
      /*
      for(i = 0; i <= W_SMALLER; i++) {
	set_widget_colour(ws,i,&ws->untop_colour[i],&ws->top_colour[i]);
      };
      */
      
      if (!(wl->win->status & WIN_DIALOG)){
	C_APPL_WRITE c_appl_write;
        R_APPL_WRITE r_appl_write;

	m.type = WM_REDRAW;
	m.length = 0;
        
	/*
          if(globals.realmove) {
        m.sid = -1; */
        /*x and y are relative to the position of the window*/
        /*	  m.area.x = 0;
                  m.area.y = 0;
          } else {
        */
        m.sid = 0;
        m.area.x = wl->win->totsize.x;
        m.area.y = wl->win->totsize.y;
        /*	}*/
	
	m.area.width = wl->win->totsize.width;
	m.area.height = wl->win->totsize.height;
	
	m.wid = wl->win->id;
	
	owner = wl->win->owner;
	
        /*
	draw_wind_elements(wl->win,&wl->win->totsize,0);
        */

	c_appl_write.addressee = owner;
	c_appl_write.length = MSG_LENGTH;
        c_appl_write.is_reference = TRUE;
	c_appl_write.msg.ref = &m;
	srv_appl_write (&c_appl_write, &r_appl_write);
      }
      
      /*
      if(wastopped) {
	WORD           i;
	C_APPL_CONTROL c_appl_control;
	
	for(i = 0; i <= W_SMALLER; i++) {
	  set_widget_colour(oldtop,i,&oldtop->untop_colour[i],
			    &oldtop->top_colour[i]);
	}
	
	draw_wind_elements (oldtop,&oldtop->totsize,0);
	
	c_appl_control.apid = wl->win->owner;
	c_appl_control.mode = APC_TOP;
	srv_appl_control(&c_appl_control);
      }
      */
    } else {
      retval = 0;
    }
  } else {
    retval = 0;
  }
  
  PUT_R_ALL(WIND_OPEN, ret, retval);
}


/*
** Exported
*/
WORD
srv_click_owner(WORD mouse_x,
		WORD mouse_y)
{
  if (mouse_cnt > 0)
  {
    return mouse_lock;
  }
  else
  {
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

    return r_wind_get.parm1; /* The owner */
  }
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
    wl = (WINLIST *)MALLOC(sizeof(WINLIST));
    wl->win = (WINSTRUCT *)MALLOC(sizeof(WINSTRUCT));
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
  int           i;

  RECT          r;
  C_WIND_CREATE c_wind_create;
  R_WIND_CREATE r_wind_create;
  C_WIND_GET    c_wind_get;
  R_WIND_GET    r_wind_get;
  C_WIND_OPEN   c_wind_open;
  R_WIND_OPEN   r_wind_open;

  WINSTRUCT *   ws;
  WINLIST *     wl;

  /* Initialize colours when we don't have enough of them */
  if(globals.num_pens < 16) {
    for(i = 0; i <= W_SMALLER; i++) {
      untop_colour[i].pattern = 0;
      untop_colour[i].fillc = WHITE;
      top_colour[i].pattern = 0;
      top_colour[i].fillc = WHITE;
    }
  }
  
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
  
  wl = (WINLIST *)MALLOC(sizeof(WINLIST));
  
  wl->win = ws;
  
  wl->next = win_vis;
  win_vis = wl;

  wl->win->rlist = (RLIST *)MALLOC(sizeof(RLIST));
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


/*
** Description
** Update the owner of the desktop window
*/
void
update_desktop_owner(void)
{
  WINSTRUCT *win;
  
  win = find_wind_description(DESKTOP_WINDOW);
  
  if (win != NULL)
  {
    win->owner = get_desktop_owner_id ();
  }
}


/*
** Description
** Set the resource tree of the desktop of an application
*/
static
WORD
set_desktop_background(WORD        apid,
                       SRV_FEATURE state)
{
  if(apps[apid].id != -1)
  {
    WORD old_deskbg_owner;
  
    old_deskbg_owner = get_deskbg_owner();

    apps[apid].deskbg = state;
    
    update_desktop_owner();

    if((apid == old_deskbg_owner) || (apid == get_deskbg_owner()))
    {
      update_desktop_background();
    }
    
    return 0;
  }

  return -1;
}


/*
** Description
** Try to top window
*/
WORD
top_window (WORD winid)
{
  WORD         wastopped = 0;
  WINSTRUCT    *oldtop = NULL;
  REDRAWSTRUCT m;
  
  RLIST        *rl = 0L;
  RLIST	       *rl2 = 0L;
  
  WINLIST      **wl = &win_vis;
  WINLIST      *wl2;
  WINLIST      *ourwl;
  
  WORD	dx,dy;

  if(winid == 0) {
    if(win_vis && (win_vis->win->status & WIN_TOPPED)) {
      win_vis->win->status &= ~WIN_TOPPED;
      
      /*
      ** FIXME
      ** Move? Remove?
      WORD i;
      
      for(i = 0; i <= W_SMALLER; i++) {
	set_widget_colour(win_vis->win,i,&win_vis->win->untop_colour[i],
			  &win_vis->win->top_colour[i]);
      };
      
      draw_wind_elements(win_vis->win,&win_vis->win->totsize,0);
      */
    };
  } else {
    while(*wl) {
      if((*wl)->win->id == winid)
	break;
      
      wl = &(*wl)->next;
    }
    
    if (*wl == NULL) {
      return 0;
    }

    wl2 = *wl;
    
    *wl = (*wl)->next;
    
    if(win_vis) {
      if(win_vis->win->status & WIN_DIALOG) {
	wl2->next = win_vis->next;
	win_vis->next = wl2;
      }
      else {
	oldtop = win_vis->win;
	wl2->next = win_vis;
	win_vis = wl2;
	
	if(!(wl2->win->status & WIN_TOPPED)) {
	  wl2->win->status |= WIN_TOPPED;
	  oldtop->status &= ~WIN_TOPPED;
	  
	  wastopped = 1;
	}
      }
    } else {	
      wl2->next = 0L;
      win_vis = wl2;
    }
    
    m.type = WM_REDRAW;
    
    if(globals.realmove) {
      m.sid = -1;
    } else {
      m.sid = 0;
    }
    
    ourwl = wl2;
    
    m.wid = ourwl->win->id;
    m.length = 0;
    
    dx = wl2->win->totsize.x;
    dy = wl2->win->totsize.y;	
    
    wl2 = wl2->next;
	
    while(wl2) {
      RLIST	*rd = 0L;
      
      Rlist_rectinter(&rl,&ourwl->win->totsize,&wl2->win->rlist);
      
      Rlist_insert(&rd,&wl2->win->rlist);
      
      wl2->win->rlist = rd;
      wl2->win->rpos = wl2->win->rlist;
      
      wl2 = wl2->next;
    }
    
    Rlist_insert(&rl2,&rl);
    
    rl = rl2;
    
    while(rl) {
      C_APPL_WRITE c_appl_write;
      R_APPL_WRITE r_appl_write;

      m.area.x = rl->r.x;
      m.area.y = rl->r.y;
      
      m.area.width = rl->r.width;
      m.area.height = rl->r.height;
      
      /*
      if(!wastopped) {
	draw_wind_elemfast(ourwl->win,&rl->r,0);
      }
      */
      
      if(globals.realmove) {
	m.area.x -= dx;
	m.area.y -= dy;
      }

      c_appl_write.addressee = ourwl->win->owner;
      c_appl_write.length = MSG_LENGTH;
      c_appl_write.is_reference = TRUE;
      c_appl_write.msg.ref = &m;
      srv_appl_write (&c_appl_write, &r_appl_write);
      
      rl = rl->next;
    }
    
    Rlist_insert(&ourwl->win->rlist,&rl2);
    
    ourwl->win->rpos = ourwl->win->rlist;
    
    if(wastopped) {
      C_APPL_CONTROL c_appl_control;
      R_APPL_CONTROL r_appl_control;
      
      /*
      ** FIXME
      ** Move? Remove?
      WORD i;
      for(i = 0; i <= W_SMALLER; i++) {
	set_widget_colour(oldtop,i,&oldtop->untop_colour[i],
			  &oldtop->top_colour[i]);
	set_widget_colour(ourwl->win,i,&ourwl->win->untop_colour[i],
			  &ourwl->win->top_colour[i]);
      };
	
      draw_wind_elements(ourwl->win,&ourwl->win->totsize,0);
      draw_wind_elements(oldtop,&oldtop->totsize,0);
      */

      c_appl_control.ap_id = ourwl->win->owner;
      c_appl_control.mode = APC_TOP;
      srv_appl_control (&c_appl_control,
                        &r_appl_control);
    }
  }
  
  return 1;
}


/*
** Description
** Put window under all other windows
*/
static
WORD
bottom_window (WORD winid)
{
  WORD          wastopped = 0;
  WINSTRUCT     *newtop = 0L;
  REDRAWSTRUCT	m;
  
  WINLIST	**wl = &win_vis;
  WINLIST	*ourwl;
  
  while(*wl) {
    if((*wl)->win->id == winid)
      break;
    
    wl = &(*wl)->next;
  };
  
  if(!*wl) {
    return 0;
  };
  
  ourwl = *wl;
  
  *wl = (*wl)->next;
  
  if(*wl) {
    if((*wl)->win->status & WIN_MENU) {
      wl = &(*wl)->next;
    };
  };
  
  if((ourwl->win->status & WIN_TOPPED) && *wl) {
    if(!((*wl)->win->status & WIN_DESKTOP)) {
      newtop = (*wl)->win;
      (*wl)->win->status |= WIN_TOPPED;
      ourwl->win->status &= ~WIN_TOPPED;
      wastopped = 1;
    };
  };
  
  m.type = WM_REDRAW;
  
  if(globals.realmove) {
    m.sid = -1;
  }
  else {
    m.sid = 0;
  };
  
  m.length = 0;
  
  while(*wl) {
    RLIST *newrects = 0L;
    
    if((*wl)->win->status & WIN_DESKTOP) {
      break;
    };
    
    Rlist_rectinter(&newrects,&(*wl)->win->totsize,&ourwl->win->rlist);
    
    Rlist_insert(&(*wl)->win->rlist,&newrects);
    
    if(!((*wl)->win->status & WIN_MENU)) {
      C_APPL_WRITE c_appl_write;
      R_APPL_WRITE r_appl_write;

      m.wid = (*wl)->win->id;
      
      m.area.x = ourwl->win->totsize.x;
      m.area.y = ourwl->win->totsize.y;
      
      m.area.width = ourwl->win->totsize.width;
      m.area.height = ourwl->win->totsize.height;
      
      /* FIXME
      if((*wl)->win != newtop) {
	draw_wind_elements((*wl)->win,&m.area,0);
      };
      */

      if(globals.realmove) {
	m.area.x -= (*wl)->win->totsize.x;
	m.area.y -= (*wl)->win->totsize.y;
      };

      c_appl_write.addressee = (*wl)->win->owner;
      c_appl_write.length = MSG_LENGTH;
      c_appl_write.is_reference = TRUE;
      c_appl_write.msg.ref = &m;
      srv_appl_write (&c_appl_write, &r_appl_write);
    };
    
    wl = &(*wl)->next;
  };
  
  ourwl->next = *wl;
  *wl = ourwl;
  
  if(wastopped) {
    C_APPL_CONTROL c_appl_control;
    R_APPL_CONTROL r_appl_control;

    /*
    ** FIXME
    ** Remove? Move?
    WORD           i;

    for(i = 0; i <= W_SMALLER; i++) {
      set_widget_colour(ourwl->win,i,&ourwl->win->untop_colour[i],&ourwl->win->top_colour[i]);
      set_widget_colour(newtop,i,&newtop->top_colour[i],&newtop->top_colour[i]);
    };
    */
    
    draw_wind_elements(ourwl->win,&ourwl->win->totsize,0);
    draw_wind_elements(newtop,&newtop->totsize,0);
    
    c_appl_control.ap_id = newtop->owner;
    c_appl_control.mode = APC_TOP;
    srv_appl_control (&c_appl_control,
                      &r_appl_control);
  }
  
  return 1;
}


/*wind_set 0x0069*/

/*
** Description
** Server part of wind_set ()
*/
void
srv_wind_set(C_WIND_SET * msg,
             R_WIND_SET * ret)
{
  WINSTRUCT * win;
  WORD        retval = 0;
  
  win = find_wind_description(msg->handle);
  
  if(win) {
    DEBUG2 ("srv.c: srv_wind_set: mode = %d (0x%x)", msg->mode, msg->mode);
    switch (msg->mode) {        
    case WF_NAME        :       /*0x0002*/
    case WF_INFO        :       /*0x0003*/
      DEBUG1 ("srv.c: srv_wind_set: no support for mode %d in server",
              msg->mode);
      retval = 0;
      break;
      
    case WF_CURRXYWH: /*0x0005*/
      retval = changewinsize (win,
                              (RECT *)&msg->parm1,
                              globals.vid,
                              0);
      break;

    case WF_HSLIDE: /*0x08*/
      retval = changeslider(win,1,HSLIDE,msg->parm1,-1);
      break;

    case WF_VSLIDE: /*0x09*/
      retval = changeslider(win,1,VSLIDE,msg->parm1,-1);
      break;

    case WF_TOP  :       /*0x000a*/
      retval = top_window(msg->handle);
      break;

    case WF_NEWDESK: /*0x000e*/
    {
      SRV_FEATURE deskbg_install;

      /* FIXME: Never pass addresses to the server */
      deskbg_install =
        ((OBJECT *)INTS2LONG(msg->parm1, msg->parm2) == NULL) ?
        NOT_INSTALLED :
        INSTALLED;
      
      if(set_desktop_background (msg->common.apid, deskbg_install) == 0)
      {
        retval = 1;
      }
      else
      {
        retval = 0;
      }
    }
    break;

    case WF_HSLSIZE: /*0x0f*/
      retval = changeslider(win,1,HSLIDE,-1,msg->parm1);
      break;

    case WF_VSLSIZE: /*0x10*/
      retval = changeslider(win,1,VSLIDE,-1,msg->parm1);
      break;
      
    case WF_COLOR:  /*0x12*/
      DB_printf("srv_wind_set WF_COLOR not implemented");
      retval = 0;
      break;

    case WF_DCOLOR: /*0x13*/
      top_colour[msg->parm1]   = *(OBJC_COLORWORD *)&msg->parm2;
      untop_colour[msg->parm1] = *(OBJC_COLORWORD *)&msg->parm3;
      
      retval = 0;
      break;

    case WF_BEVENT: /*0x18*/
      if(msg->parm1 & 1) {
        win->status |= WIN_UNTOPPABLE;
      } else {
        win->status &= ~WIN_UNTOPPABLE;
      }
      
      retval = 1;
      break;

    case WF_BOTTOM: /*0x0019*/
      retval = bottom_window(msg->handle);
      break;

    case WF_ICONIFY: /*0x1a*/
      /* FIXME:       win->origsize = win->totsize;
      set_win_elem(win->tree,IMOVER);
      win->status |= WIN_ICONIFIED;
      calcworksize(IMOVER,&win->totsize,&win->worksize,WC_WORK);
      retval = changewinsize(win,
                                         (RECT *)&msg->parm1,
                                         globals.vid,
                                         1);
      */
      break;

    case WF_UNICONIFY: /*0x1b*/
      /* FIXME:
      set_win_elem(win->tree,win->elements);
      win->status &= ~WIN_ICONIFIED;
      calcworksize(win->elements,&win->totsize,&win->worksize,WC_WORK);
      retval = changewinsize (win,
                                          (RECT *)&msg->parm1,
                                          globals.vid,
                                          1);
      */
      break;

    default:
      DB_printf("%s: Line %d: srv_wind_set:\r\n"
                "Unknown mode %d",__FILE__,__LINE__,msg->mode);
      retval = 0;
    }
  } else {
    retval = 0;     
  }

  PUT_R_ALL(WIND_SET, ret, retval);
}


/*
** Description
** Implementation of wind_new()
*/
WORD
srv_wind_new(WORD apid)
{
  WINLIST *     wl;

  wl = win_vis;
  
  while(wl)
  {
    if((wl->win->owner == apid) && !(wl->win->status & WIN_DESKTOP))
    {
      C_WIND_DELETE c_wind_delete;
      R_WIND_DELETE r_wind_delete;

      c_wind_delete.id = wl->win->id;
      srv_wind_delete(&c_wind_delete, &r_wind_delete);
      
      wl = win_vis;
    }
    else
    {
      wl = wl->next;
    }
  }
  
  wl = win_list;
  
  while(wl)
  {
    if(wl->win->owner == apid)
    {
      C_WIND_DELETE c_wind_delete;
      R_WIND_DELETE r_wind_delete;
      
      c_wind_delete.id = wl->win->id;
      srv_wind_delete(&c_wind_delete, &r_wind_delete);
      
      wl = win_list;
    }
    else
    {	
      wl = wl->next;
    }
  }
  
  return 1;
}


/*
** Description
** Implementation of wind_close ()
*/
void
srv_wind_close (C_WIND_CLOSE * par,
                R_WIND_CLOSE * ret)
{
  WINLIST **  wl = &win_vis;
  WINSTRUCT * newtop = NULL;
  WORD        retval;

  while (*wl) {
    if((*wl)->win->id == par->id) {
      break;
    }

    wl = &(*wl)->next;
  }
  
  if (*wl) {
    WINLIST      *wp = (*wl)->next;
    REDRAWSTRUCT m;

    if(((*wl)->win->status & WIN_TOPPED) && wp) {
      (*wl)->win->status &= ~WIN_TOPPED;
      wp->win->status |= WIN_TOPPED;
      newtop = wp->win;
    }
    
    while (wp) {			
      RLIST	*rl = NULL;
      RLIST	*tl;

      Rlist_rectinter (&rl, &wp->win->totsize, &(*wl)->win->rlist);

      /* Redraw "new" rectangles of windows below */
      
      if(rl) {
	C_APPL_WRITE c_appl_write;
        R_APPL_WRITE r_appl_write;

	if(!(wp->win->status & WIN_MENU)) {
	  m.type = WM_REDRAW;
	  m.length = 0;
	  m.wid = wp->win->id;
	  
	  if (FALSE /* FIXME globals.realmove*/) {
	    m.sid = -1;
	    m.area.x = (*wl)->win->totsize.x - wp->win->totsize.x;
	    m.area.y = (*wl)->win->totsize.y - wp->win->totsize.y;
	  } else {
	    m.sid = 0;
	    m.area.x = (*wl)->win->totsize.x;
	    m.area.y = (*wl)->win->totsize.y;
	  }
	  
	  m.area.width = (*wl)->win->totsize.width;
	  m.area.height = (*wl)->win->totsize.height;	

	  c_appl_write.addressee = wp->win->owner;
	  c_appl_write.length = MSG_LENGTH;
          c_appl_write.is_reference = TRUE;
	  c_appl_write.msg.ref = &m;

	  srv_appl_write (&c_appl_write, &r_appl_write);
	}
	
        /*
        ** FIXME
	if(wp->win != newtop) {
	  tl = rl;
	  
	  while (tl) {
	    draw_wind_elemfast(wp->win,&tl->r,0);
	    
	    tl = tl->next;
	  }
	}
        */
	
	Rlist_insert (&wp->win->rlist, &rl);
	wp->win->rpos = wp->win->rlist;
      }
      
      wp = wp->next;
    }		
    
    wp = *wl;
    
    *wl = (*wl)->next;
    
    wp->win->status &= ~WIN_OPEN;
    FREE(wp);
    
    if(newtop) {
      /*
      ** FIXME
      ** Move? Remove?
      WORD i;
      
      for(i = 0; i <= W_SMALLER; i++) {
	set_widget_colour(newtop,i,&newtop->untop_colour[i],
			  &newtop->top_colour[i]);
      };
      
      draw_wind_elements(newtop,&newtop->totsize,0);
      */
    }
    
    retval = 1;
  } else {
    retval = 0;
  }

  PUT_R_ALL(WIND_CLOSE, ret, retval);
}


/*
** Description
** Implementation of wind_create()
*/
void
srv_wind_create(C_WIND_CREATE * msg,
                R_WIND_CREATE * ret)
{
  WINSTRUCT	*ws;
  
  ws = winalloc();
  
  ws->status = msg->status;
  ws->owner = msg->common.apid; 

  ws->maxsize = msg->maxsize;

  ws->rlist = NULL;
  ws->rpos = NULL;
  
  ws->vslidepos = 1;
  ws->vslidesize = 1000;
  ws->hslidepos = 1;
  ws->hslidesize = 1000;

  /*
  if((ws->status & WIN_DIALOG) || (ws->status & WIN_MENU)) {
    ws->tree = 0L;
    ws->totsize = msg->maxsize;
    ws->worksize = ws->totsize;
  }
  else {
    WORD    i;
    AP_INFO *ai;
    
    ws->tree = alloc_win_elem();
    set_win_elem(ws->tree,msg->elements);
    ws->elements = msg->elements;
	
    ai = search_appl_info(msg->common.apid);
		
    if(ai) {
      ws->tree[WAPP].ob_spec.tedinfo->te_ptext =
	(char *)Mxalloc(strlen(&ai->name[2]) + 1,GLOBALMEM);
      strcpy(ws->tree[WAPP].ob_spec.tedinfo->te_ptext,&ai->name[2]);
      
      if(globals.wind_appl == 0) {
	ws->tree[WAPP].ob_spec.tedinfo->te_ptext[0] = 0;
      };
    };
    
    ws->totsize.x = ws->tree[0].ob_x;
    ws->totsize.y = ws->tree[0].ob_y;
    ws->totsize.width = ws->tree[0].ob_width;
    ws->totsize.height = ws->tree[0].ob_height;
    
    calcworksize(msg->elements,&ws->totsize,&ws->worksize,WC_WORK);
		
    for(i = 0; i <= W_SMALLER; i++) {
      ws->top_colour[i] = top_colour[i];
      ws->untop_colour[i] = untop_colour[i];
    }
    
    ws->own_colour = 0;
  };
  */

  PUT_R_ALL(WIND_CREATE, ret, ws->id);
}
