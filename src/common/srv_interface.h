#ifndef _SRV_INTERFACE_H_
#define _SRV_INTERFACE_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <netinet/in.h>
#include <vdibind.h>

#include "mesagdef.h"

/* Server calls */

enum
{
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
  SRV_VDI_CALL      = 200,
  SRV_APPL_RESERVE  = 201,
  SRV_MEMORY_ALLOC  = 202,
  SRV_MEMORY_FREE   = 203,
  SRV_MEMORY_SET    = 205,
  SRV_MEMORY_GET    = 206
};


/* Common structures */
typedef struct {
  WORD words;
  WORD apid;
  WORD pid;
  WORD call;
}C_ALL;

#define C_ALL_WORDS 4

typedef struct {
  WORD words;
  WORD retval;
}R_ALL;

#define R_ALL_WORDS 2

/* appl_* related */

typedef struct {
  C_ALL common;
  WORD  ap_id;
  WORD  mode;
}C_APPL_CONTROL;

#define C_APPL_CONTROL_WORDS 2

typedef struct {
  R_ALL common;
}R_APPL_CONTROL;

#define R_APPL_CONTROL_WORDS 0

typedef struct {
  C_ALL common;
}C_APPL_EXIT;

#define C_APPL_EXIT_WORDS 0

typedef struct {
  R_ALL common;
}R_APPL_EXIT;

#define R_APPL_EXIT_WORDS 0

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

#define C_APPL_FIND_WORDS 1 /* FIXME */

typedef struct {
  R_ALL common;
} R_APPL_FIND;

#define R_APPL_FIND_WORDS 0

typedef struct {
  C_ALL common;
  BYTE  appl_name[20];
}C_APPL_INIT;

#define C_APPL_INIT_WORDS 0

typedef struct {
  R_ALL common;
  WORD  apid;
  WORD  physical_vdi_id;
}R_APPL_INIT;

#define R_APPL_INIT_WORDS 2

typedef struct
{
  C_ALL common;
  WORD  type;   /* APP_APPLICATION or APP_ACCESSORY */
  WORD  pid;    /* Process id: FIXME: make more general id */
} C_APPL_RESERVE;

#define C_APPL_RESERVE_WORDS 0

typedef struct
{
  R_ALL common; /* The reserved application id is returned in retval */
} R_APPL_RESERVE;

#define R_APPL_RESERVE_WORDS 0

typedef struct {
  C_ALL  common;
  WORD   mode;
} C_APPL_SEARCH;

#define C_APPL_SEARCH_WORDS 1

typedef struct {
  WORD   type;
  WORD   ap_id;
  BYTE   name[20];
} APPL_SEARCH_INFO;

typedef struct {
  R_ALL            common;
  WORD             count;
  APPL_SEARCH_INFO info;
} R_APPL_SEARCH;

#define R_APPL_SEARCH_WORDS 3

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

#define C_APPL_WRITE_WORDS 11

typedef struct {
  R_ALL common;
} R_APPL_WRITE;

#define R_APPL_WRITE_WORDS 0

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

#define C_EVNT_MULTI_WORDS (sizeof(EVENTIN) / sizeof(WORD))

typedef struct {
  R_ALL    common;
  COMMSG   msg;
  EVENTOUT eventout;
}R_EVNT_MULTI;

#define R_EVNT_MULTI_WORDS ((sizeof(COMMSG) + sizeof(EVENTOUT)) / sizeof(WORD))

typedef struct {
  C_ALL common;
} C_GRAF_MKSTATE;

#define C_GRAF_MKSTATE_WORDS 0

typedef struct {
  R_ALL common;
  WORD  mx;
  WORD  my;
  WORD  mb;
  WORD  ks;
} R_GRAF_MKSTATE;

#define R_GRAF_MKSTATE_WORDS 4

typedef struct {
  C_ALL common;
  WORD  mode;
  MFORM cursor;
} C_GRAF_MOUSE;

#define C_GRAF_MOUSE_WORDS (1 + sizeof(MFORM) / sizeof(WORD))

typedef struct {
  R_ALL common;
} R_GRAF_MOUSE;

#define R_GRAF_MOUSE_WORDS 0

typedef struct {
  C_ALL    common;
  WORD     mode;
  OBJECT * tree;
} C_MENU_BAR;

#define C_MENU_BAR_WORDS 1

typedef struct {
  R_ALL common;
} R_MENU_BAR;

#define R_MENU_BAR_WORDS 0

typedef struct {
  C_ALL common;
  WORD  register_apid;
  BYTE  title[20];
} C_MENU_REGISTER;

#define C_MENU_REGISTER_WORDS 1

typedef struct {
  R_ALL common;
} R_MENU_REGISTER;

#define R_MENU_REGISTER_WORDS 0

typedef struct {
  C_ALL common;
  WORD  id;
  WORD  retval;
} C_WIND_CLOSE;

#define C_WIND_CLOSE_WORDS 2

typedef struct {
  R_ALL common;
} R_WIND_CLOSE;

#define R_WIND_CLOSE_WORDS 0

typedef struct {
  C_ALL  common;
  WORD   elements;
  RECT   maxsize;
  WORD   status;
}C_WIND_CREATE;

#define C_WIND_CREATE_WORDS 6

typedef struct {
  R_ALL common;
}R_WIND_CREATE;

#define R_WIND_CREATE_WORDS 0

typedef struct {
  C_ALL common;
  WORD id;
}C_WIND_DELETE;

#define C_WIND_DELETE_WORDS 1

typedef struct {
  R_ALL common;
}R_WIND_DELETE;

#define R_WIND_DELETE_WORDS 0

typedef struct {
  C_ALL common;
  WORD  x;
  WORD  y;
} C_WIND_FIND;

#define C_WIND_FIND_WORDS 2

typedef struct {
  R_ALL common;
} R_WIND_FIND;

#define R_WIND_FIND_WORDS 0

typedef struct {
  C_ALL common;
  WORD  handle;
  WORD  mode;
  WORD  parm1;
}C_WIND_GET;

#define C_WIND_GET_WORDS 3

typedef struct {
  R_ALL common;
  WORD  parm1;
  WORD  parm2;
  WORD  parm3;
  WORD  parm4;
}R_WIND_GET;

#define R_WIND_GET_WORDS 4

typedef struct {
  C_ALL common;
}C_WIND_NEW;

#define C_WIND_NEW_WORDS 0

typedef struct {
  R_ALL common;
}R_WIND_NEW;

#define R_WIND_NEW_WORDS 0

typedef struct {
  C_ALL common;
  WORD  handle;
  WORD  mode;
  WORD  parm1;
  WORD  parm2;
  WORD  parm3;
  WORD  parm4;
}C_WIND_SET;

#define C_WIND_SET_WORDS 6

typedef struct {
  R_ALL common;
}R_WIND_SET;

#define R_WIND_SET_WORDS 0

typedef struct {
  C_ALL common;
  WORD  id;
  RECT  size;
}C_WIND_OPEN;

#define C_WIND_OPEN_WORDS 5

typedef struct {
  R_ALL common;
}R_WIND_OPEN;

#define R_WIND_OPEN_WORDS 0

typedef struct {
  C_ALL common;
  WORD  mode;
}C_WIND_UPDATE;

#define C_WIND_UPDATE_WORDS 1

typedef struct {
  R_ALL common;
}R_WIND_UPDATE;

#define R_WIND_UPDATE_WORDS 0

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

typedef struct {
  C_ALL common;
  ULONG amount;
} C_MEMORY_ALLOC;

#define C_MEMORY_ALLOC_WORDS 0

typedef struct
{
  R_ALL common;
  ULONG address;
} R_MEMORY_ALLOC;

#define R_MEMORY_ALLOC_WORDS 0

typedef struct
{
  C_ALL common;
  ULONG address;
} C_MEMORY_FREE;

#define C_MEMORY_FREE_WORDS 0

typedef struct
{
  R_ALL common;
} R_MEMORY_FREE;

#define R_MEMORY_FREE_WORDS 0

#define MEMORY_BLOCK_SIZE 1024

typedef struct
{
  C_ALL common;
  WORD  amount;
  ULONG address;
} C_MEMORY_GET;

#define C_MEMORY_GET_WORDS 1
#define C_MEMORY_GET_LONGS 1

typedef struct
{
  R_ALL common;
  BYTE  data[MEMORY_BLOCK_SIZE];
} R_MEMORY_GET;

#define R_MEMORY_GET_WORDS 0
#define R_MEMORY_GET_LONGS 0

typedef struct
{
  C_ALL common;
  WORD  amount;
  ULONG address;
  BYTE  data[MEMORY_BLOCK_SIZE];
} C_MEMORY_SET;

#define C_MEMORY_SET_WORDS 1
#define C_MEMORY_SET_LONGS 1

typedef struct
{
  R_ALL common;
} R_MEMORY_SET;

#define R_MEMORY_SET_WORDS 0
#define R_MEMORY_SET_LONGS 0


typedef union
{
  C_ALL           common;
  C_APPL_CONTROL  appl_control;
  C_APPL_EXIT     appl_exit;
  C_APPL_FIND     appl_find;
  C_APPL_INIT     appl_init;
  C_APPL_RESERVE  appl_reserve;
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
  C_MEMORY_ALLOC  memory_alloc;
  C_MEMORY_FREE   memory_free;
  C_MEMORY_SET    memory_set;
  C_MEMORY_GET    memory_get;
} C_SRV;


typedef union {
  R_ALL           common;
  R_APPL_CONTROL  appl_control;
  R_APPL_EXIT     appl_exit;
  R_APPL_FIND     appl_find;
  R_APPL_INIT     appl_init;
  R_APPL_RESERVE  appl_reserve;
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
  R_MEMORY_ALLOC  memory_alloc;
  R_MEMORY_FREE   memory_free;
  R_MEMORY_SET    memory_set;
  R_MEMORY_GET    memory_get;
} R_SRV;


#define FIX_ENDIAN(parameters,routine,n_o_words) \
{ \
  int    i; \
  void * walk = parameters; \
  int    words = n_o_words; \
\
  for(i = 0; i < words; i++) \
  { \
    *(UWORD *)walk = routine##s(*(UWORD *)walk); \
    ((UWORD *)walk)++; \
  } \
}

#define PUT_C_ALL(callname,parameters) \
{ \
  (parameters)->common.words = C_ALL_WORDS + C_##callname##_WORDS; \
  (parameters)->common.apid = apid; \
  (parameters)->common.pid  = getpid(); \
  (parameters)->common.call = SRV_##callname; \
}

#define PUT_C_ALL_W(callname,parameters,words_in) \
{ \
  (parameters)->common.words = words_in; \
  (parameters)->common.apid = apid; \
  (parameters)->common.pid  = getpid(); \
  (parameters)->common.call = SRV_##callname; \
}

#ifndef WORDS_BIGENDIAN
#define HTON(parameters)
#else
#define HTON(parameters) FIX_ENDIAN(parameters,hton,(parameters)->common.words)
#endif

#ifndef WORDS_BIGENDIAN
#define NTOH(parameters)
#else
#define NTOH(parameters) FIX_ENDIAN(parameters,ntoh,ntohs((parameters)->common.words))
#endif

#define PUT_R_ALL(callname,parameters,retval_in) \
{ \
  (parameters)->common.words = R_ALL_WORDS + R_##callname##_WORDS; \
  (parameters)->common.retval = retval_in; \
}

#define PUT_R_ALL_W(callname,parameters,words_in) \
{ \
  (parameters)->common.words = words_in; \
  (parameters)->common.retval = retval; \
}

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
