/************************************************************************/
/*	File:	example4.c						*/
/************************************************************************/
/*									*/
/*	Example GEM program which displays a MENU and ALERT from	*/
/*	a resource file. Example of using MENU, FORM and RESOURCE	*/
/*	libraries.							*/
/*									*/
/************************************************************************/

/*------------------------------*/
/*	Standard includes	*/
/*------------------------------*/

#include <aesbind.h>
#include <vdibind.h>

#include "treeaddr.h"				/* tree address macros    */
#include "example4.h"				/* resource file offsets  */

/*------------------------------*/
/*	Global GEM arrays	*/
/*------------------------------*/

WORD	contrl[11];		/* control inputs		*/
WORD	intin[80];		/* max string length		*/
WORD	ptsin[256];		/* polygon fill points		*/
WORD	intout[45];		/* open workstation output	*/
WORD	ptsout[12];		/* points out array		*/

/*------------------------------*/
/*	Local defines		*/
/*------------------------------*/

#define	ARROW	0			/* Arrow cursor form for MOUSE	*/
#define BYTE    char
#define LONG    long
#define TRUE    1
#define FALSE   0

/*------------------------------*/
/*	Local variables		*/
/*------------------------------*/

WORD     gl_apid;			/* ID returned by appl_init 	*/
WORD     gl_rmsg[8];			/* message buffer		*/
WORD *   ad_rmsg;			/* LONG pointer to message buff */
OBJECT * main_menu;			/* Holds menu tree for MAINMENU	*/
BYTE *   about_alert;			/* Holds ABOUT alert message    */
BYTE *   open_alert;			/* Holds OPEN alert message	*/
BYTE *   quit_alert;			/* Holds QUIT alert message	*/

/*------------------------------*/
/*	application code	*/
/*------------------------------*/

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
		
	menu_bar(main_menu, TRUE);	/* Switch on menu bar		*/
	graf_mouse(ARROW, 0L);		/* Make sure mouse is an arrow	*/
	evnt_mesag(ad_rmsg);		/* wait for menu event		*/
	
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
					form_alert(1, open_alert);
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

        return FALSE;
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

	if (!rsrc_load("example4.rsc"))
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
	
		return(FALSE);
		
	rsrc_gaddr(R_TREE,   MAINMENU, &main_menu);
	rsrc_gaddr(R_STRING, ABOALERT, &about_alert);
	rsrc_gaddr(R_STRING, OPEALERT, &open_alert);
	rsrc_gaddr(R_STRING, QUIALERT, &quit_alert);
	
	while (do_menu());		/* process menu		*/

	appl_exit();			/* exit AES tidily		*/

        return 0;
}
