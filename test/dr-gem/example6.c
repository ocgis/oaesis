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

#include <aesbind.h>
#include <vdibind.h>				/* vdi binding structures */

#include "treeaddr.h"				/* tree address macros    */
#include "example6.h"				/* resource file offsets  */

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
OBJECT * time_date;			/* Address of TIMEDATE dialog	*/

/*------------------------------*/
/*	application code	*/
/*------------------------------*/

/*------------------------------*/
/*	do_dialog		*/
/*------------------------------*/
static
WORD
do_dialog(OBJECT * tree,
          WORD     start_obj)	/* Handles dialog in centre	*/
{

	int	x, y, w, h;		/* Holds X,Y,W,H of dialog	*/
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
static
WORD
initialise(void)
{

	gl_apid = appl_init();		/* return application ID	*/
	
		
	if (gl_apid == -1)

		return(FALSE);		/* unable to use AES		*/

	if (!rsrc_load("example6.rsc"))
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
		
	rsrc_gaddr(R_TREE, TIMEDATE, &time_date);

	graf_mouse(ARROW, 0L);		/* Make sure mouse is an arrow	*/

	while (do_dialog(time_date, TIME));
	
	appl_exit();			/* exit AES tidily		*/

        return 0;
}
