/*
** srv.c
**
** Copyright 1996 - 2000 Christer Gustavsson <cg@nocrew.org>
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
#include "lib_comm.h"
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


static WORD nocnf;

/* When this is set to false the server will exit */
static volatile WORD run_server = TRUE;
static volatile WORD server_running = FALSE;

void
Srv_exit_module (void);

/*
** Description
** This is the server itself
*/
static
WORD
server(LONG arg)
{
#ifndef SERVER_AS_DEVICE
  /* These variables are passed from clients */
  C_SRV         par;
  COMM_HANDLE   handle;
#endif

  /* Stop warnings from compiler about unused parameters */
  NOT_USED(arg);

#ifndef SERVER_AS_DEVICE
  /* Initialize message handling */
  DEBUG3 ("srv.c: Initializing server socket");
  Srv_open ();
#endif

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

  server_running = TRUE;

  while(run_server)
  {
#ifdef SERVER_AS_DEVICE
    DEBUG3("srv.c: usleep");
    usleep(10000);
    DEBUG3("srv.c: srv_update_keys");
    srv_update_keys(globals.vid);
    DEBUG3("srv.c: Srv_poll_events");
    Srv_poll_events();
#else
    /* Wait for another call from a client */
    DEBUG3("srv.c: Waiting for message from client");
    SRV_GET(&par, sizeof (C_SRV), handle);
    DEBUG3("srv.c: Got message from client (%p)", handle);

    if (handle != NULL) {
      NTOH(&par);

      srv_call(handle,
               &par);
    }

    DEBUG3 ("srv.c: calling srv_handle_events");

    srv_update_keys(globals.vid);
    srv_handle_events();
#endif
  }

  DEBUG2("Calling Srv_exit_module");
  Srv_exit_module();

  DEBUG2("Calling srv_term");
  srv_term (0);

  return 0;
}

/*
** Description
** Initialize server module
*/
void
Srv_init_module (WORD no_config) {
  WORD i;
  
  nocnf = no_config;


  /* FIXME: Move initializations to server process */
  for(i = 0; i < MAX_NUM_APPS; i++)
  {
    apps[i].id = -1;
    apps[i].deskbg = NOT_INSTALLED;
    apps[i].menu = NOT_INSTALLED;
  }

#ifdef SERVER_AS_DEVICE
  DEBUG3 ("srv.c: Srv_init_module: In Srv_init_module");

  DEBUG3("comm_init\n");
  comm_init();
  DEBUG3("comm_init called\n");
#endif

  DEBUG3 ("Starting server process");
  globals.srvpid = (WORD)srv_fork(server, 0, "oAESsrv");
  DEBUG3 ("Started server process");

  /* Wait for the server to initialize itself FIXME */
  while(!server_running)
  {
    sleep(1);
  }

  DEBUG3 ("Slept for a while");
}


/*
** Description
** Shutdown server module
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

#ifdef SERVER_AS_DEVICE
  comm_exit();
#endif

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
