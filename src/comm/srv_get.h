#ifndef __SRV_GET__
#define __SRV_GET__

#include "types.h"

void *
Srv_get (WORD * apid,
         WORD * pid,
         WORD * call,
         void * spec);

void
Srv_reply (void * handle,
           void * spec,
           WORD   code);
#endif
