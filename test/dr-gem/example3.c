/************************************************************************/
/*	File:	example3.c						*/
/************************************************************************/
/*									*/
/*	Example GEM program which displays ALERT forms from		*/
/*	a resource file. Example of using FORM and RESOURCE		*/
/*	libraries. Modified from EXAMPLE2.C.				*/
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
#include "example3.h"				/* resource file offsets  */

/*------------------------------*/
/*	Global GEM arrays	*/
/*------------------------------*/

GLOBAL WORD	contrl[11];		/* control inputs		*/
GLOBAL WORD	intin[80];		/* max string length		*/
GLOBAL WORD	ptsin[256];		/* polygon fill points		*/
GLOBAL WORD	intout[45];		/* open workstation output	*/
GLOBAL WORD	ptsout[12];		/* points out array		*/

/*------------------------------*/
/*	Local variables		*/
/*------------------------------*/

WORD	gl_apid;			/* ID returned by appl_init 	*/
LONG	sys_alert;			/* Holds System alert message	*/
LONG	reset_alert;			/* Holds reset alert message	*/
LONG	ignore_alert;			/* Holds ignore alert message	*/
LONG	retry_alert;			/* Holds retry alert message	*/

/*------------------------------*/
/*	application code	*/
/*------------------------------*/

/*------------------------------*/
/*	do_alert		*/
/*------------------------------*/

do_alert()

{

	WORD	wbutton;		/* Exit button from ALERT	*/
	
/*	No default default, STOP ICON					*/

	wbutton=form_alert(0, sys_alert);
	
/*	Display a NOTE alert depending on button selection		*/

	switch(wbutton)
	{
	
		case	1:		/* Button 1 - RESET		*/
		{
		
			form_alert(1, reset_alert);
			break;
			
		}
		
		case	2:		/* Button 2 - IGNORE		*/
		{
		
			form_alert(1, ignore_alert);
			break;
			
		}			
	
		case	3:		/* Button 3 - RETRY		*/
		{
		
			form_alert(1, retry_alert);
			break;
		
		}	
				
	}

}

/*------------------------------*/
/*	initialise		*/
/*------------------------------*/

WORD	initialise()

{

	gl_apid = appl_init();		/* return application ID	*/
	
	if (gl_apid == -1)

		return(FALSE);		/* unable to use AES		*/

	if (!rsrc_load(ADDR("EXAMPLE3.RSC")))
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
		
	rsrc_gaddr(R_STRING, SYSALERT, &sys_alert);
	rsrc_gaddr(R_STRING, RESALERT, &reset_alert);
	rsrc_gaddr(R_STRING, IGNALERT, &ignore_alert);
	rsrc_gaddr(R_STRING, RETALERT, &retry_alert);
	
	do_alert();			/* process alert		*/

	appl_exit();			/* exit AES tidily		*/

}
