/************************************************************************/
/*	File:	example.c						*/
/************************************************************************/
/*									*/
/*	Shows layout of a GEM application.				*/
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

GLOBAL WORD	contrl[11];			/* control inputs	   */
GLOBAL WORD	intin[80];			/* max string length	   */
GLOBAL WORD	ptsin[256];			/* polygon fill points	   */
GLOBAL WORD	intout[45];			/* open workstation output */
GLOBAL WORD	ptsout[12];			/* points out array	   */

/*------------------------------*/
/*	Local variables		*/
/*------------------------------*/

WORD	gl_apid;			/* ID returned by appl_init 	*/

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
/*	Application code	*/
/*------------------------------*/

GEMAIN()
{
}
