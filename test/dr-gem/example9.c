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
BYTE	*wdw_title = "EXAMPLE9";
BYTE	*wdw_info  = "Example of using window library";	

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

	wind_get(handle, WF_CURRXYWH, &cx, &cy, &cw, &ch);

	wind_close(handle);		/* Close window off screen	*/

	graf_shrinkbox(xstart, ystart, HBOX, WBOX, cx, cy, cw, ch);

	wind_delete(handle);		/* Clear window from system	*/
	
	graf_mouse(ARROW, 0L);		/* Change cursor to ARROW	*/
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
static
WORD
open_full(WORD   attributes,
          BYTE * title,
          BYTE * info)
{

	WORD	handle;			/* Window handle		*/
	WORD	high_word;		/* High word address		*/
	WORD	low_word;		/* Low word address		*/

	
	wind_get(DESK, WF_WORKXYWH, &xfull, &yfull, &wfull, &hfull);

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
