#ifndef __SRV_PUT__
#define __SRV_PUT__

#include "types.h"

/*
** Description
** Open client connection to server
**
** 1998-10-08 CG
*/
LONG
Client_open (void);


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

#endif
