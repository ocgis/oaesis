/************************************************************************/
/*	File:	example7.c						*/
/************************************************************************/
/*									*/
/*	Example GEM program which displays a dialog box from		*/
/*	a resource file. Example of using FORM library calls		*/
/*	for SHRINK and GROW. Introduces object handling.		*/
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
#include "example7.h"				/* resource file offsets  */

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
#define	DESK	0			/* Desk area identifier		*/
#define	WBOX	21			/* Initial width for GROWBOX	*/
#define	HBOX	21			/* Initial height for GROWBOX	*/
#define	TE_PTEXT(x)	(x + 0)		/* return pointer to text	*/

/*------------------------------*/
/*	Local variables		*/
/*------------------------------*/

WORD	gl_apid;			/* ID returned by appl_init 	*/
WORD	xfull;				/* Desk area x coordinate	*/
WORD	yfull;				/* Desk area y coordinate	*/
WORD	wfull;				/* Desk area width		*/
WORD	hfull;				/* Desk area height		*/
WORD	xstart;				/* Holds x of screen centre	*/
WORD	ystart;				/* Holds y of screen centre	*/
LONG	time_date;			/* Address of TIMEDATE dialog	*/
LONG	display;			/* Address of DISPLAY dialog	*/

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
	
	if (wobject == QUIT)
	
		return(FALSE);
		
	return(TRUE);
	
}

/*------------------------------*/
/*	process dialog		*/
/*------------------------------*/

WORD	process_dialog()

{

	BYTE	time_field[8];		/* Space for TIME field		*/
	BYTE	date_field[8];		/* Space for DATE field		*/

	get_field(time_date, TIME, time_field, sizeof(time_field));
	get_field(time_date, DATE, date_field, sizeof(time_field));
	set_field(display, DISTIME, time_field, sizeof(time_field));
	set_field(display, DISDATE, date_field, sizeof(date_field));
			
	do_dialog(display, 0);
	
}

/*------------------------------*/
/*	initialise		*/
/*------------------------------*/

WORD	initialise()

{

	gl_apid = appl_init();		/* return application ID	*/
	
		
	if (gl_apid == -1)

		return(FALSE);		/* unable to use AES		*/

	if (!rsrc_load(ADDR("EXAMPLE7.RSC")))
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

	rsrc_gaddr(R_TREE, TIMEDATE, &time_date);
	rsrc_gaddr(R_TREE, DISPLAY,  &display);

	graf_mouse(ARROW, 0L);		/* Make sure mouse is an arrow	*/

	while (do_dialog(time_date, TIME));
	
	process_dialog();		/* get fields & display again	*/
	
	appl_exit();			/* exit AES tidily		*/

}
