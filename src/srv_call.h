#ifndef __SRV_CALL__
#define __SRV_CALL__

#include "types.h"

typedef struct {
  union {
    WORD call;
    WORD retval;
  } cr;

  WORD    apid;
  void    *spec;
  WORD    pid;
}PMSG;

void Srvc_operation(WORD mode,PMSG *msg);
LONG Srvc_select(LONG *rhndl,WORD timeout);

#endif
