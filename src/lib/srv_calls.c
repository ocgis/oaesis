/****************************************************************************

 Module
  srv_calls.c
  
 Description
  Window, menu, and application server in oAESis.
  
 Author(s)
 	cg (Christer Gustavsson <d2cg@dtek.chalmers.se>)
 	
 Revision history
 
  960623 cg
   Creation date.
 
 Copyright notice
  The copyright to the program code herein belongs to the authors. It may
  be freely duplicated and distributed without fee, but not charged for.
 
 ****************************************************************************/

/*
#define SRV_DEBUG
*/

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

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

#include "aesbind.h"
#include "oconfig.h"
#include "debug.h"
#include "lib_global.h"
/*#include "lxgemdos.h"*/
#include "mesagdef.h"
#include "mintdefs.h"
#include "lib_misc.h"
#include "objc.h"
#include "resource.h"
/*#include "rlist.h"*/
/*#include "srv.h"*/
#include "srv_interface.h"
#include "srv_put.h"
#include "types.h"


/****************************************************************************
 * Macros                                                                   *
 ****************************************************************************/


#define SRV_QUE_SIZE 32

/* appl_* related */

#define TOP_APPL       (-1)
#define DESK_OWNER     (-2)
#define TOP_MENU_OWNER (-3)

/*window status codes*/

#define	WIN_OPEN       0x0001
#define	WIN_UNTOPPABLE 0x0002
#define	WIN_DESKTOP    0x0004
#define	WIN_TOPPED     0x0008
#define	WIN_DIALOG     0x0010
#define	WIN_MENU       0x0020
#define WIN_ICONIFIED  0x0040

#define IMOVER 0x8000  /* Used with set_win_elem() to make icon window */

#define D3DSIZE       2

#define INTS2LONG(a,b) (((((LONG)a)<<16)&0xffff0000L)|(((LONG)b)&0xffff))


/****************************************************************************
 * Public functions                                                         *
 ****************************************************************************/

void Srv_shake(void) {
  DB_printf ("srv.c: Entering Srv_shake\n");
  DB_printf ("How are you doing server?");
	
  Srv_put (0, SRV_SHAKE, NULL);
	
  DB_printf ("I'm fine too process!");
  DB_printf ("srv.c: Leaving Srv_shake\n");
}

/****************************************************************************
 * Srv_appl_find                                                            *
 *  Implementation of appl_find().                                          *
 ****************************************************************************/
WORD              /* Application id, or -1.                                 */
Srv_appl_find(    /*                                                        */
BYTE *fname)      /* File name of application to seek.                      */
/****************************************************************************/
{
  C_APPL_FIND par;
	
  par.fname = fname;

  return Srv_put (0, SRV_APPL_FIND, &par);
}


/****************************************************************************
 * Srv_click_owner                                                          *
 *  Find out which application that "owns" mouse clicks.                    *
 ****************************************************************************/
WORD                    /* Application to receive clicks.                   */
Srv_click_owner(void)   /*                                                  */
/****************************************************************************/
{
  C_CLICK_OWNER par;
	
  return Srv_put (0, SRV_CLICK_OWNER, &par);
}


/****************************************************************************
 * Srv_get_top_menu                                                         *
 *  Get the resource tree of the menu of an application                     *
 ****************************************************************************/
OBJECT *                /* Resource tree, or NULL.                          */
Srv_get_top_menu(void)  /*                                                  */
/****************************************************************************/
{
  C_GET_TOP_MENU par;
	
  return (OBJECT *)Srv_put (0, SRV_GET_TOP_MENU, &par);
}


/****************************************************************************
 * Srv_get_wm_info                                                          *
 *  Get window manager info on window.                                      *
 ****************************************************************************/
void *            /* Pointer to window manager structure, or NULL.          */
Srv_get_wm_info(  /*                                                        */
WORD id)          /* Window handle.                                         */
/****************************************************************************/
{
  C_GET_WM_INFO par;
	
  par.id = id;
	
  return (OBJECT *)Srv_put (0, SRV_GET_WM_INFO, &par);
}


/****************************************************************************
 * Srv_shel_envrn                                                           *
 *  Implementation of shel_envrn().                                         *
 ****************************************************************************/
WORD             /*                                                         */
Srv_shel_envrn(  /*                                                         */
BYTE **value,    /* Return address.                                         */
BYTE *name)      /* Name of variable to find.                               */
/****************************************************************************/
{
  C_SHEL_ENVRN par;
	
  par.value = value;
  par.name = name;

  return Srv_put (0, SRV_SHEL_ENVRN, &par);
}


/****************************************************************************
 * Srv_shel_write                                                           *
 *  Implementation of shel_write().                                         *
 ****************************************************************************/
WORD             /*                                                         */
Srv_shel_write(  /*                                                         */
WORD apid,       /* Application id.                                         */
WORD mode,       /* Mode.                                                   */
WORD wisgr,      /*                                                         */
WORD wiscr,      /*                                                         */
BYTE *cmd,       /* Command line.                                           */
BYTE *tail)      /* Command tail.                                           */
/****************************************************************************/
{
  C_SHEL_WRITE par;
	
  par.mode = mode;
  par.wisgr = wisgr;
  par.wiscr = wiscr;
  par.cmd = cmd;
  par.tail = tail;

  return Srv_put (apid, SRV_SHEL_WRITE, &par);
}


/****************************************************************************
 * Srv_wind_draw                                                            *
 *  Draw window widgets.                                                    *
 ****************************************************************************/
WORD             /* 0 if an error occured, or >0                            */
Srv_wind_draw(   /*                                                         */
WORD handle,     /* Handle of window.                                       */
WORD object)     /* Object to be drawn (see WF_COLOR modes).                */
/****************************************************************************/
{
  C_WIND_DRAW par;
	
  par.handle = handle;
  par.object = object;
	
  return Srv_put (0, SRV_WIND_DRAW, &par);
}

/****************************************************************************
 * Srv_wind_new                                                             *
 *  Implementation of wind_new().                                           *
 ****************************************************************************/
WORD           /* 0 if error or 1 if ok.                                    */
Srv_wind_new(  /*                                                           */
WORD apid)     /* Application whose windows should be erased.               */
/****************************************************************************/
{
	C_WIND_NEW par;

	return Srv_put (apid, SRV_WIND_NEW, &par);
}

/****************************************************************************
 * Srv_put_event                                                            *
 *  Put event message in event pipe                                         *
 ****************************************************************************/
WORD              /*  0 if ok or -1                                         */
Srv_put_event(    /*                                                        */
WORD    apid,     /* Id of application that is to receive a message.        */
void    *m,       /* Message to be sent.                                    */
WORD    length)   /* Length of message.                                     */
/****************************************************************************/
{
	C_PUT_EVENT par;
	
	par.apid = apid;
	par.er = m;
	par.length = length;
	
	return Srv_put (apid, SRV_PUT_EVENT, &par);
}

