/*
** srv_put_pmsg.c
**
** Copyright 1999 Christer Gustavsson <cg@nocrew.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**  
** Read the file COPYING for more information.
*/

#define DEBUGLEVEL 0

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_MINTBIND_H
#include <mintbind.h>
#endif

#include "debug.h"
#include "mintdefs.h"
#include "srv_pmsg.h"
#include "srv_put.h"


/*
** Exported
*/
LONG
Client_open(void)
{
  /* Nothing needs to be done here: just return OK */
  return 0;
}


/*
** Exported
*/
LONG
Client_send_recv(void * in,
		 int    bytes_in,
		 void * out,
		 int    max_bytes_out)
{
  PMSG msg;
  DATA par;

  msg.par = &par;
  msg.par->in = in;
  msg.par->out = out;
  msg.bytes_in = bytes_in;
  msg.bytes_out = max_bytes_out;

  Pmsg(MSG_READWRITE, SRVBOX, &msg);

  return msg.bytes_out;
}
