/*
** srv_menu.c
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
#include <string.h>

#include "srv_appl.h"
#include "srv_appl_info.h"
#include "srv_global.h"
#include "srv_interface.h"
#include "srv_menu.h"
#include "srv_wind.h"

/*
** Description
** Get the owner of the topped menu
*/
WORD
get_top_menu_owner(void)
{
  AP_INFO * ai = search_appl_info (TOP_MENU_OWNER);

  if(ai == NULL)
  {
    return -1;
  }
  else
  {
    return ai->id;
  }
}


/*
** Description
** Set the resource tree of the menu of an application
** FIXME: Just set true or false instead
*/
static
WORD
set_menu(WORD        apid,
         SRV_FEATURE state)
{
  if(apps[apid].id != -1)
  {
    apps[apid].menu = state;
    return 0;
  }
  else
  {
    return -1;
  }
}


/*
** Description
** Remove menu entry of application
*/
void
unregister_menu(WORD apid)
{
  AP_LIST **mwalk;	
  mwalk = &globals.applmenu;
  
  while(*mwalk)
  {
    if((*mwalk)->ai->id == apid) {
      *mwalk = (*mwalk)->mn_next;
      break;
    }
    mwalk = &(*mwalk)->mn_next;
  }
  mwalk = &globals.accmenu;
  
  while(*mwalk)
  {
    if((*mwalk)->ai->id == apid) {
      *mwalk = (*mwalk)->mn_next;
      break;
    }
    mwalk = &(*mwalk)->mn_next;
  }
}


/*
** Description
** Redraw the menu bar
*/
void
redraw_menu_bar(void)
{
  WINSTRUCT * win = find_wind_description (MENU_BAR_WINDOW);
  
  if (win != NULL)
  {
    C_APPL_WRITE c_appl_write;
    R_APPL_WRITE r_appl_write;
    REDRAWSTRUCT m;

    m.type = WM_REDRAW;
    m.sid = 0;
    m.length = 0;
    m.wid = MENU_BAR_WINDOW;
    m.area = win->totsize;
      
    c_appl_write.addressee = get_top_menu_owner ();
    c_appl_write.length = MSG_LENGTH;
    c_appl_write.is_reference = TRUE;
    c_appl_write.msg.ref = &m;
    srv_appl_write (&c_appl_write, &r_appl_write);
  }
}


/*
** Description
** Creates and updates the menu list entries for the specified menu. 
*/
static
WORD
menu_bar_install(WORD capid)
{
  set_menu(capid, INSTALLED);
  
  if (get_top_menu_owner () == capid)
  {
    redraw_menu_bar();
  }

  return 1;
}


/*
** Description
** Remove menu
*/
static
WORD
menu_bar_remove(WORD apid)
{
  set_menu(apid, NOT_INSTALLED);
  
  redraw_menu_bar();
  
  return 1;
}


/*
** Description
** Server part of 0x001e menu_bar ()
*/
void
srv_menu_bar(C_MENU_BAR * msg,
             R_MENU_BAR * ret)
{
  WORD retval;

  switch (msg->mode)
  {
  case MENU_INSTALL: 
    retval = menu_bar_install(msg->common.apid);
    break;

  case MENU_REMOVE:
    retval = menu_bar_remove (msg->common.apid);
    break;
    
  case MENU_INQUIRE:
    retval = get_top_menu_owner();
    break;

  default:
    retval = 0;
  }

  PUT_R_ALL(MENU_BAR, ret, retval);
}


/*
** Description
** Implementation of menu_register ()
*/
void
srv_menu_register (C_MENU_REGISTER * par,
                   R_MENU_REGISTER * ret)
{
  AP_LIST **mwalk;
  AP_LIST *ap;
  WORD    n_menu = par->register_apid;
  WORD    retval;
  
  ap = search_apid(par->register_apid);
  
  if (ap == AP_LIST_REF_NIL)
  {
    retval = -1;
  }
  else
  {
    /* if the menu have been registered then unlink it again */
    if (ap->ai->type & APP_ACCESSORY)
    {
      mwalk = &globals.accmenu;
    }
    else
    {
      mwalk = &globals.applmenu;
    }
    
    while(*mwalk)
    {
      if(*mwalk == ap)
      {
        *mwalk = (*mwalk)->mn_next;
        break;
      }
      mwalk = &(*mwalk)->mn_next;
    }

    /* now find a new position to enter the menu */	
    if (ap->ai->type & APP_ACCESSORY)
    {
      mwalk = &globals.accmenu;
    }
    else
    {
      mwalk = &globals.applmenu;
    }
    
    while (*mwalk)
    {	
      if (strcasecmp((*mwalk)->ai->name, par->title) > 0)
      {
        break;
      }
      mwalk = &(*mwalk)->mn_next;
    }

    /* insert the menu */	
    ap->mn_next = *mwalk;
    *mwalk = ap;
    strncpy (ap->ai->name, par->title, 20);	
    
    retval = n_menu;
  }

  PUT_R_ALL(MENU_REGISTER, ret, retval);
}
