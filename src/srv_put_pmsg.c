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

#include "mintdefs.h"
#include "srv_put.h"

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


/* Client call of server */
LONG
Srv_put (WORD   apid,
         WORD   call,
         void * spec)
{
  PMSG msg;

  msg.apid = apid;
  msg.cr.call = call;
  msg.spec = spec;
  
  Pmsg(MSG_READWRITE, SRVBOX, &msg);

  return (LONG)msg.cr.retval;
}
