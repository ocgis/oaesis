/****************************************************************************

 Module
  graf.c
  
 Description
  Dynamic graphical routines in oAESis.
  
 Author(s)
  cg (Christer Gustavsson <d2cg@dtek.chalmers.se>)

 Revision history
 
  960103 cg
   Added standard header.
  
  960424 cg
   Implemented graf_{grow,move}box.
 
  960501 cg
   Fixed bug in graf_mouse() in the M_LAST and M_RESTORE code.

  960816 jps
   All this graf settings variables checks.

   
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

#ifdef HAVE_MINTBIND_H
#include <mintbind.h>
#endif

#ifdef HAVE_OSBIND_H
#include <osbind.h>
#endif

#include <stdio.h>

#include "debug.h"
#include "evnt.h"
#include "evnthndl.h"
#include "gemdefs.h"
#include "graf.h"
#include "lib_global.h"
#include "lib_misc.h"
#include "mintdefs.h"
#include "objc.h"
#include "resource.h"
#include "srv_calls.h"
#include "srv_interface.h"
#include "srv_put.h"
#include "types.h"
#include <unistd.h>
#include "vdi.h"
#include "wind.h"

/****************************************************************************
 * Macros                                                                   *
 ****************************************************************************/

#define GRAF_STEPS 15

#define GRAFSEM 0x6f475246 /* 'oGRF' */

/****************************************************************************
 * Module global variables                                                  *
 ****************************************************************************/

static MFORM m_arrow,m_text_crsr,m_busy_bee,m_point_hand,m_flat_hand,
             m_thin_cross,m_thick_cross,m_outln_cross;
static MFORM current,last,last_saved;

WORD   grafvid;

/****************************************************************************
 * Local functions (use static!)                                            *
 ****************************************************************************/

/*
** Description
** Convert icons to MFORMs that can be passed to vsc_form ()
**
** 1999-01-02
*/
static
void
icon2mform(MFORM  *  mf,
           OBJECT * icon) {
  WORD  i;
  
  /*convert resource icons to MFORM's*/
  
  mf->mf_xhot = 0;
  mf->mf_yhot = 0;
  mf->mf_nplanes = 1;
  mf->mf_fg = 1;
  mf->mf_bg = 0;
  
  for(i = 0; i < 16; i++) {
    mf->mf_mask[i] = icon->ob_spec.iconblk->ib_pmask[i];
    mf->mf_data[i] = icon->ob_spec.iconblk->ib_pdata[i];
  }
}


/****************************************************************************
 * Public functions                                                         *
 ****************************************************************************/
 
void Graf_init_module(void) {   
  WORD work_in[] = {1,7,1,1,1,1,1,1,1,1,2};
  WORD work_out[57];

  /*
  grafvid = globals->common->vid;
  */
  Vdi_v_opnvwk(work_in,&grafvid,work_out);
  
  Psemaphore(SEM_CREATE,GRAFSEM,-1);
  Psemaphore(SEM_UNLOCK,GRAFSEM,-1);

  /*
  icon2mform (&m_arrow, &globals->common->mouseformstad[MARROW]);
  icon2mform (&m_text_crsr, &globals->common->mouseformstad[MTEXT_CRSR]);
  icon2mform (&m_busy_bee, &globals->common->mouseformstad[MBUSY_BEE]);
  icon2mform (&m_point_hand, &globals->common->mouseformstad[MPOINT_HAND]);
  icon2mform (&m_flat_hand, &globals->common->mouseformstad[MFLAT_HAND]);
  icon2mform (&m_thin_cross, &globals->common->mouseformstad[MTHIN_CROSS]);
  icon2mform (&m_thick_cross, &globals->common->mouseformstad[MTHICK_CROSS]);
  icon2mform (&m_outln_cross, &globals->common->mouseformstad[MOUTLN_CROSS]);
  */

  current = m_arrow;
  last = m_arrow;
  last_saved = m_arrow;
}

void Graf_exit_module(void) {
  Psemaphore(SEM_LOCK,GRAFSEM,-1);
  Psemaphore(SEM_DESTROY,GRAFSEM,-1);

  Vdi_v_clsvwk(grafvid);
}


/*
** Exported
**
** 1998-12-20 CG
** 1998-12-25 CG
** 1998-12-26 CG
** 1999-01-03 CG
** 1999-01-09 CG
** 1999-04-10 CG
*/
WORD
Graf_do_rubberbox (WORD   apid,
                   WORD   bx,
                   WORD   by,
                   WORD   minw,
                   WORD   minh,
                   WORD * endw,
                   WORD * endh) {
  EVENTIN ei = {
    MU_BUTTON | MU_M1,
    0,
    LEFT_BUTTON,
    0,
    MO_LEAVE,
    {0,0,1,1},
    0,
    {0,0,0,0},
    0,
    0
  };
  COMMSG        buffer;
  EVENTOUT      eo;
  WORD          xyarray[10];
  WORD          lastw;
  WORD          lasth;
  WORD          neww;
  WORD          newh;
  WORD          mx;
  WORD          my;
  WORD          mb;
  WORD          ks;
  GLOBAL_APPL * globals = get_globals (apid);

  /* Get the initial mouse state */
  Graf_do_mkstate (apid, &mx, &my, &mb, &ks);

  /* Calculate initial width and height */
  lastw = mx - bx;
  lasth = my - by;

  /* If the mouse button has been released return directly */
  if(!(mb & LEFT_BUTTON)) {     
    *endw = lastw;
    *endh = lasth;

    if(*endw < minw) {
      *endw = minw;
    }

    if(*endh < minh) {
      *endh = minh;
    }
        
    return 1;
  }

  Vdi_vsl_color (globals->vid, BLACK);
  Vdi_vswr_mode (globals->vid, MD_XOR);
  Vdi_vsl_type (globals->vid, DOTTED);

  xyarray[0] = bx;
  xyarray[1] = by;
  xyarray[2] = bx + lastw - 1;
  xyarray[3] = xyarray[1];
  xyarray[4] = xyarray[2];
  xyarray[5] = by + lasth - 1;
  xyarray[6] = xyarray[0];
  xyarray[7] = xyarray[5];
  xyarray[8] = xyarray[0];
  xyarray[9] = xyarray[1];
  
  Graf_do_mouse (apid, M_OFF, NULL);
  Vdi_v_pline (globals->vid, 5, xyarray);
  Graf_do_mouse (apid, M_ON, NULL);
  
  ei.m1r.x = mx;
  ei.m1r.y = my;

  while (TRUE) {
    Evnt_do_multi (apid, &ei, &buffer, &eo, 0, DONT_HANDLE_MENU_BAR);

    if (eo.events & MU_M1) {
      neww = eo.mx - bx;
      newh = eo.my - by;

      if(neww < minw) {
        neww = minw;
      }

      if(newh < minh) {
        newh = minh;
      }

      if ((lastw != neww) || (lasth != newh)) {
        Graf_do_mouse (apid, M_OFF, NULL);
        Vdi_v_pline (globals->vid, 5, xyarray);

        lastw = neww;
        lasth = newh;

        xyarray[0] = bx;
        xyarray[1] = by;
        xyarray[2] = bx + lastw - 1;
        xyarray[3] = xyarray[1];
        xyarray[4] = xyarray[2];
        xyarray[5] = by + lasth - 1;
        xyarray[6] = xyarray[0];
        xyarray[7] = xyarray[5];
        xyarray[8] = xyarray[0];
        xyarray[9] = xyarray[1];
         
        Vdi_v_pline (globals->vid, 5, xyarray);
        Graf_do_mouse (apid, M_ON, NULL);
      }
    }
                
    if(eo.events & MU_BUTTON) {
      if (!(eo.mb & LEFT_BUTTON)) {
        break;
      }
    }
  }

  Graf_do_mouse (apid, M_OFF, NULL);
  Vdi_v_pline (globals->vid, 5, xyarray);
  Graf_do_mouse (apid, M_ON, NULL);

  *endw = lastw;
  *endh = lasth;
        
  return 1;
}


/*
** Exported
**
** 1998-12-20 CG
** 1998-12-25 CG
*/
void
Graf_rubberbox (AES_PB *apb) {
  Wind_do_update (apb->global->apid, BEG_MCTRL);
  
  apb->int_out[0] = Graf_do_rubberbox (apb->global->apid,
                                       apb->int_in[0],
                                       apb->int_in[1],
                                       apb->int_in[2],
                                       apb->int_in[3],
                                       &apb->int_out[1],
                                       &apb->int_out[2]);

  Wind_do_update (apb->global->apid, END_MCTRL);
}


/* 0x0071 graf_dragbox() */

/*
** Exported
**
** 1998-12-20 CG
** 1998-12-25 CG
** 1998-12-26 CG
** 1999-01-03 CG
** 1999-01-09 CG
*/
WORD
Graf_do_dragbox (WORD   apid,
                 WORD   w,
                 WORD   h,
                 WORD   sx,
                 WORD   sy,
                 RECT * bound,
                 WORD * endx,
                 WORD * endy) {
  EVENTIN ei = {
    MU_BUTTON | MU_M1,
    0,
    LEFT_BUTTON,
    0,
    MO_LEAVE,
    {0,0,1,1},
    0,
    {0,0,0,0},
    0,
    0
  };
  EVENTOUT      eo;
  COMMSG        buffer;
    
  WORD          relx;
  WORD          rely;
  WORD          xyarray[10];
  WORD          lastx = sx;
  WORD          lasty = sy;
  WORD          newx;
  WORD          newy;
  WORD          mx;
  WORD          my;
  WORD          mb;
  WORD          ks;
  GLOBAL_APPL * globals = get_globals (apid);

  /* Get the initial mouse state */
  Graf_do_mkstate (apid, &mx, &my, &mb, &ks);

  /* Calculate offset from mouse position to rectangle origo */
  relx = sx - mx;
  rely = sy - my;

  /* If the mouse button is release return directly */
  if (!(mb & LEFT_BUTTON)) {   
    *endx = sx;
    *endy = sy;
    
    return 1;
  }

  Vdi_vsl_color (globals->vid, BLACK);
  Vdi_vswr_mode (globals->vid, MD_XOR);
  Vdi_vsl_type (globals->vid, DOTTED);

  xyarray[0] = lastx;
  xyarray[1] = lasty;
  xyarray[2] = lastx + w - 1;
  xyarray[3] = xyarray[1];
  xyarray[4] = xyarray[2];
  xyarray[5] = lasty + h - 1;
  xyarray[6] = xyarray[0];
  xyarray[7] = xyarray[5];
  xyarray[8] = xyarray[0];
  xyarray[9] = xyarray[1];
         
  Graf_do_mouse (apid, M_OFF, NULL);
  Vdi_v_pline (globals->vid, 5, xyarray);
  Graf_do_mouse (apid, M_ON, NULL);

  ei.m1r.x = mx;
  ei.m1r.y = my;
  
  while (TRUE) {
    Evnt_do_multi (apid, &ei, &buffer, &eo, 0, DONT_HANDLE_MENU_BAR);

    if (eo.events & MU_M1) {
      newx = eo.mx + relx;
      newy = eo.my + rely;
      
      if (newx < bound->x) {
        newx = bound->x;
      } else if (newx > (bound->x + bound->width - w)) {
        newx = bound->x + bound->width - w;
      }

      if (newy < bound->y) {
        newy = bound->y;
      } else if (newy > (bound->y + bound->height - h)) {
        newy = bound->y + bound->height - h;
      }
      
      if ((lastx != newx) || (lasty != newy)) {
        Graf_do_mouse (apid, M_OFF, NULL);
        Vdi_v_pline (globals->vid, 5, xyarray);

        xyarray[0] = newx;
        xyarray[1] = newy;
        xyarray[2] = newx + w - 1;
        xyarray[3] = xyarray[1];
        xyarray[4] = xyarray[2];
        xyarray[5] = newy + h - 1;
        xyarray[6] = xyarray[0];
        xyarray[7] = xyarray[5];
        xyarray[8] = xyarray[0];
        xyarray[9] = xyarray[1];
        
        Vdi_v_pline (globals->vid, 5, xyarray);
        Graf_do_mouse (apid, M_ON, NULL);
        
        lastx = newx;
        lasty = newy;
      }
    }
    
    if(eo.events & MU_BUTTON) {
      if (!(eo.mb & LEFT_BUTTON)) {
        break;
      }
    }
  }

  Graf_do_mouse (apid, M_OFF, NULL);
  Vdi_v_pline (globals->vid, 5, xyarray);
  Graf_do_mouse (apid, M_ON, NULL);

  *endx = lastx;
  *endy = lasty;
  
  return 1;
}


/*
** Exported
**
** 1998-12-20 CG
** 1998-12-25 CG
*/
void
Graf_dragbox (AES_PB *apb) {
  Wind_do_update (apb->global->apid, BEG_MCTRL);
  
  apb->int_out[0] = Graf_do_dragbox (apb->global->apid,
                                     apb->int_in[0],
                                     apb->int_in[1],
                                     apb->int_in[2],
                                     apb->int_in[3],
                                     (RECT *)&apb->int_in[4],
                                     &apb->int_out[1],
                                     &apb->int_out[2]);
  
  Wind_do_update (apb->global->apid, END_MCTRL);
}


/****************************************************************************
 * Graf_do_grmobox                                                          *
 *  Implementation of graf_growbox() and graf_movebox().                    *
 ****************************************************************************/
WORD              /*                                                        */
Graf_do_grmobox(  /*                                                        */
RECT *r1,         /* Start rectangle.                                       */
RECT *r2)         /* End rectangle.                                         */
/****************************************************************************/
{
  WORD f,g;
  WORD xy[10];
  
  WORD dx = (r2->x - r1->x) / GRAF_STEPS;
  WORD dy = (r2->y - r1->y) / GRAF_STEPS;
  WORD dw = (r2->width - r1->width) / GRAF_STEPS;
  WORD dh = (r2->height - r1->height) / GRAF_STEPS;
  
  Psemaphore(SEM_LOCK,GRAFSEM,-1);

  Vdi_vswr_mode(grafvid,MD_XOR);
  Vdi_vsl_type(grafvid,3);
  
  Vdi_v_hide_c(grafvid);
  
  for(g = 0; g < 2; g++) {
    xy[0] = r1->x;
    xy[1] = r1->y;
    xy[2] = r1->x + r1->width - 1;
    xy[3] = xy[1];
    xy[4] = xy[2];
    xy[5] = r1->y + r1->height - 1;
    xy[6] = xy[0];
    xy[7] = xy[5];
    xy[8] = xy[0];
    xy[9] = xy[1];
    
    for(f=0; f < GRAF_STEPS; f++) { 
      /* Draw initial growing outline */
      
      Vdi_v_pline(grafvid,5,xy);
        
      xy[0] += dx;
      xy[1] += dy;
      xy[2] += dx + (dw << 1);
      xy[3] = xy[1];
      xy[4] = xy[2];
      xy[5] += dy + (dh << 1);
      xy[6] = xy[0];
      xy[7] = xy[5];
      xy[8] = xy[0];
      xy[9] = xy[1];
    }
  }
  
  Vdi_v_show_c(grafvid,1);
  
  Vdi_vswr_mode(grafvid,MD_TRANS);
  
  Psemaphore(SEM_LOCK,GRAFSEM,-1);

  return 1;
}


/*
** Exported
**
** 1999-01-11 CG
*/
void
Graf_movebox (AES_PB *apb) {
  RECT r1,r2;
  GLOBAL_APPL * globals = get_globals (apb->global->apid);

  if(globals->common->graf_mbox) {
    r1.x = apb->int_in[2];
    r1.y = apb->int_in[3];
    r1.width = apb->int_in[0];
    r1.height = apb->int_in[1];
    
    r2.x = apb->int_in[4];
    r2.y = apb->int_in[5];
    r2.width = apb->int_in[0];
    r2.height = apb->int_in[1];
    
    Wind_do_update (apb->global->apid, BEG_UPDATE);
    
    apb->int_out[0] = Graf_do_grmobox(&r1,&r2);
    
    Wind_do_update (apb->global->apid, END_UPDATE);
  } else {
    apb->int_out[0] = 1;
  }
}


/*
** Exported
**
** 1999-01-11 CG
*/
void
Graf_growbox (AES_PB *apb) {
  GLOBAL_APPL * globals = get_globals (apb->global->apid);

  if(globals->common->graf_growbox) {
    Wind_do_update (apb->global->apid, BEG_UPDATE);
    apb->int_out[0] = Graf_do_grmobox((RECT *)&apb->int_in[0],
                                      (RECT *)&apb->int_in[4]);
    Wind_do_update (apb->global->apid, END_UPDATE);
  } else {
    apb->int_out[0] = 1;
  }
}


/*
** Exported
**
** 1999-01-11 CG
*/
void
Graf_shrinkbox (AES_PB * apb) {
  GLOBAL_APPL * globals = get_globals (apb->global->apid);

  if(globals->common->graf_shrinkbox) {
    Wind_do_update (apb->global->apid, BEG_UPDATE);
    apb->int_out[0] = Graf_do_grmobox((RECT *)&apb->int_in[4], 
                                      (RECT *)&apb->int_in[0]);
    Wind_do_update (apb->global->apid, END_UPDATE);
  } else {
    apb->int_out[0] = 1;
  }
}


/*
** Exported
**
** 1998-12-19 CG
** 1998-12-23 CG
** 1999-01-09 CG
** 1999-04-10 CG
*/
WORD
Graf_do_watchbox (WORD     apid,
                  OBJECT * tree,
                  WORD     obj,
                  WORD     instate,
                  WORD     outstate)
{
  EVENTIN ei = {
    MU_BUTTON | MU_M1,
    0,
    LEFT_BUTTON,
    0,
    0,
    {0,0,0,0},
    0,
    {0,0,0,0},
    0,
    0
  };
  EVENTOUT      eo;
  COMMSG        buffer;
  RECT          clip;
  WORD          mx;
  WORD          my;
  WORD          mb;
  WORD          ks;
  GLOBAL_APPL * globals = get_globals (apid);

  Objc_calc_clip (tree, obj, &clip);
  Objc_do_offset (tree, obj, (WORD *)&ei.m1r);
  ei.m1r.width = tree[obj].ob_width;
  ei.m1r.height = tree[obj].ob_height;
  
  /* Get the current coordinates */
  Graf_do_mkstate (apid, &mx, &my, &mb, &ks);

  if(Misc_inside(&ei.m1r, mx, my)) {
    ei.m1flag = MO_LEAVE;
    
    Graf_do_mouse (apid, M_OFF, NULL);
    Objc_do_change (globals->vid, tree, obj, &clip, instate, REDRAW);
    Graf_do_mouse (apid, M_ON, NULL);
  } else {
    ei.m1flag = MO_ENTER;
    
    Graf_do_mouse (apid, M_OFF, NULL);
    Objc_do_change (globals->vid, tree, obj, &clip, outstate, REDRAW);
    Graf_do_mouse (apid, M_ON, NULL);
  }
  
  Wind_do_update (apid, BEG_MCTRL);
  
  while (TRUE) {
    Evnt_do_multi (apid, &ei, &buffer, &eo, 0, DONT_HANDLE_MENU_BAR);
    
    if(eo.events & MU_BUTTON) {
      break;
    }
    
    if(ei.m1flag == MO_LEAVE) {
      ei.m1flag = MO_ENTER;
      Graf_do_mouse (apid, M_OFF, NULL);
      Objc_do_change (globals->vid, tree, obj, &clip, outstate, REDRAW);
      Graf_do_mouse (apid, M_ON, NULL);
    } else {
      ei.m1flag = MO_LEAVE;
      Graf_do_mouse (apid, M_OFF, NULL);
      Objc_do_change (globals->vid, tree, obj, &clip, instate, REDRAW);
      Graf_do_mouse (apid, M_ON, NULL);
    }
  }
  
  Wind_do_update (apid, END_MCTRL);
  
  return (ei.m1flag == MO_LEAVE) ? 1 : 0;
}


/*
** Exported
**
** 1998-12-20 CG
*/
void
Graf_watchbox (AES_PB *apb) {
  apb->int_out[0] = Graf_do_watchbox(apb->global->apid,
                                     (OBJECT *)apb->addr_in[0],
                                     apb->int_in[1],
                                     apb->int_in[2],
                                     apb->int_in[3]);
}


/*
** Exported
**
** 1998-12-20 CG
** 1998-12-25 CG
*/
WORD
Graf_do_slidebox (WORD     apid,
                  OBJECT * tree,
                  WORD     parent,
                  WORD     obj,
                  WORD     orient) {
  RECT bound;
  RECT slid;
  WORD x,y;
        
  if (tree == NULL) {
    return 0;
  }
        
  Objc_area_needed(tree,obj,&slid);
  Objc_area_needed(tree,parent,&bound);
        
  Wind_do_update (apid, BEG_MCTRL);
                
  Graf_do_dragbox (apid,
                   slid.width,
                   slid.height,
                   slid.x,slid.y,
                   &bound,
                   &x,
                   &y);
                                                                        
  Wind_do_update (apid, END_MCTRL);
        
  if(orient == 0) {
    if(tree[obj].ob_width != tree[parent].ob_width) {
      return (WORD)((((LONG)x - (LONG)bound.x) * 1000L) /
                    ((LONG)(tree[parent].ob_width - tree[obj].ob_width)));
    }
  } else {
    if(tree[obj].ob_height != tree[parent].ob_height) {
      return (WORD)((((LONG)y - (LONG)bound.y) * 1000L) /
                    ((LONG)(tree[parent].ob_height - tree[obj].ob_height)));
    }
  }
                                                                        
  return 0;
}


/*
** Exported
**
** 1998-12-20 CG
*/
void
Graf_slidebox (AES_PB *apb) {
  apb->int_out[0] = Graf_do_slidebox(apb->global->apid,
                                     (OBJECT *)apb->addr_in[0],
                                     apb->int_in[0],
                                     apb->int_in[1],
                                     apb->int_in[2]);
}


/*graf_handle 0x004d*/

/*
** Exported
**
** 1999-01-09 CG
*/
void
Graf_do_handle (WORD * cwidth,
                WORD * cheight,
                WORD * width,
                WORD * height) {
  GLOBAL_COMMON * globals = get_global_common ();

  *cwidth = globals->clwidth;
  *cheight = globals->clheight;
  *width = globals->blwidth;
  *height = globals->blheight;
}


void    Graf_handle(AES_PB *apb) {
  GLOBAL_APPL * globals = get_globals (apb->global->apid);

  apb->int_out[0] = globals->common->vid;
        
  Graf_do_handle(&apb->int_out[1]
                 ,&apb->int_out[2],&apb->int_out[3]
                 ,&apb->int_out[4]);    
}


/*
** Exported
**
** 1999-01-01 CG
*/
WORD
Graf_do_mouse (WORD    apid,
               WORD    mode,
               MFORM * formptr) {
  WORD retval = 1;
  MFORM tmp;
  GLOBAL_APPL * globals = get_globals (apid);

  C_GRAF_MOUSE par;
  R_GRAF_MOUSE ret;

  par.common.call = SRV_GRAF_MOUSE;
  par.common.apid = apid;
  par.common.pid = getpid ();
  par.mode = mode;
	
  Client_send_recv (&par,
                    sizeof (C_GRAF_MOUSE),
                    &ret,
                    sizeof (R_GRAF_MOUSE));

  return ret.common.retval;

  switch (mode) {
  case ARROW: /*0x000*/
    last = current;
    current = m_arrow;
    Vdi_v_hide_c (globals->vid);
    Vdi_vsc_form (globals->vid, &m_arrow);
    Vdi_v_show_c (globals->vid, 0);
    break;
    
  case TEXT_CRSR: /*0x001*/
    last = current;
    current = m_text_crsr;
    Vdi_v_hide_c (globals->vid);
    Vdi_vsc_form(globals->vid, &m_text_crsr);
    Vdi_v_show_c(globals->vid, 0);
    break;
    
  case BUSY_BEE: /*0x002*/
    last = current;
    current = m_busy_bee;
    Vdi_v_hide_c (globals->vid);
    Vdi_vsc_form (globals->vid, &m_busy_bee);
    Vdi_v_show_c (globals->vid, 0);
    break;
    
  case POINT_HAND: /*0x003*/
    last = current;
    current = m_point_hand;
    Vdi_v_hide_c (globals->vid);
    Vdi_vsc_form (globals->vid, &m_point_hand);
    Vdi_v_show_c (globals->vid, 0);
    break;
    
  case FLAT_HAND: /*0x004*/
    last = current;
    current = m_flat_hand;
    Vdi_v_hide_c (globals->vid);
    Vdi_vsc_form (globals->vid, &m_flat_hand);
    Vdi_v_show_c (globals->vid, 0);
    break;
    
  case THIN_CROSS: /*0x005*/
    last = current;
    current = m_thin_cross;
    Vdi_v_hide_c (globals->vid);
    Vdi_vsc_form (globals->vid, &m_thin_cross);
    Vdi_v_show_c (globals->vid, 0);
    break;
    
  case THICK_CROSS: /*0x006*/
    last = current;
    current = m_thick_cross;
    Vdi_v_hide_c (globals->vid);
    Vdi_vsc_form (globals->vid, &m_thick_cross);
    Vdi_v_show_c (globals->vid, 0);
    break;
    
  case OUTLN_CROSS: /*0x007*/
    last = current;
    current = m_outln_cross;
    Vdi_v_hide_c (globals->vid);
    Vdi_vsc_form (globals->vid, &m_outln_cross);
    Vdi_v_show_c (globals->vid, 0);
    break;
    
  case USER_DEF :
    last = current;
    current = *formptr;
    Vdi_v_hide_c (globals->vid);
    Vdi_vsc_form (globals->vid, formptr);
    Vdi_v_show_c (globals->vid, 0);
    break;
    
  case M_OFF    :
    Vdi_v_hide_c (globals->vid);
    break;
                                
  case M_ON     :
    Vdi_v_show_c (globals->vid, 0);
    break;
    
  case M_SAVE:
    last_saved = current;
    break;
    
  case M_LAST:
    last = current;
    current = last_saved;
    
    Vdi_v_hide_c (globals->vid);
    Vdi_vsc_form (globals->vid, &current);
    Vdi_v_show_c (globals->vid, 0);
    
    break;
    
  case M_RESTORE:
    tmp = current;
    current = last;
    last = tmp;
    
    Vdi_v_hide_c(globals->vid);
    Vdi_vsc_form(globals->vid, &current);
    Vdi_v_show_c(globals->vid, 0);
    
    break;                      
    
  default:
    DB_printf("%s: Line %d: Graf_do_mouse:\r\n"
              "Unknown mode %d\r\n",__FILE__,__LINE__,mode);
    retval = 0;
  };

  return retval;
}


/*
** Exported
**
** 1999-01-01 CG
*/
void
Graf_mouse (AES_PB *apb) {
  apb->int_out[0] = Graf_do_mouse (apb->global->apid,
                                   apb->int_in[0],
                                   (MFORM *)apb->addr_in[0]);
}


/*
** Exported
**
** 1998-12-23 CG
*/
WORD
Graf_do_mkstate (WORD   apid,
                 WORD * mx,
                 WORD * my,
                 WORD * mb,
                 WORD * ks) {
  C_GRAF_MKSTATE par;
  R_GRAF_MKSTATE ret;

  par.common.call = SRV_GRAF_MKSTATE;
  par.common.apid = apid;
  par.common.pid = getpid ();
	
  Client_send_recv (&par,
                    sizeof (C_GRAF_MKSTATE),
                    &ret,
                    sizeof (R_GRAF_MKSTATE));

  *mx = ret.mx;
  *my = ret.my;
  *mb = ret.mb;
  *ks = ret.ks;

  return ret.common.retval;
}


/*
** Exported
**
** 1998-12-23 CG
*/
void
Graf_mkstate (AES_PB *apb) {
  apb->int_out[0] = Graf_do_mkstate (apb->global->apid,
                                     &apb->int_out [1],
                                     &apb->int_out [2],
                                     &apb->int_out [3],
                                     &apb->int_out [4]);
}
