/****************************************************************************

 Module
  srv_call.c
  
 Description
  Server calling interface for oAESis.
  
 Author(s)
  cg (Christer Gustavsson <d2cg@dtek.chalmers.se>)
 	
 ****************************************************************************/

#include <mintbind.h>

#include "mintdefs.h"
#include "srv_call.h"

#define SRVBOX 0x6f535256l /*'oSRV'*/

/* Client call of server */
void Srvc_operation(WORD mode,PMSG *msg) {
  if(mode == MSG_WRITE) {
    Pmsg(mode,0xffff0000L | msg->pid,msg);
  }
  else {
    Pmsg(mode,SRVBOX,msg);
  };
}

/* Server select/read/reply */
LONG Srvc_select(LONG *rhndl,WORD timeout) {
  return 0;
}


