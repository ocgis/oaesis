/****************************************************************************

 Module
  wind.c
  
 Description
  Window routines used in oAESis.
  
 Author(s)
 	cg     (Christer Gustavsson <d2cg@dtek.chalmers.se>)

 Revision history
 
  951226 cg
   Added standard header.
  960101 cg
   Added BEG_MCTRL and END_MCTRL modes to wind_update.
  960102 cg
   WF_TOP mode of wind_get() implemented.
  960103 cg
   WF_NEWDESK mode of wind_set() implemented.
   WF_HSLIDE, WF_VSLIDE, WF_VSLSIZE and WF_HSLSIZE modes of wind_set()
   and wind_get() implemented.
 
 Copyright notice
  The copyright to the program code herein belongs to the authors. It may
  be freely duplicated and distributed without fee, but not charged for.
 
 ****************************************************************************/

/****************************************************************************
 * Used interfaces                                                          *
 ****************************************************************************/

#include	<alloc.h>
#include	<basepage.h>
#include	<mintbind.h>
#include	<osbind.h>
#include	<signal.h>
#include	<stdio.h>
#include	<string.h>
#include	<support.h>

#include "debug.h"
#include "gemdefs.h"
#include "global.h"
#include "mintdefs.h"
#include "mesagdef.h"
#include "misc.h"
#include "objc.h"
#include "resource.h"
#include "rlist.h"
#include "srv.h"
#include "types.h"
#include "vdi.h"
#include "wind.h"

/****************************************************************************
 * Macros                                                                   *
 ****************************************************************************/

#define IMOVER 0x8000  /* Used with set_win_elem() to make icon window */

/****************************************************************************
 * Local functions (use static!)                                            *
 ****************************************************************************/

/*calcworksize calculates the worksize or the total size of
a window. If dir == WC_WORK the worksize will be calculated and 
otherwise the total size will be calculated.*/

static void	calcworksize(WORD elem,RECT *orig,RECT *new,WORD dir) {
	WORD	bottomsize = 1;
	WORD	headsize = 1;
	WORD	leftsize = 1;
	WORD	rightsize = 1;
	WORD	topsize;
	
	if((HSLIDE | LFARROW | RTARROW) & elem) {
		bottomsize = globals.windowtad[WLEFT].ob_height + (D3DSIZE << 1);
	};
	
	if((CLOSER | MOVER | FULLER | NAME) & elem) {
		topsize = globals.windowtad[WMOVER].ob_height + (D3DSIZE << 1);
	}
	else if(IMOVER & elem) {
		topsize = globals.csheight + 2 + D3DSIZE * 2;
	}
	else {
		topsize = 0;
	};
	
	if(INFO & elem) {
		headsize = topsize + globals.windowtad[WINFO].ob_height + 2 * D3DSIZE;
	}
	else {
		if(topsize)
			headsize = topsize;
		else
			headsize = 1;
	};
	
	if((LFARROW | HSLIDE | RTARROW) & elem) {
		bottomsize = globals.windowtad[WLEFT].ob_height + (D3DSIZE << 1);
	};
	
	if(((bottomsize < globals.windowtad[WLEFT].ob_height) && (SIZER & elem))
		|| ((VSLIDE | UPARROW | DNARROW) & elem))
	{
		rightsize = globals.windowtad[WSIZER].ob_width + (D3DSIZE << 1);
	};

	if(dir == WC_WORK) {
		new->x = orig->x + leftsize;
		new->y = orig->y + headsize;
		new->width = orig->width - leftsize - rightsize;
		new->height = orig->height - headsize - bottomsize;
	}
	else {
		new->x = orig->x - leftsize;
		new->y = orig->y - headsize;
		new->width = orig->width + leftsize + rightsize;
		new->height = orig->height + headsize + bottomsize;
	};
}

/****************************************************************************
 * Public functions                                                         *
 ****************************************************************************/

/*wind_create 0x0064*/

void	Wind_create(AES_PB *apb) {	
	apb->int_out[0] = Srv_wind_create(apb->global->apid
		,apb->int_in[0],(RECT *)&apb->int_in[1],0);
}

/*wind_open 0x0065*/

void Wind_open(AES_PB *apb) {
	apb->int_out[0] = Srv_wind_open(apb->int_in[0],(RECT *)&apb->int_in[1]);
}

/*wind_close 0x0066*/

void	Wind_close(AES_PB *apb) {
	apb->int_out[0] = Srv_wind_close(apb->int_in[0]);
}


/*wind_delete 0x0067*/
void	Wind_delete(AES_PB *apb) {
	apb->int_out[0] = Srv_wind_delete(apb->int_in[0]);
}

/*wind_get 0x0068*/


void	Wind_get(AES_PB *apb) {
	apb->int_out[0] = Srv_wind_get(apb->int_in[0],apb->int_in[1],
																&apb->int_out[1],&apb->int_out[2],
																&apb->int_out[3],&apb->int_out[4]);
}


/*wind_set 0x0069*/
void	Wind_set(AES_PB *apb) {
	apb->int_out[0] = Srv_wind_set(apb->global->apid,
		apb->int_in[0],
		apb->int_in[1],
		apb->int_in[2],
		apb->int_in[3],
		apb->int_in[4],
		apb->int_in[5]);
}


/*wind_find 0x006a*/
void	Wind_find(AES_PB *apb) {
	apb->int_out[0] = Srv_wind_find(apb->int_in[0],apb->int_in[1]);
}

/*wind_update 0x006b*/
void	Wind_update(AES_PB *apb) {
	if(apb->int_in[0] & 0x0002) {
		apb->int_out[0] = Srv_wind_update(apb->global->apid,apb->int_in[0]);
	}
	else {
		apb->int_out[0] = Srv_wind_update(Pgetpid(),apb->int_in[0]);
	};
}



/*wind_calc 0x006c*/
void	Wind_calc(AES_PB *apb) {
	calcworksize(apb->int_in[1],(RECT *)&apb->int_in[2]
		,(RECT *)&apb->int_out[1],apb->int_in[0]);
	
	apb->int_out[0] = 1;
}

/****************************************************************************
 * Wind_new                                                                 *
 *  0x006d wind_new().                                                      *
 ****************************************************************************/
void              /*                                                        */
Wind_new(         /*                                                        */
AES_PB *apb)      /* AES parameter block.                                   */
/****************************************************************************/
{
	apb->int_out[0] = Srv_wind_new(apb->global->apid);
}