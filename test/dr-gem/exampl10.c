/************************************************************************/
/*	File:	exampl10.c						*/
/************************************************************************/
/*									*/
/*	Extension of example 9. Shows further handling of window	*/
/*	events. 			 				*/
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
#define	PSIZE	100			/* Page size in scroller units	*/
#define	LSIZE	10			/* Line size in scroller units	*/

/*------------------------------*/
/*	Local variables		*/
/*------------------------------*/

WORD	gl_apid;			/* ID returned by appl_init 	*/
WORD	gl_rmsg[8];			/* Message buffer		*/
WORD *  ad_rmsg;			/* Pointer to message buffer	*/
int	xfull;				/* Desk area X coordinate	*/
int	yfull;				/* Desk area Y coordinate	*/
int	wfull;				/* Desk area width		*/
int	hfull;				/* Desk area height		*/
WORD	xstart;				/* Screen centre X position	*/
WORD	ystart;				/* Screen centre Y position	*/
WORD	w1handle;			/* Handle for window 1		*/
BYTE	*wdw_title = "EXAMPL10";
BYTE	*wdw_info  = "More advanced example of using window library";	

static
void
do_hslider(WORD handle,
           WORD position);

static
void
do_vslider(WORD handle,
           WORD position);

/*------------------------------*/
/*	Application code	*/
/*------------------------------*/

/*------------------------------*/
/*	do_arrowed		*/
/*------------------------------*/
/* Support paging/column moves	*/
static
void
do_arrowed(WORD handle,
           WORD action)
{
	int	hslider;		/* Horizontal slider position	*/
	int	vslider;		/* Vertical slider position	*/
	
	wind_get(handle, WF_HSLIDE, &hslider, 0, 0, 0);
	wind_get(handle, WF_VSLIDE, &vslider, 0, 0, 0);
	
	switch(action)
	{
		case	WA_UPPAGE:	/* Up a page requested		*/
		{
			vslider -= PSIZE;
			
			if (vslider < 1)
				vslider = 1;
			break;
		}

		case	WA_DNPAGE:	/* Down a page requested	*/
		{
			vslider += PSIZE;
			
			if (vslider > 1000)
				vslider = 1000;
			break;
		}
		
		case	WA_UPLINE:	/* Up a line requested		*/
		{
			vslider -= LSIZE;
			
			if (vslider < 1)
				vslider = 1;
			break;
		}

		case	WA_DNLINE:	/* Down a line requested	*/
		{
			vslider += LSIZE;
						
			if (vslider > 1000)
				vslider = 1000;
			break;
		}

		case	WA_LFPAGE:	/* Left a page requested	*/
		{
			hslider -= PSIZE;
			
			if (hslider < 1)
				hslider = 1;
			break;
		}

		case	WA_RTPAGE:	/* Right a page requested	*/
		{
			hslider += PSIZE;
			
			if (hslider > 1000)
				hslider = 1000;
			break;
		}

		case	WA_LFLINE:	/* Left a line requested	*/
		{
			hslider -= LSIZE;
			
			if (hslider < 1)
				hslider = 1;
			break;
		}

		case	WA_RTLINE:	/* Right a line requested	*/
		{
			hslider += LSIZE;
						
			if (hslider > 1000)
				hslider = 1000;
			break;
		}
	}

	do_hslider(handle, hslider);	/* Alter horizontal slider	*/
	do_vslider(handle, vslider);	/* Alter vertical slider	*/
	
}

/*------------------------------*/
/*	do_hslider		*/
/*------------------------------*/
/* Alter horizontal slider	*/
static
void
do_hslider(WORD handle,
           WORD position)
{
	wind_set(handle, WF_HSLIDE, position, 0, 0, 0);
}

/*------------------------------*/
/*	do_vslider		*/
/*------------------------------*/
/* Alter vertical slider	*/
static
void
do_vslider(WORD handle,
           WORD position)
{
	wind_set(handle, WF_VSLIDE, position, 0, 0, 0);
}

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
/*	full_window		*/
/*------------------------------*/
static
VOID	full_window(handle)		/* Toggle between prev. & full	*/

WORD	handle;				/* Window handle		*/

{
	int	cx, cy, cw, ch;		/* Current XYWH parameters	*/
	int	px, py, pw, ph;		/* Previous XYWH parameters	*/
	int	fx, fy, fw, fh;		/* Full window XYWH parameters	*/
	
	wind_get(handle, WF_CXYWH, &cx, &cy, &cw, &ch);
	wind_get(handle, WF_PXYWH, &px, &py, &pw, &ph);
	wind_get(handle, WF_FXYWH, &fx, &fy, &fw, &fh);
				
	if ((cx == fx) && (cy == fy) && (cw == fw) && (ch == fh))
	{
		graf_shrinkbox(px, py, pw, ph, fx, fy, fw, fh);
		size_window(handle, px, py, pw, ph);
	}
	else
	{
		graf_growbox(cx, cy, cw, ch, fx, fy, fw, fh);
		size_window(handle, fx, fy, fw, fh);
	}	 
}

/*------------------------------*/
/*	hndl_window		*/
/*------------------------------*/
static
WORD
hndl_window(void)
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
		
		case	WM_FULLED:	/* Window full has been clicked	*/
		{
			full_window(wdw_hndl);
			break;
		}
		
		case	WM_ARROWED:	/* Page/Row scrolling requested	*/
		{

			evnt_action = gl_rmsg[4];
			
			do_arrowed(wdw_hndl, evnt_action);
			break;	
		}
		
		case	WM_HSLID:	/* Horizontal slider moved	*/
		{

			evnt_action = gl_rmsg[4];
			
			do_hslider(wdw_hndl, evnt_action);
			break;
		}
		
		case	WM_VSLID:	/* Vertical slider moved	*/
		{

			evnt_action = gl_rmsg[4];
			
			do_vslider(wdw_hndl, evnt_action);
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

	return(TRUE);			/* ID returned successfully	*/
	
}

/*------------------------------*/
/*	GEMAIN			*/
/*------------------------------*/
int
main(void)
{

	WORD	win_attr;		/* Window attributes		*/

	if (!initialise())
        {
          return -1;
	}
	
	win_attr  = NAME|CLOSER|FULLER|MOVER|INFO|SIZER|UPARROW|DNARROW;
	win_attr |= VSLIDE|LFARROW|RTARROW|HSLIDE;
	
	w1handle = open_full(win_attr, wdw_title, wdw_info);
	
	if (w1handle > 0)
	
		while(hndl_window());
			
	appl_exit();			/* Exit AES tidily		*/

	return 0;
}
