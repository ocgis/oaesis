/*
** srv_appl.c
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

#include <mintbind.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "srv_appl.h"
#include "srv_appl_info.h"
#include "srv_event.h"
#include "srv_global.h"
#include "srv_interface.h"
#include "srv_menu.h"
#include "srv_wind.h"


/* appl_* related */
static WORD	next_ap = 0;
static AP_LIST *ap_resvd = NULL;


/*
** Description
** Place application on top
*/
static
WORD
top_appl (WORD apid)
{
  AP_LIST ** al;
  OBJECT  *  deskbg = NULL;
  WORD       deskbgcount = 0;
  WORD       lasttop;

  DEBUG2 ("srv.c: top_appl: apid = %d", apid);
  lasttop = ap_pri->ai->id;

  if (lasttop != apid)
  {
    WINLIST * wl;

    /* Find the first window of the application and top it */
    wl = win_list;

    while (wl)
    {
      if (wl->win->owner == apid)
      {
        top_window (wl->win->id);
        break;
      }

      wl = wl->next;
    }

    al = &ap_pri;
    
    while(*al)
    {
      if ((*al)->ai->id == apid)
      {
        AP_LIST *tmp = *al;
        
        *al = (*al)->next;
        
        tmp->next = ap_pri;
        ap_pri = tmp;
        
        deskbg = tmp->ai->deskbg;
        
        break;
      }
      
      if((*al)->ai->deskbg)
      {
        deskbgcount++;
      }
      
      al = &(*al)->next;
    }
    
    if(deskbg && deskbgcount)
    {
      /*
      ** The desktop owner only needs to be updated if there are other
      ** applications with desktop background and one of them were
      ** in top before this call. The same goes for the background.
      */
      update_desktop_owner();
      update_desktop_background();
    }
    
    redraw_menu_bar();
  }

  return lasttop;
}


/*
** Description
** Server part of srv_appl_control
*/
void
srv_appl_control(C_APPL_CONTROL * msg,
                 R_APPL_CONTROL * ret)
{
  WORD retval;

  switch(msg->mode) {
  case APC_TOPNEXT:
    {
      AP_LIST *al = ap_pri;
      WORD    nexttop = -1;
      
      while(al) {
	nexttop = al->ai->id;
	
	al = al->next;
      }
      
      if(nexttop != -1) {
	top_appl(nexttop);
      }
      
      retval = 1;
    }
    break;
    
  case APC_KILL:
    {
      AP_INFO *ai = search_appl_info (msg->ap_id);
      
      if((ai->newmsg & 0x1) && (ai->killtries < 20)) {
	COMMSG       m;
	C_APPL_WRITE c_appl_write;
        R_APPL_WRITE r_appl_write;
	
	ai->killtries++;

	DB_printf("Sending AP_TERM to %d", msg->ap_id);
	
	m.type = AP_TERM;
	m.sid = 0;
	m.length = 0;
	m.msg2 = AP_TERM;
	
	c_appl_write.addressee = msg->ap_id;
	c_appl_write.length = MSG_LENGTH;
        c_appl_write.is_reference = TRUE;
	c_appl_write.msg.ref = &m;
	srv_appl_write (&c_appl_write, &r_appl_write);
      } else {
        C_APPL_EXIT par;
        R_APPL_EXIT ret;

	DB_printf("Killing apid %d", msg->ap_id);
	
	(void)Pkill(ai->pid,SIGKILL);

	par.common.apid = msg->ap_id;
	srv_appl_exit (&par, &ret);
      }
      
      retval = 1;
    }
    break;
    
  case APC_TOP:
    top_appl (msg->ap_id);
    retval = 1;
    break;

  default:
    DB_printf("srv_appl_control doesn't support mode %d", msg->mode);
    retval = 0;
  }

  PUT_R_ALL(APPL_CONTROL, ret, retval);
}

/*
** Description
** Implementation of appl_exit ()
*/
void
srv_appl_exit (C_APPL_EXIT * par,
               R_APPL_EXIT * ret)
{
  /*C_WIND_SET cws = {0,WF_NEWDESK,0,0,0,0};*/
  C_MENU_BAR c_menu_bar;
  R_MENU_BAR r_menu_bar;

  /*clean up*/

  c_menu_bar.common.apid = par->common.apid;
  c_menu_bar.tree = NULL;
  c_menu_bar.mode = MENU_REMOVE;
  srv_menu_bar (&c_menu_bar,
                &r_menu_bar);
  unregister_menu(par->common.apid);
  /*  srv_wind_set(par->common.apid,&cws);*/
  srv_wind_new(par->common.apid);
  apinfofree(par->common.apid);

  PUT_R_ALL(APPL_EXIT, ret, 1);
}


/*
** Description
** Implementation of appl_find()
*/
void
srv_appl_find(C_APPL_FIND * msg,
              R_APPL_FIND * ret)
{
  AP_LIST * al;
  LONG      w;
  _DTA    * olddta,newdta;
  BYTE      pname[30];
  WORD      retval;

  switch (msg->mode) {
  case APPL_FIND_PID_TO_APID :
    /* convert MiNT to AES pid */
    al = search_mpid (msg->data.pid);
    
    if (al) {
      retval = al->ai->id;
    } else {
      retval = -1;
    }
    break;

  case APPL_FIND_APID_TO_PID :
    /* convert from AES to MINT pid */
    al = search_apid (msg->data.apid);
    if(al) {
      retval = al->ai->pid;
    } else {
      retval = -1;
    }
    break;

  default:
    /* FIXME: This mode needs to be overhauled */
    /* Now find the pid of process with the passed name */
    olddta = Fgetdta();
    
    Fsetdta( &newdta);
    w = -1;
    sprintf (pname, "u:\\proc\\%s.*", msg->data.name);
    
    if(Fsfirst(pname, 0) == 0) {
      w = atoi(&newdta.dta_name[9]);
    }
    
    Fsetdta(olddta);
    
    /* map the MINT-pid to aes */
    if(w != -1) {
      al = search_mpid((WORD)w);
      
      if(al) {
        retval = al->ai->id;
      } else {
        retval = -1;
      }
    } else {
      retval = -1;
    }
  }

  PUT_R_ALL(APPL_FIND, ret, retval);
}


/*
** Description
** Reserve structure of internal application information
*/
static
AP_INFO *
srv_info_alloc(WORD pid,
               WORD type,
               WORD alloc_only)
{
  AP_LIST	*al;
   
  DEBUG2 ("srv.c: srv_info_alloc: Allocating memory");
  al = (AP_LIST *)malloc(sizeof(AP_LIST));
  
  if(!al) {
    DB_printf("%s: Line %d: srv_info_alloc:\r\n"
	      "out of memory!\r\n",__FILE__,__LINE__);
    return NULL;
  };
  
  while(apps[next_ap].id != -1) {
    next_ap = ((next_ap + 1) % MAX_NUM_APPS);
  };
  
  al->ai = &apps[next_ap];
  
  al->ai->id = next_ap;
  al->ai->pid = pid;
  al->ai->deskbg = NULL;
  al->ai->menu = NULL;
  al->ai->type = type;
  al->ai->newmsg = 0;
  al->ai->killtries = 0;
 
  al->ai->rshdr = 0L;

  /* Message handling initialization */
  al->ai->message_head = 0;
  al->ai->message_size = 0;
  
  if(!alloc_only)
  {
    al->next = ap_pri;
    ap_pri = al;
  }
  else
  {
    al->next = ap_resvd;
    ap_resvd = al;
  }
  
  return al->ai;
}


/*
** Description
** Reserve an application id
*/
void
srv_appl_reserve(C_APPL_RESERVE * par,
                 R_APPL_RESERVE * ret)
{
  AP_INFO_REF ai;
  WORD        ap_id;

  ai = srv_info_alloc(par->pid,
                      par->type,
                      TRUE);

  if(ai == AP_INFO_REF_NIL)
  {
    ap_id = 0;
  }
  else
  {
    ap_id = ai->id;
  }
  
  PUT_R_ALL(APPL_RESERVE, ret, ap_id);
}


/*
** Description
** appl_init help call
*/
void
srv_appl_init(C_APPL_INIT * par,
              R_APPL_INIT * ret)
{
  AP_INFO * ai;
  AP_LIST * al;
  
  DEBUG2 ("oaesis: srv.c: srv_appl_init: Beginning");

  al = search_mpid(par->common.pid);
  
  if (al == AP_LIST_REF_NIL) {     
    /* Has an info structure already been reserved? */
    
    AP_LIST **awalk = &ap_resvd;
    
    while(*awalk) {
      if((*awalk)->ai->pid == par->common.pid)
      {
	break;
      }
      
      awalk = &(*awalk)->next;
    }
    
    if(*awalk)
    {
      al = *awalk;
      *awalk = al->next;
      
      al->next = ap_pri;
      ap_pri = al;
      
      ai = al->ai;
    }
    else
    {
      ai = srv_info_alloc (par->common.pid, APP_APPLICATION, 0);
    }
  }
  else
  {
    ai = al->ai;
  }
  
  if(ai)
  {
    ret->apid = ai->id;

    DEBUG2 ("oaesis: srv.c: srv_appl_init: apid=%d",
	    (int)ret->apid);
  }
  else
  {
    ret->apid = -1;
  }

  ret->physical_vdi_id = globals.vid;

  if ((ret->apid >= 0) && (ai->type & APP_APPLICATION))
  {
    C_MENU_REGISTER c_menu_register;
    R_MENU_REGISTER r_menu_register;

    c_menu_register.common = par->common;
    c_menu_register.common.apid = ai->id;
    sprintf (c_menu_register.title, "  %s", par->appl_name);
    
    srv_menu_register (&c_menu_register, &r_menu_register);
  }

  PUT_R_ALL(APPL_INIT, ret, 0);
}


/*
** Description
** Implementation of appl_search ()
*/
void
srv_appl_search(C_APPL_SEARCH * msg,
                R_APPL_SEARCH * ret)
{
  AP_LIST * this;
  AP_LIST * p;
  WORD      retval;
  
  this = search_apid (msg->common.apid);
  
  if (this == AP_LIST_REF_NIL) {
    retval = 0;
  } else {
    switch (msg->mode) {
    case APP_FIRST:
      this->ai->ap_search_next = ap_pri;
      /* there will always have atleast ourself to return */

    case APP_NEXT:
      p = this->ai->ap_search_next;
      
      if (p == AP_LIST_REF_NIL) {
        retval = 0;
      } else {
        strncpy (ret->info.name, p->ai->name, 18); /* the 'pretty name' */
        
        ret->info.type =  p->ai->type;           /* sys/app/acc */
        
        ret->info.ap_id = p->ai->id;
        
        /* get the next... */	
        this->ai->ap_search_next = p->next;
        
        retval = p->next ? 1: 0;
      }
      break;

    case 2:        /* search system shell (??) */
      DB_printf("srv_appl_search(2,...) not implemented yet.\r\n");
      retval = 0;
      break;
      
    default:
      DB_printf("%s: Line %d: srv_appl_search\r\n"
                "Unknown mode %d",__FILE__,__LINE__,msg->mode);
      retval = 0;
    }
  }

  PUT_R_ALL(APPL_SEARCH, ret, retval);
}


/*
** Description
**  Implementation of appl_write()
*/
void
srv_appl_write (C_APPL_WRITE * msg,
                R_APPL_WRITE * ret)
{
  AP_INFO * ai;
  WORD      retval;
  
  ai = search_appl_info (msg->addressee);
  
  if (ai != NULL) {
    if (ai->message_size <= (MSG_BUFFER_SIZE - MSG_LENGTH)) {
      if (msg->is_reference) {
        memcpy (&ai->message_buffer[(ai->message_head +
                                     ai->message_size)
                                   % MSG_BUFFER_SIZE],
                msg->msg.ref,
                msg->length);
      } else {
        memcpy (&ai->message_buffer[(ai->message_head +
                                     ai->message_size)
                                   % MSG_BUFFER_SIZE],
                &msg->msg.event,
                msg->length);
      }
      
      ai->message_size += msg->length;
      
      retval = 0;

      srv_wake_appl_if_waiting_for_msg (msg->addressee);
    } else {
      retval = 1;
    }
  } else {
    DB_printf ("srv.c: srv_appl_write: couldn't find appl info");
    retval = 1;
  }

  PUT_R_ALL(APPL_WRITE, ret, retval);
}


/*
** Description
** Get the id of the application that owns the desktop
*/
WORD
get_desktop_owner_id (void)
{
  AP_INFO_REF ai;
  
  ai = search_appl_info (DESK_OWNER);
  
  if (ai != AP_INFO_REF_NIL)
  {
    return ai->id;
  }

  /* The default is the first application */
  return 0;
}


/*
** Description
** Update all of the desk background
*/
void
update_desktop_background (void)
{
  C_APPL_WRITE c_appl_write;
  R_APPL_WRITE r_appl_write;
  
  c_appl_write.msg.redraw.type = WM_REDRAW;
  
  c_appl_write.msg.redraw.sid = 0;
 
  c_appl_write.msg.redraw.length = 0;
  c_appl_write.msg.redraw.wid = DESKTOP_WINDOW;
  
  c_appl_write.msg.redraw.area.x = globals.screen.x;
  c_appl_write.msg.redraw.area.y = globals.screen.y;
  c_appl_write.msg.redraw.area.width = globals.screen.width;
  c_appl_write.msg.redraw.area.height = globals.screen.height;
  
  c_appl_write.addressee = get_desktop_owner_id ();
  c_appl_write.length = MSG_LENGTH;
  c_appl_write.is_reference = FALSE;
  srv_appl_write (&c_appl_write, &r_appl_write);
}
