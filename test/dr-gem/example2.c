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

/*------------------------------*/
/*	Local variables		*/
/*------------------------------*/

int apid;                                       /* ID returned by appl_init */

/*------------------------------*/
/*	Test Procedure		*/
/*------------------------------*/

void
do_alert(void)

{

	WORD	wbutton;		/* Exit button from ALERT	*/
	
        /* No default default, STOP ICON */
	wbutton=form_alert(0, "[3][System error][Reset|Ignore|Retry]");
	
/*	Display a NOTE alert depending on button selection		*/

	switch(wbutton)
	{
	
		case	1:		/* Button 1 - RESET		*/
		{
		
			form_alert(1, "[1][Reset selected][ OK ]");
			break;
			
		}
		
		case	2:		/* Button 2 - IGNORE		*/
		{
		
			form_alert(1, "[1][Ignore selected][ OK ]");
			break;
			
		}			
	
		case	3:		/* Button 3 - RETRY		*/
		{
		
			form_alert(1, "[1][Retry selected][ OK ]");
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

        do_alert();                             /* Test what we want to test */

        appl_exit();
        return(TRUE);			        /* ID returned successfully */
	
}
