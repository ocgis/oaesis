/****************************************************************************

 Module
  misc.c
  
 Description
  Miscellaneous routines in oAESis.
  
 Author(s)
 	cg (Christer Gustavsson <d2cg@dtek.chalmers.se>)
 	
 Revision history
 
  951225 cg
   Added standard header.
   Added Misc_copy_area.
 
 Copyright notice
  The copyright to the program code herein belongs to the authors. It may
  be freely duplicated and distributed without fee, but not charged for.
 
 ****************************************************************************/

#define DEBUGLEVEL 0

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef MINT_TARGET
#include <pthread.h>
#endif

#ifdef HAVE_BASEPAGE_H
#include <basepage.h>
#endif

#include <ctype.h>

#ifdef HAVE_IOCTL_H
#include <ioctl.h>
#ifdef MINT_TARGET
#endif /* MINT_TARGET */
#endif /* HAVE_IOCTL_H */

#ifdef HAVE_MINTBIND_H
#include <mintbind.h>
#endif

#ifdef HAVE_MINT_DCNTL_H
#include <mint/dcntl.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <vdibind.h>

#include "debug.h"
#include "srv_global.h"
#include "lxgemdos.h"
#include "srv_misc.h"
#include "types.h"

#ifdef HAVE_SYSVARS_H
#include <sysvars.h>
#endif

/****************************************************************************
 * Public functions                                                         *
 ****************************************************************************/

#ifdef MINT_TARGET
WORD
srv_get_cookie (LONG   code,
		 LONG * value) {
  register COOKIE *cookie;
  void            *stack;
  
  stack = (void*)Super(0L);
  cookie = *(COOKIE **)_p_cookies;
  Super(stack);

  while(cookie->cookie) {
    if(cookie->cookie == code) {
      *value = cookie->value;		
      
      return TRUE;
    };
    
    cookie++;
  };
  
  return FALSE;
}
#endif

#ifdef MINT_TARGET
typedef WORD (*Thread)(LONG arg);

static
void
CDECL
startup(register BASEPAGE * bp)
{
  Thread     func;
  LONG       arg;
  short      handle;
  ULONG *    cmd;
  
  /* Get command line parameters */
  cmd = (ULONG *)bp->p_cmdlin;
  func = (Thread)(cmd[0]);
  arg = (LONG)(cmd[1]);
	
  /* Call function and exit*/
  Pterm(func(arg));
}
#else
void startup () {}
#endif /* MINT_TARGET */


#ifdef MINT_TARGET
LONG
srv_fork (WORD   (*func)(LONG),
          LONG   arg,
          BYTE * name)
{
  BASEPAGE * bp_child;
  BASEPAGE * bp_parent;
  LONG       pid;
  int        parent_handle;
  ULONG *    cmd;

  /* Create a basepage for child process */
  bp_child = (BASEPAGE *)Pexec(PE_CBASEPAGE, 0L, "", 0L);

  /* Shrink to only include stack */
  (void)Mshrink(bp_child, sizeof(BASEPAGE) + STKSIZE + 50);

  /* Read parent basepage */
  parent_handle = Fopen("U:\\PROC\\.-1", O_RDONLY);
  Fcntl(parent_handle, &bp_parent, PBASEADDR);
  Fclose(parent_handle);

  /* Copy parent basepage */
  memcpy(bp_child, bp_parent, sizeof(BASEPAGE));

  /* Get correct parent */
  bp_child->p_parent = bp_parent;

  /* Pass parameters to child via command line buffer */
  cmd = (ULONG *)bp_child->p_cmdlin;
  *cmd++ = (ULONG)func;
  *cmd = (ULONG)arg;

  /* Allocate stack */
  bp_child->p_bbase = (char *)(bp_child + 1);
  bp_child->p_blen = STKSIZE;

  /* Setup start parameters */
  bp_child->p_tbase = (BYTE *)&startup;
 
  /* Start child process */
  pid = Pexec(PE_ASYNC_GO_FREE, name, bp_child, 0L);

  return pid;
}
#else /* MINT_TARGET */
LONG
srv_fork (WORD   (*func)(LONG),
	   LONG   arg,
	   BYTE * name) {
  pthread_attr_t thread;

  pthread_create (&thread, NULL, func, (void *)arg);

  return 0;
}

#endif /* MINT_TARGET */


/*
** Description
** End thread
**
** 1999-07-26 CG
*/
void
srv_term (WORD retval) {
  DEBUG3 ("srv_misc.c: srv_term: Terminating thread");
#ifndef MINT_TARGET
  pthread_exit ((void *)retval);
#endif
}


/****************************************************************************
 *  srv_copy_area                                                          *
 *   Copy one area of the screen to another.                                *
 ****************************************************************************/
void              /*                                                        */
srv_copy_area(   /*                                                        */
WORD vid,         /* VDI workstation id.                                    */
RECT *dst,        /* Where to the area is to be copied.                     */
RECT *src)        /* The original area.                                     */
/****************************************************************************/
{
  MFDB mfdbd, mfdbs;
  int  koordl[8];
  
  mfdbd.fd_addr = 0L;
  mfdbs.fd_addr = 0L;
  
  koordl[0] = src->x;
  koordl[1] = src->y + src->height - 1;
  koordl[2] = src->x + src->width - 1;
  koordl[3] = src->y;
  koordl[4] = dst->x;
  koordl[5] = dst->y + dst->height - 1;
  koordl[6] = dst->x + dst->width - 1;
  koordl[7] = dst->y;
  
  vro_cpyfm(vid,S_ONLY,koordl,&mfdbs,&mfdbd);
}


/****************************************************************************
 *  srv_intersect                                                          *
 *   Get intersection of two rectangles.                                    *
 ****************************************************************************/
WORD              /* 0  Rectangles don't intersect.                         */
                  /* 1  Rectangles intersect but not completely.            */
                  /* 2  r2 is completely covered by r1.                     */
srv_intersect(   /*                                                        */
RECT *r1,         /* Rectangle r1.                                          */
RECT *r2,         /* Rectangle r2.                                          */
RECT *rinter)     /* Buffer where the intersecting part is returned.        */
/****************************************************************************/
{
	rinter->x = max(r1->x,r2->x);
	rinter->width = min(r1->x + r1->width, r2->x + r2->width) - rinter->x;
	rinter->y = max(r1->y,r2->y);
	rinter->height = min(r1->y + r1->height, r2->y + r2->height) - rinter->y;
	
	if((r2->x == rinter->x) && (r2->y == rinter->y)
		&& (r2->width == rinter->width) && (r2->height == rinter->height))
	{
		return 2;
	}
	else if((rinter->width > 0) && (rinter->height > 0))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

/****************************************************************************
 * srv_inside                                                              *
 *  Check if coordinates is within rectangle.                               *
 ****************************************************************************/
WORD              /* 0  Outside of rectangle.                               */
                  /* 1  Inside rectangle.                                   */
srv_inside(      /*                                                        */
RECT *r,          /* Rectangle.                                             */
WORD x,           /* X coordinate.                                          */
WORD y)           /* Y coordinate.                                          */
/****************************************************************************/
{
	return (((x -= r->x) >= 0) && 
	        (x < r->width) && 
	        ((y -= r->y) >= 0) && 
	        (y < r->height));
}

/****************************************************************************
 * srv_setpath                                                             *
 *  Set current working directory. This one is stolen from the Mint-libs    *
 *  and modified because of the idiotic functionality of Dsetpath().        *
 ****************************************************************************/
WORD              /* 0 ok, or -1.                                           */
srv_setpath(     /*                                                        */
BYTE *dir)        /* New directory.                                         */
/****************************************************************************/
{
	WORD drv, old;
	BYTE *d;

	d = dir;
	old = Dgetdrv();
	if(*d && (*(d+1) == ':')) {
		drv = toupper(*d) - 'A';
		d += 2;
		(void)Dsetdrv(drv);
	};

	if(!*d) {		/* empty path means root directory */
		*d = '\\';
		*(d + 1) = '\0';
	};
	
	if(Dsetpath(d) < 0) {
		(void)Dsetdrv(old);
		return -1;
	};
	
	return 0;
}
