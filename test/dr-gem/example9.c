/************************************************************************/
/*	File:	example9.c						*/
/************************************************************************/
/*									*/
/*	Example GEM program which creates a WINDOW and exits on a	*/
/*	close box selection. Shows use of window library.		*/
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
WORD	w1handle;			/* Handle for window 1		*/
BYTE	*wdw_title = "EXAMPLE9";
BYTE	*wdw_info  = "Example of using window library";	

/*------------------------------*/
/*	Application code	*/
/*------------------------------*/

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
/*	initialise		*/
/*------------------------------*/

WORD	initialise()

{

	ad_rmsg = ADDR((BYTE *) &gl_rmsg[0]);
	
	gl_apid = appl_init();		/* return application ID	*/
	
	if (gl_apid == -1)

		return(FALSE);		/* unable to use AES		*/

	return(TRUE);			/* ID returned successfully	*/
	
}

/*------------------------------*/
/*	GEMAIN			*/
/*------------------------------*/

GEMAIN()
{

	WORD	win_attr;		/* Window attributes		*/

	if (!initialise())
		return(FALSE);
	
	
	win_attr  = NAME|CLOSER|FULLER|MOVER|INFO|SIZER|UPARROW|DNARROW;
	win_attr |= VSLIDE|LFARROW|RTARROW|HSLIDE;
	
	w1handle = open_full(win_attr, wdw_title, wdw_info);
	
	if (w1handle > 0)
	
		while(hndl_window());
			
	appl_exit();			/* Exit AES tidily		*/
	
}
