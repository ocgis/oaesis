/*
** srv_call.c
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

#define DEBUGLEVEL 0

#include "srv_trace.h"

/*
#undef  TRACE
#define TRACE (*kerinf->trace)
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

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <vdibind.h>

#include "aesbind.h"
#include "oconfig.h"
#include "debug.h"
#include "lxgemdos.h"
#include "mesagdef.h"
#include "mintdefs.h"
#include "srv_appl.h"
#include "srv_appl_info.h"
#include "srv_call.h"
#include "srv_comm.h"
#include "srv_comm_device.h"
#include "srv_event.h"
#include "srv_global.h"
#include "srv_misc.h"
#include "resource.h"
#include "rlist.h"
#include "srv.h"
#include "srv_event.h"
#include "srv_get.h"
#include "srv_interface.h"
#include "srv_menu.h"
#include "srv_wind.h"
#include "types.h"

/* Description
** Copy MFDB with network order conversion
*/
static
void
srv_copy_mfdb (MFDB * dst,
               MFDB * src) {
  dst->fd_addr = (void *)ntohl ((long)src->fd_addr);
  dst->fd_w = ntohs (src->fd_w);
  dst->fd_h = ntohs (src->fd_h);
  dst->fd_wdwidth = ntohs (src->fd_wdwidth);
  dst->fd_stand = ntohs (src->fd_stand);
  dst->fd_nplanes = ntohs (src->fd_nplanes);
  dst->fd_r1 = ntohs (src->fd_r1);
  dst->fd_r2 = ntohs (src->fd_r2);
  dst->fd_r3 = ntohs (src->fd_r3);
}


/*
** Description
** Do a vdi call requested by a client
*/
static
void
srv_vdi_call (COMM_HANDLE  handle,
              C_VDI_CALL * par)
{
  R_VDI_CALL       ret;
  static VDIPARBLK e_vdiparblk;
  static VDIPB     vpb = { e_vdiparblk.contrl,
                           e_vdiparblk.intin, e_vdiparblk.ptsin,
                           e_vdiparblk.intout, e_vdiparblk.ptsout };
  MFDB             mfdb_src;
  MFDB             mfdb_dst;
  int              i;
  int              j;
  int              retval = 0;

  /* Copy contrl array */
  for (i = 0; i < 15; i++) {
    vpb.contrl[i] = par->contrl[i];
  }
  
  /* Copy ptsin array */
  for (i = 0, j = 0; i < (vpb.contrl[1] * 2); i++, j++)
  {
    vpb.ptsin[i] = par->inpar[j];
  }
  
  /* Copy intin array */
  for (i = 0; i < vpb.contrl[3]; i++, j++)
  {
    vpb.intin[i] = par->inpar[j];
  }

  /* Copy MFDBs when available */
  if ((vpb.contrl[0] == 109)  ||  /* vro_cpyfm */
      (vpb.contrl[0] == 110)  ||  /* vr_trnfm  */
      (vpb.contrl[0] == 121))     /* vrt_cpyfm */
  {
    srv_copy_mfdb (&mfdb_src, (MFDB *)&par->inpar[j]);
    j += sizeof (MFDB) / 2;

    srv_copy_mfdb (&mfdb_dst, (MFDB *)&par->inpar[j]);

    *(MFDB **)&vpb.contrl[7] = &mfdb_src;
    *(MFDB **)&vpb.contrl[9] = &mfdb_dst;
  }

  /* Reset INTOUT and PTSOUT count in case the call fails */
  vpb.contrl[2] = 0;
  vpb.contrl[4] = 0;

  DEBUG3("srv.c: vdi_call %d", vpb.contrl[0]);
  vdi_call (&vpb);
  DEBUG3("srv.c: vdi_call %d finished", vpb.contrl[0]);
  
  /* Copy contrl array */
  for (i = 0; i < 15; i++) {
    ret.contrl[i] = vpb.contrl[i];
  }

  /* Copy ptsout array */
  for (i = 0, j = 0; i < (vpb.contrl[2] * 2); i++, j++) {
    ret.outpar[j] = vpb.ptsout[i];
  }
  
  /* Copy intout array */
  for(i = 0; i < vpb.contrl[4]; i++, j++) {
    ret.outpar[j] = vpb.intout[i];
  }

  PUT_R_ALL_W(VDI_CALL,&ret,R_ALL_WORDS + 15 + j);
  SRV_REPLY(handle,
            &ret,
            sizeof (R_ALL) +
            sizeof (WORD) * (15 + j));
}


/*
** Description
** Handle incoming server request
*/
void
srv_call(COMM_HANDLE handle,
         C_SRV *     par)
{
  R_SRV ret;
  char  deb[100];

  sprintf(deb, "srv_call: %d", par->common.call);
  TRACE(deb);

  DEBUG2 ("srv.c: Call no %d 0x%x", par->common.call, par->common.call);
  switch (par->common.call) {
  case SRV_APPL_CONTROL:
    srv_appl_control (&par->appl_control, &ret.appl_control);
    
    SRV_REPLY(handle, &ret, sizeof (R_APPL_CONTROL));
    break;
    
  case SRV_APPL_EXIT:
    srv_appl_exit (&par->appl_exit, &ret.appl_exit);
    
    SRV_REPLY(handle, &ret, sizeof (R_APPL_EXIT));
    break;
    
  case SRV_APPL_FIND:
    srv_appl_find (&par->appl_find, &ret.appl_find);
    
    SRV_REPLY(handle, &ret, sizeof (R_APPL_FIND));
    break;
    
  case SRV_APPL_INIT:
    srv_appl_init (handle,
                   &par->appl_init,
                   &ret.appl_init);
    TRACE("Replying for srv_appl_init");
    SRV_REPLY(handle, &ret, sizeof (R_APPL_INIT));
    TRACE("Replyed for srv_appl_init");
    break;
        
  case SRV_APPL_RESERVE:
    srv_appl_reserve(&par->appl_reserve, &ret.appl_reserve);
    TRACE("Replying for srv_appl_reserve");
    SRV_REPLY(handle, &ret, sizeof (R_APPL_RESERVE));
    TRACE("Replyed for srv_appl_reserve");
    break;
    
  case SRV_APPL_SEARCH:
    srv_appl_search (&par->appl_search, &ret.appl_search);
    
    SRV_REPLY(handle, &ret, sizeof (R_APPL_SEARCH));
    break;
    
  case SRV_APPL_WRITE:
    srv_appl_write (&par->appl_write, &ret.appl_write);
    
    SRV_REPLY(handle, &ret, sizeof (R_APPL_WRITE));
    break;
    
  case SRV_EVNT_MULTI:
    srv_wait_for_event (handle, &par->evnt_multi);
    break;
    
  case SRV_GRAF_MKSTATE :
    srv_graf_mkstate (&par->graf_mkstate, &ret.graf_mkstate);
    
    SRV_REPLY(handle, &ret, sizeof (R_GRAF_MKSTATE));
    break;

  case SRV_GRAF_MOUSE :
    srv_graf_mouse(&par->graf_mouse, &ret.graf_mouse);
    
    SRV_REPLY(handle, &ret, sizeof (R_GRAF_MOUSE));
    break;

  case SRV_MENU_BAR:
    srv_menu_bar (&par->menu_bar, &ret.menu_bar);
    
    SRV_REPLY(handle, &ret, sizeof (R_MENU_BAR));
    break;
    
  case SRV_MENU_REGISTER:
    srv_menu_register (&par->menu_register, &ret.menu_register);
    
    SRV_REPLY(handle, &ret, sizeof (R_MENU_REGISTER));
    break;
    
  case SRV_WIND_CLOSE:
    srv_wind_close (&par->wind_close, &ret.wind_close);
    
    SRV_REPLY(handle, &ret, sizeof (R_WIND_CLOSE));
    break;
    
  case SRV_WIND_CREATE:
    srv_wind_create (&par->wind_create, &ret.wind_create);
    
    SRV_REPLY(handle, &ret, sizeof (R_WIND_CREATE));
    break;
    
  case SRV_WIND_DELETE:
    srv_wind_delete (&par->wind_delete,
                     &ret.wind_delete);
    
    SRV_REPLY(handle, &ret, sizeof (R_WIND_DELETE));
    break;
    
  case SRV_WIND_FIND:
    srv_wind_find (&par->wind_find,
                   &ret.wind_find);
    
    SRV_REPLY(handle, &ret, sizeof (R_WIND_FIND));
    break;
    
  case SRV_WIND_GET:
    srv_wind_get (&par->wind_get,
                  &ret.wind_get);
    
    SRV_REPLY(handle, &ret, sizeof (R_WIND_GET));
    break;
    
  case SRV_WIND_NEW:
    ret.common.retval = 
      srv_wind_new (par->wind_new.common.apid);
    PUT_R_ALL(WIND_NEW, &ret, ret.common.retval);
    
    SRV_REPLY(handle, &ret, sizeof (R_WIND_NEW));
    break;
    
  case SRV_WIND_OPEN:
    srv_wind_open (&par->wind_open, &ret.wind_open);
    
    SRV_REPLY(handle, &ret, sizeof (R_WIND_OPEN));
    break;
        
  case SRV_WIND_SET:
    srv_wind_set (&par->wind_set, &ret.wind_set);
    
    SRV_REPLY(handle, &ret, sizeof (R_WIND_SET));
    break;
    
  case SRV_WIND_UPDATE:
    srv_wind_update (handle,
                     &par->wind_update);
    break;
    
  case SRV_VDI_CALL:
    srv_vdi_call (handle,
                  &par->vdi_call);
    break;
    
  case SRV_MEMORY_ALLOC:
    ret.memory_alloc.address = (ULONG)malloc(par->memory_alloc.amount);
    PUT_R_ALL(MEMORY_ALLOC, &ret, 0);
    SRV_REPLY(handle, &ret, sizeof (R_MEMORY_ALLOC));
    break;
    
  case SRV_MEMORY_FREE:
    free((void *)par->memory_free.address);
    
    PUT_R_ALL(MEMORY_FREE, &ret, 0);
    SRV_REPLY(handle, &ret, sizeof (R_MEMORY_FREE));
    break;
    
  case SRV_MEMORY_SET:
    memcpy((void *)par->memory_set.address,
           &par->memory_set.data,
           par->memory_set.amount);
    PUT_R_ALL(MEMORY_SET, &ret, 0);
    SRV_REPLY(handle, &ret, sizeof(R_MEMORY_SET));
    break;

  case SRV_MEMORY_GET:
    memcpy(&ret.memory_get.data,
           (void *)par->memory_get.address,
           par->memory_get.amount);
    PUT_R_ALL(MEMORY_GET, &ret, 0);
    SRV_REPLY(handle, &ret, sizeof(R_MEMORY_GET));
    break;

  case SRV_CONN_LOST:
    {
      AP_LIST_REF al;

      al = search_comm_handle(handle);

      if(al != AP_LIST_REF_NIL)
      {
        AP_INFO_REF ai;

        ai = get_appl_info(al);

        if(ai != AP_INFO_REF_NIL)
        {
          C_APPL_EXIT c_appl_exit;
          R_APPL_EXIT r_appl_exit;

          c_appl_exit.common.apid = ai->id;

          srv_appl_exit(&c_appl_exit,
                        &r_appl_exit);
        }
      }
    }
    break;

  default:
    DEBUG1("%s: Line %d:\r\n"
           "Unknown call %d (0x%x) to server!",
           __FILE__, __LINE__, par->common.call, par->common.call);
    SRV_REPLY(handle, par, sizeof(R_ALL)); /* FIXME */
  }

  TRACE("srv_call returned");
}
