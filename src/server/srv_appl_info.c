/*
** srv_appl_info.c
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

#include <stdlib.h>
#include "oconfig.h"
#include "srv_appl_info.h"

AP_INFO     apps[MAX_NUM_APPS];
AP_LIST_REF ap_pri = NULL;


/*
** Exported
**
** 1998-12-08 CG
*/
AP_INFO_REF
get_appl_info (AP_LIST_REF element) {
  return element->ai;
}


/*
** Exported
**
** 1998-12-08 CG
*/
AP_LIST_REF
next_appl_list_element (AP_LIST_REF element) {
  return element->next;
}


/*
** Description
** Find MiNT-PID & return AP_LIST entry for that 
*/
AP_LIST_REF
search_mpid(WORD pid)
{
  AP_LIST_REF al;
  
  al = ap_pri;
  
  while(al) 
  {
    if( al->ai->pid == pid) 
			break;
    al = al->next;
  }
  return al;	
}


/*
** Exported
**
** 1998-12-08 CG
*/
AP_INFO *
search_appl_info(WORD apid) {
  switch(apid) {
  case TOP_APPL:
    if(ap_pri) {
      return ap_pri->ai;
    } else {
      return NULL;
    }

  case DESK_OWNER:
  {
    AP_LIST *al = ap_pri;
			
    while(al) {
      if(al->ai->deskbg) {
        return al->ai;
      }
				
      al = al->next;
    }
  }
  return NULL;

  case TOP_MENU_OWNER:
  {
    AP_LIST *al = ap_pri;
			
    while(al) {
      if(al->ai->menu) {
        return al->ai;
      };
				
      al = al->next;
    }
  }	
  return NULL;
	
  default:
    if(apps[apid].id == apid) {
      return &apps[apid];
    }
    return NULL;
  }
}


/*
** Description
** Free AP_INFO structure of apid
*/
void
apinfofree(WORD id)
{
  AP_LIST_REF * al;
  
  al = &ap_pri;
  
  while(*al)
  {
    if((*al)->ai->id == id)
    {
      AP_LIST	*altemp = *al;
      
      *al = (*al)->next;
      
      altemp->ai->vid = -1;
      altemp->ai->msgpipe = -1;
      altemp->ai->eventpipe = -1;
      
      if(altemp->ai->rshdr)
      {
        Mfree(altemp->ai->rshdr);	/*free resource memory*/
      }
      
      altemp->ai->id = -1;
      /* FIXME Mfree(altemp);*/
      
      break;
    }
    al = &(*al)->next;
  }
}


/*
** Description
** Get currently topped application
*/
WORD
get_top_appl(void)
{
  if(ap_pri)
  {
    return ap_pri->ai->id;
  }
  else
  {
    return -1;
  }
}
