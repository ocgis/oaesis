/*
** graf.c
**
** Copyright 1996 - 2000 Christer Gustavsson <cg@nocrew.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**  
** Read the file COPYING for more information.
*/

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
#include <unistd.h>

#include "aesbind.h"
#include "cursors.h"
#include "debug.h"
#include "evnt.h"
#include "evnthndl.h"
#include "graf.h"
#include "lib_comm.h"
#include "lib_global.h"
#include "lib_misc.h"
#include "mintdefs.h"
#include "objc.h"
#include "resource.h"
#include "srv_interface.h"
#include "srv_put.h"
#include "types.h"
#include "wind.h"

#define GRAF_STEPS 15

static MFORM m_arrow,m_text_crsr,m_busy_bee,m_point_hand,m_flat_hand,
             m_thin_cross,m_thick_cross,m_outln_cross;


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
  
  for(i = 0; i < 16; i++)
  {
    /*
    ** I'm not sure that this is the right place to convert the byte order
    ** or if it should be converted at all
    */
    mf->mf_mask[i] = ntohs(icon->ob_spec.iconblk->ib_pmask[i]);
    mf->mf_data[i] = ntohs(icon->ob_spec.iconblk->ib_pdata[i]);
  }
}


void
Graf_init_mouseforms(void)
{
  GLOBAL_COMMON * globals;

  globals = get_global_common();

  icon2mform(&m_arrow, &globals->mouseformstad[MARROW]);
  icon2mform(&m_text_crsr, &globals->mouseformstad[MTEXT_CRSR]);
  icon2mform(&m_busy_bee, &globals->mouseformstad[MBUSY_BEE]);
  icon2mform(&m_point_hand, &globals->mouseformstad[MPOINT_HAND]);
  icon2mform(&m_flat_hand, &globals->mouseformstad[MFLAT_HAND]);
  icon2mform(&m_thin_cross, &globals->mouseformstad[MTHIN_CROSS]);
  icon2mform(&m_thick_cross, &globals->mouseformstad[MTHICK_CROSS]);
  icon2mform(&m_outln_cross, &globals->mouseformstad[MOUTLN_CROSS]);
}


/*
** Exported
*/
WORD
Graf_do_rubberbox(WORD   apid,
                  WORD   bx,
                  WORD   by,
                  WORD   minw,
                  WORD   minh,
                  WORD * endw,
                  WORD * endh)
{
  EVENTIN ei =
  {
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
  int           xyarray[10];
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
  if(!(mb & LEFT_BUTTON))
  {
    *endw = lastw;
    *endh = lasth;

    if(*endw < minw)
    {
      *endw = minw;
    }

    if(*endh < minh)
    {
      *endh = minh;
    }
        
    return 1;
  }

  vsl_color (globals->vid, BLACK);
  vswr_mode (globals->vid, MD_XOR);
  vsl_type (globals->vid, DOTTED);

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
  v_pline (globals->vid, 5, xyarray);
  Graf_do_mouse (apid, M_ON, NULL);
  
  ei.m1r.x = mx;
  ei.m1r.y = my;

  while(TRUE)
  {
    Evnt_do_multi (apid, &ei, &buffer, &eo, 0, DONT_HANDLE_MENU_BAR);

    if(eo.events & MU_M1)
    {
      neww = eo.mx - bx;
      newh = eo.my - by;

      if(neww < minw)
      {
        neww = minw;
      }

      if(newh < minh)
      {
        newh = minh;
      }

      if((lastw != neww) || (lasth != newh))
      {
        Graf_do_mouse (apid, M_OFF, NULL);
        v_pline (globals->vid, 5, xyarray);

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
         
        v_pline (globals->vid, 5, xyarray);
        Graf_do_mouse (apid, M_ON, NULL);
      }

      /* Update coordinates for mouse leave rectangle */
      ei.m1r.x = eo.mx;
      ei.m1r.y = eo.my;
    }
                
    if(eo.events & MU_BUTTON)
    {
      if (!(eo.mb & LEFT_BUTTON))
      {
        break;
      }
    }
  }

  Graf_do_mouse (apid, M_OFF, NULL);
  v_pline (globals->vid, 5, xyarray);
  Graf_do_mouse (apid, M_ON, NULL);

  *endw = lastw;
  *endh = lasth;
        
  return 1;
}


/*
** Exported
*/
void
Graf_rubberbox (AES_PB *apb)
{
  CHECK_APID(apb->global->apid);

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
*/
WORD
Graf_do_dragbox (WORD   apid,
                 WORD   w,
                 WORD   h,
                 WORD   sx,
                 WORD   sy,
                 RECT * bound,
                 WORD * endx,
                 WORD * endy)
{
  EVENTIN ei =
  {
    MU_BUTTON | MU_M1 /*| 0x8000*/,
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
  int           xyarray[10];
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
  Graf_do_mkstate(apid, &mx, &my, &mb, &ks);

  /* Calculate offset from mouse position to rectangle origo */
  relx = sx - mx;
  rely = sy - my;

  /* If the mouse button is release return directly */
  if (!(mb & LEFT_BUTTON))
  {
    *endx = sx;
    *endy = sy;
    
    return 1;
  }

  vsl_color (globals->vid, BLACK);
  vswr_mode (globals->vid, MD_XOR);
  vsl_type (globals->vid, DOTTED);

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
  v_pline (globals->vid, 5, xyarray);
  Graf_do_mouse (apid, M_ON, NULL);

  ei.m1r.x = mx;
  ei.m1r.y = my;
  
  while(TRUE)
  {
    Evnt_do_multi (apid, &ei, &buffer, &eo, 0, DONT_HANDLE_MENU_BAR);

    if (eo.events & MU_M1)
    {
      newx = eo.mx + relx;
      newy = eo.my + rely;
      
      if (newx < bound->x)
      {
        newx = bound->x;
      }
      else if (newx > (bound->x + bound->width - w))
      {
        newx = bound->x + bound->width - w;
      }

      if (newy < bound->y)
      {
        newy = bound->y;
      }
      else if (newy > (bound->y + bound->height - h))
      {
        newy = bound->y + bound->height - h;
      }
      
      if ((lastx != newx) || (lasty != newy))
      {
        Graf_do_mouse (apid, M_OFF, NULL);
        v_pline (globals->vid, 5, xyarray);

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
        
        v_pline (globals->vid, 5, xyarray);
        Graf_do_mouse (apid, M_ON, NULL);
        
        lastx = newx;
        lasty = newy;
      }

      /* Update coordinates for mouse leave rectangle */
      ei.m1r.x = eo.mx;
      ei.m1r.y = eo.my;
    }
    
    if(eo.events & MU_BUTTON)
    {
      if (!(eo.mb & LEFT_BUTTON))
      {
        break;
      }
    }
  }

  Graf_do_mouse (apid, M_OFF, NULL);
  v_pline (globals->vid, 5, xyarray);
  Graf_do_mouse (apid, M_ON, NULL);

  *endx = lastx;
  *endy = lasty;
  
  return 1;
}


/*
** Exported
*/
void
Graf_dragbox(AES_PB *apb)
{
  CHECK_APID(apb->global->apid);

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


/*
** Description
** Implementation of graf_growbox() and graf_movebox()
*/
WORD
Graf_do_grmobox(WORD   apid,
                RECT * r1,
                RECT * r2)
{
  WORD f,g;
  int  xy[10];
  GLOBAL_APPL * globals = get_globals(apid);
  
  WORD dx = (r2->x - r1->x) / GRAF_STEPS;
  WORD dy = (r2->y - r1->y) / GRAF_STEPS;
  WORD dw = (r2->width - r1->width) / GRAF_STEPS;
  WORD dh = (r2->height - r1->height) / GRAF_STEPS;
  
  Wind_do_update(apid, BEG_UPDATE);

  vswr_mode(globals->vid,MD_XOR);
  vsl_type(globals->vid,3);
  
  v_hide_c(globals->vid);
  
  for(g = 0; g < 2; g++)
  {
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
    
    for(f=0; f < GRAF_STEPS; f++)
    { 
      /* Draw initial growing outline */
      
      v_pline(globals->vid,5,xy);
        
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
  
  v_show_c(globals->vid, 1);
  
  Wind_do_update(apid, END_UPDATE);

  return 1;
}


/*
** Exported
*/
void
Graf_movebox (AES_PB *apb)
{
  RECT r1,r2;
  GLOBAL_APPL * globals;

  CHECK_APID(apb->global->apid);

  globals = get_globals (apb->global->apid);

  if(globals->common->graf_mbox)
  {
    r1.x = apb->int_in[2];
    r1.y = apb->int_in[3];
    r1.width = apb->int_in[0];
    r1.height = apb->int_in[1];
    
    r2.x = apb->int_in[4];
    r2.y = apb->int_in[5];
    r2.width = apb->int_in[0];
    r2.height = apb->int_in[1];
    
    apb->int_out[0] = Graf_do_grmobox(apb->global->apid, &r1, &r2);
  }
  else
  {
    apb->int_out[0] = 1;
  }
}


/*
** Exported
*/
void
Graf_growbox(AES_PB *apb)
{
  GLOBAL_APPL * globals;

  CHECK_APID(apb->global->apid);

  globals = get_globals (apb->global->apid);

  if(globals->common->graf_growbox)
  {
    apb->int_out[0] = Graf_do_grmobox(apb->global->apid,
                                      (RECT *)&apb->int_in[0],
                                      (RECT *)&apb->int_in[4]);
  }
  else
  {
    apb->int_out[0] = 1;
  }
}


/*
** Exported
*/
void
Graf_shrinkbox (AES_PB * apb) {
  GLOBAL_APPL * globals;

  CHECK_APID(apb->global->apid);

  globals = get_globals (apb->global->apid);

  if(globals->common->graf_shrinkbox)
  {
    apb->int_out[0] = Graf_do_grmobox(apb->global->apid,
                                      (RECT *)&apb->int_in[4], 
                                      (RECT *)&apb->int_in[0]);
  }
  else
  {
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
*/
void
Graf_watchbox (AES_PB *apb)
{
  CHECK_APID(apb->global->apid);

  apb->int_out[0] = Graf_do_watchbox(apb->global->apid,
                                     (OBJECT *)apb->addr_in[0],
                                     apb->int_in[1],
                                     apb->int_in[2],
                                     apb->int_in[3]);
}


/*
** Exported
*/
WORD
Graf_do_slidebox (WORD     apid,
                  OBJECT * tree,
                  WORD     parent,
                  WORD     obj,
                  WORD     orient)
{
  RECT bound;
  RECT slid;
  WORD x,y;
        
  if (tree == NULL)
  {
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
*/
void
Graf_slidebox (AES_PB *apb)
{
  CHECK_APID(apb->global->apid);

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


/*
** Exported
**
** 1999-08-08 CG
*/
void
Graf_handle(AES_PB *apb) {
  GLOBAL_COMMON * globals = get_global_common ();

  apb->int_out[0] = globals->physical_vdi_id;
        
  Graf_do_handle(&apb->int_out[1]
                 ,&apb->int_out[2],&apb->int_out[3]
                 ,&apb->int_out[4]);    
}


/*
** Exported
*/
WORD
Graf_do_mouse(WORD    apid,
              WORD    mode,
              MFORM * formptr)
{
  WORD          retval = 1;
  GLOBAL_APPL * globals;

  C_GRAF_MOUSE         par;
  static R_GRAF_MOUSE  ret;

  globals = get_globals (apid);

  switch (mode)
  {
  case ARROW: /*0x000*/
    par.cursor = m_arrow;
    break;
    
  case TEXT_CRSR: /*0x001*/
    par.cursor = m_text_crsr;
    break;
    
  case BUSY_BEE: /*0x002*/
    par.cursor = m_busy_bee;
    break;
    
  case POINT_HAND: /*0x003*/
    par.cursor = m_point_hand;
    break;
    
  case FLAT_HAND: /*0x004*/
    par.cursor = m_flat_hand;
    break;
    
  case THIN_CROSS: /*0x005*/
    par.cursor = m_thin_cross;
    break;
    
  case THICK_CROSS: /*0x006*/
    par.cursor = m_thick_cross;
    break;
    
  case OUTLN_CROSS: /*0x007*/
    par.cursor = m_outln_cross;
    break;
    
  case USER_DEF:
    par.cursor = *formptr;
    break;

  case M_OFF:
  case M_ON:
  case M_SAVE:
  case M_LAST:
  case M_RESTORE:
  default:
    ;
  }

  switch (mode)
  {
  case ARROW: /*0x000*/
  case TEXT_CRSR: /*0x001*/
  case BUSY_BEE: /*0x002*/
  case POINT_HAND: /*0x003*/
  case FLAT_HAND: /*0x004*/
  case THIN_CROSS: /*0x005*/
  case THICK_CROSS: /*0x006*/
  case OUTLN_CROSS: /*0x007*/
  case USER_DEF:
  case M_RESTORE:
  case M_SAVE:
  case M_LAST:
    PUT_C_ALL(GRAF_MOUSE, &par);
    
    par.mode = mode;
    
    CLIENT_SEND_RECV(&par,
                     sizeof (C_GRAF_MOUSE),
                     &ret,
                     sizeof (R_GRAF_MOUSE));

    retval = ret.common.retval;
    break;

  case M_OFF:
  case M_ON:
  default:
    ;
  }

  switch (mode)
  {
  case ARROW: /*0x000*/
  case TEXT_CRSR: /*0x001*/
  case BUSY_BEE: /*0x002*/
  case POINT_HAND: /*0x003*/
  case FLAT_HAND: /*0x004*/
  case THIN_CROSS: /*0x005*/
  case THICK_CROSS: /*0x006*/
  case OUTLN_CROSS: /*0x007*/
  case USER_DEF:
  case M_RESTORE:
  case M_LAST:
    v_hide_c (globals->vid);
    vsc_form (globals->vid, &ret.cursor);
    v_show_c (globals->vid, 0);
    break;

  case M_OFF    :
    v_hide_c (globals->vid);
    break;

  case M_ON     :
    v_show_c (globals->vid, 0);
    break;

  case M_SAVE:
    break;

  default:
    DB_printf("%s: Line %d: Graf_do_mouse:\r\n"
              "Unknown mode %d\r\n",__FILE__,__LINE__,mode);
    retval = 0;
  }

  return retval;
}


/*
** Exported
*/
void
Graf_mouse (AES_PB *apb)
{
  CHECK_APID(apb->global->apid);

  apb->int_out[0] = Graf_do_mouse (apb->global->apid,
                                   apb->int_in[0],
                                   (MFORM *)apb->addr_in[0]);
}


/*
** Exported
*/
WORD
Graf_do_mkstate (WORD   apid,
                 WORD * mx,
                 WORD * my,
                 WORD * mb,
                 WORD * ks) {
  C_GRAF_MKSTATE par;
  R_GRAF_MKSTATE ret;

  PUT_C_ALL(GRAF_MKSTATE, &par);
	
  CLIENT_SEND_RECV(&par,
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
*/
void
Graf_mkstate (AES_PB *apb)
{
  CHECK_APID(apb->global->apid);

  apb->int_out[0] = Graf_do_mkstate (apb->global->apid,
                                     &apb->int_out [1],
                                     &apb->int_out [2],
                                     &apb->int_out [3],
                                     &apb->int_out [4]);
}
