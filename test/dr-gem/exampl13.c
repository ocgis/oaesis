/************************************************************************/
/*	File:	exampl13.c						*/
/************************************************************************/
/*									*/
/*	Example GEM program which makes use of dialogs, menus		*/
/*	and mouse events. Example of using event multi.			*/
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
#include "exampl13.h"				/* resource file offsets  */

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

#define	ARROW	0			/* Arrow cursor form for MOUSE	*/
#define	CROSS	5			/* Thin cross hair		*/
#define	DESK	0			/* Desk area identifier		*/
#define	WBOX	21			/* Initial width for GROWBOX	*/
#define	HBOX	21			/* Initial height for GROWBOX	*/
#define	TE_PTEXT(x)	(x + 0)		/* return pointer to text	*/

/*------------------------------*/
/*	Local variables		*/
/*------------------------------*/

WORD	gl_apid;			/* ID returned by appl_init 	*/
WORD	gl_rmsg[8];			/* Message buffer		*/
LONG	ad_rmsg;			/* LONG pointer to message buff	*/
WORD	xfull;				/* Desk area x coordinate	*/
WORD	yfull;				/* Desk area y coordinate	*/
WORD	wfull;				/* Desk area width		*/
WORD	hfull;				/* Desk area height		*/
WORD	xstart;				/* Holds x of screen centre	*/
WORD	ystart;				/* Holds y of screen centre	*/
UWORD	mousex;				/* Holds mouse x position	*/
UWORD	mousey;				/* Holds mouse y position	*/
UWORD	bstate;				/* Holds button state		*/
UWORD	kstate;				/* Holds keyboard state		*/
UWORD	krtrn;				/* Holds keyboard return state	*/
UWORD	brtrn;				/* Holds button return state	*/
WORD	m1inout;			/* Enter or exit flag for mouse	*/
LONG	main_menu;			/* Address of MAINMENU menu	*/
LONG	open_dial;			/* Address of OPENDIAL dialog	*/
LONG	close_dial;			/* Address of CLOSDIAL dialog	*/
LONG	about_dial;			/* Address of ABOUT dialog	*/
LONG	quit_alert;			/* Address of QUIT alert tree	*/

/*------------------------------*/
/*	application code	*/
/*------------------------------*/

/*------------------------------*/
/*	get_field		*/
/*------------------------------*/

VOID	get_field(tree, item, field, length)

LONG	tree;				/* object tree			*/
WORD	item;				/* item number			*/
BYTE	*field;				/* field area			*/
WORD	length;				/* length of field		*/

{

	LONG	ted_info;		/* tedinfo address		*/
	LONG	te_ptext;		/* text pointer			*/
	BYTE	i;			/* counter			*/
		
	ted_info=LLGET(OB_SPEC(item));	/* return ted info for item	*/
	te_ptext=TE_PTEXT(ted_info);	/* return text address		*/
	
	for (i = 0; i < length; i++)
	
		*field++ = LBGET(te_ptext++);

}

/*------------------------------*/
/*	set_field		*/
/*------------------------------*/

VOID	set_field(tree, item, field, length)

LONG	tree;				/* object tree			*/
WORD	item;				/* item number			*/
BYTE	*field;				/* field area			*/
WORD	length;				/* length of field		*/

{

	LONG	ted_info;		/* tedinfo address		*/
	LONG	te_ptext;		/* text pointer			*/
	BYTE	i;			/* counter			*/
	
	ted_info=LLGET(OB_SPEC(item));	/* return ted info for item	*/
	te_ptext=TE_PTEXT(ted_info);	/* return text address		*/
	
	for (i = 0; i < length; i++)
	
		LBSET(te_ptext++, *field++);

}

/*------------------------------*/
/*	undo_obj		*/
/*------------------------------*/

VOID	undo_obj(tree, item, bit)	/* Clear bit in item of tree	*/

LONG	tree;				/* Tree containing object	*/
WORD	item;				/* Item to be affected		*/
WORD	bit;				/* Bit number to change		*/

{
	WORD	wstate;			/* Holds current state		*/
	LONG	wobstate;		/* Holds object state word	*/
	
	wobstate = OB_STATE(item);	/* Get OB_STATE addr. for item	*/
	
	wstate = LWGET(wobstate);	/* Get current state of item	*/
	wstate = wstate & ~bit;		/* Remove bit by AND of comp.	*/
	 
	LWSET(wobstate, wstate);	/* Set new state of item	*/
}

/*------------------------------*/
/*	dsel_obj		*/
/*------------------------------*/

VOID	dsel_obj(tree, item)		/* Deselect item of tree	*/

LONG	tree;				/* Tree containing object	*/
WORD	item;				/* Item to deselect		*/

{
	undo_obj(tree, item, SELECTED);	/* Clear SELECTED state flag	*/
}

/*------------------------------*/
/*	do_menu			*/
/*------------------------------*/

WORD	do_menu()

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
					do_dialog(about_dial, 0);
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
					do_dialog(open_dial, USERNAME);
					process_dialog();
					break;
				}
				
				case	CLOSE:
				{
					do_dialog(close_dial, 0);
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
/*	do_dialog		*/
/*------------------------------*/

WORD	do_dialog(tree, start_obj)	/* Handles dialog in centre	*/

LONG	tree;				/* Dialog tree address		*/
WORD	start_obj;			/* Start object number		*/

{

	WORD	x, y, w, h;		/* Holds X,Y,W,H of dialog	*/
	WORD	wobject;		/* Object that caused exit	*/
	
	xstart = wfull / 2;		/* Calculate x of screen centre	*/
	ystart = hfull / 2;		/* Calculate y of screen centre	*/
	
	form_center(tree, &x, &y, &w, &h);
	
	form_dial(FMD_START, xstart, ystart, WBOX, HBOX, x, y, w, h);
	form_dial(FMD_GROW,  xstart, ystart, WBOX, HBOX, x, y, w, h);
		
	objc_draw(tree, ROOT, MAX_DEPTH, x, y, w, h);
	
	wobject = form_do(tree, start_obj);
	
	form_dial(FMD_SHRINK, xstart, ystart, WBOX, HBOX, x, y, w, h);
	form_dial(FMD_FINISH, xstart, ystart, WBOX, HBOX, x, y, w, h);
	  
	dsel_obj(tree, wobject);
	
	return(wobject);
	
}

/*------------------------------*/
/*	process dialog		*/
/*------------------------------*/

WORD	process_dialog()

{

	BYTE	user_name[30];		/* Space for user name field	*/

	get_field(open_dial,  USERNAME, user_name, sizeof(user_name));
	set_field(close_dial, USERDISP, user_name, sizeof(user_name));
			
}

/*------------------------------*/
/*	do_events		*/
/*------------------------------*/

VOID	do_events()

{

	UWORD	ev;			/* Event list			*/
	UWORD	mflags;			/* Event flags			*/
	
	mflags = MU_MESAG|MU_M1;	/* Wait on message & mouse evnt */
		
	m1inout = TRUE;			/* Set mouse rectangle 1 LEAVE	*/

	FOREVER
	{
		ev = evnt_multi(mflags,	/* Flags to say which events	*/
			
					/* EVNT_BUTTON parameters	*/
				0,	/* Number of clicks to wait for	*/
				0,	/* What mouse buttons we want	*/
				0,	/* What button state up/down	*/
			
					/* EVNT_MOUSE rectangle 1	*/
				m1inout,/* Event on ENTER or EXIT ?	*/
				xfull,	/* X of enter/leave rectangle	*/
				yfull,	/* Y of enter/leave rectangle	*/
				wfull,	/* W of enter/leave rectangle	*/
				hfull,	/* H of enter/leave rectangle	*/
			   
					/* EVNT_MOUSE rectangle 2	*/
				0,	/* Event on ENTER or EXIT ?	*/
				0,	/* X of enter/leave rectangle	*/
				0,	/* Y of enter/leave rectangle	*/
				0,	/* W of enter/leave rectangle	*/
				0,	/* H of enter/leave rectangle	*/
			
					/* EVNT_MESAG parameters	*/
				ad_rmsg,/* Address of message buffer	*/
				
					/* EVNT_TIMER parameters	*/
				0,	/* Low count of timer value	*/
				0,	/* High count of timer value	*/
				
					/* EVNT_MOUSE returns		*/
				&mousex,/* Mouse X when event occurred	*/
				&mousey,/* Mouse Y when event occurred	*/
				&bstate,/* Button state at event time	*/
				&kstate,/* Keyboard state at event time	*/
				
					/* EVNT_KEYBD returns		*/
				&krtrn,	/* Keyboard code returned	*/
				
					/* EVNT_BUTTON returns		*/
				&brtrn);/* Returned mouse button state	*/
				
		if (ev & MU_MESAG)	/* Message event occurred	*/
		{
			if(do_menu())
			
				continue;

			return;
		}
		
		if (ev & MU_M1)		/* Mouse event occurred		*/
		{
			if (m1inout)
				graf_mouse(ARROW, 0L);
			else
				graf_mouse(CROSS, 0L);
				
			m1inout = !m1inout;
		}
	
	}
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

	if (!rsrc_load(ADDR("EXAMPL13.RSC")))
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
		
	wind_get(DESK, WF_WXYWH, &xfull, &yfull, &wfull, &hfull);

	rsrc_gaddr(R_TREE,   MAINMENU, &main_menu);
	rsrc_gaddr(R_TREE,   ABOUDIAL, &about_dial);
	rsrc_gaddr(R_TREE,   OPENDIAL, &open_dial);
	rsrc_gaddr(R_TREE,   CLOSDIAL, &close_dial);
	rsrc_gaddr(R_STRING, QUIALERT, &quit_alert);
	
	menu_bar(main_menu, TRUE);	/* Switch on menu bar		*/
	
	graf_mouse(CROSS, 0L);	/* Make sure mouse is a cross	*/

	while (do_events());		/* Process events		*/
	
	menu_bar(main_menu, FALSE);	/* Switch off menu bar		*/
		
	appl_exit();			/* exit AES tidily		*/

}
