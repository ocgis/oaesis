/************************************************************************/
/*	File:	example1.c						*/
/************************************************************************/
/*									*/
/*	Example GEM program which displays an ALERT and			*/
/*	exits. Shows use of FORM_ALERT from FORM library.		*/
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

void testproc(void)
{
                                                /* First button default, No ICON */
        form_alert(1,"[0][Welcome to OSIS|From NoCrew][ OK ]");
}

/*------------------------------*/
/*	Main code               */
/*------------------------------*/

int main ()
{

        apid = appl_init();		        /* return application ID */
        if (apid == -1)
		return(FALSE);		        /* unable to use AES */

        testproc();                             /* Test what we want to test */

        appl_exit();
        return(TRUE);			        /* ID returned successfully */
	
}
