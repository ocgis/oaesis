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
#define CHAR  char

/*------------------------------*/
/*	Local variables		*/
/*------------------------------*/

WORD    apid;                           /* ID returned by appl_init */
CHAR *	sys_alert;			/* Holds System alert message	*/
CHAR *	reset_alert;			/* Holds reset alert message	*/
CHAR *	ignore_alert;			/* Holds ignore alert message	*/
CHAR *	retry_alert;			/* Holds retry alert message	*/

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
/*	Main code               */
/*------------------------------*/

int main ()
{

        apid = appl_init();		        /* return application ID */
        if (apid == -1)
		return(FALSE);		        /* unable to use AES */

	if (!rsrc_load("./example3.rsc"))
	{
		form_alert(1, "[3][Unable to load resource][ Abort ]");
		return(FALSE);		/* unable to load resource	*/
	}

        do_alert();                             /* Test what we want to test */

        appl_exit();
        return(TRUE);			        /* ID returned successfully */
	
}
