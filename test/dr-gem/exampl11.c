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

#include <aesbind.h>				/* aes binding structures  */
#include <mintbind.h>				/* mint binding structures */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vdibind.h>				/* vdi binding structures */

#include "exampl11.h"
 
/*------------------------------*/
/*	Some Defines    	*/
/*------------------------------*/

#define TRUE  1
#define FALSE 0
#define LONG int
#define BYTE char
#define VOID void
#define LLOWD(x) ((UWORD)((LONG)x))
						/* return high word of	*/
						/*   a long value	*/
#define LHIWD(x) ((UWORD)((LONG)x >> 16))
						/* return low byte of	*/
						/*   a word value	*/

/*------------------------------*/
/*	Local defines		*/
/*------------------------------*/

#define	ARROW	0			/* Arrow cursor form for mouse	*/
#define	HOUR	2			/* Hourglass cursor form	*/
#define	WBOX	21			/* Initial width for GROWBOX	*/
#define	HBOX	21			/* Initial height for GROWBOX	*/

/*------------------------------*/
/*	Local variables		*/
/*------------------------------*/

WORD	 gl_apid;			/* ID returned by appl_init 	*/
WORD	 gl_rmsg[8];			/* Message buffer		*/
WORD *   ad_rmsg;			/* Pointer to message buffer	*/
OBJECT * main_menu;			/* Pointer to MAINMENU tree	*/
BYTE *	 about_alert;			/* Pointer to ABOALERT tree	*/
BYTE *	 quit_alert;			/* Pointer to QUIALERT tree	*/
int	 xfull;				/* Desk area X coordinate	*/
int	 yfull;				/* Desk area Y coordinate	*/
int	 wfull;				/* Desk area width		*/
int	 hfull;				/* Desk area height		*/
WORD	 xstart;			/* Screen centre X position	*/
WORD	 ystart;			/* Screen centre Y position	*/
WORD	 w1handle;			/* Handle for window 1		*/
WORD	 active     = FALSE;		/* Flag shows active window	*/
BYTE *   wdw_title = "EXAMPL11";
BYTE *   wdw_info  = "Windows under the control of menus";	

/*------------------------------*/
/*	Application code	*/
/*------------------------------*/

/*------------------------------*/
/*	close_window		*/
/*------------------------------*/
static
void
close_window(WORD handle)
{
	int	cx, cy, cw, ch;		/* Holds current XYWH position	*/
	
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
static
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
static
WORD
hndl_window(void)
{

	WORD	evnt_type;		/* Event type			*/
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
static
WORD
open_full(WORD   attributes,
          BYTE * title,
          BYTE * info)
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
	
		form_alert(1, "[3][No windows left][ QUIT ]");
		appl_exit();

		return(handle);
	
	}

	if (attributes & NAME)		/* Title present ?		*/
	{
	  	low_word  =  LLOWD(title);
		high_word =  LHIWD(title);
	
		wind_set(handle, WF_NAME, high_word, low_word, 0 ,0);
	}
	
	if (attributes & INFO)		/* Information line present ?	*/
	{
		low_word  = LLOWD(info);
		high_word = LHIWD(info);
	
		wind_set(handle, WF_INFO, high_word, low_word,0 ,0 );
	}
	
	graf_growbox(xstart, ystart, HBOX, WBOX, xfull, yfull, wfull, hfull);
	wind_open(handle, xfull, yfull, wfull, hfull);

	graf_mouse(ARROW, 0L);		/* Restore mouse form		*/
	
	return(handle);
	
}

/*------------------------------*/
/*	do_menu			*/
/*------------------------------*/
static
WORD
do_menu(void)
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

        return TRUE;
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
static
WORD
initialise(void)
{

	ad_rmsg = &gl_rmsg[0];
	
	gl_apid = appl_init();		/* return application ID	*/
	
	if (gl_apid == -1)

		return(FALSE);		/* unable to use AES		*/

	if (!rsrc_load("exampl11.rsc"))
	{
	
		form_alert(1, "[3][Unable to load resource][ Abort ]");

		return(FALSE);		/* unable to load resource	*/
		
	}

	return(TRUE);			/* ID returned successfully	*/
	
}

/*------------------------------*/
/*	GEMAIN			*/
/*------------------------------*/
int
main(void)
{

	if (!initialise())
        {
          return -1;
	}
	
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
	
        return 0;
}
