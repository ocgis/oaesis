#ifndef __SRV_PUT__
#define __SRV_PUT__

#include "types.h"

/*
** Description
** Put a message to the server and wait for a reply
**
** 1998-09-25 CG
*/
LONG
Srv_put (WORD   apid,
         WORD   call,
         void * spec);

/*
** Description
** Put a message to the server and wait for a reply
**
** 1998-09-26 CG
*/
LONG
Client_send_recv (void * in,
                  int    bytes_in,
                  void * out,
                  int    max_bytes_out);

LONG Srvc_select(LONG *rhndl,WORD timeout);

#endif
