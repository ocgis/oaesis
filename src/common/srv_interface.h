#ifndef _SRV_INTERFACE_H_
#define _SRV_INTERFACE_H_

#include <vdibind.h>

#include "mesagdef.h"

/* Server calls */

enum {
  SRV_APPL_CONTROL,
  SRV_APPL_INIT     =  10,
  SRV_APPL_WRITE    =  12,
  SRV_APPL_FIND     =  13,
  SRV_APPL_SEARCH   =  18,
  SRV_APPL_EXIT     =  19,
  SRV_EVNT_MULTI    =  25,
  SRV_MENU_BAR      =  30,
  SRV_MENU_REGISTER =  35,
  SRV_GRAF_MOUSE    =  78,
  SRV_GRAF_MKSTATE  =  79,
  SRV_WIND_CREATE   = 100,
  SRV_WIND_OPEN     = 101,
  SRV_WIND_CLOSE    = 102,
  SRV_WIND_DELETE   = 103,
  SRV_WIND_GET      = 104,
  SRV_WIND_SET      = 105,
  SRV_WIND_FIND     = 106,
  SRV_WIND_UPDATE   = 107,
  SRV_WIND_NEW      = 109,
  SRV_VDI_CALL      = 200
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
  C_ALL common;
  WORD  ap_id;
  WORD  mode;
}C_APPL_CONTROL;

typedef struct {
  R_ALL common;
}R_APPL_CONTROL;

typedef struct {
  C_ALL common;
}C_APPL_EXIT;

typedef struct {
  R_ALL common;
}R_APPL_EXIT;

#define APPL_FIND_NAME_TO_APID 0
#define APPL_FIND_PID_TO_APID  1
#define APPL_FIND_APID_TO_PID  2

typedef struct {
  C_ALL  common;
  WORD   mode;
  union {
    BYTE   name[20];
    WORD   pid;
    WORD   apid;
  } data;
} C_APPL_FIND;

typedef struct {
  R_ALL common;
} R_APPL_FIND;

typedef struct {
  C_ALL          common;
  BYTE           appl_name[10];
}C_APPL_INIT;

typedef struct {
  R_ALL common;
  WORD  apid;
  WORD  physical_vdi_id;
}R_APPL_INIT;

typedef struct {
  C_ALL  common;
  WORD   mode;
} C_APPL_SEARCH;

typedef struct {
  BYTE   name[20];
  WORD   type;
  WORD   ap_id;
} APPL_SEARCH_INFO;

typedef struct {
  R_ALL            common;
  WORD             count;
  APPL_SEARCH_INFO info;
} R_APPL_SEARCH;

typedef struct {
  C_ALL common;
  WORD  addressee;
  WORD  length;
  WORD  is_reference; /* != 0 => msg.ref is a pointer to the buffer */
  union {
    void *       ref;
    COMMSG       event;
    REDRAWSTRUCT redraw;
  }msg;
} C_APPL_WRITE;

typedef struct {
  R_ALL common;
} R_APPL_WRITE;

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
  C_ALL common;
} C_GRAF_MKSTATE;

typedef struct {
  R_ALL common;
  WORD  mx;
  WORD  my;
  WORD  mb;
  WORD  ks;
} R_GRAF_MKSTATE;

typedef struct {
  C_ALL common;
  WORD  mode;
  MFORM cursor;
} C_GRAF_MOUSE;

typedef struct {
  R_ALL common;
} R_GRAF_MOUSE;

typedef struct {
  C_ALL    common;
  OBJECT * tree;
  WORD     mode;
} C_MENU_BAR;

typedef struct {
  R_ALL common;
} R_MENU_BAR;

typedef struct {
  C_ALL common;
  BYTE  title[20];
} C_MENU_REGISTER;

typedef struct {
  R_ALL common;
} R_MENU_REGISTER;

typedef struct {
  C_ALL common;
  WORD  id;
  WORD  retval;
} C_WIND_CLOSE;

typedef struct {
  R_ALL common;
} R_WIND_CLOSE;

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
  C_ALL common;
  WORD  x;
  WORD  y;
} C_WIND_FIND;

typedef struct {
  R_ALL common;
} R_WIND_FIND;

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
  C_ALL common;
}C_WIND_NEW;

typedef struct {
  R_ALL common;
}R_WIND_NEW;

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

typedef struct {
  C_ALL common;
  WORD  contrl[15];
  WORD  inpar[132+145];
} C_VDI_CALL;

typedef struct {
  R_ALL common;
  WORD  contrl[15];
  WORD  outpar[140+145];
} R_VDI_CALL;

typedef union {
  C_ALL           common;
  C_APPL_CONTROL  appl_control;
  C_APPL_EXIT     appl_exit;
  C_APPL_FIND     appl_find;
  C_APPL_INIT     appl_init;
  C_APPL_SEARCH   appl_search;
  C_APPL_WRITE    appl_write;
  C_EVNT_MULTI    evnt_multi;
  C_GRAF_MKSTATE  graf_mkstate;
  C_GRAF_MOUSE    graf_mouse;
  C_MENU_BAR      menu_bar;
  C_MENU_REGISTER menu_register;
  C_WIND_CLOSE    wind_close;
  C_WIND_CREATE   wind_create;
  C_WIND_DELETE   wind_delete;
  C_WIND_FIND     wind_find;
  C_WIND_GET      wind_get;
  C_WIND_NEW      wind_new;
  C_WIND_OPEN     wind_open;
  C_WIND_SET      wind_set;
  C_WIND_UPDATE   wind_update;
  C_VDI_CALL      vdi_call;
} C_SRV;


typedef union {
  R_ALL           common;
  R_APPL_CONTROL  appl_control;
  R_APPL_EXIT     appl_exit;
  R_APPL_FIND     appl_find;
  R_APPL_INIT     appl_init;
  R_APPL_SEARCH   appl_search;
  R_APPL_WRITE    appl_write;
  R_EVNT_MULTI    evnt_multi;
  R_GRAF_MKSTATE  graf_mkstate;
  R_GRAF_MOUSE    graf_mouse;
  R_MENU_BAR      menu_bar;
  R_MENU_REGISTER menu_register;
  R_WIND_CLOSE    wind_close;
  R_WIND_CREATE   wind_create;
  R_WIND_DELETE   wind_delete;
  R_WIND_FIND     wind_find;
  R_WIND_GET      wind_get;
  R_WIND_NEW      wind_new;
  R_WIND_OPEN     wind_open;
  R_WIND_SET      wind_set;
  R_WIND_UPDATE   wind_update;
  R_VDI_CALL      vdi_call;
} R_SRV;

/*
** Predefined window ids
**
** 1999-01-09 CG
*/
#define DESKTOP_WINDOW  0
#define MENU_BAR_WINDOW 1

/*
** Internal events used with evnt_multi ()
**
** 1999-01-09 CG
*/
#define MU_MENU_BAR 0x8000

#endif /* _SRV_INTERFACE_H_ */
