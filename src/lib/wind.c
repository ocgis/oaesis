/****************************************************************************

 Module
  wind.c
  
 Description
  Window routines used in oAESis.
  
 Author(s)
  cg     (Christer Gustavsson <d2cg@dtek.chalmers.se>)

 Revision history
 
  951226 cg
   Added standard header.
  960101 cg
   Added BEG_MCTRL and END_MCTRL modes to wind_update.
  960102 cg
   WF_TOP mode of wind_get() implemented.
  960103 cg
   WF_NEWDESK mode of wind_set() implemented.
   WF_HSLIDE, WF_VSLIDE, WF_VSLSIZE and WF_HSLSIZE modes of wind_set()
   and wind_get() implemented.
 
 Copyright notice
  The copyright to the program code herein belongs to the authors. It may
  be freely duplicated and distributed without fee, but not charged for.
 
 ****************************************************************************/

/****************************************************************************
 * Used interfaces                                                          *
 ****************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_ALLOC_H
#include	<alloc.h>
#endif

#ifdef HAVE_BASEPAGE_H
#include	<basepage.h>
#endif

#ifdef HAVE_MINTBIND_H
#include	<mintbind.h>
#endif

#ifdef HAVE_OSBIND_H
#include	<osbind.h>
#endif

#include	<signal.h>
#include	<stdio.h>
#include	<string.h>

#ifdef HAVE_SUPPORT_H
#include	<support.h>
#endif

#include <unistd.h>

#include "debug.h"
#include "evnthndl.h"
#include "gemdefs.h"
#include "lib_global.h"
#include "mintdefs.h"
#include "mesagdef.h"
#include "lib_misc.h"
#include "objc.h"
#include "resource.h"
#include "srv_calls.h"
#include "srv_interface.h"
#include "srv_put.h"
#include "types.h"
#include "vdi.h"
#include "wind.h"

/****************************************************************************
 * Macros                                                                   *
 ****************************************************************************/

#define IMOVER 0x8000  /* Used with set_win_elem() to make icon window */


/*
** Structures used with windows
*/
typedef struct window_struct {
  WORD     id;
  WORD     status;
  WORD     elements;
  RECT     maxsize;
  RECT     totsize;
  RECT     worksize;
  RECT     lastsize;
  OBJECT * tree;

  WORD hslidepos;     /*position of the horizontal slider*/
  WORD vslidepos;     /*position of the vertical slider*/
  WORD hslidesize;    /*size of the horizontal slider*/
  WORD vslidesize;   /*size of the vertical slider*/
  
  OBJC_COLORWORD top_colour[20];
  OBJC_COLORWORD untop_colour[20];
  WORD           own_colour;
} WINDOW_STRUCT;

typedef struct window_list {
  WINDOW_STRUCT        ws;
  struct window_list * next;
} WINDOW_LIST;
typedef WINDOW_LIST * WINDOW_LIST_REF;
#define WINDOW_LIST_REF_NIL ((WINDOW_LIST_REF)NULL)

static WORD	elemnumber = -1;
static WORD	tednumber;

static WORD widgetmap[] = {
	0,0,WCLOSER,WMOVER,WFULLER,
	WINFO,0,0,WSIZER,0,
	WUP,WDOWN,WVSB,WVSLIDER,0,
	WLEFT,WRIGHT,WHSB,WHSLIDER,WSMALLER
};

/*calcworksize calculates the worksize or the total size of
a window. If dir == WC_WORK the worksize will be calculated and 
otherwise the total size will be calculated.*/

static
void
calcworksize (WORD apid,
              WORD elem,
              RECT *orig,
              RECT *new,
              WORD dir) {
  WORD	bottomsize = 1;
  WORD	headsize = 1;
  WORD	leftsize = 1;
  WORD	rightsize = 1;
  WORD	topsize;
  GLOBAL_COMMON * globals_common = get_global_common ();
	
  if((HSLIDE | LFARROW | RTARROW) & elem) {
    bottomsize = globals_common->windowtad[WLEFT].ob_height + (D3DSIZE << 1);
  };
	
  if((CLOSER | MOVER | FULLER | NAME) & elem) {
    topsize = globals_common->windowtad[WMOVER].ob_height + (D3DSIZE << 1);
  }
  else if(IMOVER & elem) {
    topsize = globals_common->csheight + 2 + D3DSIZE * 2;
  }
  else {
    topsize = 0;
  };
	
  if(INFO & elem) {
    headsize = topsize + globals_common->windowtad[WINFO].ob_height + 2 * D3DSIZE;
  }
  else {
    if(topsize)
      headsize = topsize;
    else
      headsize = 1;
  };
	
  if((LFARROW | HSLIDE | RTARROW) & elem) {
    bottomsize = globals_common->windowtad[WLEFT].ob_height + (D3DSIZE << 1);
  };
	
  if(((bottomsize < globals_common->windowtad[WLEFT].ob_height) && (SIZER & elem))
     || ((VSLIDE | UPARROW | DNARROW) & elem))
  {
    rightsize = globals_common->windowtad[WSIZER].ob_width + (D3DSIZE << 1);
  }

  if(dir == WC_WORK) {
    new->x = orig->x + leftsize;
    new->y = orig->y + headsize;
    new->width = orig->width - leftsize - rightsize;
    new->height = orig->height - headsize - bottomsize;
  }
  else {
    new->x = orig->x - leftsize;
    new->y = orig->y - headsize;
    new->width = orig->width + leftsize + rightsize;
    new->height = orig->height + headsize + bottomsize;
  }
}

/*
** Description
** Wind_set_size sets the size and position of window <win> to <size>
**
** 1998-10-11 CG
*/
static
void
Wind_set_size (WORD   apid,
               WORD   id,
               RECT * size) {
  WORD	dx,dy,dw,dh;
  GLOBAL_APPL * globals = get_globals (apid);
  WINDOW_LIST * wl;

  wl = globals->windows;

  /* Find the window structure for our window */
  while (wl) {
    if (wl->ws.id == id) {
      break;
    }
  }

  if (wl != NULL) {
    wl->ws.lastsize = wl->ws.totsize;
    
    wl->ws.totsize = * size;
    
    dx = size->x - wl->ws.lastsize.x;
    dy = size->y - wl->ws.lastsize.y;
    dw = size->width - wl->ws.lastsize.width;
    dh = size->height - wl->ws.lastsize.height;
    
    wl->ws.worksize.x += dx;
    wl->ws.worksize.y += dy;
    wl->ws.worksize.width += dw;
    wl->ws.worksize.height += dh;
    
    if(wl->ws.tree) {
      wl->ws.tree[0].ob_x = wl->ws.totsize.x;
      wl->ws.tree[0].ob_y = wl->ws.totsize.y;
      wl->ws.tree[0].ob_width = wl->ws.totsize.width;
      wl->ws.tree[0].ob_height = wl->ws.totsize.height;
      
      wl->ws.tree[WMOVER].ob_width += dw;
      
      wl->ws.tree[WFULLER].ob_x += dw;
      
      wl->ws.tree[WSMALLER].ob_x += dw;
      
      wl->ws.tree[WDOWN].ob_x += dw;
      wl->ws.tree[WDOWN].ob_y += dh;	
      
      wl->ws.tree[WSIZER].ob_x += dw;
      wl->ws.tree[WSIZER].ob_y += dh;	
      
      wl->ws.tree[WRIGHT].ob_x += dw;
      wl->ws.tree[WRIGHT].ob_y += dh;	
      
      wl->ws.tree[WLEFT].ob_y += dh;	
      
      wl->ws.tree[WVSB].ob_x += dw;
      wl->ws.tree[WVSB].ob_height += dh;	
      
      wl->ws.tree[WHSB].ob_y += dh;
      wl->ws.tree[WHSB].ob_width += dw;	
      
      wl->ws.tree[WINFO].ob_width += dw;
      
      wl->ws.tree[WUP].ob_x += dw;
      
      wl->ws.tree[TFILLOUT].ob_width += dw;
      
      wl->ws.tree[RFILLOUT].ob_height += dh;
      wl->ws.tree[RFILLOUT].ob_x += dw;
      
      wl->ws.tree[BFILLOUT].ob_width += dw;
      wl->ws.tree[BFILLOUT].ob_y += dy;
      
      wl->ws.tree[SFILLOUT].ob_x += dw;
      wl->ws.tree[SFILLOUT].ob_y += dh;
      
      wl->ws.tree[WAPP].ob_width = wl->ws.tree[WMOVER].ob_width;
      
      /*
      changeslider(win,0,HSLIDE | VSLIDE,-1,-1);
      */
    }
  }
}


/*winalloc creates/fetches a free window structure/id*/

static
WINDOW_STRUCT *
winalloc (WORD apid) {
  WINDOW_LIST * wl;
  GLOBAL_APPL * globals = get_globals (apid);

  /*	
        if(win_free) {
        wl = win_free;
        win_free = win_free->next;
        }
        else {
        */
  wl = (WINDOW_LIST *)Mxalloc(sizeof(WINDOW_LIST),GLOBALMEM);

  /*
    };
    */
	
  wl->next = globals->windows;
  globals->windows = wl;
	
  return &wl->ws;
}


/*
** Description
** Allocate a resource for a new window
**
** 1998-09-30 CG
*/
static
OBJECT *
allocate_window_elements (void) {
  WORD    i = 0,tnr = 0;
  OBJECT  *t;
  TEDINFO *ti;
  LONG    size;
  GLOBAL_COMMON * globals = get_global_common ();

  DB_printf ("wind.c: allocate_window_elements: 1");
  DB_printf ("wind.c: allocate_window_elements: globals=0x%x", globals);
  DB_printf ("wind.c: allocate_window_elements: globals->windowtad=0x%x", globals->windowtad);
  while(elemnumber == -1) {
    switch (globals->windowtad[i].ob_type) {
    case	G_TEXT		:
    case	G_BOXTEXT	:
    case	G_FTEXT		:
    case	G_FBOXTEXT	:
      tnr++;
    }
		
    if (globals->windowtad[i].ob_flags & LASTOB)
    {
      elemnumber = i + 1;
      tednumber = tnr;
    }
		
    i++;
  }
	
  DB_printf ("wind.c: allocate_window_elements: 2");

  size = sizeof(OBJECT) * elemnumber + sizeof(TEDINFO) * tednumber;
	
  t = (OBJECT *)Mxalloc(size,GLOBALMEM);
	
  DB_printf ("wind.c: allocate_window_elements: 3");

  if(t != NULL) {
    ti = (TEDINFO *)&t[elemnumber];
		
    memcpy(t,globals->windowtad,sizeof(OBJECT) * elemnumber);
		
    for(i = 0; i < elemnumber; i++) {
      switch(globals->windowtad[i].ob_type) {
      case	G_TEXT		:
      case	G_BOXTEXT	:
      case	G_FTEXT		:
      case	G_FBOXTEXT	:
        t[i].ob_spec.tedinfo = ti;
        memcpy(ti, globals->windowtad[i].ob_spec.tedinfo, sizeof(TEDINFO));
        ti++;
      };
    };
  };

  DB_printf ("wind.c: allocate_window_elements: 4");
	
  return t;
}


/*
** Description
** Pack window elements
**
** 1998-09-30 CG
*/
static
void
packelem (OBJECT * tree,
          WORD     object,
          WORD     left,
          WORD     right,
          WORD     top,
          WORD     bottom) {
    if((left != -1) && (right != -1)) {
    if(left == 0) {
      tree[object].ob_x = D3DSIZE;
    }
    else {
      tree[object].ob_x = tree[left].ob_x + tree[left].ob_width + D3DSIZE * 2;
    };
		
    if(right == 0) {
      tree[object].ob_width = tree[0].ob_width - tree[object].ob_x - D3DSIZE;
    }
    else {
      tree[object].ob_width = tree[right].ob_x - tree[object].ob_x - D3DSIZE * 2;
    }
  }
  else if(left != -1) {
    if(left == 0) {
      tree[object].ob_x = D3DSIZE;
    }
    else {
      tree[object].ob_x = tree[left].ob_x + tree[left].ob_width + D3DSIZE * 2;
    };		
  }
  else if(right != -1) {
    if(right == 0) {
      tree[object].ob_x = tree[0].ob_width - tree[object].ob_width - D3DSIZE;
    }
    else {
      tree[object].ob_x = tree[right].ob_x - tree[object].ob_width - D3DSIZE * 2;
    };
  };
	
	
  if((top != -1) && (bottom != -1)) {
    if(top == 0) {
      tree[object].ob_y = D3DSIZE;
    }
    else {
      tree[object].ob_y = tree[top].ob_y + tree[top].ob_height + D3DSIZE * 2;
    };
		
    if(bottom == 0) {
      tree[object].ob_height = tree[0].ob_height - tree[object].ob_y - D3DSIZE;
    }
    else {
      tree[object].ob_height = tree[bottom].ob_y - tree[object].ob_y - D3DSIZE * 2;
    }
  }
  else if(top != -1) {
    if(top == 0) {
      tree[object].ob_y = D3DSIZE;
    }
    else {
      tree[object].ob_y = tree[top].ob_y + tree[top].ob_height + D3DSIZE * 2;
    };		
  }
  else if(bottom != -1) {
    if(bottom == 0) {
      tree[object].ob_y = tree[0].ob_height - tree[object].ob_height - D3DSIZE;
    }
    else {
      tree[object].ob_y = tree[bottom].ob_y - tree[object].ob_height - D3DSIZE * 2;
    };
  };	
}


/*
** Description
** Set the elements to use for a window (in the resource tree).
**
** 1998-09-30 CG
*/
static
void
set_win_elem (OBJECT *tree,
              WORD elem) {
  WORD bottomsize = 0;
  WORD rightsize = 0;
  WORD left = 0,right = 0,top = 0,bottom = 0;

  if((HSLIDE | LFARROW | RTARROW) & elem) {
    bottomsize = tree[WLEFT].ob_height + (D3DSIZE << 1);
  };
	
  if((LFARROW | HSLIDE | RTARROW) & elem) {
    bottomsize = tree[WLEFT].ob_height + (D3DSIZE << 1);
  };
	
  if(((bottomsize == 0) && (SIZER & elem))
     || ((VSLIDE | UPARROW | DNARROW) & elem)) {
    rightsize = tree[WSIZER].ob_width + (D3DSIZE << 1);
  };
	
  if(CLOSER & elem) {
    tree[WCLOSER].ob_flags &= ~HIDETREE;	
		
    packelem(tree,WCLOSER,0,-1,0,-1);
    left = WCLOSER;
  }	
  else {
    tree[WCLOSER].ob_flags |= HIDETREE;
  }
	
  if(FULLER & elem) {
    tree[WFULLER].ob_flags &= ~HIDETREE;	
		
    packelem(tree,WFULLER,-1,0,0,-1);
    right = WFULLER;
  }	
  else {
    tree[WFULLER].ob_flags |= HIDETREE;
  }
		
  if(SMALLER & elem) {
    tree[WSMALLER].ob_flags &= ~HIDETREE;	
		
    packelem(tree,WSMALLER,-1,right,0,-1);
    right = WSMALLER;
  }	
  else {
    tree[WSMALLER].ob_flags |= HIDETREE;
  }
		
  if(MOVER & elem) {
    tree[WMOVER].ob_flags &= ~HIDETREE;
    tree[TFILLOUT].ob_flags |= HIDETREE;
		
    tree[WMOVER].ob_height = tree[WCLOSER].ob_height;
    tree[WMOVER].ob_spec.tedinfo->te_font = IBM;

    packelem(tree,WMOVER,left,right,0,-1);
    top = WMOVER;
  }
  else {
    tree[WMOVER].ob_flags |= HIDETREE;

    if((left != 0) || (right != 0)) {
      tree[TFILLOUT].ob_flags &= ~HIDETREE;

      packelem(tree,TFILLOUT,left,right,0,-1);
      top = TFILLOUT;
    }
    else {
      tree[TFILLOUT].ob_flags |= HIDETREE;
    };
  };
	
  if(INFO & elem) {
    tree[WINFO].ob_flags &= ~HIDETREE;

    packelem(tree,WINFO,0,0,top,-1);
    top = WINFO;		
  }
  else {
    tree[WINFO].ob_flags |= HIDETREE;
  };

  right = 0;
  left = 0;

  if(elem & UPARROW) {
    tree[WUP].ob_flags &= ~HIDETREE;
		
    packelem(tree,WUP,-1,0,top,-1);
    top = WUP;
  }
  else {
    tree[WUP].ob_flags |= HIDETREE;
  };

  if(SIZER & elem) {
    tree[WSIZER].ob_flags &= ~HIDETREE;
    tree[SFILLOUT].ob_flags |= HIDETREE;	
		
    packelem(tree,WSIZER,-1,0,-1,0);
    bottom = right = WSIZER;
  }	
  else {
    tree[WSIZER].ob_flags |= HIDETREE;
		
    if((bottomsize > 0) && (rightsize > 0)) {
      tree[SFILLOUT].ob_flags &= ~HIDETREE;
			
      packelem(tree,SFILLOUT,-1,0,-1,0);
      bottom = right = SFILLOUT;
    }
    else {
      tree[SFILLOUT].ob_flags |= HIDETREE;
    }
  }
	
  if(elem & DNARROW) {
    tree[WDOWN].ob_flags &= ~HIDETREE;

    packelem(tree,WDOWN,-1,0,-1,bottom);
    bottom = WDOWN;		
  }
  else {
    tree[WDOWN].ob_flags |= HIDETREE;
  };
	
  if(elem & VSLIDE) {
    tree[WVSB].ob_flags &= ~HIDETREE;

    packelem(tree,WVSB,-1,0,top,bottom);		
  }
  else
  {
    tree[WVSB].ob_flags |= HIDETREE;
  }
	
  if(!(VSLIDE & elem) && (rightsize > 0))
  {
    tree[RFILLOUT].ob_flags &= ~HIDETREE;

    packelem(tree,RFILLOUT,-1,0,top,bottom);		
  }
  else {
    tree[RFILLOUT].ob_flags |= HIDETREE;
  }
	
  if(LFARROW & elem) {
    tree[WLEFT].ob_flags &= ~HIDETREE;
		
    packelem(tree,WLEFT,0,-1,-1,0);
    left = WLEFT;
  }
  else {
    tree[WLEFT].ob_flags |= HIDETREE;
  }
	
  if(RTARROW & elem) {
    tree[WRIGHT].ob_flags &= ~HIDETREE;
		
    packelem(tree,WRIGHT,-1,right,-1,0);
    right = WRIGHT;
  }
  else {
    tree[WRIGHT].ob_flags |= HIDETREE;
  };
	
  if(elem & HSLIDE) {
    tree[WHSB].ob_flags &= ~HIDETREE;
		
    packelem(tree,WHSB,left,right,-1,0);
  }
  else {
    tree[WHSB].ob_flags |= HIDETREE;
  }
	
  if(!(HSLIDE & elem) && (bottomsize > 0)) {
    tree[BFILLOUT].ob_flags &= ~HIDETREE;
		
    packelem(tree,BFILLOUT,left,right,-1,0);
  }
  else {
    tree[BFILLOUT].ob_flags |= HIDETREE;
  };
	
  if(IMOVER & elem) {
    tree[WMOVER].ob_flags &= ~HIDETREE;
    tree[WMOVER].ob_height = /*globals.csheight*/ + 2;
    tree[WMOVER].ob_spec.tedinfo->te_font = SMALL;
    packelem(tree,WMOVER,0,0,0,-1);

    tree[WAPP].ob_flags |= HIDETREE;
  }
  else {
    tree[WAPP].ob_flags &= ~HIDETREE;
  };
}


/*
** Description
** Create the struct for a new window
**
** 1998-09-28 CG
*/
static
void
create_new_window_struct (WORD apid,
                          WORD id,
                          WORD status,
                          WORD elements) {
  WINDOW_STRUCT * ws;

  if((status & WIN_DIALOG) || (status & WIN_MENU)) {
    ws->tree = 0L;
  } else {
    WORD    i;
    /*
    AP_INFO *ai;
    */
    
    ws->tree = allocate_window_elements ();
    set_win_elem (ws->tree, elements);
    ws->elements = elements;

    /*
    ai = internal_appl_info(msg->common.apid);
		
    if(ai) {
      ws->tree[WAPP].ob_spec.tedinfo->te_ptext =
	(char *)Mxalloc(strlen(&ai->name[2]) + 1,GLOBALMEM);
      strcpy(ws->tree[WAPP].ob_spec.tedinfo->te_ptext,&ai->name[2]);
      
      if(globals->common->wind_appl == 0) {
	ws->tree[WAPP].ob_spec.tedinfo->te_ptext[0] = 0;
      };
    };
    */

    ws->totsize.x = ws->tree[0].ob_x;
    ws->totsize.y = ws->tree[0].ob_y;
    ws->totsize.width = ws->tree[0].ob_width;
    ws->totsize.height = ws->tree[0].ob_height;
    
    calcworksize (apid,
                  elements,
                  &ws->totsize,
                  &ws->worksize,
                  WC_WORK);
		
    /*
    for(i = 0; i <= W_SMALLER; i++) {
      ws->top_colour[i] = top_colour[i];
      ws->untop_colour[i] = untop_colour[i];
    }
    
    ws->own_colour = 0;
    */
  };
}


/*
** Description
** Find the window struct of window <id>. The window must belong to
** application <apid>
**
** 1998-10-11 CG
*/
WINDOW_STRUCT *
find_window_struct (WORD apid,
                    WORD id)
{
  GLOBAL_APPL * globals = get_globals (apid);
  WINDOW_LIST * wl = globals->windows;

  while (wl != NULL) {
    if (wl->ws.id == id) {
      return &wl->ws;
    }
  }

  return NULL;
}


/*
** Description
** Draw window elements of window <id> with a clipping rectangle <clip>.
** If <id> is REDRAW_ALL all windows of the application will be redrawn.
**
** 1998-10-11 CG
*/
void
Wind_redraw_elements (
WORD   apid,
WORD   id,
RECT * clip,
WORD   start)
{
  GLOBAL_APPL * globals = get_globals (apid);
  WINDOW_LIST * wl = globals->windows;
  
  DB_printf ("wind.c: Wind_redraw_elements: id=%d", id);

  while (wl) {
    if (wl->ws.id == id) {
      if(wl->ws.tree != NULL) {
        RECT r;

        Wind_do_get (apid,
                     wl->ws.id,
                     WF_FIRSTXYWH,
                     &r.x,
                     &r.y,
                     &r.width,
                     &r.height,
                     FALSE);

        /* Loop while there are more rectangles to redraw */
        while (!((r.width == 0) && (r.height == 0))) {
          RECT	r2;
          
          DB_printf ("wind.c: Wind_redraw_elements: x=%d y=%d w=%d h=%d",
                     r.x, r.y, r.width, r.height);

          if(Misc_intersect(&r, clip, &r2)) {
            Objc_do_draw (globals->vid, wl->ws.tree, start, 3, &r2);
          }
          
          if (Wind_do_get (apid,
                           wl->ws.id,
                           WF_NEXTXYWH,
                           &r.x,
                           &r.y,
                           &r.width,
                           &r.height,
                           FALSE) == 0) {
            /* An error occurred in Wind_do_get */
            break;
          }
        }
      }
      
      /* Get out of the loop if only one window was supposed to be redrawn */
      if (!(id == REDRAW_ALL)) {
        break;
      }
    }

    /* Continue with the next window */
    wl = wl->next;
  }
}


/****************************************************************************
 * Wind_do_create                                                           *
 *  Implementation of wind_create().                                        *
 ****************************************************************************/
WORD             /* 0 if error or 1 if ok.                                  */
Wind_do_create(  /*                                                         */
WORD   apid,     /* Owner of window.                                        */
WORD   elements, /* Elements of window.                                     */
RECT * maxsize,  /* Maximum size allowed.                                   */
WORD   status)   /* Status of window.                                       */
/****************************************************************************/
{
  C_WIND_CREATE   par;
  R_WIND_CREATE   ret;
  int             count;
  WINDOW_STRUCT * ws;
  GLOBAL_APPL   * globals = get_globals (apid);
  GLOBAL_COMMON * global_common = globals->common;
  
  par.common.call = SRV_WIND_CREATE;
  par.common.apid = apid;
  par.common.pid = getpid ();
  par.elements = elements;
  par.maxsize = *maxsize;
  par.status = status;
	
  count = Client_send_recv (&par,
                            sizeof (C_WIND_CREATE),
                            &ret,
                            sizeof (R_WIND_CREATE));

  DB_printf ("wind.c: Wind_do_create: count=%d id=%d",
             count,
             ret.common.retval);

  ws = winalloc (apid);
  DB_printf ("wind.c: Wind_do_create: ws=0x%x", ws);

  ws->id = ret.common.retval;

  ws->status = status;
	
  ws->maxsize = *maxsize;

  ws->vslidepos = 1;
  ws->vslidesize = 1000;
  ws->hslidepos = 1;
  ws->hslidesize = 1000;

  if((ws->status & WIN_DIALOG) || (ws->status & WIN_MENU)) {
    ws->tree = 0L;
    ws->totsize = ws->maxsize;
    ws->worksize = ws->totsize;
  } else {
    WORD    i;
    /*
    AP_INFO *ai;
    */

    DB_printf ("wind.c: Wind_do_create: will call allocate_window_elements");
    ws->tree = allocate_window_elements ();
    DB_printf ("wind.c: Wind_do_create: ws->tree=0x%x", ws->tree);

    ws->elements = elements;
    set_win_elem (ws->tree, ws->elements);

    /*
    ai = internal_appl_info(msg->common.apid);
    
    if(ai) {
      ws->tree[WAPP].ob_spec.tedinfo->te_ptext =
	(char *)Mxalloc(strlen(&ai->name[2]) + 1,GLOBALMEM);
      strcpy(ws->tree[WAPP].ob_spec.tedinfo->te_ptext,&ai->name[2]);
      
      if(globals.wind_appl == 0) {
	ws->tree[WAPP].ob_spec.tedinfo->te_ptext[0] = 0;
      };
    };
    */
    
    ws->totsize.x = ws->tree[0].ob_x;
    ws->totsize.y = ws->tree[0].ob_y;
    ws->totsize.width = ws->tree[0].ob_width;
    ws->totsize.height = ws->tree[0].ob_height;
    
    calcworksize (apid, ws->elements, &ws->totsize, &ws->worksize, WC_WORK);
		
    for(i = 0; i <= W_SMALLER; i++) {
      ws->top_colour[i] = global_common->top_colour[i];
      ws->untop_colour[i] = global_common->untop_colour[i];
    }
    
    ws->own_colour = 0;
  }

  return ret.common.retval;
}


/* wind_create 0x0064 */

void
Wind_create (AES_PB *apb)
{	
  apb->int_out[0] = Wind_do_create(apb->global->apid,
                                   apb->int_in[0],
                                   (RECT *)&apb->int_in[1],
                                   0);
}


/*wind_open 0x0065*/

/*
** Exported
**
** 1998-12-20 CG
*/
WORD
Wind_do_open (WORD   apid,
              WORD   id,
              RECT * size) {
  C_WIND_OPEN par;
  R_WIND_OPEN ret;
  
  DB_printf ("Wind_do_open entered");

  par.common.call = SRV_WIND_OPEN;
  par.common.apid = apid;
  par.common.pid = getpid ();
  par.id = id;
  par.size = *size;
	
  Client_send_recv (&par,
                    sizeof (C_WIND_OPEN),
                    &ret,
                    sizeof (R_WIND_OPEN));

  /* Set the size of the window elements */
  Wind_set_size (apid, id, size);

  DB_printf ("Wind_do_open returned");

  return ret.common.retval;
}


void Wind_open(AES_PB *apb) {
  apb->int_out[0] = Wind_do_open (apb->global->apid,
                                  apb->int_in[0],
                                  (RECT *)&apb->int_in[1]);
}


/*
** Exported
**
** 1998-12-20 CG
*/
WORD
Wind_do_close (WORD apid,
               WORD wid) {
  C_WIND_CLOSE par;
  R_WIND_CLOSE ret;
  
  DB_printf ("Wind_do_close entered");

  par.common.call = SRV_WIND_CLOSE;
  par.common.apid = apid;
  par.common.pid = getpid ();
  par.id = wid;
	
  Client_send_recv (&par,
                    sizeof (C_WIND_CLOSE),
                    &ret,
                    sizeof (R_WIND_CLOSE));

  DB_printf ("Wind_do_close returned");

  return ret.common.retval;
}


/*
** Exported
**
** 1998-12-20 CG
*/
void
Wind_close (AES_PB *apb) {
  apb->int_out[0] = Wind_do_close (apb->global->apid, apb->int_in[0]);
}


/****************************************************************************
 * Wind_do_delete                                                           *
 *  Implementation of wind_delete().                                        *
 ****************************************************************************/
WORD             /* 0 if error or 1 if ok.                                  */
Wind_do_delete(  /*                                                         */
WORD apid,
WORD wid)        /* Identification number of window to close.               */
/****************************************************************************/
{
  C_WIND_DELETE par;
  R_WIND_DELETE ret;

  par.common.call = SRV_WIND_DELETE;
  par.common.apid = apid;
  par.common.pid = getpid ();
  par.id = wid;
	
  Client_send_recv (&par,
                    sizeof (C_WIND_DELETE),
                    &ret,
                    sizeof (R_WIND_DELETE));

  return ret.common.retval;
}


/*wind_delete 0x0067*/
void Wind_delete(AES_PB *apb) {
  DB_printf ("wind.c: Wind_delete entered");
  apb->int_out[0] = Wind_do_delete(apb->global->apid,
                                   apb->int_in[0]);
  DB_printf ("wind.c: Wind_delete exited");
}

/*wind_get 0x0068*/


/*
** Description
** Lib part of wind_get
**
** 1998-10-04 CG
*/
WORD
Wind_do_get (WORD   apid,
             WORD   handle,
             WORD   mode,
             WORD * parm1,
             WORD * parm2,
             WORD * parm3,
             WORD * parm4,
             WORD   in_workarea)
{
  C_WIND_GET par;
  R_WIND_GET ret;
  RECT       workarea;

  /* We can handle some calls without calling the server */
  if (mode == WF_WORKXYWH) {
    WINDOW_STRUCT * ws = find_window_struct (apid, handle);

    if (ws == NULL) {
      /* An error occurred */
      return 0;
    } else {
      *parm1 = ws->worksize.x;
      *parm2 = ws->worksize.y;
      *parm3 = ws->worksize.width;
      *parm4 = ws->worksize.height;
      return 1;
    }
  }

  if (in_workarea && ((mode == WF_FIRSTXYWH) || (mode == WF_NEXTXYWH))) {
    /* Get work area of window for later use */
    if (Wind_do_get (apid,
                     handle,
                     WF_WORKXYWH,
                     &workarea.x,
                     &workarea.y,
                     &workarea.width,
                     &workarea.height,
                     TRUE) == 0) {
      /* An error occurred */
      return 0;
    }
  }

  par.common.call = SRV_WIND_GET;
  par.common.apid = apid;
  par.common.pid = getpid ();
  par.handle = handle;
  par.mode = mode;
	

  /* Loop until we get the data we need (needed for WF_{FIRST,NEXT}XYWH */
  while (TRUE) {
    Client_send_recv (&par,
                      sizeof (C_WIND_GET),
                      &ret,
                      sizeof (R_WIND_GET));
    
    if (in_workarea && ((mode == WF_FIRSTXYWH) || (mode == WF_NEXTXYWH))) {
      RECT r;

      /* If the rectangle is empty anyhow just exit the loop */
      if ((ret.parm3 == 0) && (ret.parm4 == 0)) {
        break;
      }
      
      if (Misc_intersect ((RECT *)&ret.parm1, &workarea, &r) > 0) {
        /* Rectangles do intersect. Now return the intersecting area */
        *(RECT *)&ret.parm1 = r;
        break;
      }

      par.mode = WF_NEXTXYWH;
    } else {
      /*
      ** Get out of the loop. It is only needed for WF_{FIRST,NEXT}XYWH 
      ** when getting rectangles within workarea
      */
      break;
    }
  } /* while (TRUE) */
    
  *parm1 = ret.parm1;
  *parm2 = ret.parm2;
  *parm3 = ret.parm3;
  *parm4 = ret.parm4;
	
  return ret.common.retval;
}


/*
** Exported
**
** 1998-12-20 CG
*/
void
Wind_get (AES_PB *apb)
{
  apb->int_out[0] = Wind_do_get (apb->global->apid,
                                 apb->int_in[0],
                                 apb->int_in[1],
                                 &apb->int_out[1],
                                 &apb->int_out[2],
                                 &apb->int_out[3],
                                 &apb->int_out[4],
                                 TRUE);
}


/*wind_set 0x0069*/
void	Wind_set(AES_PB *apb) {
	apb->int_out[0] = Srv_wind_set(apb->global->apid,
		apb->int_in[0],
		apb->int_in[1],
		apb->int_in[2],
		apb->int_in[3],
		apb->int_in[4],
		apb->int_in[5]);
}


/*
** Exported
**
** 1998-12-20 CG
*/
WORD
Wind_do_find (WORD apid,
              WORD x,
              WORD y) {
  C_WIND_FIND par;
  R_WIND_FIND ret;

  par.common.call = SRV_WIND_FIND;
  par.common.apid = apid;
  par.common.pid = getpid ();
  par.x = x;
  par.y = y;
	
  Client_send_recv (&par,
                    sizeof (C_WIND_FIND),
                    &ret,
                    sizeof (R_WIND_FIND));

  return ret.common.retval;
}


/*
** Exported
**
** 1998-12-20 CG
*/
void
Wind_find (AES_PB *apb) {
  apb->int_out[0] = Wind_do_find (apb->global->apid,
                                  apb->int_in[0],
                                  apb->int_in[1]);
}


/*wind_update 0x006b*/

/*
** Description
** Library part of wind_update
**
** 1998-10-04 CG
*/
WORD
Wind_do_update (WORD apid,
                WORD mode)
{
  C_WIND_UPDATE par;
  R_WIND_UPDATE ret;

  par.common.call = SRV_WIND_UPDATE;
  par.common.apid = apid;
  par.common.pid = getpid ();
  par.mode = mode;
	
  Client_send_recv (&par,
                    sizeof (C_WIND_UPDATE),
                    &ret,
                    sizeof (R_WIND_UPDATE));

  return ret.common.retval;
}

void
Wind_update (AES_PB *apb) {
  apb->int_out[0] = Wind_do_update (apb->global->apid,
                                    apb->int_in[0]);
}



/*wind_calc 0x006c*/
void
Wind_calc (AES_PB *apb) {
  calcworksize (apb->global->apid,
                apb->int_in[1],
                (RECT *)&apb->int_in[2],
		(RECT *)&apb->int_out[1],
                apb->int_in[0]);
  
  apb->int_out[0] = 1;
}

/****************************************************************************
 * Wind_new                                                                 *
 *  0x006d wind_new().                                                      *
 ****************************************************************************/
void              /*                                                        */
Wind_new(         /*                                                        */
AES_PB *apb)      /* AES parameter block.                                   */
/****************************************************************************/
{
	apb->int_out[0] = Srv_wind_new(apb->global->apid);
}


/*
** Exported
**
** 1998-12-20 CG
*/
OBJECT *
Wind_get_rsrc (WORD apid,
               WORD id) {
  GLOBAL_APPL *   globals = get_globals (apid);
  WINDOW_LIST_REF wl;

  wl = globals->windows;

  /* Find the window structure for our window */
  while (wl != WINDOW_LIST_REF_NIL) {
    if (wl->ws.id == id) {
      return wl->ws.tree;
    }
  }

  return NULL;
}


/*
** Exported
**
** 1998-12-20 CG
*/
WORD
Wind_change (WORD apid,
             WORD id,
             WORD object,
             WORD newstate)
{
  WINDOW_STRUCT * win = find_window_struct (apid, id);
	
  if(win) {
    if (newstate != win->tree[widgetmap [object]].ob_state) {
      GLOBAL_COMMON * globals = get_global_common ();
      win->tree [widgetmap [object]].ob_state = newstate;
      Wind_redraw_elements (apid, id, &globals->screen, widgetmap [object]);
    }
    
    return 1;
  }
  
  return 0;
}