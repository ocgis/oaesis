/*
** srv.c
**
** Copyright 1996 - 1999 Christer Gustavsson <cg@nocrew.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**  
** Read the file COPYING for more information.
*/

#define DEBUGLEVEL 0

/****************************************************************************
 * Used interfaces                                                          *
 ****************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_BASEPAGE_H
#include <basepage.h>
#endif

#ifdef HAVE_MINTBIND_H
#include <mintbind.h>
#endif

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <vdibind.h>

#include "aesbind.h"
#include "oconfig.h"
#include "debug.h"
#include "lxgemdos.h"
#include "mesagdef.h"
#include "mintdefs.h"
#include "srv_appl_info.h"
#include "srv_comm.h"
#include "srv_event.h"
#include "srv_global.h"
#include "srv_misc.h"
#include "resource.h"
#include "rlist.h"
#include "srv.h"
#include "srv_event.h"
#include "srv_get.h"
#include "srv_interface.h"
#include "srv_wind.h"
#include "types.h"


/****************************************************************************
 * Macros                                                                   *
 ****************************************************************************/


#define SRV_QUE_SIZE 32

#define IMOVER 0x8000  /* Used with set_win_elem() to make icon window */

#define D3DSIZE       2

#define INTS2LONG(a,b) (((((LONG)a)<<16)&0xffff0000L)|(((LONG)b)&0xffff))
#define MSW(a) ((unsigned short)((unsigned long)(a) >> 16))
#define LSW(a) ((unsigned short)((unsigned long)(a) & 0xffffUL))

/****************************************************************************
 * Module global variables                                                  *
 ****************************************************************************/

static WORD nocnf;

/* appl_* related */

static WORD	next_ap = 0;

static AP_LIST *ap_resvd = NULL;

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

/* When this is set to false the server will exit */
static volatile WORD run_server = TRUE;

/* This has to be fixed */
#ifndef MINT_TARGET
void accstart (void) {}
#endif

static
void
srv_appl_exit (C_APPL_EXIT * par,
               R_APPL_EXIT * ret);

void
srv_appl_write (C_APPL_WRITE * msg,
                R_APPL_WRITE * ret);

void
Srv_exit_module (void);

/****************************************************************************
 * srv_wind_new                                                             *
 *  Implementation of wind_new().                                           *
 ****************************************************************************/
static WORD    /* 0 if error or 1 if ok.                                    */
srv_wind_new(  /*                                                           */
WORD apid);    /* Application whose windows should be erased.               */
/****************************************************************************/

static
void
srv_wind_set (C_WIND_SET * msg,
              R_WIND_SET * ret);

/****************************************************************************
 * draw_wind_elements                                                       *
 *  Draw the elements of the window win that intersects with the rectangle  *
 *  r.                                                                      *
 ****************************************************************************/
static void          /*                                                     */
draw_wind_elements(  /*                                                     */
WINSTRUCT *win,      /* Window description.                                 */
RECT *r,             /* Clipping rectangle.                                 */
WORD start);         /* Start object.                                       */
/****************************************************************************/

/****************************************************************************
 * find_wind_description                                                    *
 *  Find the window structure of the window with identification number id.  *
 ****************************************************************************/
WINSTRUCT	*     /* Found description or NULL.                       */
find_wind_description(  /*                                                  */
WORD id);               /* Identification number of window.                 */
/****************************************************************************/

/****************************************************************************
 * top_appl                                                                 *
 *  Top application.                                                        *
 ****************************************************************************/
static WORD       /* Previously topped application.                         */
top_appl(         /*                                                        */
WORD apid);       /* Id of application.                                     */
/****************************************************************************/

static
WORD
top_window (WORD winid);

/* 
 * Find MiNT-PID & return AP_LIST entry for that 
 */
static AP_LIST *search_mpid(WORD pid)
{
	AP_LIST	*al;

	al = ap_pri;
	
	while(al) 
	{
		if( al->ai->pid == pid) 
			break;
		al = al->next;
	}
	return al;	
}

/****************************************************************************
 * srv_info_alloc                                                           *
 *  Reserve structure of internal application information.                  *
 ****************************************************************************/
static AP_INFO *    /* Application description or NULL.                     */
srv_info_alloc(     /*                                                      */
WORD pid,           /* MiNT process id.                                     */
WORD type,          /* Type of application (appl, acc, desktop etc)         */
WORD alloc_only)    /* Should the structure only be allocated?              */
/****************************************************************************/
{
  AP_LIST	*al;
   
  DEBUG2 ("srv.c: srv_info_alloc: Allocating memory");
  al = (AP_LIST *)Mxalloc(sizeof(AP_LIST),GLOBALMEM);
  
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
  
  if(!alloc_only) {
    al->next = ap_pri;
    ap_pri = al;
  }
  else {
    al->next = ap_resvd;
    ap_resvd = al;
  };
  
  return al->ai;
}

static void apinfofree(WORD id) {
	AP_LIST	**al;
	
	al = &ap_pri;
	
	while(*al) {
		if((*al)->ai->id == id) {
			AP_LIST	*altemp = *al;
			
			*al = (*al)->next;
	
			altemp->ai->vid = -1;
			altemp->ai->msgpipe = -1;
			altemp->ai->eventpipe = -1;
			
			if(altemp->ai->rshdr) {
				Mfree(altemp->ai->rshdr);	/*free resource memory*/
			};

			altemp->ai->id = -1;
			/* FIXME Mfree(altemp);*/

			break;
		};
		al = &(*al)->next;
	};
}

/****************************************************************************
 * search_apid                                                              *
 *  Find AES-id & return AP_LIST entry for that.                            *
 ****************************************************************************/
AP_LIST *         /* Entry or NULL.                                         */
search_apid(      /*                                                        */
WORD apid)        /* Application id of the searched application.            */
/****************************************************************************/
{
	AP_LIST	*al;

	al = ap_pri;
	
	while(al) {
		if(al->ai->id == apid) 
			break;
		al = al->next;
	};
	
	return al;	
}

/*********************************************************************8‡`***
 * get_top_appl                                                             *
 *  Get currently topped application.                                       *
 ****************************************************************************/
WORD               /* Id of topped application.                             */
get_top_appl(void) /*                                                       */
/****************************************************************************/
{
	if(ap_pri) {
		return ap_pri->ai->id;
	}
	else {
		return -1;
	};
}


/****************************************************************************
 * srv_get_appl_info                                                        *
 ****************************************************************************/
WORD                       /* 0 if ok or -1.                                */
srv_get_appl_info(         /*                                               */
WORD apid,                 /* Application id.                               */
SRV_APPL_INFO *appl_info)  /* Returned info.                                */
/****************************************************************************/
{
  AP_INFO *ai;
  WORD    retval = 0;

  ai = search_appl_info(apid);			

  if(ai) {
    appl_info->eventpipe = ai->eventpipe;
    appl_info->msgpipe = ai->msgpipe;
    appl_info->vid = ai->vid;
  }
  else
  {
    DB_printf(
              "%s: Line %d: Couldn't find description of application %d\r\n",
              __FILE__,__LINE__,apid);

    retval = -1;
  };

  return retval;
}


/****************************************************************************
 * set_menu                                                                 *
 *  Set the resource tree of the menu of an application                     *
 ****************************************************************************/
static WORD       /* 0 if ok or -1.                                         */
set_menu(         /*                                                        */
WORD apid,        /* Id of application.                                     */
OBJECT *tree)     /* Resource tree.                                         */
/****************************************************************************/
{
  if(apps[apid].id != -1) {
    apps[apid].menu = tree;
    return 0;
  }
  
  return -1;
}


/*
** Description
** Get the owner of the topped menu
**
** 1999-03-15 CHGU
*/
WORD
get_top_menu_owner (void) {
  AP_INFO * ai = search_appl_info (TOP_MENU_OWNER);

  if (ai == NULL) {
    return -1;
  } else {
    return ai->id;
  }
}


/****************************************************************************
 * unregister_menu                                                          *
 *  Remove menu entry of application.                                       *
 ****************************************************************************/
static void       /*                                                        */
unregister_menu(  /*                                                        */
WORD apid)        /* Application id.                                        */
/****************************************************************************/
{
  AP_LIST **mwalk;	
  mwalk = &globals.applmenu;
  
  while(*mwalk) {
    if((*mwalk)->ai->id == apid) {
      *mwalk = (*mwalk)->mn_next;
      break;
    };
    mwalk = &(*mwalk)->mn_next;
  };
  mwalk = &globals.accmenu;
  
  while(*mwalk) {
    if((*mwalk)->ai->id == apid) {
      *mwalk = (*mwalk)->mn_next;
      break;
    };
    mwalk = &(*mwalk)->mn_next;
  };
}


/*
** Description
** Redraw the menu bar
**
** 1999-01-09 CG
** 1999-03-21 CG
*/
static
void
redraw_menu_bar (void) {
  WINSTRUCT * win = find_wind_description (MENU_BAR_WINDOW);
  
  if (win != NULL) {
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
**
** 1999-01-09 CG
** 1999-03-21 CG
*/
static
WORD
menu_bar_install (OBJECT * tree,
                  WORD     capid) {
  set_menu (capid, tree);
  
  if (get_top_menu_owner () == capid) {
    redraw_menu_bar();
  }

  return 1;
}


/****************************************************************************
 * menu_bar_remove                                                          *
 *  Remove menu.                                                            *
 ****************************************************************************/
static WORD       /*                                                        */
menu_bar_remove(  /*                                                        */
WORD apid)        /* Application whose menu is to be removed.               */
/****************************************************************************/
{
	set_menu(apid,NULL);

	redraw_menu_bar();

	return 1;
}

/*
** Description
** Server part of 0x001e menu_bar ()
*/
static
void
srv_menu_bar (C_MENU_BAR * msg,
              R_MENU_BAR * ret) {
  WORD retval;

  switch (msg->mode) {
  case MENU_INSTALL: 
    retval = menu_bar_install (msg->tree, msg->common.apid);
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
                   R_MENU_REGISTER * ret) {
  AP_LIST **mwalk;
  AP_LIST *ap;
  WORD    n_menu = par->common.apid;
  WORD    retval;
  
  ap = search_apid (par->common.apid);
  
  if (ap == AP_LIST_REF_NIL) {
    retval = -1;
  } else {
    /* if the menu have been registered then unlink it again */
    if (ap->ai->type & APP_ACCESSORY) {
      mwalk = &globals.accmenu;
    } else {
      mwalk = &globals.applmenu;
    }
    
    while(*mwalk) {
      if(*mwalk == ap) {
        *mwalk = (*mwalk)->mn_next;
        break;
      }
      mwalk = &(*mwalk)->mn_next;
    }

    /* now find a new position to enter the menu */	
    if (ap->ai->type & APP_ACCESSORY) {
      mwalk = &globals.accmenu;
    } else {
      mwalk = &globals.applmenu;
    }
    
    while (*mwalk) {	
      if (strcasecmp((*mwalk)->ai->name, par->title) > 0) {
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


/*
** Description
** Server part of srv_appl_control
*/
static
void
srv_appl_control (C_APPL_CONTROL * msg,
                  R_APPL_CONTROL * ret) {
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
static
void
srv_appl_exit (C_APPL_EXIT * par,
               R_APPL_EXIT * ret) {
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
** Implementation of appl_find ()
*/
static
void
srv_appl_find (C_APPL_FIND * msg,
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
** appl_init help call
*/
static void
srv_appl_init(C_APPL_INIT * par,
              R_APPL_INIT * ret) {
  AP_INFO * ai;
  AP_LIST * al;
  
  DEBUG2 ("oaesis: srv.c: srv_appl_init: Beginning");

  al = search_mpid(par->common.pid);
  
  if (al == AP_LIST_REF_NIL) {     
    /* Has an info structure already been reserved? */
    
    AP_LIST **awalk = &ap_resvd;
    
    while(*awalk) {
      if((*awalk)->ai->pid == par->common.pid) {
	break;
      };
      
      awalk = &(*awalk)->next;
    }
    
    if(*awalk) {
      al = *awalk;
      *awalk = al->next;
      
      al->next = ap_pri;
      ap_pri = al;
      
      ai = al->ai;
    } else {
      ai = srv_info_alloc (par->common.pid, APP_APPLICATION, 0);
    }
  } else {
    ai = al->ai;
  }
  
  if(ai) {
    ret->apid = ai->id;

    DEBUG2 ("oaesis: srv.c: srv_appl_init: apid=%d",
	    (int)ret->apid);
  } else {
    ret->apid = -1;
  }

  ret->physical_vdi_id = globals.vid;

  if ((ret->apid >= 0) && (ai->type & APP_APPLICATION)) {
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
srv_appl_search (C_APPL_SEARCH * msg,
                 R_APPL_SEARCH * ret) {
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
**  Implementation of appl_write().                                         *
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


/* Description
** Get the id of the application that owns the desktop
**
** 1999-01-01 CG
*/
static
WORD
get_desktop_owner_id (void) {
  AP_INFO_REF ai;
  
  ai = search_appl_info (DESK_OWNER);
  
  if (ai != AP_INFO_REF_NIL) {
    return ai->id;
  }

  /* The default is the first application */
  return 0;
}


/*
** Description
** Update all of the desk background
**
** 1999-01-01 CG
** 1999-01-10 CG
*/
void
update_desktop_background (void) {
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


/*
** Description
** Get the resource tree of the top desktop
**
** 1999-01-01 CG
*/
static
OBJECT *
get_deskbg (void) {
  OBJECT  *retval = NULL;
  AP_INFO *ai;
  
  ai = search_appl_info (DESK_OWNER);
  
  if(ai) {
    retval = ai->deskbg;
  }
		
  return retval;
}


/*
** Description
** Update the owner of the desktop window
**
** 1999-01-01 CG
*/
static
void
update_desktop_owner (void) {
  WINSTRUCT *win = find_wind_description (DESKTOP_WINDOW);
  
  if (win != NULL) {
    win->owner = get_desktop_owner_id ();
  }
}


/*
** Description
** Set the resource tree of the desktop of an application
**
** 1999-01-01 CG
*/
static
WORD
set_desktop_background (WORD     apid,
                        OBJECT * tree) {
  OBJECT * deskbg;
  OBJECT * olddeskbg = apps[apid].deskbg;
  
  if(apps[apid].id != -1) {
    apps[apid].deskbg = tree;
    
    deskbg = get_deskbg ();
    
    if(((deskbg == tree) && (deskbg != olddeskbg)) || !tree) {
      update_desktop_background ();
    }

    update_desktop_owner ();
    
    return 0;
  }

  return -1;
}


/*
** Description
** Place application on top
**
** 1999-01-01 CG
** 1999-04-13 CG
** 1999-04-18 CG
*/
static
WORD
top_appl (WORD apid) {
  AP_LIST ** al;
  OBJECT  *  deskbg = NULL;
  WORD       deskbgcount = 0;
  WORD       lasttop;

  DEBUG2 ("srv.c: top_appl: apid = %d", apid);
  lasttop = ap_pri->ai->id;

  if (lasttop != apid) {
    WINLIST * wl;

    /* Find the first window of the application and top it */
    wl = win_list;

    while (wl) {
      if (wl->win->owner == apid) {
        top_window (wl->win->id);
        break;
      }

      wl = wl->next;
    }

    al = &ap_pri;
    
    while (*al) {
      if ((*al)->ai->id == apid) {
        AP_LIST *tmp = *al;
        
        *al = (*al)->next;
        
        tmp->next = ap_pri;
        ap_pri = tmp;
        
        deskbg = tmp->ai->deskbg;
        
        break;
      }
      
      if ((*al)->ai->deskbg) {
        deskbgcount++;
      }
      
      al = &(*al)->next;
    }
    
    if(deskbg && deskbgcount) {
      update_desktop_background ();
    }
    
    redraw_menu_bar();
  }

  return lasttop;
}


/****************************************************************************
 * draw_wind_elements                                                       *
 *  Draw the elements of the window win that intersects with the rectangle  *
 *  r.                                                                      *
 ****************************************************************************/
static void          /*                                                     */
draw_wind_elements(  /*                                                     */
WINSTRUCT *win,      /* Window description.                                 */
RECT *r,             /* Clipping rectangle.                                 */
WORD start)          /* Start object.                                       */
/****************************************************************************/
{
  RLIST	*rl = win->rlist;

  if(win->id == 0) {
    OBJECT *deskbg;
	
    deskbg = get_deskbg();
		
    if(deskbg) {
      while(rl) {		
        RECT	r2;
			
        if(srv_intersect(&rl->r,r,&r2)) {
          /*
            Objc_do_draw(deskbg,start,9,&r2);
            */
        };
					
        rl = rl->next;
      };
    };
  }
  else if(win->tree) {	
    while(rl) {		
      RECT	r2;
		
      if(srv_intersect(&rl->r,r,&r2)) {
        /*
          Objc_do_draw(win->tree,start,3,&r2);
          */
      };
			
      rl = rl->next;
    };
  };
}

/****************************************************************************
 * draw_wind_elemfast                                                       *
 *  Draw the elements of the window win that intersects with the rectangle  *
 *  r.                                                                      *
 ****************************************************************************/
void                 /*                                                     */
draw_wind_elemfast(  /*                                                     */
WINSTRUCT *win,      /* Window description.                                 */
RECT *r,             /* Clipping rectangle.                                 */
WORD start)          /* Start object.                                       */
/****************************************************************************/
{
  if(win->id == 0) {
    OBJECT *deskbg;
	
    deskbg = get_deskbg();
		
    if(deskbg) {
      /*
      Objc_do_draw(deskbg,start,9,r);
      */
    };
  }
  else if(win->tree) {	
    /*
    Objc_do_draw(win->tree,start,3,r);
    */
  };
}


/*
** Description
** Change window slider position and size
**
** 1999-03-28 CG
*/
static
WORD
changeslider (WINSTRUCT * win,
              WORD        redraw,
              WORD        which,
              WORD        position,
              WORD        size) {	
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
**
** 1998-10-04 CG
*/
static
void
setwinsize (WINSTRUCT * win,
            RECT      * size) {
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

/****************************************************************************
 * find_wind_description                                                    *
 *  Find the window structure of the window with identification number id.  *
 ****************************************************************************/
WINSTRUCT	*     /* Found description or NULL.                       */
find_wind_description(  /*                                                  */
WORD id)                /* Identification number of window.                 */
/****************************************************************************/
{
	WINLIST	*wl;

	wl = win_list;
	
	while(wl) {
		if(wl->win->id == id)
		{
			return wl->win;
		}

		wl = wl->next;
	};

	return NULL;
}


/*
** Description
** Change the size of a window
**
** 1998-12-25 CG
** 1999-01-01 CG
** 1999-05-22 CG
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
** Description
** Put window under all other windows
**
** 1999-04-18 CG
*/
static
WORD
bottom_window (WORD winid) { 
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
      
      if((*wl)->win != newtop) {
	draw_wind_elements((*wl)->win,&m.area,0);
      };
      
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


/*
** Description
** Try to top window
**
** 1999-03-28 CG
** 1999-04-18 CG
*/
static
WORD
top_window (WORD winid) { 
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
** Implementation of wind_close ()
*/
static
void
srv_wind_close (C_WIND_CLOSE * par,
                R_WIND_CLOSE * ret) {
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
    Mfree(wp);
    
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


/****************************************************************************
 * srv_wind_create                                                          *
 *  Implementation of wind_create().                                        *
 ****************************************************************************/
void
srv_wind_create( /*                                                         */
C_WIND_CREATE * msg,
R_WIND_CREATE * ret)
/****************************************************************************/
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

/*
** Description
** Implementation of wind_delete()
*/
static
void
srv_wind_delete (C_WIND_DELETE * msg,
                 R_WIND_DELETE * ret) {
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
      
      Mfree(wt->win);
      Mfree(wt);
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
              
              Mfree(wltemp->win);
              Mfree(wltemp);
              
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
               R_WIND_OPEN * ret) {
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
      wl = (WINLIST *)Mxalloc(sizeof(WINLIST),GLOBALMEM);
      
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


/*wind_set 0x0069*/

/*
** Description
** Server part of wind_set ()
*/
static
void
srv_wind_set (C_WIND_SET * msg,
              R_WIND_SET * ret) {
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
      OBJECT *tree = (OBJECT *)INTS2LONG(msg->parm1, msg->parm2);
      
      if (set_desktop_background (msg->common.apid, tree) == 0) {
        retval = 1;
      } else {
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
      top_colour[msg->parm1] = *(OBJC_COLORWORD *)&msg->parm2;
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

/****************************************************************************
 * srv_wind_new                                                             *
 *  Implementation of wind_new().                                           *
 ****************************************************************************/
static WORD    /* 0 if error or 1 if ok.                                    */
srv_wind_new(  /*                                                           */
WORD apid)     /* Application whose windows should be erased.               */
/****************************************************************************/
{
  WINLIST *     wl;

  wl = win_vis;
  
  while(wl) {
    if((wl->win->owner == apid) && !(wl->win->status & WIN_DESKTOP)) {
      C_WIND_DELETE c_wind_delete;
      R_WIND_DELETE r_wind_delete;

      c_wind_delete.id = wl->win->id;
      srv_wind_delete(&c_wind_delete, &r_wind_delete);
      
      wl = win_vis;
    }
    else {
      wl = wl->next;
    };
  };
  
  wl = win_list;
  
  while(wl) {
    if(wl->win->owner == apid) {
      C_WIND_DELETE c_wind_delete;
      R_WIND_DELETE r_wind_delete;
      
      c_wind_delete.id = wl->win->id;
      srv_wind_delete(&c_wind_delete, &r_wind_delete);
      
      wl = win_list;
    }
    else {	
      wl = wl->next;
    };
  };
  
  return 1;
}


/* Description
** Copy MFDB with network order conversion
**
** 1999-05-23 CG
*/
static
void
srv_copy_mfdb (MFDB * dst,
               MFDB * src) {
  dst->fd_addr = (void *)ntohl ((long)src->fd_addr);
  dst->fd_w = ntohs (src->fd_w);
  dst->fd_h = ntohs (src->fd_h);
  dst->fd_wdwidth = ntohs (src->fd_wdwidth);
  dst->fd_stand = ntohs (src->fd_stand);
  dst->fd_nplanes = ntohs (src->fd_nplanes);
  dst->fd_r1 = ntohs (src->fd_r1);
  dst->fd_r2 = ntohs (src->fd_r2);
  dst->fd_r3 = ntohs (src->fd_r3);
}


/*
** Description
** Do a vdi call requested by a client
*/
static
void
srv_vdi_call (COMM_HANDLE  handle,
              C_VDI_CALL * par) {
  R_VDI_CALL       ret;
  static VDIPARBLK e_vdiparblk;
  static VDIPB     vpb = { e_vdiparblk.contrl,
                           e_vdiparblk.intin, e_vdiparblk.ptsin,
                           e_vdiparblk.intout, e_vdiparblk.ptsout };
  MFDB             mfdb_src;
  MFDB             mfdb_dst;
  int              i;
  int              j;
  int              retval = 0;

  /* Copy contrl array */
  for (i = 0; i < 15; i++) {
    vpb.contrl[i] = par->contrl[i];
  }
  
  DEBUG2 ("srv_vdi_call: Call no %d (0x%x)", vpb.contrl[0], vpb.contrl[0]);

  /* Copy ptsin array */
  for (i = 0, j = 0; i < (vpb.contrl[1] * 2); i++, j++) {
    vpb.ptsin[i] = par->inpar[j];
  }
  
  /* Copy intin array */
  for (i = 0; i < vpb.contrl[3]; i++, j++) {
    vpb.intin[i] = par->inpar[j];
  }

  /* Copy MFDBs when available */
  if ((vpb.contrl[0] == 109)  ||  /* vro_cpyfm */
      (vpb.contrl[0] == 110)  ||  /* vr_trnfm  */
      (vpb.contrl[0] == 121)) {   /* vrt_cpyfm */
    srv_copy_mfdb (&mfdb_src, (MFDB *)&par->inpar[j]);
    j += sizeof (MFDB) / 2;
    srv_copy_mfdb (&mfdb_dst, (MFDB *)&par->inpar[j]);
    vpb.contrl[7] = MSW(&mfdb_src);
    vpb.contrl[8] = LSW(&mfdb_src);
    vpb.contrl[9] = MSW(&mfdb_dst);
    vpb.contrl[10] = LSW(&mfdb_dst);
  }

  vdi_call (&vpb);
  
  /* Copy contrl array */
  for (i = 0; i < 15; i++) {
    ret.contrl[i] = vpb.contrl[i];
  }
  
  /* Copy ptsout array */
  for (i = 0, j = 0; i < (vpb.contrl[2] * 2); i++, j++) {
    ret.outpar[j] = vpb.ptsout[i];
  }
  
  /* Copy intout array */
  for (i = 0; i < vpb.contrl[4]; i++, j++) {
    ret.outpar[j] = vpb.intout[i];
  }

  PUT_R_ALL_W(VDI_CALL,&ret,R_ALL_WORDS + 15 + j);
  SRV_REPLY(handle,
            &ret,
            sizeof (R_ALL) +
            sizeof (WORD) * (15 + j));
}


/*
** Description
** Handle incoming server request
*/
static
inline
void
srv_call(COMM_HANDLE handle,
         C_SRV *     par)
{
  R_SRV ret;

  DEBUG2 ("srv.c: Call no %d 0x%x", par->common.call, par->common.call);
  switch (par->common.call) {
  case SRV_APPL_CONTROL:
    srv_appl_control (&par->appl_control, &ret.appl_control);
    
    SRV_REPLY(handle, &ret, sizeof (R_APPL_CONTROL));
    break;
    
  case SRV_APPL_EXIT:
    srv_appl_exit (&par->appl_exit, &ret.appl_exit);
    
    SRV_REPLY(handle, &ret, sizeof (R_APPL_EXIT));
    break;
    
  case SRV_APPL_FIND:
    srv_appl_find (&par->appl_find, &ret.appl_find);
    
    SRV_REPLY(handle, &ret, sizeof (R_APPL_FIND));
    break;
    
  case SRV_APPL_INIT:
    srv_appl_init (&par->appl_init, &ret.appl_init);
    
    SRV_REPLY(handle, &ret, sizeof (R_APPL_INIT));
    break;
        
  case SRV_APPL_SEARCH:
    srv_appl_search (&par->appl_search, &ret.appl_search);
    
    SRV_REPLY(handle, &ret, sizeof (R_APPL_SEARCH));
    break;
    
  case SRV_APPL_WRITE:
    srv_appl_write (&par->appl_write, &ret.appl_write);
    
    SRV_REPLY(handle, &ret, sizeof (R_APPL_WRITE));
    break;
    
  case SRV_EVNT_MULTI:
    srv_wait_for_event (handle, &par->evnt_multi);
    break;
    
  case SRV_GRAF_MKSTATE :
    srv_graf_mkstate (&par->graf_mkstate, &ret.graf_mkstate);
    
    SRV_REPLY(handle, &ret, sizeof (R_GRAF_MKSTATE));
    break;
    
  case SRV_GRAF_MOUSE :
    srv_graf_mouse (globals.vid, &par->graf_mouse, &ret.graf_mouse);
    
    SRV_REPLY(handle, &ret, sizeof (R_GRAF_MOUSE));
    break;
    
  case SRV_MENU_BAR:
    srv_menu_bar (&par->menu_bar, &ret.menu_bar);
    
    SRV_REPLY(handle, &ret, sizeof (R_MENU_BAR));
    break;
    
  case SRV_MENU_REGISTER:
    srv_menu_register (&par->menu_register, &ret.menu_register);
    
    SRV_REPLY(handle, &ret, sizeof (R_MENU_REGISTER));
    break;
    
  case SRV_WIND_CLOSE:
    srv_wind_close (&par->wind_close, &ret.wind_close);
    
    SRV_REPLY(handle, &ret, sizeof (R_WIND_CLOSE));
    break;
    
  case SRV_WIND_CREATE:
    srv_wind_create (&par->wind_create, &ret.wind_create);
    
    SRV_REPLY(handle, &ret, sizeof (R_WIND_CREATE));
    break;
    
  case SRV_WIND_DELETE:
    srv_wind_delete (&par->wind_delete,
                     &ret.wind_delete);
    
    SRV_REPLY(handle, &ret, sizeof (R_WIND_DELETE));
    break;
    
  case SRV_WIND_FIND:
    srv_wind_find (&par->wind_find,
                   &ret.wind_find);
    
    SRV_REPLY(handle, &ret, sizeof (R_WIND_FIND));
    break;
    
  case SRV_WIND_GET:
    srv_wind_get (&par->wind_get,
                  &ret.wind_get);
    
    SRV_REPLY(handle, &ret, sizeof (R_WIND_GET));
    break;
    
  case SRV_WIND_NEW:
    ret.common.retval = 
      srv_wind_new (par->wind_new.common.apid);
    PUT_R_ALL(WIND_NEW, &ret, ret.common.retval);
    
    SRV_REPLY(handle, &ret, sizeof (R_WIND_NEW));
    break;
    
  case SRV_WIND_OPEN:
    srv_wind_open (&par->wind_open, &ret.wind_open);
    
    SRV_REPLY(handle, &ret, sizeof (R_WIND_OPEN));
    break;
        
  case SRV_WIND_SET:
    srv_wind_set (&par->wind_set, &ret.wind_set);
    
    SRV_REPLY(handle, &ret, sizeof (R_WIND_SET));
    break;
    
  case SRV_WIND_UPDATE:
    srv_wind_update (handle,
                     &par->wind_update);
    break;
    
  case SRV_VDI_CALL:
    srv_vdi_call (handle,
                  &par->vdi_call);
    break;
    
  case SRV_MALLOC:
    ret.malloc.address = (ULONG)malloc (par->malloc.amount);
    PUT_R_ALL(MALLOC, &ret, 0);
    SRV_REPLY(handle, &ret, sizeof (R_MALLOC));
    break;
    
  case SRV_FREE:
    free ((void *)par->free.address);
    
    PUT_R_ALL(FREE, &ret, 0);
    SRV_REPLY(handle, &ret, sizeof (R_FREE));
    break;
    
  default:
    DB_printf("%s: Line %d:\r\n"
              "Unknown call %d (0x%x) to server!",
              __FILE__, __LINE__, par->common.call, par->common.call);
    SRV_REPLY(handle, par, -1); /* FIXME */
  }
}


/*
** Description
** This is the server itself
*/
static
WORD
server (LONG arg) {
  /* These variables are passed from clients */
  C_SRV         par;
  COMM_HANDLE   handle;

  /* Stop warnings from compiler about unused parameters */
  NOT_USED(arg);

  /* Initialize message handling */
  DEBUG3 ("srv.c: Initializing server socket");
  Srv_open ();

  /* Initialize global variables */
  DEBUG3 ("srv.c: Initializing global variables");
  srv_init_global (nocnf);

  /* Initialize desktop and menu bar windows */
  srv_init_windows();
  
  DEBUG3 ("srv.c: Intializing event handler");
  /* Setup event vectors */
  srv_init_event_handler (globals.vid);

  DEBUG3 ("srv.c: Show mouse cursor");
  /* Show mouse cursor */
  v_show_c (globals.vid, 1);

  while (run_server) {
    /* Wait for another call from a client */
    DEBUG3 ("srv.c: Waiting for message from client");
    SRV_GET (&par, sizeof (C_SRV), handle);
    DEBUG3 ("srv.c: Got message from client (%p)", handle);

    if (handle != NULL) {
      NTOH(&par);

      srv_call(handle,
               &par);
    }

    DEBUG3 ("srv.c: calling srv_handle_events");
    srv_handle_events (globals.vid);
  }

  Srv_exit_module ();

  srv_term (0);

  return 0;
}

/****************************************************************************
 * Public functions                                                         *
 ****************************************************************************/

/*
** Description
** Initialize server module
*/
void
Srv_init_module (WORD no_config) {
  WORD i;
  
  nocnf = no_config;

  DEBUG3 ("srv.c: Srv_init_module: In Srv_init_module");

  /* FIXME: Move initializations to server process */
  for(i = 0; i < MAX_NUM_APPS; i++) {
    apps[i].id = -1;
    apps[i].eventpipe = -1;
    apps[i].msgpipe = -1;
    apps[i].rshdr = NULL;
    apps[i].deskbg = NULL;
    apps[i].menu = NULL;
    apps[i].deskmenu = -1;
  }
  
  if(globals.num_pens < 16) {
    for(i = 0; i <= W_SMALLER; i++) {
      untop_colour[i].pattern = 0;
      untop_colour[i].fillc = WHITE;
      top_colour[i].pattern = 0;
      top_colour[i].fillc = WHITE;
    }
  }
  

  DEBUG3 ("Starting server process");
  globals.srvpid = (WORD)srv_fork(server,0,"oAESsrv");
  DEBUG3 ("Started server process");
}


/*
** Description
** Shutdown server module
**
** 1998-12-22 CG
** 1999-01-09 CG
** 1999-04-18 CG
** 1999-05-20 CG
** 1999-07-26 CG
*/
void
Srv_exit_module (void) {
  DEBUG2("Killing off alive processes");
  
  /* Kill all AES processes */
  while (ap_pri) {
    C_APPL_CONTROL msg;
    R_APPL_CONTROL ret;

    msg.ap_id = ap_pri->ai->id;
    msg.mode = APC_KILL;

    srv_appl_control (&msg, &ret);
  }

  srv_exit_global ();
}


/*
** Description
** Request the server to stop
**
** 1999-07-27 CG
*/
void
Srv_stop (void) {
  DEBUG2 ("Assigning run_server FALSE");
  run_server = FALSE;
}
