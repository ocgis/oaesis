/************************************************************************/
/*	File:	example5.c						*/
/************************************************************************/
/*									*/
/*	Example GEM program which looks more closely at the MENU	*/
/*	library. Example of enabling and disabling MENU items		*/
/*	changing text and setting check marks.				*/
/*									*/
/************************************************************************/

/*------------------------------*/
/*	Standard includes	*/
/*------------------------------*/

#include <aesbind.h>
#include <vdibind.h>				/* vdi binding structures */

#include "treeaddr.h"				/* tree address macros    */
#include "example5.h"				/* resource file offsets  */

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

WORD	 gl_apid;			/* ID returned by appl_init 	*/
WORD	 gl_rmsg[8];			/* message buffer		*/
WORD *	 ad_rmsg;			/* LONG pointer to message buff */
OBJECT * main_menu;			/* Holds menu tree for MAINMENU	*/
BYTE *   about_alert;			/* Holds ABOUT alert message    */
BYTE *   quit_alert;			/* Holds QUIT alert message	*/
BYTE	 wimode;			/* Insert mode 0 = off 1 = on	*/

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
					menu_icheck(main_menu, OPEN,  1);
					menu_icheck(main_menu, CLOSE, 0);
					break;
				}
				
				case	CLOSE:
				{
					menu_icheck(main_menu, CLOSE, 1);
					menu_icheck(main_menu, OPEN,  0);
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

		case	SWITMENU:	
		{
			switch(menu_option)
			{
				case	ON:
				{
					menu_ienable(main_menu, OFF, 1);
					menu_ienable(main_menu, ON,  0);
					break;
				}
				
				case	OFF:
				{
					menu_ienable(main_menu, ON,  1);
					menu_ienable(main_menu, OFF, 0);
					break;
				}
			}
		
			menu_tnormal(main_menu, SWITMENU, 1);
			return(TRUE);
		}

		case	WORDMENU:	
		{
			switch(menu_option)
			{
				case	INSERT:
				{
					if (wimode == 0)
					{
					    menu_text(main_menu,
					  	      INSERT,
						      "  Insert off");
					    wimode = 1;
					}
					else
					{
					    menu_text(main_menu,
						      INSERT,
						      "  Insert on ");
					    wimode = 0;
					}
					break;
				}
			}
		
			menu_tnormal(main_menu, WORDMENU, 1);
			return(TRUE);
		}
	}

        return TRUE;
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

	if (!rsrc_load("example5.rsc"))
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
	
	wimode = 0;			/* set insert off initially	*/

	menu_bar(main_menu, TRUE);	/* Switch on menu bar		*/
	graf_mouse(ARROW, 0L);		/* Make sure mouse is an arrow	*/

	while (do_menu());		/* process menu			*/

	menu_bar(main_menu, FALSE);	/* Switch off menu bar		*/

	appl_exit();			/* exit AES tidily		*/

        return 0;
}
