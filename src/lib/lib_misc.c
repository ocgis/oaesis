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

/****************************************************************************
 * Used interfaces                                                          *
 ****************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_BASEPAGE_H
#include <basepage.h>
#endif

#include <ctype.h>

#ifdef HAVE_IOCTL_H
#include <ioctl.h>
#endif

#ifdef HAVE_MINTBIND_H
#include <mintbind.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <vdibind.h>

#include "aesbind.h"
#include "lib_global.h"
/*#include "lxgemdos.h"*/
#include "lib_misc.h"
#include "srv_put.h"
#include "srv_interface.h"
#include "types.h"

#ifdef HAVE_SYSVARS_H
#include <sysvars.h>
#endif

/****************************************************************************
 * Public functions                                                         *
 ****************************************************************************/


WORD	max(WORD a,WORD b) {
	if(a > b) 
		return a;
	else
		return b;
}

WORD	min(WORD a,WORD b) {
	if(a < b) 
		return a;
	else
		return b;
}

/****************************************************************************
 *  Misc_copy_area                                                          *
 *   Copy one area of the screen to another.                                *
 ****************************************************************************/
void              /*                                                        */
Misc_copy_area(   /*                                                        */
WORD vid,         /* VDI workstation id.                                    */
RECT *dst,        /* Where to the area is to be copied.                     */
RECT *src)        /* The original area.                                     */
/****************************************************************************/
{
	MFDB	mfdbd,mfdbs;
	int	koordl[8];
	
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
 *  Misc_intersect                                                          *
 *   Get intersection of two rectangles.                                    *
 ****************************************************************************/
WORD              /* 0  Rectangles don't intersect.                         */
                  /* 1  Rectangles intersect but not completely.            */
                  /* 2  r2 is completely covered by r1.                     */
Misc_intersect(   /*                                                        */
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
 * Misc_inside                                                              *
 *  Check if coordinates is within rectangle.                               *
 ****************************************************************************/
WORD              /* 0  Outside of rectangle.                               */
                  /* 1  Inside rectangle.                                   */
Misc_inside(      /*                                                        */
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
 * Misc_setpath                                                             *
 *  Set current working directory. This one is stolen from the Mint-libs    *
 *  and modified because of the idiotic functionality of Dsetpath().        *
 ****************************************************************************/
WORD              /* 0 ok, or -1.                                           */
Misc_setpath(     /*                                                        */
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


/****************************************************************************
 * Misc_Vdi_Malloc                                                          *
 *  Reserve a memory block for use with VDI calls. The server does the      *
 *  actual malloc if VDI calls are tunnelled.                               *
 ****************************************************************************/
void *              /* Address of reserved memory.                          */
Misc_Vdi_Malloc(    /*                                                      */
size_t amount)      /* Amount of bytes to reserve.                          */ 
/****************************************************************************/
{
#ifdef TUNNEL_VDI_CALLS
  C_MALLOC par;
  R_MALLOC ret;

  par.common.call = htons (SRV_MALLOC);
  par.amount = htonl ((ULONG)amount);

  /* Pass the call to the server */
  Client_send_recv (&par,
                    sizeof (C_MALLOC),
                    &ret,
                    sizeof (R_MALLOC));
  
  return (void *)ntohl (ret.address);
#else /* TUNNEL_VDI_CALLS */
  return malloc (amount);
#endif /* TUNNEL_VDI_CALLS */
}

/****************************************************************************
 * Misc_Vdi_Free                                                            *
 *  Free a memory block reserved by Misc_Vdi_Malloc.                        *
 ****************************************************************************/
void                /*                                                      */
Misc_Vdi_Free(      /*                                                      */
void *address)      /* Pointer to block to free.                            */ 
/****************************************************************************/
{
#ifdef TUNNEL_VDI_CALLS
  C_FREE par;
  R_FREE ret;

  par.common.call = htons (SRV_FREE);
  par.address = htonl ((ULONG)address);

  /* Pass the call to the server */
  Client_send_recv (&par,
                    sizeof (C_FREE),
                    &ret,
                    sizeof (R_FREE));  
#else /* TUNNEL_VDI_CALLS */
  free (address);
#endif /* TUNNEL_VDI_CALLS */
}
