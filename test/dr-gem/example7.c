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

#include <aesbind.h>
#include <vdibind.h>				/* vdi binding structures */

#include "treeaddr.h"				/* tree address macros    */
#include "example7.h"				/* resource file offsets  */

/*------------------------------*/
/*	Local defines		*/
/*------------------------------*/

#define	ARROW	0			/* Arrow cursor form for MOUSE	*/
#define BYTE    char
#define LONG    long
#define VOID    void
#define TRUE    1
#define FALSE   0
#define	WBOX	21			/* Initial width for GROWBOX	*/
#define	HBOX	21			/* Initial height for GROWBOX	*/

/*------------------------------*/
/*	Local variables		*/
/*------------------------------*/

WORD     gl_apid;			/* ID returned by appl_init 	*/
int      xfull;				/* Desk area x coordinate	*/
int      yfull;				/* Desk area y coordinate	*/
int      wfull;				/* Desk area width		*/
int      hfull;				/* Desk area height		*/
WORD     xstart;			/* Holds x of screen centre	*/
WORD     ystart;			/* Holds y of screen centre	*/
OBJECT * time_date;			/* Address of TIMEDATE dialog	*/
OBJECT * display;			/* Address of DISPLAY dialog	*/

/*------------------------------*/
/*	application code	*/
/*------------------------------*/

/*------------------------------*/
/*	get_field		*/
/*------------------------------*/
static
VOID
get_field(OBJECT * tree,
          WORD     item,
          BYTE *   field,
          WORD     length)
{
	TEDINFO * ted_info;		/* tedinfo address		*/
	BYTE *    te_ptext;		/* text pointer			*/
	BYTE      i;			/* counter			*/
		
	ted_info = tree[item].ob_spec.tedinfo; /* return ted info for item */
	te_ptext = ted_info->te_ptext;	       /* return text address      */
	
	for (i = 0; i < length; i++)
        {
          *field++ = *te_ptext++;
        }
}

/*------------------------------*/
/*	set_field		*/
/*------------------------------*/
static
VOID
set_field(OBJECT * tree,
          WORD     item,
          BYTE *   field,
          WORD     length)
{
	TEDINFO * ted_info;		/* tedinfo address		*/
	BYTE *    te_ptext;		/* text pointer			*/
	BYTE      i;			/* counter			*/
	
	ted_info = tree[item].ob_spec.tedinfo;	/* return ted info for item */
	te_ptext = ted_info->te_ptext;          /* return text address      */
	
	for (i = 0; i < length; i++)
        {
          *te_ptext++ = *field++;
        }
}

/*------------------------------*/
/*	undo_obj		*/
/*------------------------------*/
static
VOID
undo_obj(OBJECT * tree,
         WORD     item,
         WORD     bit)
{
	tree[item].ob_spec.index &= ~bit;
}

/*------------------------------*/
/*	dsel_obj		*/
/*------------------------------*/
static
VOID
dsel_obj(OBJECT * tree,
         WORD     item)		/* Deselect item of tree	*/
{
	undo_obj(tree, item, SELECTED);	/* Clear SELECTED state flag	*/
}

/*------------------------------*/
/*	do_dialog		*/
/*------------------------------*/
static
WORD
do_dialog(OBJECT * tree,
          WORD     start_obj)	/* Handles dialog in centre	*/
{
	int     x, y, w, h;		/* Holds X,Y,W,H of dialog	*/
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
static
void
process_dialog(void)
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
static
WORD
initialise(void)
{

	gl_apid = appl_init();		/* return application ID	*/
	
		
	if (gl_apid == -1)

		return(FALSE);		/* unable to use AES		*/

	if (!rsrc_load("example7.rsc"))
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

	wind_get(DESK, WF_WXYWH, &xfull, &yfull, &wfull, &hfull);

	rsrc_gaddr(R_TREE, TIMEDATE, &time_date);
	rsrc_gaddr(R_TREE, DISPLAY,  &display);

	graf_mouse(ARROW, 0L);		/* Make sure mouse is an arrow	*/

	while (do_dialog(time_date, TIME));
	
	process_dialog();		/* get fields & display again	*/
	
	appl_exit();			/* exit AES tidily		*/

        return 0;
}
