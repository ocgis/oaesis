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

#define DEBUGLEVEL 0

/****************************************************************************
 * Used interfaces                                                          *
 ****************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_ALLOC_H
#include <alloc.h>
#endif

#ifdef HAVE_BASEPAGE_H
#include <basepage.h>
#endif

#ifdef HAVE_MINTBIND_H
#include <mintbind.h>
#endif

#ifdef HAVE_OSBIND_H
#include <osbind.h>
#endif

#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef HAVE_SUPPORT_H
#include <support.h>
#endif

#include <unistd.h>

#include "aesbind.h"
#include "debug.h"
#include "evnthndl.h"
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
#include "wind.h"

/****************************************************************************
 * Macros                                                                   *
 ****************************************************************************/
#define INTS2LONG(a,b) (((((LONG)a)<<16)&0xffff0000L)|(((LONG)b)&0xffff))

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

static WORD     elemnumber = -1;
static WORD     tednumber;

static WORD widgetmap[] = {
  0,        /* W_BOX     */
  0,        /* W_TITLE   */
  WCLOSER,  /* W_CLOSER  */
  WMOVER,   /* W_NAME    */
  WFULLER,  /* W_FULLER  */
  WINFO,    /* W_INFO    */
  0,        /* W_DATA    */
  0,        /* W_WORK    */
  WSIZER,   /* W_SIZER   */
  0,        /* W_VBAR    */
  WUP,      /* W_UPARROW */
  WDOWN,    /* W_DNARROW */
  WVSB,     /* W_VSLIDE  */
  WVSLIDER, /* W_VELEV   */
  0,        /* W_HBAR    */
  WLEFT,    /* W_LFARROW */
  WRIGHT,   /* W_RTARROW */
  WHSB,     /* W_HSLIDE  */
  WHSLIDER, /* W_HELEV   */
  WSMALLER  /* W_SMALLER */
};

static
WINDOW_STRUCT *
find_window_struct (WORD apid,
                    WORD id);

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
  WORD  bottomsize = 1;
  WORD  headsize = 1;
  WORD  leftsize = 1;
  WORD  rightsize = 1;
  WORD  topsize;
  GLOBAL_COMMON * globals_common = get_global_common ();
        
  DEBUG3 ("wind.c: calcworksize: %d %d %d %d",
          orig->x, orig->y, orig->width, orig->height);
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

  DEBUG3 ("wind.c: calcworksize: new %d %d %d %d",
          new->x, new->y, new->width, new->height);
}

/*
** Description
** Change window slider position and size
**
** 1999-03-28 CG
*/
static
WORD
Wind_set_slider (WORD        apid,
                 WORD        id,
                 WORD        redraw,
                 WORD        which,
                 WORD        position,
                 WORD        size) {	
  WINDOW_STRUCT * win = find_window_struct (apid, id);
  WORD redraw2 = 0;
  GLOBAL_COMMON * globals = get_global_common ();
  
  DEBUG3 ("wind.c: Wind_set_slider: position %d size %d", position, size);

  if(which & VSLIDE) {
    WORD newheight,newy;
    
    if (position != -1) {
      if(position > 1000) {
        win->vslidepos = 1000;
      } else if (position < 1) {
        win->vslidepos = 1;
      } else {
        win->vslidepos = position;
      }
    }
		
    if (size != -1) {
      if (size > 1000) {
        win->vslidesize = 1000;
      } else if (size < 1) {
        win->vslidesize = 1;
      } else {
        win->vslidesize = size;
      }
    }

    newy = (WORD)(((LONG)win->vslidepos *
                   (LONG)(win->tree[WVSB].ob_height -
                          win->tree[WVSLIDER].ob_height)) / 1000L);
    newheight = (WORD)(((LONG)win->vslidesize *
                        (LONG)win->tree[WVSB].ob_height) / 1000L);
    
    if((win->tree[WVSLIDER].ob_y != newy) ||
       (win->tree[WVSLIDER].ob_height != newheight)) {
      win->tree[WVSLIDER].ob_y = newy;
      win->tree[WVSLIDER].ob_height = newheight;
      
      redraw2 = 1;
    }
  }
  
  if(which & HSLIDE) {
    WORD newx,newwidth;
    
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
    
    newx = (WORD)(((LONG)win->hslidepos *
                   (LONG)(win->tree[WHSB].ob_width -
                          win->tree[WHSLIDER].ob_width)) / 1000L);
    newwidth = (WORD)(((LONG)win->hslidesize *
                       (LONG)win->tree[WHSB].ob_width) / 1000L);
    
    if((win->tree[WHSLIDER].ob_x != newx) ||
       (win->tree[WHSLIDER].ob_width != newwidth)) {
      win->tree[WHSLIDER].ob_x = newx;
      win->tree[WHSLIDER].ob_width = newwidth;
      
      redraw2 = 1;
    }
  }

  if(redraw && redraw2 && (win->status & WIN_OPEN)) {
    DEBUG3 ("screen : %d %d %d %d",
            globals->screen.x,
            globals->screen.y,
            globals->screen.width,
            globals->screen.height);
    if(which & VSLIDE) { 
      Wind_redraw_elements (apid, id, &globals->screen, WVSB);
    }
    
    if(which & HSLIDE) {
      Wind_redraw_elements (apid, id, &globals->screen, WHSB);
    }
  }
  
  return 1;
}


/*
** Description
** Wind_set_size sets the size and position of window <win> to <size>
**
** 1998-10-11 CG
** 1998-12-25 CG
** 1999-01-01 CG
** 1999-01-10 CG
** 1999-04-10 CG
** 1999-04-11 CG
*/
static
WORD
Wind_set_size (WORD   apid,
               WORD   id,
               RECT * size) {
  WORD            dx,dy,dw,dh;
  WINDOW_STRUCT * ws = find_window_struct (apid, id);

  if (ws != NULL) {
    ws->lastsize = ws->totsize;
    
    dx = size->x - ws->lastsize.x;
    dy = size->y - ws->lastsize.y;
    dw = size->width - ws->lastsize.width;
    dh = size->height - ws->lastsize.height;

    /* Update areas where widget elements were placed */
    if (dw > 0) {
      REDRAWSTRUCT m;

      m.type = WM_REDRAW;
      m.sid = apid;
      m.length = sizeof (REDRAWSTRUCT);
      m.wid = id;

      m.area.x = ws->worksize.x + ws->worksize.width + dx;
      m.area.y = ws->worksize.y + dy;
      m.area.width = ws->totsize.x + ws->totsize.width - m.area.x;

      if (dw < m.area.width) {
        m.area.width = dw;
      }

      m.area.height = ws->totsize.y + ws->totsize.height - ws->worksize.y;

      if (dh < 0) {
        m.area.height += dh;
      }

      if ((m.area.width > 0) && (m.area.height > 0)) {
        Appl_do_write (apid, apid, m.length, m);
      }
    }

    if (dh > 0) {
      REDRAWSTRUCT m;

      m.type = WM_REDRAW;
      m.sid = apid;
      m.length = sizeof (REDRAWSTRUCT);
      m.wid = id;

      m.area.x = ws->worksize.x + dx;
      m.area.y = ws->worksize.y + ws->worksize.height + dy;
      m.area.width = ws->totsize.x + ws->totsize.width - ws->worksize.x;

      if (dw < 0) {
        m.area.width += dw;
      }

      m.area.height = ws->totsize.y + ws->totsize.height - m.area.y;

      if (dh < m.area.height) {
        m.area.height = dh;
      }

      if ((m.area.width > 0) && (m.area.height > 0)) {
        Appl_do_write (apid, apid, m.length, m);
      }
    }

    ws->totsize = *size;
    
    ws->worksize.x += dx;
    ws->worksize.y += dy;
    ws->worksize.width += dw;
    ws->worksize.height += dh;
    
    if(ws->tree) {
      ws->tree[0].ob_x = ws->totsize.x;
      ws->tree[0].ob_y = ws->totsize.y;
      ws->tree[0].ob_width = ws->totsize.width;
      ws->tree[0].ob_height = ws->totsize.height;
      
      ws->tree[WMOVER].ob_width += dw;
      
      ws->tree[WFULLER].ob_x += dw;
      
      ws->tree[WSMALLER].ob_x += dw;
      
      ws->tree[WDOWN].ob_x += dw;
      ws->tree[WDOWN].ob_y += dh;    
      
      ws->tree[WSIZER].ob_x += dw;
      ws->tree[WSIZER].ob_y += dh;   
      
      ws->tree[WRIGHT].ob_x += dw;
      ws->tree[WRIGHT].ob_y += dh;   
      
      ws->tree[WLEFT].ob_y += dh;    
      
      ws->tree[WVSB].ob_x += dw;
      ws->tree[WVSB].ob_height += dh;        
      
      ws->tree[WHSB].ob_y += dh;
      ws->tree[WHSB].ob_width += dw; 
      
      ws->tree[WINFO].ob_width += dw;
      
      ws->tree[WUP].ob_x += dw;
      
      ws->tree[TFILLOUT].ob_width += dw;
      
      ws->tree[RFILLOUT].ob_height += dh;
      ws->tree[RFILLOUT].ob_x += dw;
      
      ws->tree[BFILLOUT].ob_width += dw;
      ws->tree[BFILLOUT].ob_y += dy;
      
      ws->tree[SFILLOUT].ob_x += dw;
      ws->tree[SFILLOUT].ob_y += dh;
      
      ws->tree[WAPP].ob_width = ws->tree[WMOVER].ob_width;
      
      Wind_set_slider (apid,
                       id,
                       0,
                       HSLIDE | VSLIDE,
                       -1,
                       -1);
    }

    /* If the window changed size we need to redraw the elements */
    if ((dw != 0) || (dh != 0)) {
      Wind_redraw_elements (apid, id, size, 0);
    }
  }

  /* OK */
  return 1;
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

  while(elemnumber == -1) {
    switch (globals->windowtad[i].ob_type) {
    case        G_TEXT          :
    case        G_BOXTEXT       :
    case        G_FTEXT         :
    case        G_FBOXTEXT      :
      tnr++;
    }
                
    if (globals->windowtad[i].ob_flags & LASTOB)
    {
      elemnumber = i + 1;
      tednumber = tnr;
    }
                
    i++;
  }
        
  size = sizeof(OBJECT) * elemnumber + sizeof(TEDINFO) * tednumber;
        
  t = (OBJECT *)Mxalloc(size,GLOBALMEM);
        
  if(t != NULL) {
    ti = (TEDINFO *)&t[elemnumber];
                
    memcpy(t,globals->windowtad,sizeof(OBJECT) * elemnumber);
                
    for(i = 0; i < elemnumber; i++) {
      switch(globals->windowtad[i].ob_type) {
      case      G_TEXT          :
      case      G_BOXTEXT       :
      case      G_FTEXT         :
      case      G_FBOXTEXT      :
        t[i].ob_spec.tedinfo = ti;
        memcpy(ti, globals->windowtad[i].ob_spec.tedinfo, sizeof(TEDINFO));
        ti++;
      };
    };
  };

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
** 1999-01-10 CG
*/
static
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

    wl = wl->next;
  }

  return NULL;
}


/*
** Description
** Draw window elements of window <id> with a clipping rectangle <clip>.
** If <id> is REDRAW_ALL all windows of the application will be redrawn.
**
** 1998-10-11 CG
** 1999-01-01 CG
** 1999-01-09 CG
*/
void
Wind_redraw_elements (WORD   apid,
                      WORD   id,
                      RECT * clip,
                      WORD   start) {
  GLOBAL_APPL * globals = get_globals (apid);
  WINDOW_LIST * wl = globals->windows;
  
  /* Handle desktop and menu separately */
  if ((id == DESKTOP_WINDOW) || (id == MENU_BAR_WINDOW)) {
    OBJECT * tree =
      (id == DESKTOP_WINDOW) ? globals->desktop_background : globals->menu;

    if (tree != NULL) {
      RECT r;
     
      Wind_do_get (apid,
                   id,
                   WF_FIRSTXYWH,
                   &r.x,
                   &r.y,
                   &r.width,
                   &r.height,
                   FALSE);
      
      /* Loop while there are more rectangles to redraw */
      while (!((r.width == 0) && (r.height == 0))) {
        RECT  r2;
        
        if(Misc_intersect(&r, clip, &r2)) {
          Objc_do_draw (globals->vid,
                        tree,
                        start,
                        3,
                        &r2);
        }
        
        if (Wind_do_get (apid,
                         id,
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
  } else { /* Window handle != 0 */
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
            RECT  r2;
            
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
}


/*
** Exported
**
** 1999-01-10 CG
*/
WORD
Wind_do_create (WORD   apid,
                WORD   elements,
                RECT * maxsize,
                WORD   status) {
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
        
  Client_send_recv (&par,
                    sizeof (C_WIND_CREATE),
                    &ret,
                    sizeof (R_WIND_CREATE));
  
  ws = winalloc (apid);

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

    ws->tree = allocate_window_elements ();

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
** 1999-01-10 CG
** 1999-04-09 CG
*/
WORD
Wind_do_open (WORD   apid,
              WORD   id,
              RECT * size) {
  C_WIND_OPEN     par;
  R_WIND_OPEN     ret;
  WINDOW_STRUCT * win = find_window_struct (apid, id);

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

  win->status |= WIN_OPEN;

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
  
  par.common.call = SRV_WIND_CLOSE;
  par.common.apid = apid;
  par.common.pid = getpid ();
  par.id = wid;
        
  Client_send_recv (&par,
                    sizeof (C_WIND_CLOSE),
                    &ret,
                    sizeof (R_WIND_CLOSE));

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
  apb->int_out[0] = Wind_do_delete(apb->global->apid,
                                   apb->int_in[0]);
}

/*wind_get 0x0068*/


/*
** Description
** Lib part of wind_get
**
** 1998-10-04 CG
** 1999-01-02 CG
** 1999-01-09 CG
** 1999-03-30 CG
*/
WORD
Wind_do_get (WORD   apid,
             WORD   handle,
             WORD   mode,
             WORD * parm1,
             WORD * parm2,
             WORD * parm3,
             WORD * parm4,
             WORD   in_workarea) {
  C_WIND_GET par;
  R_WIND_GET ret;
  RECT       workarea;

  /*
  ** We can handle some calls without calling the server.
  */
  switch (mode) {
  case WF_WORKXYWH :
  case WF_FULLXYWH :
  {
    WINDOW_STRUCT * ws;
    
    ws = find_window_struct (apid, handle);
    /*
    ** Return area if we have it. If not just do a server call the normal
    ** way.
    */
    if (ws != NULL) {
      RECT * size;

      switch (mode) {
      case WF_WORKXYWH :
        size = &ws->worksize;
        break;

      case WF_FULLXYWH :
        size = &ws->maxsize;
        break;
      }

      *parm1 = size->x;
      *parm2 = size->y;
      *parm3 = size->width;
      *parm4 = size->height;

      /* OK */
      return 1;
    }
  }
  break;
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


/*
** Description
** Set the name or the info string of a window
**
** 1998-12-26 CG
** 1999-01-01 CG
*/
WORD
Wind_set_name_or_info (WORD   apid,
                       WORD   id,
                       WORD   mode,
                       BYTE * str) {
  WORD object = (mode == WF_NAME) ? WMOVER : WINFO;
  WINDOW_STRUCT * win = find_window_struct (apid, id);

  if (win != NULL) {
    if (win->tree != NULL) {
      win->tree[object].ob_spec.tedinfo->te_ptext =
        (BYTE *)malloc (strlen (str));
      
      strcpy (win->tree[object].ob_spec.tedinfo->te_ptext, str);
      
      Wind_redraw_elements (apid, id, &win->totsize, widgetmap [object]);

      return 1;
    }
  }

  return 0;
}
      

/*
** Description
** Set desktop background
**
** 1999-01-01 CG
** 1999-01-06 CG
** 1999-01-09 CG
*/
static
WORD
Wind_set_desktop_background (WORD     apid,
                             OBJECT * tree) {
  GLOBAL_APPL * globals = get_globals (apid);

  globals->desktop_background = tree;

  /* Adjust the size of the resource to the size of the desktop */
  if (tree != NULL) {
    Wind_do_get (apid,
                 0,
                 WF_WORKXYWH,
                 &tree->ob_x,
                 &tree->ob_y,
                 &tree->ob_width,
                 &tree->ob_height,
                 FALSE);
  }

  /* Return OK */
  return 1;
}


/*
** Exported
**
** 1998-12-25 CG
** 1999-01-01 CG
** 1999-04-07 CG
*/
WORD
Wind_do_set (WORD apid,
             WORD handle,
             WORD mode,
             WORD parm1,
             WORD parm2,
             WORD parm3,
             WORD parm4) {
  C_WIND_SET par;
  R_WIND_SET ret;
  
  switch (mode) {
  case WF_NAME :
  case WF_INFO :
    ret.common.retval = 1;
    break;

  default:
    par.common.call = SRV_WIND_SET;
    par.common.apid = apid;
    par.common.pid = getpid ();
    
    par.handle = handle;
    par.mode = mode;
    
    par.parm1 = parm1;
    par.parm2 = parm2;
    par.parm3 = parm3;
    par.parm4 = parm4;
    
    Client_send_recv (&par,
                      sizeof (C_WIND_SET),
                      &ret,
                      sizeof (R_WIND_SET));
  }

  /* FIXME
  ** Check the retval if the server operation went ok
  */
  switch (mode) {
  case WF_NAME :
  case WF_INFO :
    return Wind_set_name_or_info (apid,
                                  handle,
                                  mode,
                                  (BYTE *)INTS2LONG (parm1,
                                                     parm2));

  case WF_CURRXYWH :
    return Wind_set_size (apid, handle, (RECT *)&par.parm1);
    
  case WF_NEWDESK :
    return Wind_set_desktop_background (apid,
                                        (OBJECT *)INTS2LONG (parm1, parm2));

  case WF_HSLIDE :
    return Wind_set_slider (apid,
                            handle,
                            1,
                            HSLIDE,
                            parm1,
                            -1);

  case WF_VSLIDE :
    return Wind_set_slider (apid,
                            handle,
                            1,
                            VSLIDE,
                            parm1,
                            -1);

  case WF_HSLSIZE :
    return Wind_set_slider (apid,
                            handle,
                            1,
                            HSLIDE,
                            -1,
                            parm1);

  case WF_VSLSIZE :
    return Wind_set_slider (apid,
                            handle,
                            1,
                            VSLIDE,
                            -1,
                            parm1);
  }

  return ret.common.retval;
}


/*
** Exported
**
** 1998-12-25 CG
*/
void
Wind_set (AES_PB *apb) {
  apb->int_out[0] = Wind_do_set (apb->global->apid,
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
** Exported
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


/*
** Description
** Implementation of wind_new ()
**
** 1999-06-10 CG
*/
WORD
Wind_do_new (WORD apid) {
  C_WIND_NEW par;
  R_WIND_NEW ret;

  par.common.call = SRV_WIND_NEW;
  par.common.apid = apid;
  par.common.pid = getpid ();

  Client_send_recv (&par,
                    sizeof (C_WIND_NEW),
                    &ret,
                    sizeof (R_WIND_NEW));

  /* FIXME: Remove structures on lib side */

  return ret.common.retval;
}


/*
** Description
** 0x006d wind_new ()
**
** 1999-06-10 CG
*/
void
Wind_new (AES_PB * apb)
{
  apb->int_out[0] = Wind_do_new(apb->global->apid);
}


/*
** Exported
**
** 1998-12-20 CG
** 1999-01-10 CG
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

    wl = wl->next;
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
