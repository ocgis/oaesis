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
