/****************************************************************************

 Module
  srvcalls.c
  
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

#include "oconfig.h"
#include "debug.h"
#include "gemdefs.h"
#include "global.h"
#include "lxgemdos.h"
#include "mesagdef.h"
#include "mintdefs.h"
#include "misc.h"
#include "objc.h"
#include "resource.h"
#include "rlist.h"
#include "srv.h"
#include "srv_get.h"
#include "srv_put.h"
#include "types.h"
#include "vdi.h"


/****************************************************************************
 * Macros                                                                   *
 ****************************************************************************/


#define SRV_QUE_SIZE 32

/* Server calls */

enum {
  SRV_SHAKE,
  SRV_SHUTDOWN,
  SRV_APPL_CONTROL,
  SRV_APPL_EXIT,
  SRV_APPL_FIND,
  SRV_APPL_INIT,
  SRV_APPL_SEARCH,
  SRV_APPL_WRITE,
  SRV_CLICK_OWNER,
  SRV_GET_APPL_INFO,
  SRV_GET_TOP_MENU,
  SRV_GET_WM_INFO,
  SRV_MENU_BAR,
  SRV_MENU_REGISTER,
  SRV_PUT_EVENT,
  SRV_SHEL_ENVRN,
  SRV_SHEL_WRITE,
  SRV_WIND_CHANGE,
  SRV_WIND_CLOSE,
  SRV_WIND_CREATE,
  SRV_WIND_DELETE,
  SRV_WIND_DRAW,
  SRV_WIND_FIND,
  SRV_WIND_GET,
  SRV_WIND_NEW,
  SRV_WIND_OPEN,
  SRV_WIND_SET
};

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
 * Typedefs of module global interest                                       *
 ****************************************************************************/

/* appl_* related */

typedef struct {
	WORD apid;
	WORD mode;
	WORD retval;
}C_APPL_CONTROL;

typedef struct {
	WORD retval;
}C_APPL_EXIT;

typedef struct {
	BYTE *fname;
	WORD retval;
}C_APPL_FIND;

typedef struct {
	WORD         vid;
	BYTE         msgname[20];
	WORD         msghandle;
	BYTE         eventname[20];
	WORD         eventhandle;
	GLOBAL_ARRAY *global;
}C_APPL_INIT;

typedef struct {
	WORD mode;
	BYTE *name;
	WORD type;
	WORD ap_id;
	WORD retval;
}C_APPL_SEARCH;

typedef struct {
	WORD apid;
	WORD length;
	void *msg;
	WORD retval;
}C_APPL_WRITE;

typedef struct {
	WORD retval;
}C_CLICK_OWNER;

typedef struct {
	SRV_APPL_INFO *appl_info;
	WORD      retval;
}C_GET_APPL_INFO;

typedef struct {
	void *retval;
}C_GET_TOP_MENU;

typedef struct {
	WORD id;
	void *retval;
}C_GET_WM_INFO;

typedef struct {
	WORD   apid;
	OBJECT *tree;
	WORD   mode;
	WORD   retval;
}C_MENU_BAR;

typedef struct {
	BYTE *title;
	WORD retval;
}C_MENU_REGISTER;

typedef struct {
	WORD    apid;
	EVNTREC *er;
	WORD    length;
	WORD    retval;
}C_PUT_EVENT;

typedef struct {
	WORD pid;
	WORD type;
	WORD retval;
}C_REGISTER_PRG;

typedef struct {
	BYTE **value;
	BYTE *name;
	WORD retval;
}C_SHEL_ENVRN;

typedef struct {
	WORD mode;
	WORD wisgr;
	WORD wiscr;
	BYTE *cmd;
	BYTE *tail;
	WORD retval;
}C_SHEL_WRITE;

typedef struct {
	WORD id;
	WORD object;
	WORD newstate;
	WORD retval;
}C_WIND_CHANGE;
 
typedef struct {
	WORD id;
	WORD retval;
}C_WIND_CLOSE;

typedef struct {
	WORD owner;
	WORD elements;
	RECT *maxsize;
	WORD status;
	WORD retval;
}C_WIND_CREATE;

typedef C_WIND_CLOSE C_WIND_DELETE;

typedef struct {
	WORD handle;
	WORD object;
	WORD retval;
}C_WIND_DRAW;

typedef struct {
	WORD x;
	WORD y;
	WORD retval;
}C_WIND_FIND;

typedef struct {
	WORD handle;
	WORD mode;
	WORD parm1;
	WORD parm2;
	WORD parm3;
	WORD parm4;
	WORD retval;
}C_WIND_GET;

typedef struct {
	WORD retval;
}C_WIND_NEW;

typedef C_WIND_GET C_WIND_SET;

typedef struct {
	WORD id;
	RECT *size;
	WORD retval;
}C_WIND_OPEN;

typedef union {
	C_APPL_CONTROL  appl_control;
	C_APPL_EXIT     appl_exit;
	C_APPL_FIND     appl_find;
	C_APPL_INIT     appl_init;
	C_APPL_SEARCH   appl_search;
	C_APPL_WRITE    appl_write;
	C_CLICK_OWNER   click_owner;
	C_GET_APPL_INFO get_appl_info;
	C_GET_TOP_MENU  get_top_menu;
	C_GET_WM_INFO   get_wm_info;
	C_MENU_BAR      menu_bar;
	C_MENU_REGISTER menu_register;
	C_PUT_EVENT     put_event;
	C_REGISTER_PRG  register_prg;
	C_SHEL_ENVRN    shel_envrn;
	C_SHEL_WRITE    shel_write;
	C_WIND_CHANGE   wind_change;
	C_WIND_CLOSE    wind_close;
	C_WIND_CREATE   wind_create;
	C_WIND_DELETE   wind_delete;
	C_WIND_DRAW     wind_draw;
	C_WIND_FIND     wind_find;
	C_WIND_GET      wind_get;
	C_WIND_NEW      wind_new;
	C_WIND_OPEN     wind_open;
	C_WIND_SET      wind_set;
}C_SRV;

/****************************************************************************
 * Public functions                                                         *
 ****************************************************************************/

void Srv_shake(void) {
  fprintf (stderr, "srv.c: Entering Srv_shake\n");
  DB_printf("How are you doing server?");
	
  Srv_put (0, SRV_SHAKE, NULL);
	
  DB_printf("I'm fine too process!");
  fprintf (stderr, "srv.c: Leaving Srv_shake\n");
}

/****************************************************************************
 * Srv_appl_control                                                         *
 ****************************************************************************/
WORD                /* 0 if error or >0.                                    */
Srv_appl_control(   /*                                                      */
WORD apid,          /* Application to control.                              */
WORD mode)          /* What to do.                                          */
/****************************************************************************/
{
  C_APPL_CONTROL par;
	
  par.apid = apid;
  par.mode = mode;

  return Srv_put (apid, SRV_APPL_CONTROL, &par);
}

/****************************************************************************
 * Srv_appl_exit                                                            *
 *  Implementation of appl_exit().                                          *
 ****************************************************************************/
WORD            /* 0 if error, or 1.                                        */
Srv_appl_exit(  /*                                                          */
WORD apid)      /* Application id.                                          */
/****************************************************************************/
{
  C_APPL_EXIT   par;
  SRV_APPL_INFO apinf;
  WORD          code;

  Srv_get_appl_info(apid, &apinf);

  code = Srv_put (apid, SRV_APPL_EXIT, &par);

  Fclose(apinf.msgpipe);
  Fclose(apinf.eventpipe);
  Vdi_v_clsvwk(apinf.vid);

  return code;
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
 * Srv_appl_init                                                            *
 *  Implementation of appl_init().                                          *
 ****************************************************************************/
WORD                   /* Application id, or -1.                            */
Srv_appl_init(         /*                                                   */
GLOBAL_ARRAY *global)  /* Global array.                                     */
/****************************************************************************/
{
  WORD        pid = Pgetpid();
  C_APPL_INIT par;
  LONG        fnr;
  WORD        work_in[] = {1,7,1,1,1,1,1,1,1,1,2};
  WORD        work_out[57];
	

  sprintf(par.msgname,"u:\\pipe\\applmsg.%03d",pid);		
  sprintf(par.eventname,"u:\\pipe\\applevnt.%03d",pid);		

  par.msghandle = par.eventhandle = 0;

  if((fnr = Fopen(par.msgname,0)) >= 0)
  {
    Fclose((WORD)fnr);
  }
  else if((par.msghandle = (WORD)Fcreate(par.msgname,0)) < 0)
  {
    Fclose(par.msghandle);
		
    return -1;
  }
	
  if((fnr = Fopen(par.eventname,0)) >= 0)
  {
    Fclose((WORD)fnr);
  }
  else if((par.eventhandle = (WORD)Fcreate(par.eventname,0)) < 0)
  {
    Fclose(par.msghandle);
    Fclose(par.eventhandle);
		
    return -1;
  }
	

  par.global = global;

  par.vid = globals.vid;
  Vdi_v_opnvwk(work_in, &par.vid, work_out);

  Srv_put (0, SRV_APPL_INIT, &par);
		
  if(global->apid >= 0) {
    return global->apid;
  };
			
  Vdi_v_clsvwk(par.vid);
  Fclose(par.msghandle);
  Fclose(par.eventhandle);
	
  return -1;
}


/****************************************************************************
 * Srv_appl_search                                                          *
 *  Implementation of appl_search().                                        *
 ****************************************************************************/
WORD              /* 0 if no more applications exist, or 1.                 */
Srv_appl_search(  /*                                                        */
WORD apid,        /* pid of caller..                                        */
WORD mode,        /* Search mode.                                           */
BYTE *name,       /* Pretty name of found application.                      */
WORD *type,       /* Type of found application.                             */
WORD *ap_id)      /* Application id of found application.                   */
/****************************************************************************/
{
  C_APPL_SEARCH par;
  WORD          code;
	
  par.mode = mode;
  par.name = name;

  code = Srv_put (apid, SRV_APPL_SEARCH, &par);

  *type = par.type;
  *ap_id = par.ap_id;

  return code;
}


/****************************************************************************
 * Srv_appl_write                                                           *
 *  Implementation of appl_write().                                         *
 ****************************************************************************/
WORD            /* 0 if ok, or 1.                                           */
Srv_appl_write( /*                                                          */
WORD apid,      /* Id of application to receive message.                    */
WORD length,    /* Length of message structure.                             */
void *m)        /* Pointer to message structure.                            */
/****************************************************************************/
{
  C_APPL_WRITE par;

  par.apid = apid;
  par.length = length;
  par.msg = m;
	
  return Srv_put (apid, SRV_APPL_WRITE, &par);
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
 * Srv_get_appl_info                                                        *
 ****************************************************************************/
WORD                       /* 0 if ok or -1.                                */
Srv_get_appl_info(         /*                                               */
WORD apid,                 /* Application id.                               */
SRV_APPL_INFO *appl_info)  /* Returned information.                         */
/****************************************************************************/
{
  /*  appl_info->msgpipe = apps[apid].msgpipe;
  appl_info->eventpipe = apps[apid].eventpipe;
  appl_info->vid = apps[apid].vid;*/

  return 0;
/*
	C_GET_APPL_INFO par;
	PMSG            msg;
	
	par.appl_info = appl_info;
	
	msg.cr.call = SRV_GET_APPL_INFO;
	msg.apid = apid;
	msg.spec = &par;

	Srvc_operation(MSG_READWRITE,&msg);

	return msg.cr.retval;
*/
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
 * Srv_menu_bar                                                             *
 *  0x001e menu_bar() library code.                                         *
 ****************************************************************************/
WORD              /*                                                        */
Srv_menu_bar(     /*                                                        */
WORD   apid,      /* Application id.                                        */
OBJECT *tree,     /* Menu object tree.                                      */
WORD   mode)      /* Mode.                                                  */
/****************************************************************************/
{
  C_MENU_BAR par;

  par.apid = apid;
  par.tree = tree;
  par.mode = mode;
	
  return Srv_put (apid, SRV_MENU_BAR, &par);
}


/****************************************************************************
 * Srv_menu_register                                                        *
 *  Implementation of menu_register().                                      *
 ****************************************************************************/
WORD               /* Menu identification, or -1.                           */
Srv_menu_register( /*                                                       */
WORD apid,         /* Application id, or -1.                                */
BYTE *title)       /* Title to register application under.                  */
/****************************************************************************/
{
  C_MENU_REGISTER par;
	
  par.title = title;
	
  return Srv_put (apid, SRV_MENU_REGISTER, &par);
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
 * Srv_wind_change                                                          *
 *  Change state of window widget.                                          *
 ****************************************************************************/
WORD               /* 1 if ok, 0 if error.                                 */
Srv_wind_change(   /*                                                       */
WORD id,           /* Window handle.                                        */
WORD object,       /* Widget to change state for.                           */
WORD newstate)     /* New state of widget.                                  */
/****************************************************************************/
{
  C_WIND_CHANGE par;
	
  par.id = id;
  par.object = object;
  par.newstate = newstate;
	
  return Srv_put (0, SRV_WIND_CHANGE, &par);
}


/****************************************************************************
 * Srv_wind_close                                                           *
 *  Implementation of wind_close().                                         *
 ****************************************************************************/
WORD            /* 0 if error or 1 if ok.                                   */
Srv_wind_close( /*                                                          */
WORD wid)       /* Identification number of window to close.                */
/****************************************************************************/
{
  C_WIND_CLOSE par;
	
  par.id = wid;
	
  return Srv_put (0, SRV_WIND_CLOSE, &par);
}

/****************************************************************************
 * Srv_wind_create                                                          *
 *  Implementation of wind_create().                                        *
 ****************************************************************************/
WORD             /* 0 if error or 1 if ok.                                  */
Srv_wind_create( /*                                                         */
WORD owner,      /* Owner of window.                                        */
WORD elements,   /* Elements of window.                                     */
RECT *maxsize,   /* Maximum size allowed.                                   */
WORD status)     /* Status of window.                                       */
/****************************************************************************/
{
  C_WIND_CREATE par;
	
  par.owner = owner;
  par.elements = elements;
  par.maxsize = maxsize;
  par.status = status;
	
  return Srv_put (0, SRV_WIND_CREATE, &par);
}

/****************************************************************************
 * Srv_wind_delete                                                          *
 *  Implementation of wind_delete().                                        *
 ****************************************************************************/
WORD             /* 0 if error or 1 if ok.                                  */
Srv_wind_delete( /*                                                         */
WORD wid)        /* Identification number of window to close.               */
/****************************************************************************/
{
  C_WIND_DELETE par;
	
  par.id = wid;
	
  return Srv_put (0, SRV_WIND_DELETE, &par);
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
 * Srv_wind_find                                                            *
 *  Find window on known coordinates.                                       *
 ****************************************************************************/
WORD           /* Window handle.                                            */
Srv_wind_find( /*                                                           */
WORD x,        /* X coordinate.                                             */
WORD y)        /* Y coordinate.                                             */
/****************************************************************************/
{
  C_WIND_FIND par;
	
  par.x = x;
  par.y = y;
	
  return Srv_put (0, SRV_WIND_FIND, &par);
}

/****************************************************************************
 * Srv_wind_get                                                             *
 *  Get information about window.                                           *
 ****************************************************************************/
WORD           /*                                                           */
Srv_wind_get(  /*                                                           */
WORD handle,   /* Identification number of window.                          */
WORD mode,     /* Tells what to return.                                     */
WORD *parm1,   /* Parameter 1.                                              */
WORD *parm2,   /* Parameter 2.                                              */
WORD *parm3,   /* Parameter 3.                                              */
WORD *parm4)   /* Parameter 4.                                              */
/****************************************************************************/
{
  C_WIND_GET par;
  WORD       code;
	
  par.handle = handle;
  par.mode = mode;
	
  code = Srv_put (0, SRV_WIND_GET, &par);

  *parm1 = par.parm1;
  *parm2 = par.parm2;
  *parm3 = par.parm3;
  *parm4 = par.parm4;
	
  return code;
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
 * Srv_wind_open                                                            *
 *  Implementation of wind_open().                                          *
 ****************************************************************************/
WORD           /* 0 if error or 1 if ok.                                    */
Srv_wind_open( /*                                                           */
WORD id,       /* Identification number of window to open.                  */
RECT *size)    /* Initial size of window.                                   */
/****************************************************************************/
{
	C_WIND_OPEN par;
	
	par.id = id;
	par.size = size;
	
	return Srv_put (0, SRV_WIND_OPEN, &par);
}

/****************************************************************************
 * Srv_wind_set                                                             *
 ****************************************************************************/
WORD           /*                                                           */
Srv_wind_set(  /*                                                           */
WORD apid,     /*                                                           */
WORD handle,   /* Identification number of window.                          */
WORD mode,     /* Tells what to return.                                     */
WORD parm1,    /* Parameter 1.                                              */
WORD parm2,    /* Parameter 2.                                              */
WORD parm3,    /* Parameter 3.                                              */
WORD parm4)    /* Parameter 4.                                              */
/****************************************************************************/
{
	C_WIND_SET par;
	
	par.handle = handle;
	par.mode = mode;
	
	par.parm1 = parm1;
	par.parm2 = parm2;
	par.parm3 = parm3;
	par.parm4 = parm4;

	return Srv_put (apid, SRV_WIND_SET, &par);
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

