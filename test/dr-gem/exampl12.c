/************************************************************************/
/*	File:	exampl12.c						*/
/************************************************************************/
/*									*/
/*	Example GEM program which displays a dialog box from		*/
/*	a resource file in a window. Summary of previous examples	*/
/*	combines window, form and menu library calls.			*/
/*									*/
/************************************************************************/

/*------------------------------*/
/*	Standard includes	*/
/*------------------------------*/
#include <aesbind.h>				/* vdi binding structures */
#include <vdibind.h>				/* vdi binding structures */

#include "treeaddr.h"				/* tree address macros    */
#include "exampl12.h"				/* resource file offsets  */

/*------------------------------*/
/*	Local defines		*/
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
#define	ARROW	0			/* Arrow cursor form for MOUSE	*/
#define	HOUR	2			/* Hourglass cursor form	*/
#define	WBOX	21			/* Initial width for GROWBOX	*/
#define	HBOX	21			/* Initial height for GROWBOX	*/

/*------------------------------*/
/*	Local variables		*/
/*------------------------------*/

WORD	 gl_apid;			/* ID returned by appl_init 	*/
WORD	 gl_rmsg[8];			/* Message buffer		*/
WORD *   ad_rmsg;			/* Pointer to message buffer	*/
int	 xfull;				/* Desk area x coordinate	*/
int	 yfull;				/* Desk area y coordinate	*/
int	 wfull;				/* Desk area width		*/
int	 hfull;				/* Desk area height		*/
WORD	 xstart;			/* Holds x of screen centre	*/
WORD	 ystart;			/* Holds y of screen centre	*/
OBJECT * time_date;			/* Address of TIMEDATE dialog	*/
OBJECT * main_menu;			/* Pointer to MAINMENU tree	*/
BYTE *   about_alert;			/* Pointer to ABOALERT tree	*/
BYTE *   quit_alert;			/* Pointer to QUIALERT tree	*/
WORD	 w1handle;			/* Handle for window 1		*/
WORD	 active     = FALSE;		/* Flag shows active window	*/
WORD	 wattr      = NAME|MOVER|INFO;	/* Window attributes		*/
BYTE *   wdw_title = "EXAMPL12";
BYTE *   wdw_info  = "Windows with dialogs under the control of menus";	


static
VOID
do_dialog(void);

static
WORD
do_menu(void);

/*------------------------------*/
/*	application code	*/
/*------------------------------*/

/*------------------------------*/
/*	min			*/
/*------------------------------*/
static
WORD	min(a, b)			/* return min of two values */

WORD		a, b;

{
	return( (a < b) ? a : b );
}

/*------------------------------*/
/*	max			*/
/*------------------------------*/
static
WORD	max(a, b)			/* return max of two values */

WORD		a, b;

{
	return( (a > b) ? a : b );
}

/*------------------------------*/
/*	erc_intersect		*/
/*------------------------------*/
static
WORD	erc_intersect(sx, sy, sw, sh, dx, dy, dw, dh)

WORD	sx, sy, sw, sh;			/* Update rectangle XYWH	*/
WORD	*dx, *dy, *dw, *dh;		/* XYWH from window rectangles	*/

{

	WORD	tx, ty, tw, th;

	tw = min(*dx + *dw, sx + sw);
	th = min(*dy + *dh, sy + sh);
	tx = max(*dx, sx);
	ty = max(*dy, sy);

	*dx = tx;
	*dy = ty;
	*dw = tw - tx;
	*dh = th - ty;

	return((tw > tx) && (th > ty));

}

/*------------------------------*/
/*	do_redraw	 	*/
/*------------------------------*/
static
VOID
do_redraw(WORD cx,
          WORD cy,
          WORD cw,
          WORD ch)
{

	int	x, y, w, h;		/* Holds XYWH of rectangle list	*/
	
	graf_mouse(M_OFF, 0L);		/* Turn off mouse form		*/
	
	wind_update(TRUE);		/* Freeze rectangle list	*/
	
	wind_get(w1handle, WF_FIRSTXYWH, &x, &y, &w, &h);
	
	while ((w > 0) && (h > 0))
	{
		if (erc_intersect(cx, cy, cw, ch, &x, &y, &w, &h))
		
			objc_draw(time_date, ROOT, MAX_DEPTH, x, y, w, h);
			
		wind_get(w1handle, WF_NEXTXYWH, &x, &y, &w, &h);
	}
	
	wind_update(FALSE);		/* Release screen semaphore	*/
	
	graf_mouse(M_ON, 0L);		/* Turn on mouse form		*/
	
}

/*------------------------------*/
/*	close_window	 	*/
/*------------------------------*/
static
VOID
close_window(void)
{
	int	cx, cy, cw, ch;		/* Holds current XYWH position	*/
	
	graf_mouse(HOUR, 0L);		/* Show hourglass		*/

	wind_get(w1handle, WF_CXYWH, &cx, &cy, &cw, &ch);

	wind_close(w1handle);		/* Close window off screen	*/

	graf_shrinkbox(xstart, ystart, HBOX, WBOX, cx, cy, cw, ch);

	wind_delete(w1handle);		/* Clear window from system	*/
	
	graf_mouse(ARROW, 0L);		/* Change cursor to ARROW	*/
}


/*------------------------------*/
/*	hndl_window		*/
/*------------------------------*/
static
VOID
hndl_window(void)
{

	WORD	 evnt_type;		/* Event type			*/
	WORD	 wdw_hndl;		/* Handle of window in event	*/
	WORD	 wx, wy, ww, wh;	/* Event XYWH parameters	*/
	int	 cx, cy, cw, ch;	/* Holds window work XYWH	*/
	OBJECT * tree;			/* Holds tree address		*/
	
	evnt_type = gl_rmsg[0];
	wdw_hndl  = gl_rmsg[3];

	switch(evnt_type)
	{

		case	WM_TOPPED:	/* Window has been topped	*/
		{
			wind_set(w1handle, WF_TOP, 0, 0, 0, 0);
			break;
		}	

		case	WM_REDRAW:	/* Redraw requested		*/
		{
			wx = gl_rmsg[4];
			wy = gl_rmsg[5];
			ww = gl_rmsg[6];
			wh = gl_rmsg[7];

			do_redraw(wx, wy, ww, wh);
			break;
		}

		case	WM_MOVED:	/* Window has been moved	*/
		{
			wx = gl_rmsg[4];
			wy = gl_rmsg[5];
			ww = gl_rmsg[6];
			wh = gl_rmsg[7];

			wind_set(w1handle, WF_CXYWH, wx, wy, ww, wh);
			wind_get(w1handle, WF_WXYWH, &cx, &cy, &cw, &ch);
			
			tree = time_date;

			tree[ROOT].ob_x = cx;
			tree[ROOT].ob_y = cy;
			
			break;
		}
	}

}

/*------------------------------*/
/*	do_open			*/
/*------------------------------*/
static
WORD
do_open(void)
{

	WORD     high_word;		/* High word address		*/
	WORD     low_word;		/* Low word address		*/
	OBJECT * tree;			/* Object tree data structure	*/
	int      wx, wy, ww, wh;	/* Work area XYWH parameters	*/
	int      bx, by, bw, bh;	/* Border area XYWH parameters	*/
	
	wind_get(DESK, WF_WXYWH, &xfull, &yfull, &wfull, &hfull);

	xstart = wfull / 2;		/* Calculate X of screen centre	*/
	ystart = hfull / 2;		/* Calculate Y of screen centre	*/

	graf_mouse(HOUR, 0L);		/* Show hour glass		*/

	form_center(time_date, &wx, &wy, &ww, &wh);
	
	wind_calc(WC_BORDER, wattr, wx, wy, ww, wh, &bx, &by, &bw, &bh);
	
	w1handle = wind_create(wattr, bx, by, bw, bh);

	if (w1handle <= 0)
	{
	
		form_alert(1, "[3][No windows left][ QUIT ]");
		appl_exit();

		return(FALSE);
	
	}

	if (wattr & NAME)		/* Title present ?		*/
	{
	  	low_word  = (WORD) LLOWD(wdw_title);
		high_word = (WORD) LHIWD(wdw_title);
	
		wind_set(w1handle, WF_NAME, high_word, low_word, 0, 0);
	}
	
	if (wattr & INFO)		/* Information line present ?	*/
	{
		low_word  = (WORD) LLOWD(wdw_info);
		high_word = (WORD) LHIWD(wdw_info);
	
		wind_set(w1handle, WF_INFO, high_word, low_word, 0, 0);
	}
	
	graf_growbox(xstart, ystart, HBOX, WBOX, bx, by, bw, bh);
	wind_open(w1handle, bx, by, bw, bh);
	wind_get(w1handle, WF_WXYWH, &wx, &wy, &ww, &wh);

	tree = time_date;

	tree[ROOT].ob_x = wx;
	tree[ROOT].ob_y = wy;
	tree[ROOT].ob_width = ww;
	tree[ROOT].ob_height = wh;

	objc_draw(time_date, ROOT, MAX_DEPTH, wx, wy, ww, wh);
	
	graf_mouse(ARROW, 0L);		/* Restore mouse form		*/
	
	return(TRUE);
	
}

/*------------------------------*/
/*	hndl_events		*/
/*------------------------------*/
/* Process MENU or WINDOW evnts	*/
static
WORD
hndl_events(void)
{
	evnt_mesag(ad_rmsg);		/* Wait for events		*/

	if (gl_rmsg[0] == MN_SELECTED)

		return(do_menu());

	else
	{
		hndl_window();
		return(TRUE);
	}
}

/*------------------------------*/
/*	dsel_obj		*/
/*------------------------------*/
/* Deselect object in tree	*/
static
VOID
dsel_obj(OBJECT * tree,
         WORD     item)
{
	int	wx, wy, ww, wh;		/* Holds work area XYWH		*/
	
	wind_get(w1handle, WF_WXYWH, &wx, &wy, &ww, &wh);
	
	objc_change(tree, item, 0, wx, wy, ww, wh, NORMAL, TRUE);
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
					if (!do_open())

						return(FALSE);

					menu_ienable(main_menu,OPEN, 0);
					menu_ienable(main_menu,CLOSE,1);
					menu_ienable(main_menu,EDIT, 1);

					active = TRUE;
					break;
				}
				
				case	CLOSE:
				{
					close_window();
				
					menu_ienable(main_menu,OPEN, 1);
					menu_ienable(main_menu,EDIT ,0);
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

		case	EDITMENU:
		{
			switch(menu_option)
			{
				case	EDIT:
				{
					do_dialog();
					break;
				}
			}
		
			menu_tnormal(main_menu, EDITMENU, 1);
			return(TRUE);
		}			

	}

        return TRUE;
}

/*------------------------------*/
/*	do_dialog		*/
/*------------------------------*/
/* Handles dialog in window	*/
static
VOID
do_dialog(void)
{
	int	wx, wy, ww, wh;		/* Window work area XYWH	*/
	
	wind_set(w1handle, WF_TOP, 0, 0, 0, 0);
	wind_get(w1handle, WF_WXYWH, &wx, &wy, &ww, &wh);
	
	graf_mouse(M_OFF, 0L);		/* Turn off mouse form		*/

	do_redraw(wx, wy, ww, wh);	/* Force a redraw		*/

	form_dial(FMD_START, wx, wy, ww, wh, wx, wy, ww, wh);
	form_do(time_date, TIME);
	form_dial(FMD_FINISH, wx, wy, ww, wh, wx, wy, ww, wh);
	  
	dsel_obj(time_date, OK);

	graf_mouse(M_ON, 0L);		/* Turn on mouse form		*/
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

	if (!rsrc_load("exampl12.rsc"))
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
	rsrc_gaddr(R_TREE,   TIMEDATE, &time_date);
	rsrc_gaddr(R_STRING, ABOALERT, &about_alert);
	rsrc_gaddr(R_STRING, QUIALERT, &quit_alert);
	
	menu_bar(main_menu, TRUE);	/* Turn on menu, disable items  */
	menu_ienable(main_menu,CLOSE,0);
	menu_ienable(main_menu,EDIT, 0);
	
	graf_mouse(ARROW, 0L);		/* Make sure mouse is an arrow	*/

	while(hndl_events());		/* Handle menu & window events	*/

	if (active)			/* If window still active	*/
	
		close_window();		/* Make sure window is closed	*/
	
	menu_bar(main_menu, FALSE);	/* Switch off menu bar		*/
		

	graf_mouse(ARROW, 0L);		/* Make sure mouse is an arrow	*/

	appl_exit();			/* exit AES tidily		*/

        return 0;
}
