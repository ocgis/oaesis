/*
** lib_global.h
**
** Copyright 1996 - 2001 Christer Gustavsson <cg@nocrew.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**  
** Read the file COPYING for more information.
*/

#ifndef	__LIB_GLOBAL__
#define	__LIB_GLOBAL__

#include "config.h"
#include "oaesis.h"
#include "types.h"

#define	STKSIZE	4096

/* Semaphore used in Shel_do_write under MiNT */
#define SHEL_WRITE_LOCK 0x6f415357L /* oASW */

/*
** global_common contains variables that are global across applications.
*/
typedef struct global_common {
  WORD    vmode;
  WORD    vmodecode;
  LONG    video;
  RECT    screen;     /* size of physical screen */	
  WORD    num_planes; /* number of bitplanes in the display */

  WORD    realmove;   /* if set realtime moving of windows is enabled */
  WORD    realsize;
  WORD    realslide;  /* if set realtime sliding of windows is enabled */
  WORD    wind_appl;  /* show application name in window mover */

  WORD fsel_extern;    /* set 1 to enable extern fileselectors */
  WORD fsel_sorted;    /* set 1 to have sorted directories with internal fileselecto r*/

  WORD graf_mbox;      /* set 1 to enable graf_mbox/graf_movebox */
  WORD graf_growbox;   /* set 1 to enable graf_growbox */
  WORD graf_shrinkbox; /* set 1 to enable graf_shrinkbox */

  WORD aes_trace;      /* set 1 to trace aes calls */

  int     physical_vdi_id;
  WORD    num_pens;   /* number of available vdi pens */

  WORD    mouse_x;
  WORD    mouse_y;
  WORD    mouse_button;
	
  WORD    arrowrepeat;
  ULONG   time;
	
  WORD    cswidth;
  WORD    csheight;
  WORD    bswidth;
  WORD    bsheight;

  WORD    clwidth;
  WORD    clheight;
  WORD    blwidth;
  WORD    blheight;

  WORD    fnt_small_id; /* id & size of system fonts */
  WORD    fnt_small_sz;
  WORD    fnt_regul_id;
  WORD    fnt_regul_sz;

  WORD    icon_width;   /* Width of iconified window */
  WORD    icon_height;  /* Height of iconified window */

  OBJECT  *aiconstad;
  OBJECT  *alerttad;
  OBJECT  *deskbgtad;
  OBJECT  *fiseltad;
  OBJECT  *informtad;
  OBJECT  *menutad;
  OBJECT  *mouseformstad;
  OBJECT  *pmenutad;
  BYTE    **fr_string;

  /* Window elements resource and counters */
  OBJECT  * windowtad;
  WORD      elemnumber;
  WORD      tednumber;

  WORD    mouse_owner;
  WORD    mouse_mode;

  WORD    top_colour[20];
  WORD    untop_colour[20];

#ifndef MINT_TARGET
  WORD           (*callback_handler)(void *, void *);
#endif
}GLOBAL_COMMON;


typedef struct {
  char name[20];
  WORD type;
  WORD ap_id;
} APPL_ENTRY;

typedef struct {
  WORD         count;
  WORD         size;
  APPL_ENTRY * entries;
} APPL_LIST;

/*
** Description
** global_appl contains variables that are global within a special application
*/
typedef struct global_appl
{
  GLOBAL_COMMON *  common;
  WORD             pid;
  WORD             vid;
  void *           windows;
  OBJECT *         desktop_background;
  OBJECT *         menu;
  RECT             menu_bar_rect;
  RSHDR *          rshdr;
  OAESIS_PATH_MODE path_mode;
  APPL_LIST        appl_menu;
  APPL_LIST        acc_menu;
  BYTE             application_name[20];
} GLOBAL_APPL;

/*
** Description
** Initialize global variables, open vdi workstation etc
**
** 1998-11-15 CG
** 1999-08-25 CG
*/
void
init_global (WORD physical_vdi_id);

/*
** Description
** Initialize application specific variables
*/
void
init_global_appl (int    apid,
		  int    physical_vdi_id,
                  char * appl_name);

void	exit_global(void);

GLOBAL_APPL   * get_globals (WORD apid);

extern GLOBAL_COMMON global_common;
#define get_global_common() (&global_common)

int check_apid(int apid);

#define CHECK_APID(apid) apid = check_apid(apid);

#ifdef WORDS_BIGENDIAN
#define CW_TO_HW(cw) (cw)
#define HW_TO_CW(hw) (hw)
#define CL_TO_HL(cl) ((LONG)cl)
#define HL_TO_CL(hl) ((LONG)hl)

#else /* WORDS_BIGENDIAN */

/* FIXME */
#include <netinet/in.h>

extern OAESIS_ENDIAN oaesis_client_endian;

#define CW_TO_HW(word) \
        ((oaesis_client_endian == OAESIS_ENDIAN_HOST) ? \
          (word) : \
          (oaesis_client_endian == OAESIS_ENDIAN_BIG) ? \
            ntohs((WORD)word) : \
            (word))

#define HW_TO_CW(word) \
        ((oaesis_client_endian == OAESIS_ENDIAN_HOST) ? \
          (word) : \
          (oaesis_client_endian == OAESIS_ENDIAN_BIG) ? \
            htons((WORD)word) : \
            (word))

#define CL_TO_HL(cl) \
        ((oaesis_client_endian == OAESIS_ENDIAN_HOST) ? \
          ((LONG)cl) : \
          (oaesis_client_endian == OAESIS_ENDIAN_BIG) ? \
            ntohl((LONG)cl) : \
            ((LONG)cl))

#define HL_TO_CL(hl) \
        ((oaesis_client_endian == OAESIS_ENDIAN_HOST) ? \
          ((LONG)hl) : \
          (oaesis_client_endian == OAESIS_ENDIAN_BIG) ? \
            htonl((LONG)hl) : \
            ((LONG)hl))

#endif /* WORDS_BIGENDIAN */

#endif
