/*
** srv.c
**
** Copyright 1996 - 1999 Christer Gustavsson <cg@nocrew.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**  
** Read the file COPYING for more information.
*/

#define DEBUGLEVEL 0

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
#include "srv_comm.h"
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


/****************************************************************************
 * Macros                                                                   *
 ****************************************************************************/


#define SRV_QUE_SIZE 32

#define IMOVER 0x8000  /* Used with set_win_elem() to make icon window */

#define D3DSIZE       2

/****************************************************************************
 * Module global variables                                                  *
 ****************************************************************************/

static WORD nocnf;


/* When this is set to false the server will exit */
static volatile WORD run_server = TRUE;

/* FIXME This has to be fixed */
#ifndef MINT_TARGET
void accstart (void) {}
#endif


void
Srv_exit_module (void);



/* Description
** Copy MFDB with network order conversion
**
** 1999-05-23 CG
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
              C_VDI_CALL * par) {
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
  
  DEBUG3("srv_vdi_call: Call no %d (0x%x)", vpb.contrl[0], vpb.contrl[0]);

  /* Copy ptsin array */
  for (i = 0, j = 0; i < (vpb.contrl[1] * 2); i++, j++) {
    vpb.ptsin[i] = par->inpar[j];
  }
  
  /* Copy intin array */
  for (i = 0; i < vpb.contrl[3]; i++, j++) {
    vpb.intin[i] = par->inpar[j];
  }

  /* Copy MFDBs when available */
  if ((vpb.contrl[0] == 109)  ||  /* vro_cpyfm */
      (vpb.contrl[0] == 110)  ||  /* vr_trnfm  */
      (vpb.contrl[0] == 121)) {   /* vrt_cpyfm */
    srv_copy_mfdb (&mfdb_src, (MFDB *)&par->inpar[j]);
    j += sizeof (MFDB) / 2;
    srv_copy_mfdb (&mfdb_dst, (MFDB *)&par->inpar[j]);
    vpb.contrl[7] = MSW(&mfdb_src);
    vpb.contrl[8] = LSW(&mfdb_src);
    vpb.contrl[9] = MSW(&mfdb_dst);
    vpb.contrl[10] = LSW(&mfdb_dst);
  }

  vdi_call (&vpb);
  
  /* Copy contrl array */
  for (i = 0; i < 15; i++) {
    ret.contrl[i] = vpb.contrl[i];
  }
  
  /* Copy ptsout array */
  for (i = 0, j = 0; i < (vpb.contrl[2] * 2); i++, j++) {
    ret.outpar[j] = vpb.ptsout[i];
  }
  
  /* Copy intout array */
  for (i = 0; i < vpb.contrl[4]; i++, j++) {
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
static
inline
void
srv_call(COMM_HANDLE handle,
         C_SRV *     par)
{
  R_SRV ret;

  DEBUG3 ("srv.c: Call no %d 0x%x", par->common.call, par->common.call);
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
    srv_appl_init (&par->appl_init, &ret.appl_init);
    
    SRV_REPLY(handle, &ret, sizeof (R_APPL_INIT));
    break;
        
  case SRV_APPL_RESERVE:
    srv_appl_reserve(&par->appl_reserve, &ret.appl_reserve);
    
    SRV_REPLY(handle, &ret, sizeof (R_APPL_RESERVE));
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
    srv_graf_mouse (globals.vid, &par->graf_mouse, &ret.graf_mouse);
    
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
    
  case SRV_MALLOC:
    ret.malloc.address = (ULONG)malloc (par->malloc.amount);
    PUT_R_ALL(MALLOC, &ret, 0);
    SRV_REPLY(handle, &ret, sizeof (R_MALLOC));
    break;
    
  case SRV_FREE:
    free ((void *)par->free.address);
    
    PUT_R_ALL(FREE, &ret, 0);
    SRV_REPLY(handle, &ret, sizeof (R_FREE));
    break;
    
  default:
    DB_printf("%s: Line %d:\r\n"
              "Unknown call %d (0x%x) to server!",
              __FILE__, __LINE__, par->common.call, par->common.call);
    SRV_REPLY(handle, par, -1); /* FIXME */
  }
}


/*
** Description
** This is the server itself
*/
static
WORD
server (LONG arg) {
  /* These variables are passed from clients */
  C_SRV         par;
  COMM_HANDLE   handle;

  /* Stop warnings from compiler about unused parameters */
  NOT_USED(arg);

  /* Initialize message handling */
  DEBUG3 ("srv.c: Initializing server socket");
  Srv_open ();

  /* Initialize global variables */
  DEBUG3 ("srv.c: Initializing global variables");
  srv_init_global (nocnf);

  /* Initialize desktop and menu bar windows */
  srv_init_windows();
  
  DEBUG3 ("srv.c: Intializing event handler");
  /* Setup event vectors */
  srv_init_event_handler (globals.vid);

  DEBUG3 ("srv.c: Show mouse cursor");
  /* Show mouse cursor */
  v_show_c (globals.vid, 1);

  while (run_server) {
    /* Wait for another call from a client */
    DEBUG3 ("srv.c: Waiting for message from client");
    SRV_GET (&par, sizeof (C_SRV), handle);
    DEBUG3 ("srv.c: Got message from client (%p)", handle);

    if (handle != NULL) {
      NTOH(&par);

      srv_call(handle,
               &par);
    }

    DEBUG3 ("srv.c: calling srv_handle_events");
    srv_handle_events (globals.vid);
  }

  DEBUG2("Calling Srv_exit_module");
  Srv_exit_module();

  DEBUG2("Calling srv_term");
  srv_term (0);

  return 0;
}

/****************************************************************************
 * Public functions                                                         *
 ****************************************************************************/

/*
** Description
** Initialize server module
*/
void
Srv_init_module (WORD no_config) {
  WORD i;
  
  nocnf = no_config;

  DEBUG3 ("srv.c: Srv_init_module: In Srv_init_module");

  /* FIXME: Move initializations to server process */
  for(i = 0; i < MAX_NUM_APPS; i++) {
    apps[i].id = -1;
    apps[i].eventpipe = -1;
    apps[i].msgpipe = -1;
    apps[i].rshdr = NULL;
    apps[i].deskbg = NULL;
    apps[i].menu = NULL;
    apps[i].deskmenu = -1;
  }
  
  DEBUG3 ("Starting server process");
  globals.srvpid = (WORD)srv_fork(server,0,"oAESsrv");
  DEBUG3 ("Started server process");
}


/*
** Description
** Shutdown server module
**
** 1998-12-22 CG
** 1999-01-09 CG
** 1999-04-18 CG
** 1999-05-20 CG
** 1999-07-26 CG
*/
void
Srv_exit_module (void) {
  DEBUG2("Killing off alive processes");
  
  /* Kill all AES processes */
  while (ap_pri) {
    C_APPL_CONTROL msg;
    R_APPL_CONTROL ret;

    msg.ap_id = ap_pri->ai->id;
    msg.mode = APC_KILL;

    srv_appl_control (&msg, &ret);
  }

  srv_exit_global ();
}


/*
** Description
** Request the server to stop
*/
void
Srv_stop(void)
{
  DEBUG2("Assigning run_server FALSE");
  run_server = FALSE;

  /* FIXME Give the server some time to finish */
  sleep(1);
}
