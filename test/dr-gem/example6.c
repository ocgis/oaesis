/************************************************************************/
/*	File:	example6.c						*/
/************************************************************************/
/*									*/
/*	Example GEM program which displays a dialog box from		*/
/*	a resource file. Example of using FORM library calls		*/
/*	and FORM_DO.							*/
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
#include "example6.h"				/* resource file offsets  */

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

/*------------------------------*/
/*	Local variables		*/
/*------------------------------*/

WORD	gl_apid;			/* ID returned by appl_init 	*/
LONG	time_date;			/* Address of TIMEDATE dialog	*/

/*------------------------------*/
/*	application code	*/
/*------------------------------*/

/*------------------------------*/
/*	do_dialog		*/
/*------------------------------*/

WORD	do_dialog(tree, start_obj)	/* Handles dialog in centre	*/

LONG	tree;				/* Dialog tree address		*/
WORD	start_obj;			/* Start object number		*/

{

	WORD	x, y, w, h;		/* Holds X,Y,W,H of dialog	*/
	WORD	wobject;		/* Object that caused exit	*/
	
	form_center(tree, &x, &y, &w, &h);
	
	form_dial(FMD_START, x, y, w, h, x, y, w, h);
		
	objc_draw(tree, ROOT, MAX_DEPTH, x, y, w, h);
	
	wobject = form_do(tree, start_obj);
	
	form_dial(FMD_FINISH, x, y, w, h, x, y, w, h);
	  
	if (wobject == QUIT)
	
		return(FALSE);
		
	return(TRUE);
	
}

/*------------------------------*/
/*	initialise		*/
/*------------------------------*/

WORD	initialise()

{

	gl_apid = appl_init();		/* return application ID	*/
	
		
	if (gl_apid == -1)

		return(FALSE);		/* unable to use AES		*/

	if (!rsrc_load(ADDR("EXAMPLE6.RSC")))
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
		
	rsrc_gaddr(R_TREE, TIMEDATE, &time_date);

	graf_mouse(ARROW, 0L);		/* Make sure mouse is an arrow	*/

	while (do_dialog(time_date, TIME));
	
	appl_exit();			/* exit AES tidily		*/

}
