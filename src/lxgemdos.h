#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_OSBIND_H
#include <osbind.h>
#endif

#include "types.h"

void link_in(void);
void link_remove(void);
void set_stack(void *addr);
void newmvec(void);
void newbvec(void);
void newtvec(void);
void vdicall(void *contrl);
void accstart(void);
#ifdef __GNUC__
#define VsetMode(mode) \
  (short)trap_14_ww((short)(0x58),(short)mode)
#define VsetScreen(log,phys,mode,modecode) \
  (void)trap_14_wllww((short)(0x05),(long)log,(long)phys,(short)mode, \
		      (short)modecode)
#else
void VsetScreen(void *log,void *phys,WORD mode,WORD modecode);
WORD VsetMode(WORD mode);
#endif
void aescall(void *aespb); 
