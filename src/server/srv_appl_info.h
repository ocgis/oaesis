/*
** srv_appl_info.h
**
** Copyright 1998 Christer Gustavsson <cg@nocrew.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**  
** Read the file COPYING for more information.
*/

#ifndef _SRV_APPL_INFO_H_
#define _SRV_APPL_INFO_H_

#include "oconfig.h"
#include "types.h"

/* appl_* related */
#define TOP_APPL       (-1)
#define DESK_OWNER     (-2)
#define TOP_MENU_OWNER (-3)

#define MSG_BUFFER_SIZE 1024

typedef enum
{
  NOT_INSTALLED = 0,
  INSTALLED     = 1
}SRV_FEATURE;

typedef struct ap_list * AP_LIST_REF;
#define AP_LIST_REF_NIL ((AP_LIST_REF)NULL)

typedef struct ap_info
{
  WORD        id;        /* Application id                                  */
  WORD        pid;       /* Process id of the main process of the app       */
  SRV_FEATURE deskbg;    /* Tells if application has installed a desktop    */
  SRV_FEATURE menu;      /* Tells if application has installed a menu       */
  WORD        newmsg;    /* Indicates which messages that are understood    */
  WORD        killtries; /* Number of times someone has tried to kill the   */
                         /* application.                                    */
  WORD        type;      /* Application type (acc or app etc)               */
  AP_LIST_REF ap_search_next; /* appl_search() pointer to next app          */
  BYTE        name[21];  /* Pretty name of process, init. filename          */
  BYTE        message_buffer[MSG_BUFFER_SIZE];
  WORD        message_head;
  WORD        message_size;
} AP_INFO;

typedef AP_INFO * AP_INFO_REF;
#define AP_INFO_REF_NIL ((AP_INFO_REF)NULL)


typedef struct ap_list {
  AP_INFO        *ai;
  struct ap_list *next;
  struct ap_list *mn_next; /* menu link */
}AP_LIST;

/* Global variables. These should be removed from here. FIXME */
extern AP_INFO     apps[MAX_NUM_APPS];
extern AP_LIST_REF ap_pri;

/*
** Description
** Extract AP_INFO_REF from AP_LIST_REF
**
** 1998-12-08 CG
*/
AP_INFO_REF
get_appl_info (AP_LIST_REF element);

/*
** Description
** Get the next element of an AP_LIST_REF
**
** 1998-12-08 CG
*/
AP_LIST_REF
next_appl_list_element (AP_LIST_REF element);

/*
** Description
** Find AES-id & return AP_LIST entry for that
*/
AP_LIST_REF
search_apid(WORD apid);

/*
** Description
** Find MiNT-PID & return AP_LIST entry for that 
*/
AP_LIST_REF
search_mpid(WORD pid);

/*
** Description
** Get internal information about application.
**
** 1998-12-08 CG
*/
AP_INFO_REF
search_appl_info (WORD apid);


/*
** Description
** Free AP_INFO structure of apid
*/
void
apinfofree(WORD id);

/*
** Description
** Get currently topped application
*/
WORD
get_top_appl(void);

#endif /* _SRV_EVENT_H_ */
