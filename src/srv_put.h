#ifndef __SRV_PUT__
#define __SRV_PUT__

#include "types.h"

LONG
Srv_put (WORD   apid,
         WORD   call,
         void * spec);

LONG Srvc_select(LONG *rhndl,WORD timeout);

#endif
