/*
** menu.c
**
** Copyright 1996 - 2001 Christer Gustavsson <cg@nocrew.org>
** Copyright 1996 Martin Budsjö <marbud@nocrew.org>
** Copyright 1996 Klaus Pedersen <kkp@gamma.dou.dk>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**  
** Read the file COPYING for more information.
**
*/

#define DEBUGLEVEL 3


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_BASEPAGE_H
#include <basepage.h>
#endif

#include <ctype.h>

#ifdef HAVE_MINTBIND_H
#include <mintbind.h>
#endif

#ifdef HAVE_PROCESS_H
#include <process.h>
#endif

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "aesbind.h"
#include "appl.h"
/*#include "boot.h"*/
#include "debug.h"
#include "evnt.h"
#include "form.h"
#include "fsel.h"
#include "graf.h"
#include "lib_comm.h"
#include "lib_global.h"
#include "lib_misc.h"
#include "mintdefs.h"
#include "menu.h"
#include "mesagdef.h"
#include "objc.h"
#include "resource.h"
#include "rsrc.h"
#include "srv_put.h"
#include "srv_interface.h"
#include "types.h"
#include "wind.h"

#ifdef HAVE_SYSVARS_H
#include <sysvars.h>
#endif


/*
** Description
** Creates and updates the menu list entries for the specified menu. 
*/
static
WORD
Menu_bar_install(WORD     apid,
                 OBJECT * tree)
{
  WORD          i;
  WORD          cheight;
  WORD          cwidth;
  WORD          height;
  WORD          width;
  GLOBAL_APPL * globals = get_globals (apid);

  globals->menu = tree;
  
  Graf_do_handle (&cwidth, &cheight, &width, &height);

  DEBUG2 ("menu.c: Menu_bar_install: cheight = %d", cheight);

  /* Modify width of menu*/
  OB_WIDTH_PUT(&tree[0], globals->common->screen.width);
  OB_WIDTH_PUT(&tree[OB_HEAD(&tree[0])], globals->common->screen.width);

  /* Modify height of bar ... */
  OB_HEIGHT_PUT(&tree[OB_HEAD(&tree[0])], cheight + 3);
  /* ... and titles */
  OB_HEIGHT_PUT(&tree[OB_HEAD(&tree[OB_HEAD(&tree[0])])], cheight + 3);

#if 0
  i = OB_HEAD(&tree[OB_HEAD(&tree[OB_HEAD(&tree[0])])]);
  
  while(i != -1)
  {		
    OB_HEIGHT_SET(&tree[i], cheight + 3);
    
    if(i == OB_TAIL(&tree[OB_NEXT(&tree[i])]))
    {
      break;
    }
    
    i = OB_NEXT(&tree[i]);
  }
#endif
  
  /* Mark all drop down menus with HIDETREE and set y position */
  OB_Y_PUT(&tree[OB_NEXT(&tree[OB_HEAD(&tree[0])])], cheight + 3);
  
  i = OB_HEAD(&tree[OB_NEXT(&tree[OB_HEAD(&tree[0])])]);
  while(i != -1)
  {		
    OB_FLAGS_SET(&tree[i], HIDETREE);
    
    if(i == OB_TAIL(&tree[OB_NEXT(&tree[i])]))
    {
      break;
    }
    
    i = OB_NEXT(&tree[i]);
  }

  /* Calculate menu bar area */
  i = OB_HEAD(&tree[OB_HEAD(&tree[0])]);
  
  globals->menu_bar_rect.x =
    OB_X(&tree[0]) + OB_X(&tree[OB_HEAD(&tree[0])]) + OB_X(&tree[i]);
  globals->menu_bar_rect.width =
    OB_X(&tree[OB_TAIL(&tree[i])]) + OB_WIDTH(&tree[OB_TAIL(&tree[i])]);
  globals->menu_bar_rect.y = 0;
  globals->menu_bar_rect.height = cheight + 3;

  DEBUG2 ("menu_bar_rect = %d %d %d %d",
          globals->menu_bar_rect.x,
          globals->menu_bar_rect.y,
          globals->menu_bar_rect.width,
          globals->menu_bar_rect.height);

  /* Return OK */
  return 1;
}


/*
** Exported
*/
WORD
Menu_do_bar (WORD     apid,
             OBJECT * tree,
             WORD     mode) {
  C_MENU_BAR    par;
  R_MENU_BAR    ret;
  GLOBAL_APPL * globals = get_globals (apid);

  PUT_C_ALL(MENU_BAR, &par);

  par.mode = mode;

  CLIENT_SEND_RECV(&par,
                   sizeof (C_MENU_BAR),
                   &ret,
                   sizeof (R_MENU_BAR));

  /*
  ** FIXME
  ** Check retval that the operation went fine
  */
  switch (mode)
  {
  case MENU_REMOVE :
    globals->menu = NULL;
    break;
  case MENU_INSTALL :
    Menu_bar_install (apid, tree);
    break;
  }

  return ret.common.retval;
}


/*
** Exported
*/
void
Menu_bar (AES_PB *apb)
{
  CHECK_APID(apb->global->apid);

  apb->int_out[0] = Menu_do_bar (apb->global->apid,
                                 (OBJECT *)apb->addr_in[0],
                                 apb->int_in[0]);
}


/*
** Description
** Modify a menu entry
*/
#define MENU_MODIFY(object,ison,isoff,onstate,state) \
  if(state == isoff) \
  { \
    OB_STATE_CLEAR(object, state); \
  } \
  else if (state == ison) \
  { \
    OB_STATE_SET(object, state); \
  }


/*
** Description
** 0x001f menu_icheck()
*/
void
Menu_icheck(AES_PB * apb)
{
  MENU_MODIFY(&((OBJECT *)apb->addr_in[0])[apb->int_in[0]],
              CHECK,
              UNCHECK,
              CHECKED,
              apb->int_in[1]);

  apb->int_out[0] = 1;
}


/*
** Description
** 0x0020 menu_ienable()
*/
void
Menu_ienable(AES_PB * apb)
{
  MENU_MODIFY(&((OBJECT *)apb->addr_in[0])[apb->int_in[0]],
              DISABLE,
              ENABLE,
              DISABLED,
              apb->int_in[1]);

  apb->int_out[0] = 1;
}

/*
** Description
** 0x0021 menu_tnormal()
*/
void
Menu_tnormal(AES_PB * apb)
{
  MENU_MODIFY(&((OBJECT *)apb->addr_in[0])[apb->int_in[0]],
              HIGHLIGHT,
              UNHIGHLIGHT,
              SELECTED,
              apb->int_in[1]);

  apb->int_out[0] = 1;
}


/* Menu_text return values */
#define MENU_TEXT_OK    1
#define MENU_TEXT_ERROR 0

/*
** Description
** Implementation of menu_text()
*/
static
inline
WORD
Menu_do_text(OBJECT * tree,
             WORD     obj,
             BYTE *   text)
{
  TEDINFO * ti;

  switch(OB_TYPE(&tree[obj]))
  {
  case G_TEXT:
  case G_BOXTEXT:
  case G_FTEXT:
  case G_FBOXTEXT:
    if(OB_FLAGS(&tree[obj]) & INDIRECT)
    {
      ti = *((TEDINFO **)OB_SPEC(&tree[obj]));
    }
    else
    {
      ti = (TEDINFO *)OB_SPEC(&tree[obj]);
    }
    
    strcpy(TE_PTEXT(ti),
           text);
    break;

  case G_STRING:
  case G_TITLE:
    strcpy((char *)OB_SPEC(&tree[obj]), text);
    break;

  default:
    /* No way to handle objects without a text */
    return MENU_TEXT_ERROR;
  }

  return MENU_TEXT_OK;
}


/*
** Description
** 0x0022 menu_text()
*/
void
Menu_text(AES_PB * apb)
{
  apb->int_out[0] = Menu_do_text((OBJECT *)apb->addr_in[0],
                                 apb->int_in[0],
                                 (BYTE *)apb->addr_in[1]);
}


/*
** Description
** Implementation of menu_register ()
*/
static
WORD
Menu_do_register(WORD   apid,
                 WORD   register_apid,
                 BYTE * title)
{
  C_MENU_REGISTER par;
  R_MENU_REGISTER ret;
  GLOBAL_APPL *   globals = get_globals(apid);
 
  PUT_C_ALL(MENU_REGISTER, &par);

  par.register_apid = register_apid;
  strncpy (par.title, title, sizeof (par.title) - 1);
  par.title[sizeof (par.title) - 1] = 0;

  strcpy(globals->application_name, par.title);

  CLIENT_SEND_RECV(&par,
                   sizeof (C_MENU_REGISTER),
                   &ret,
                   sizeof (R_MENU_REGISTER));

  return ret.common.retval;
}

/*
** Exported
** 0x0023 menu_register ()
*/
void
Menu_register(AES_PB * apb)
{
  CHECK_APID(apb->global->apid);

  apb->int_out[0] = Menu_do_register(apb->global->apid,
                                     apb->int_in[0],
                                     (BYTE *)apb->addr_in[0]);
}



static
int
wait_for_selection(int      apid,
                   OBJECT * mn_tree,
                   int    * sel_obj,
                   RECT *   clip)
{
  EVENTIN       ei =
  {
    MU_BUTTON | MU_M1,
    1,           /* One click                         */
    LEFT_BUTTON, /* Only check left button            */
    LEFT_BUTTON, /* Return when the button is pressed */
    MO_LEAVE,
    {0,0,0,0},
    0,
    {0,0,0,0},
    0,
    0
  };
  COMMSG        buffer;
  EVENTOUT      eo;
  WORD          xy[2];
  GLOBAL_APPL * globals = get_globals(apid);
  int           new_sel;

  while(TRUE)
  {
    Objc_do_offset(mn_tree, *sel_obj, xy);

    ei.m1r.x = xy[0];
    ei.m1r.y = xy[1];
    ei.m1r.width = OB_WIDTH(&mn_tree[*sel_obj]);
    ei.m1r.height = OB_HEIGHT(&mn_tree[*sel_obj]);

    Evnt_do_multi(apid, &ei, &buffer, &eo, 0, DONT_HANDLE_MENU_BAR);

    if(eo.events & MU_BUTTON)
    {
      new_sel = *sel_obj;

      Objc_do_change(globals->vid,
                     mn_tree,
                     *sel_obj,
                     clip,
                     (~SELECTED) & OB_STATE(&mn_tree[*sel_obj]),
                     NO_DRAW);
      break;
    }

    if(eo.events & MU_M1)
    {
      new_sel = Objc_do_find(mn_tree, 0, 9, eo.mx, eo.my, 0);
      
      if(new_sel == -1)
      {
        break;
      }
      
      Objc_do_change(globals->vid,
                     mn_tree,
                     *sel_obj,
                     clip,
                     (~SELECTED) & OB_STATE(&mn_tree[*sel_obj]),
                     REDRAW);
      
      *sel_obj = new_sel;
      
      Objc_do_change(globals->vid,
                     mn_tree,
                     *sel_obj,
                     clip,
                     SELECTED | OB_STATE(&mn_tree[*sel_obj]),
                     REDRAW);

      eo.events &= ~MU_M1;
    }

    if(eo.events)
    {
      DEBUG0("menu.c: wait_for_selection: Unhandled event: %x!",
             eo.events);
    }
  }

  return new_sel;
}


/*
** Description
** Implementation of menu_popup()
*/
static
int
Menu_do_popup(int    apid,
              MENU * menu,
              int    xpos,
              int    ypos,
              MENU * mdata)
{
  WORD          xy[2];
  RECT          clip;
  GLOBAL_APPL * globals = get_globals(apid);
  int           sel_obj;
  int           retval;

  /* First we calculate the current coordinates of the "hot" object... */
  Objc_do_offset(menu->mn_tree, menu->mn_item, xy);

  /* ... then we adjust the position of the root object, ... */
  OB_X_PUT(&menu->mn_tree[0], OB_X(&menu->mn_tree[0]) + xpos - xy[0]);
  OB_Y_PUT(&menu->mn_tree[0], OB_Y(&menu->mn_tree[0]) + ypos - xy[1]);

  /* ... calculate the area we need ... */
  Objc_area_needed(menu->mn_tree, 0, &clip);

  /* ... and set the default item */
  sel_obj = menu->mn_item;

  Objc_do_change(globals->vid,
                 menu->mn_tree,
                 sel_obj,
                 &clip,
                 SELECTED | OB_STATE(&menu->mn_tree[sel_obj]),
                 NO_DRAW);

  /*Lock screen and get exclusive mouse control */
  /* FIXME: make sure we don't lock anything up */
  Wind_do_update(apid, BEG_MCTRL);
  Wind_do_update(apid, BEG_UPDATE);
  
  /* Reserve the area we need... */
  Form_do_dial(apid, FMD_START, &clip, &clip);

  /* ... and draw the popup */
  Objc_do_draw(globals->vid, menu->mn_tree, 0, 9, &clip);

  if(wait_for_selection(apid, menu->mn_tree, &sel_obj, &clip) != -1)
  {
    mdata->mn_tree     = menu->mn_tree;
    mdata->mn_menu     = menu->mn_menu;
    mdata->mn_item     = sel_obj;
    mdata->mn_scroll   = menu->mn_scroll;
    mdata->mn_keystate = menu->mn_keystate;

    retval = 1;
  }
  else
  {
    retval = 0;
  }

  /* Release the screen area */
  Form_do_dial(apid, FMD_FINISH, &clip, &clip);

  /* Release screen and mouse */
  Wind_do_update(apid, END_UPDATE);
  Wind_do_update(apid, END_MCTRL);

  return retval;
}


/*
** Description
** 0x0024 menu_popup
*/
void
Menu_popup(AES_PB * apb)
{
  CHECK_APID(apb->global->apid);

  apb->int_out[0] = Menu_do_popup(apb->global->apid,
                                  (MENU *)apb->addr_in[0],
                                  apb->int_in[0],
                                  apb->int_in[1],
                                  (MENU *)apb->addr_in[1]);
}
