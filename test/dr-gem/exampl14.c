/************************************************************************/
/*	File:	exampl14.c						*/
/************************************************************************/
/*									*/
/*	Example GEM program which introduces the VDI. Displays the	*/
/*	rectangles in a window as filled bars and exits on CLOSE.	*/
/*									*/
/************************************************************************/

/*------------------------------*/
/*	Standard includes	*/
/*------------------------------*/

#include "portab.h"				/* portable coding macros */
#include "machine.h"				/* machine dependencies   */
#include "obdefs.h"				/* object definitions	  */
#include "treeaddr.h"				/* tree address macros    */
#include "vdibind.h"				/* vdi binding structures */
#include "gembind.h"				/* gem binding structures */

/*------------------------------*/
/*	Global GEM arrays	*/
/*------------------------------*/

GLOBAL WORD	contrl[11];		/* control inputs		*/
GLOBAL WORD	intin[80];		/* max string length		*/
GLOBAL WORD	ptsin[256];		/* polygon fill points		*/
GLOBAL WORD	intout[45];		/* open workstation output	*/
GLOBAL WORD	ptsout[12];		/* points out array		*/

/*------------------------------*/
/*	Local defines		*/
/*------------------------------*/

#define	ARROW	0			/* Arrow cursor form for mouse	*/
#define	HOUR	2			/* Hourglass cursor form	*/
#define	DESK	0			/* DESK area identifier		*/
#define	WBOX	21			/* Initial width for GROWBOX	*/
#define	HBOX	21			/* Initial height for GROWBOX	*/

/*------------------------------*/
/*	Local variables		*/
/*------------------------------*/

WORD	gl_apid;			/* ID returned by appl_init 	*/
WORD	gl_rmsg[8];			/* Message buffer		*/
LONG	ad_rmsg;			/* Pointer to message buffer	*/
WORD	xfull;				/* Desk area X coordinate	*/
WORD	yfull;				/* Desk area Y coordinate	*/
WORD	wfull;				/* Desk area width		*/
WORD	hfull;				/* Desk area height		*/
WORD	xstart;				/* Screen centre X position	*/
WORD	ystart;				/* Screen centre Y position	*/
WORD	hwchar;				/* Width of system font cell	*/
WORD	hhchar;				/* Height of system font cell	*/
WORD	hwbox;				/* Width of system font box	*/
WORD	hhbox;				/* Height of system font box	*/
WORD	work_in[11];			/* Input to v_opnvwk		*/
WORD	work_out[57];			/* Output from v_opnvwk		*/
WORD	pxyarray[4];			/* Points array for rectangle	*/
WORD	gem_handle;			/* Holds physical workstation	*/
WORD	vdi_handle;			/* Holds virtual workstation	*/
WORD	set_interior;			/* Interior fill style		*/
WORD	set_style;			/* Interior fill index		*/
WORD	w1handle;			/* Handle for window 1		*/
BYTE	*wdw_title = "EXAMPLE14";
BYTE	*wdw_info  = "Example of using GEM VDI calls";	

/*------------------------------*/
/*	Application code	*/
/*------------------------------*/

/*------------------------------*/
/*	grect_to_array		*/
/*------------------------------*/

VOID	grect_to_array(sx, sy, sw, sh, tlx, tly, brx, bry)

WORD	sx, sy, sw, sh;			/* Source rectangle XYWH	*/
WORD	*tlx, *tly, *brx, *bry;		/* Top left XY & Bot. right XY	*/

{
	*tlx = sx;			/* Set top left X coordinate	*/
	*tly = sy;			/* Set top right Y coordinate	*/
	*brx = sx + sw - 1;		/* Calculate bottom right X	*/
	*bry = sy + sh - 1;		/* Calculate bottom right Y	*/
}

/*------------------------------*/
/*	close_window		*/
/*------------------------------*/

WORD	close_window(handle)

WORD	handle;				/* Window handle		*/

{
	WORD	cx, cy, cw, ch;		/* Holds current XYWH position	*/
	
	graf_mouse(HOUR, 0L);		/* Show hourglass		*/

	wind_get(handle, WF_CXYWH, &cx, &cy, &cw, &ch);

	wind_close(handle);		/* Close window off screen	*/

	graf_shrinkbox(xstart, ystart, HBOX, WBOX, cx, cy, cw, ch);

	wind_delete(handle);		/* Clear window from system	*/
	
	graf_mouse(ARROW, 0L);		/* Change cursor to ARROW	*/
}

/*------------------------------*/
/*	hndl_window		*/
/*------------------------------*/

WORD	hndl_window()

{

	WORD	evnt_type;		/* Event type			*/
	WORD	evnt_action;		/* Requested action for scroll	*/
	WORD	wdw_hndl;		/* Handle of window in event	*/
	WORD	wx;			/* Event x coordinate		*/
	WORD	wy;			/* Event y coordinate		*/
	WORD	ww;			/* Event window width		*/
	WORD	wh;			/* Event window height		*/
	
	evnt_mesag(ad_rmsg);	/* get events			*/
	
	evnt_type = gl_rmsg[0];
	wdw_hndl  = gl_rmsg[3];

	switch(evnt_type)
	{
		case	WM_CLOSED:	/* Close box selected		*/
		{
			close_window(wdw_hndl);
			return(FALSE);
		}
	}

	return(TRUE);

}

/*------------------------------*/
/*	open_full		*/
/*------------------------------*/

WORD	open_full(attributes, title, info)

WORD	attributes;			/* Window attributes		*/
BYTE	*title;				/* Window title			*/
BYTE	*info;				/* Window information line	*/

{

	WORD	handle;			/* Window handle		*/
	WORD	high_word;		/* High word address		*/
	WORD	low_word;		/* Low word address		*/

	
	wind_get(DESK, WF_WXYWH, &xfull, &yfull, &wfull, &hfull);

	xstart = wfull / 2;		/* Calculate X of screen centre	*/
	ystart = hfull / 2;		/* Calculate Y of screen centre	*/

	graf_mouse(HOUR, 0L);		/* Show hour glass		*/
	
	handle = wind_create(attributes, xfull, yfull, wfull, hfull);

	if (handle <= 0)
	{
	
		form_alert(1, ADDR("[3][No windows left][ QUIT ]"));
		appl_exit();

		return(handle);
	
	}

	if (attributes & NAME)		/* Title present ?		*/
	{
	  	low_word  = (WORD) LLOWD(ADDR(title));
		high_word = (WORD) LHIWD(ADDR(title));
	
		wind_set(handle, WF_NAME, low_word, high_word);
	}
	
	if (attributes & INFO)		/* Information line present ?	*/
	{
		low_word  = (WORD) LLOWD(ADDR(info));
		high_word = (WORD) LHIWD(ADDR(info));
	
		wind_set(handle, WF_INFO, low_word, high_word);
	}
	
	graf_growbox(xstart, ystart, HBOX, WBOX, xfull, yfull, wfull, hfull);
	wind_open(handle, xfull, yfull, wfull, hfull);

	graf_mouse(ARROW, 0L);		/* Restore mouse form		*/
	
	return(handle);
	
}

/*------------------------------*/
/*	do_vdidemo		*/
/*------------------------------*/

VOID do_vdidemo()

{
	WORD	wx, wy, ww, wh;		/* Window work area XYWH	*/
	WORD	tlx, tly, brx, bry;	/* Holds rectangle XY XY	*/ 
	WORD	style;			/* Style index counter		*/
	
	wind_get(w1handle, WF_WXYWH, &wx, &wy, &ww, &wh);
	
	grect_to_array(wx, wy, ww, wh, &tlx, &tly, &brx, &bry);
	
	set_interior = vsf_interior(vdi_handle, FIS_PATTERN);
	
	pxyarray[0]  = tlx;		/* Set top left X of rectangle	*/
	pxyarray[1]  = tly;		/* Set top left Y of rectangle	*/
	pxyarray[2]  = brx;		/* Set bottom right X		*/
	pxyarray[3]  = bry;		/* Set bottom right Y		*/
	
	v_hide_c(vdi_handle);		/* Same as graf_mouse(M_OFF, 0)	*/

	for (style = 1; style <= 24; style++)
	{
		set_style    = vsf_style(vdi_handle, style);
		vr_recfl(vdi_handle, pxyarray);
	}

	v_show_c(vdi_handle, FALSE);	/* Same as graf_mouse(M_ON, 0)	*/
}

/*------------------------------*/
/*	do_window		*/
/*------------------------------*/

VOID	do_window()
{
	w1handle = open_full(NAME|CLOSER|INFO, wdw_title, wdw_info);
	
	if (w1handle > 0)
	{
		do_vdidemo();	
		while(hndl_window());
	}	
}

/*------------------------------*/
/*	init_vdi		*/
/*------------------------------*/

VOID	init_vdi()			/* Get VDI handle & initialise	*/

{
	gem_handle = graf_handle(&hwchar, &hhchar, &hwbox, &hhbox);

	vdi_handle = gem_handle;	/* Prepare virtual workstation	*/
	
	work_in[0]  = 1;		/* Device number 1 (screen)	*/
	work_in[1]  = 1;		/* Initial line type 1 (solid)	*/
	work_in[2]  = BLACK;		/* Line colour index (black)	*/
	work_in[3]  = 1;		/* Initial marker type (dot)	*/
	work_in[4]  = BLACK;		/* Marker colour index (black)	*/
	work_in[5]  = 1;		/* Initial text face (system)	*/
	work_in[6]  = BLACK;		/* Text colour index (black)	*/
	work_in[7]  = FIS_SOLID;	/* Fill interior style (solid)	*/
	work_in[8]  = 1;		/* Fill style index (solid)	*/
	work_in[9]  = BLACK;		/* Fill colour index (black)	*/
	work_in[10] = 2;		/* Use RC (raster coordinates)	*/
	
	v_opnvwk(work_in, &vdi_handle, work_out);
}

/*------------------------------*/
/*	initialise		*/
/*------------------------------*/

WORD	initialise()

{

	ad_rmsg = ADDR((BYTE *) &gl_rmsg[0]);
	
	gl_apid = appl_init();		/* return application ID	*/
	
	if (gl_apid == -1)

		return(FALSE);		/* unable to use AES		*/

	init_vdi();			/* Initialise GEM VDI		*/

	return(TRUE);			/* ID returned successfully	*/
	
}

/*------------------------------*/
/*	GEMAIN			*/
/*------------------------------*/

GEMAIN()
{

	if (!initialise())
		return(FALSE);

	do_window();			/* Display VDI demo in window	*/
			
	appl_exit();			/* Exit AES tidily		*/
	
}
