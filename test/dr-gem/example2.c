/************************************************************************/
/*	File:	example2.c						*/
/************************************************************************/
/*									*/
/*	Example GEM program which displays an ALERT and			*/
/*	acts on button selection. Shows use of FORM_ALERT.		*/
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

	wbutton=form_alert(0, ADDR("[3][System error][Reset|Ignore|Retry]"));
	
/*	Display a NOTE alert depending on button selection		*/

	switch(wbutton)
	{
	
		case	1:		/* Button 1 - RESET		*/
		{
		
			form_alert(1, ADDR("[1][Reset selected][ OK ]"));
			break;
			
		}
		
		case	2:		/* Button 2 - IGNORE		*/
		{
		
			form_alert(1, ADDR("[1][Ignore selected][ OK ]"));
			break;
			
		}			
	
		case	3:		/* Button 3 - RETRY		*/
		{
		
			form_alert(1, ADDR("[1][Retry selected][ OK ]"));
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

	return(TRUE);			/* ID returned successfully	*/
	
}

/*------------------------------*/
/*	GEMAIN			*/
/*------------------------------*/

GEMAIN()

{

	if (!initialise())
	
		return(FALSE);
		
	do_alert();			/* process alert		*/

	appl_exit();			/* exit AES tidily		*/

}
