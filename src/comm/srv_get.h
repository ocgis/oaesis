#ifndef __SRV_GET__
#define __SRV_GET__

#include "types.h"

typedef struct comm_handle_s * COMM_HANDLE;
#define COMM_HANDLE_NIL ((COMM_HANDLE)NULL)

/*
** Description
** Open the server connection
**
** 1998-09-25 CG
*/
void
Srv_open (void);


/*
** Description
** Wait for a message from a client
**
** 1998-09-25 CG
** 1998-12-13 CG
*/
COMM_HANDLE
Srv_get (void * in,
         int    max_bytes_in);


/*
** Description
** Wake a waiting Srv_get
**
** 1998-12-06 CG
*/
void
Srv_wake (void);

/*
** Description
** Reply with a message to a client
**
** 1998-09-25 CG
** 1998-12-13 CG
*/
void
Srv_reply (COMM_HANDLE handle,
           void *      out,
           WORD        bytes_out);
#endif
