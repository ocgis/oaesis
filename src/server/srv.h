#ifndef __SRV__
#define __SRV__

#define SRV_COMPLETE_MCTRL 0x1000

/* Internal appl_control() modes */

#define APC_TOPNEXT 0
#define APC_KILL    1


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

typedef struct {
	WORD eventpipe;
	WORD msgpipe;
	WORD vid;
}SRV_APPL_INFO;

/****************************************************************************
 * Server_init_module                                                       *
 *  Initialize server module.                                               *
 ****************************************************************************/
void                   /*                                                   */
Srv_init_module(void); /*                                                   */
/****************************************************************************/

/****************************************************************************
 * Srv_exit_module                                                          *
 *  Shutdown server module.                                                 *
 ****************************************************************************/
void                   /*                                                   */
Srv_exit_module(void); /*                                                   */
/****************************************************************************/

void Srv_shake(void);

/****************************************************************************
 * Srv_appl_control                                                         *
 ****************************************************************************/
WORD                /* 0 if error or >0.                                    */
Srv_appl_control(   /*                                                      */
WORD apid,          /* Application to control.                              */
WORD mode);         /* What to do.                                          */
/****************************************************************************/

/****************************************************************************
 * Srv_appl_exit                                                            *
 *  Implementation of appl_exit().                                          *
 ****************************************************************************/
WORD            /* 0 if error, or 1.                                        */
Srv_appl_exit(  /*                                                          */
WORD apid);     /* Application id.                                          */
/****************************************************************************/

/****************************************************************************
 * Srv_appl_find                                                            *
 *  Implementation of appl_find().                                          *
 ****************************************************************************/
WORD              /* Application id, or -1.                                 */
Srv_appl_find(    /*                                                        */
BYTE *fname);     /* File name of application to seek.                      */
/****************************************************************************/

/****************************************************************************
 * Srv_appl_init                                                            *
 *  Implementation of appl_init().                                          *
 ****************************************************************************/
WORD                   /* Application id, or -1.                            */
Srv_appl_init(         /*                                                   */
GLOBAL_ARRAY *global); /* Global array.                                     */
/****************************************************************************/

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
WORD *ap_id);     /* Application id of found application.                   */
/****************************************************************************/

/****************************************************************************
 * Srv_appl_write                                                           *
 *  Implementation of appl_write().                                         *
 ****************************************************************************/
WORD            /* 0 if ok, or 1.                                           */
Srv_appl_write( /*                                                          */
WORD apid,      /* Id of application to receive message.                    */
WORD length,    /* Length of message structure.                             */
void *m);       /* Pointer to message structure.                            */
/****************************************************************************/

/****************************************************************************
 * Srv_click_owner                                                          *
 *  Find out which application that "owns" mouse clicks.                    *
 ****************************************************************************/
WORD                    /* Application to receive clicks.                   */
Srv_click_owner(void);  /*                                                  */
/****************************************************************************/

/****************************************************************************
 * Srv_get_appl_info                                                        *
 ****************************************************************************/
WORD                       /* 0 if ok or -1.                                */
Srv_get_appl_info(         /*                                               */
WORD apid,                 /* Application id.                               */
SRV_APPL_INFO *appl_info); /* Returned information.                         */
/****************************************************************************/

/****************************************************************************
 * Srv_get_top_menu                                                         *
 *  Get the resource tree of the menu of an application                     *
 ****************************************************************************/
OBJECT *                /* Resource tree, or NULL.                          */
Srv_get_top_menu(void); /*                                                  */
/****************************************************************************/

/****************************************************************************
 * Srv_get_wm_info                                                          *
 *  Get window manager info on window.                                      *
 ****************************************************************************/
void *            /* Pointer to window manager structure, or NULL.          */
Srv_get_wm_info(  /*                                                        */
WORD id);         /* Window handle.                                         */
/****************************************************************************/

/****************************************************************************
 * Srv_menu_bar                                                             *
 *  0x001e menu_bar() library code.                                         *
 ****************************************************************************/
WORD              /*                                                        */
Srv_menu_bar(     /*                                                        */
WORD   apid,      /* Application id.                                        */
OBJECT *tree,     /* Menu object tree.                                      */
WORD   mode);     /* Mode.                                                  */
/****************************************************************************/

/****************************************************************************
 * Srv_menu_register                                                        *
 *  Implementation of menu_register().                                      *
 ****************************************************************************/
WORD               /* Menu identification, or -1.                           */
Srv_menu_register( /*                                                       */
WORD apid,         /* Application id, or -1.                                */
BYTE *title);      /* Title to register application under.                  */
/****************************************************************************/

/****************************************************************************
 * Srv_put_event                                                            *
 *  Put event message in event pipe                                         *
 ****************************************************************************/
WORD              /*  0 if ok or -1                                         */
Srv_put_event(    /*                                                        */
WORD    apid,     /* Id of application that is to receive a message.        */
void    *m,       /* Message to be sent.                                    */
WORD    length);  /* Length of message.                                     */
/****************************************************************************/

/****************************************************************************
 * Srv_shel_envrn                                                           *
 *  Implementation of shel_envrn().                                         *
 ****************************************************************************/
WORD             /*                                                         */
Srv_shel_envrn(  /*                                                         */
BYTE **value,    /* Return address.                                         */
BYTE *name);     /* Name of variable to find.                               */
/****************************************************************************/

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
BYTE *tail);     /* Command tail.                                           */
/****************************************************************************/

/****************************************************************************
 * Srv_wind_change                                                          *
 *  Change state of window widget.                                          *
 ****************************************************************************/
WORD               /* 1 if ok, 0 if error.                                 */
Srv_wind_change(   /*                                                       */
WORD id,           /* Window handle.                                        */
WORD object,       /* Widget to change state for.                           */
WORD newstate);    /* New state of widget.                                  */
/****************************************************************************/

/****************************************************************************
 * Srv_wind_close                                                           *
 *  Implementation of wind_close().                                         *
 ****************************************************************************/
WORD            /* 0 if error or 1 if ok.                                   */
Srv_wind_close( /*                                                          */
WORD wid);      /* Identification number of window to close.                */
/****************************************************************************/

/****************************************************************************
 * Srv_wind_create                                                          *
 *  Implementation of wind_create().                                        *
 ****************************************************************************/
WORD             /* 0 if error or 1 if ok.                                  */
Srv_wind_create( /*                                                         */
WORD owner,      /* Owner of window.                                        */
WORD elements,   /* Elements of window.                                     */
RECT *maxsize,   /* Maximum size allowed.                                   */
WORD status);    /* Status of window.                                       */
/****************************************************************************/

/****************************************************************************
 * Srv_wind_delete                                                          *
 *  Implementation of wind_delete().                                        *
 ****************************************************************************/
WORD             /* 0 if error or 1 if ok.                                  */
Srv_wind_delete( /*                                                         */
WORD wid);       /* Identification number of window to close.               */
/****************************************************************************/

/****************************************************************************
 * Srv_wind_draw                                                            *
 *  Draw window widgets.                                                    *
 ****************************************************************************/
WORD             /* 0 if an error occured, or >0                            */
Srv_wind_draw(   /*                                                         */
WORD handle,     /* Handle of window.                                       */
WORD object);    /* Object to be drawn (see WF_COLOR modes).                */
/****************************************************************************/

/****************************************************************************
 * Srv_wind_find                                                            *
 *  Find window on known coordinates.                                       *
 ****************************************************************************/
WORD           /* Window handle.                                            */
Srv_wind_find( /*                                                           */
WORD x,        /* X coordinate.                                             */
WORD y);       /* Y coordinate.                                             */
/****************************************************************************/

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
WORD *parm4);  /* Parameter 4.                                              */
/****************************************************************************/

/****************************************************************************
 * Srv_wind_new                                                             *
 *  Implementation of wind_new().                                           *
 ****************************************************************************/
WORD           /* 0 if error or 1 if ok.                                    */
Srv_wind_new(  /*                                                           */
WORD apid);    /* Application whose windows should be erased.               */
/****************************************************************************/

/****************************************************************************
 * Srv_wind_open                                                            *
 *  Implementation of wind_open().                                          *
 ****************************************************************************/
WORD           /* 0 if error or 1 if ok.                                    */
Srv_wind_open( /*                                                           */
WORD id,       /* Identification number of window to open.                  */
RECT *size);   /* Initial size of window.                                   */
/****************************************************************************/

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
WORD parm4);   /* Parameter 4.                                              */
/****************************************************************************/

#endif