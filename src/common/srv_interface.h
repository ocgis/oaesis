#ifndef _SRV_INTERFACE_H_
#define _SRV_INTERFACE_H_

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
  SRV_EVNT_MULTI,
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
  SRV_WIND_SET,
  SRV_WIND_UPDATE
};


/* Common structures */
typedef struct {
  WORD apid;
  WORD pid;
  WORD call;
}C_ALL;

typedef struct {
  WORD retval;
}R_ALL;

/* appl_* related */

typedef struct {
  WORD apid;
  WORD mode;
  WORD retval;
}C_APPL_CONTROL;

typedef struct {
  C_ALL common;
}C_APPL_EXIT;

typedef struct {
  R_ALL common;
}R_APPL_EXIT;

typedef struct {
  BYTE *fname;
  WORD retval;
}C_APPL_FIND;

typedef struct {
  C_ALL          common;
  /*  WORD           vid;
  BYTE           msgname[20];
  WORD           msghandle;
  BYTE           eventname[20];
  WORD           eventhandle;*/
  GLOBAL_ARRAY * global;
}C_APPL_INIT;

typedef struct {
  R_ALL common;
  WORD  apid;
}R_APPL_INIT;

typedef struct {
  WORD mode;
  BYTE *name;
  WORD type;
  WORD ap_id;
  WORD retval;
}C_APPL_SEARCH;

typedef struct {
  C_ALL common;
  WORD  addressee;
  WORD  length;
  WORD  is_reference; /* != 0 => msg.ref is a pointer to the buffer */
  union {
    void * ref;
    COMMSG event;
  }msg;
}C_APPL_WRITE;

typedef struct {
  R_ALL common;
}R_APPL_WRITE;

typedef struct {
  WORD retval;
}C_CLICK_OWNER;

/*
** Events used in evnt_multi
*/
#define MU_KEYBD        0x0001
#define MU_BUTTON       0x0002
#define MU_M1           0x0004
#define MU_M2           0x0008
#define MU_MESAG        0x0010
#define MU_TIMER        0x0020

typedef struct {
  C_ALL   common;
  EVENTIN eventin;
}C_EVNT_MULTI;

typedef struct {
  R_ALL    common;
  COMMSG   msg;
  EVENTOUT eventout;
}R_EVNT_MULTI;

typedef struct {
  /*  SRV_APPL_INFO *appl_info;*/
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
  C_ALL  common;
  WORD   elements;
  RECT   maxsize;
  WORD   status;
  WORD   retval;
}C_WIND_CREATE;

typedef struct {
  R_ALL common;
}R_WIND_CREATE;

typedef struct {
  C_ALL common;
  WORD id;
}C_WIND_DELETE;

typedef struct {
  R_ALL common;
}R_WIND_DELETE;

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
  C_ALL common;
  WORD  handle;
  WORD  mode;
}C_WIND_GET;

typedef struct {
  R_ALL common;
  WORD  parm1;
  WORD  parm2;
  WORD  parm3;
  WORD  parm4;
}R_WIND_GET;

typedef struct {
  WORD retval;
}C_WIND_NEW;

typedef struct {
  C_ALL common;
  WORD  handle;
  WORD  mode;
  WORD  parm1;
  WORD  parm2;
  WORD  parm3;
  WORD  parm4;
}C_WIND_SET;

typedef struct {
  R_ALL common;
}R_WIND_SET;

typedef struct {
  C_ALL common;
  WORD  id;
  RECT  size;
}C_WIND_OPEN;

typedef struct {
  R_ALL common;
}R_WIND_OPEN;

typedef struct {
  C_ALL common;
  WORD  mode;
}C_WIND_UPDATE;

typedef struct {
  R_ALL common;
}R_WIND_UPDATE;

typedef union {
  C_ALL           common;
  C_APPL_CONTROL  appl_control;
  C_APPL_EXIT     appl_exit;
  C_APPL_FIND     appl_find;
  C_APPL_INIT     appl_init;
  C_APPL_SEARCH   appl_search;
  C_APPL_WRITE    appl_write;
  C_CLICK_OWNER   click_owner;
  C_EVNT_MULTI    evnt_multi;
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
  C_WIND_UPDATE   wind_update;
}C_SRV;


typedef union {
  R_ALL         common;
  R_APPL_EXIT   appl_exit;
  R_APPL_INIT   appl_init;
  R_APPL_WRITE  appl_write;
  R_EVNT_MULTI  evnt_multi;
  R_WIND_CREATE wind_create;
  R_WIND_DELETE wind_delete;
  R_WIND_GET    wind_get;
  R_WIND_OPEN   wind_open;
  R_WIND_UPDATE wind_update;
}R_SRV;

#endif /* _SRV_INTERFACE_H_ */
