/****************************************************************************

 Module
  srv_call.c
  
 Description
  Server calling interface for oAESis.
  
 Author(s)
  cg (Christer Gustavsson <d2cg@dtek.chalmers.se>)
 	
 ****************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_MINTBIND_H
#include <mintbind.h>
#endif

#include <stdio.h>

#include "mintdefs.h"
#include "srv_get.h"

#define SRVBOX 0x6f535256l /*'oSRV'*/

typedef struct {
  union {
    WORD call;
    WORD retval;
  } cr;

  WORD    apid;
  void    *spec;
  WORD    pid;
} PMSG;


/*
** Description
** Get a message from an client to the server
**
** 1998-09-05 CG
*/
void *
Srv_get (WORD * apid,
         WORD * pid,
         WORD * call,
         void * spec) {
  static PMSG handle;
  int         err;

  err = Pmsg (MSG_READ, SRVBOX, &handle);

  /* Did it work ok? */
  if (err < 0) {
    fprintf (stderr, "oaesis: srv_get_pmsg.c: Srv_get: Got error message from Pmsg\n");
  }

  /* Copy data that we received */
  *apid = handle.apid;
  *pid = handle.pid;
  *call = handle.cr.call;

  /* TBD Copy stuff from handle.spec to spec */

  return &handle;
}


/*
** Description
** Reply to a client that has sent a message to the server
**
** 1998-09-05 CG
*/
void
Srv_reply (void * handle,
           void * spec,
           WORD   code)
{
  Pmsg (MSG_WRITE, 0xffff0000L | ((PMSG *)handle)->pid, handle);
}
