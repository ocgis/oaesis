/*
** srv_get_pmsg.c
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_MINTBIND_H
#include <mintbind.h>
#endif

#include <stdio.h>

#include "mintdefs.h"
#include "srv_get.h"
#include "srv_pmsg.h"


/*
** Description
** Initialize communication
*/
void
Srv_open(void)
{
  /* Nothing needs to be done here */
}


/*
** Description
** Get a message from an client to the server
*/
COMM_HANDLE
Srv_get(void * in,
	int    max_bytes_in)
{
  PMSG * handle = (PMSG *)Malloc(sizeof(PMSG)); /* FIXME */
  int    i = 0;
  int    err;

  while(i++ < 20)
  {
    err = Pmsg(MSG_READ | MSG_NOWAIT, SRVBOX, handle);

    if(err == 0)
    {
      break;
    }

    (void)Syield();
  }

  if(err == 0)
  {
    memcpy(in,
	   handle->par->in,
	   (max_bytes_in < handle->bytes_in) ?
	   max_bytes_in : handle->bytes_in);
	   
    return (COMM_HANDLE)handle;
  }
  else
  {
    Mfree(handle);

    return COMM_HANDLE_NIL;
  }
}


/*
** Description
** Reply to a client that has sent a message to the server
*/
void
Srv_reply(COMM_HANDLE handle,
	  void *      out,
	  WORD        bytes_out)
{
  ((PMSG *)handle)->bytes_out = (bytes_out < ((PMSG *)handle)->bytes_out) ?
                                bytes_out :
                                ((PMSG *)handle)->bytes_out;

  memcpy(((PMSG *)handle)->par->out, out, ((PMSG *)handle)->bytes_out);
  Pmsg (MSG_WRITE, 0xffff0000L | ((PMSG *)handle)->pid, handle);

  /* FIXME */
  Mfree(handle);
}
