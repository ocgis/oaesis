/************************************************************************/
/*	File:	exampl11.c						*/
/************************************************************************/
/*									*/
/*	Extension of example 10. Shows handling of window events	*/
/*	and menus. Introduction to multiple event handling.		*/
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
#include "exampl11.h"				/* Object tree resources  */

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
LONG	main_menu;			/* Pointer to MAINMENU tree	*/
LONG	about_alert;			/* Pointer to ABOALERT tree	*/
LONG	quit_alert;			/* Pointer to QUIALERT tree	*/
WORD	xfull;				/* Desk area X coordinate	*/
WORD	yfull;				/* Desk area Y coordinate	*/
WORD	wfull;				/* Desk area width		*/
WORD	hfull;				/* Desk area height		*/
WORD	xstart;				/* Screen centre X position	*/
WORD	ystart;				/* Screen centre Y position	*/
WORD	w1handle;			/* Handle for window 1		*/
WORD	active     = FALSE;		/* Flag shows active window	*/
BYTE	*wdw_title = "EXAMPL11";
BYTE	*wdw_info  = "Windows under the control of menus";	

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
/*	size_window		*/
/*------------------------------*/

VOID	size_window(handle, x, y, w, h)	/* Set current window size	*/

WORD	handle;				/* Window handle to size	*/
WORD	x;				/* New X position		*/
WORD	y;				/* New Y position		*/
WORD	w;				/* New width			*/
WORD	h;				/* New height			*/

{
	wind_set(handle, WF_CXYWH, x, y, w, h);
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
	
	evnt_type = gl_rmsg[0];
	wdw_hndl  = gl_rmsg[3];

	switch(evnt_type)
	{
		case	WM_SIZED:	/* Window has been sized	*/
		case	WM_MOVED:	/* Window has been moved	*/
		{
			wx = gl_rmsg[4];
			wy = gl_rmsg[5];
			ww = gl_rmsg[6];
			wh = gl_rmsg[7];

			size_window(wdw_hndl, wx, wy, ww, wh);
			break;
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
/*	do_menu			*/
/*------------------------------*/

WORD	do_menu()

{

	WORD	menu_title;		/* Holds menu title number	*/
	WORD	menu_option;		/* Holds menu option number	*/
	WORD	wbutton;		/* Button chosen in QUIT alert	*/
	WORD	win_attr;		/* Window attributes		*/

		
	menu_title  = gl_rmsg[3];	/* Number of menu title 	*/
	menu_option = gl_rmsg[4];	/* number of menu option	*/
	
	switch(menu_title)
	{
	
		case	DESKMENU:
		{
			switch(menu_option)
			{
				case	ABOUT:
				{
					form_alert(1, about_alert);
					break;
				}
			}
		
			menu_tnormal(main_menu, DESKMENU, 1);
			return(TRUE);
		}			

		case	FILEMENU:	
		{
			switch(menu_option)
			{
				case	OPEN:
				{
					win_attr=NAME|MOVER|INFO|SIZER;
	
					w1handle=open_full(win_attr,
							   wdw_title,
							   wdw_info);

					if (w1handle <= 0)

						return(FALSE);

					menu_ienable(main_menu,OPEN, 0);
					menu_ienable(main_menu,CLOSE,1);

					active = TRUE;
					break;
				}
				
				case	CLOSE:
				{
					close_window(w1handle);
				
					menu_ienable(main_menu,OPEN, 1);
					menu_ienable(main_menu,CLOSE,0);

					active = FALSE;
					break;
				}
				
				case	QUIT:
				{
					wbutton = form_alert(1, quit_alert);
					
					if (wbutton == 1)
						return(FALSE);
					
					break;
				}
			}
			
			menu_tnormal(main_menu, FILEMENU, 1);
			return(TRUE);
		}
	}
}

/*------------------------------*/
/*	hndl_events		*/
/*------------------------------*/

WORD	hndl_events()			/* Process MENU or WINDOW evnts	*/

{

	evnt_mesag(ad_rmsg);		/* Wait for events		*/

	if (gl_rmsg[0] == MN_SELECTED)

		return(do_menu());

	else
	
		return(hndl_window());

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

	if (!rsrc_load(ADDR("EXAMPL11.RSC")))
	{
	
		form_alert(1, ADDR("[3][Unable to load resource][ Abort ]"));

		return(FALSE);		/* unable to load resource	*/
		
	}

	return(TRUE);			/* ID returned successfully	*/
	
}

/*------------------------------*/
/*	GEMAIN			*/
/*------------------------------*/

GEMAIN()
{

	if (!initialise())

		return(FALSE);
	
	rsrc_gaddr(R_TREE,   MAINMENU, &main_menu);
	rsrc_gaddr(R_STRING, ABOALERT, &about_alert);
	rsrc_gaddr(R_STRING, QUIALERT, &quit_alert);
	
	menu_bar(main_menu, TRUE);	/* Turn on menu, disable CLOSE */
	menu_ienable(main_menu,CLOSE,0);

	graf_mouse(ARROW, 0L);		/* Make sure mouse is an arrow	*/

	while(hndl_events());		/* Handle menu & window events	*/

	if (active)			/* If window still active	*/
	
		close_window(w1handle);	/* Make sure window is closed	*/
	
	menu_bar(main_menu, FALSE);	/* Switch off menu bar		*/
		
	appl_exit();			/* Exit AES tidily		*/
	
}
