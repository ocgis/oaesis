/****************************************************************************

 Module
  menu.c
  
 Description
  Menu routines used in oAESis.
  
 Author(s)
 	cg     (Christer Gustavsson <d2cg@dtek.chalmers.se>)
 	marbud (Martin Budsj” <marbud@tripnet.se>) 

 Revision history
 
  951226 cg
   Concatenated menu_lib.c and menu_srv.c into menu.c.
   Added standard header.

  960101 cg
   menu_ienable() implemented in Menu_ienable().
   menu_icheck() implemented in Menu_icheck().
   menu_tnormal() implemented in Menu_tnormal().
  960306 kkp   The menu structure integrated with AP_INFO.    
 Copyright notice
  The copyright to the program code herein belongs to the authors. It may
  be freely duplicated and distributed without fee, but not charged for.
 
 ****************************************************************************/

#define DEBUGLEVEL 0

/****************************************************************************
 * Used interfaces                                                          *
 ****************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_BASEPAGE_H
#include <basepage.h>
#endif

#include <ctype.h>

#ifdef HAVE_MINTBIND_H
#include <mintbind.h>
#endif

#ifdef HAVE_PROCESS_H
#include <process.h>
#endif

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "aesbind.h"
#include "appl.h"
/*#include "boot.h"*/
#include "debug.h"
#include "evnt.h"
#include "form.h"
#include "fsel.h"
#include "graf.h"
#include "lib_comm.h"
#include "lib_global.h"
#include "lib_misc.h"
#include "mintdefs.h"
#include "menu.h"
#include "mesagdef.h"
#include "objc.h"
#include "resource.h"
#include "rsrc.h"
#include "srv_put.h"
#include "srv_interface.h"
#include "types.h"
#include "wind.h"

#ifdef HAVE_SYSVARS_H
#include <sysvars.h>
#endif


/*
** Description
** Creates and updates the menu list entries for the specified menu. 
**
** 1999-01-09 CG
** 1999-08-15 CG
*/
static
WORD
Menu_bar_install (WORD     apid,
                  OBJECT * tree) {
  WORD          i;
  WORD          cheight;
  WORD          cwidth;
  WORD          height;
  WORD          width;
  GLOBAL_APPL * globals = get_globals (apid);

  globals->menu = tree;
  
  Graf_do_handle (&cwidth, &cheight, &width, &height);

  DEBUG2 ("menu.c: Menu_bar_install: cheight = %d", cheight);

  /* Modify width of menu*/
  tree[0].ob_width = globals->common->screen.width;
  tree[tree[0].ob_head].ob_width = globals->common->screen.width;

  /* Modify height of bar ... */
  tree[tree[0].ob_head].ob_height = cheight + 3;
  /* ... and titles */
  tree[tree[tree[0].ob_head].ob_head].ob_height = cheight + 3;

#if 0
  i = tree[tree[tree[0].ob_head].ob_head].ob_head;
  
  while(i != -1) {		
    tree[i].ob_height = cheight + 3;
    
    if(i == tree[tree[i].ob_next].ob_tail) {
      break;
    }
    
    i = tree[i].ob_next;
  }
#endif
  
  /* Mark all drop down menus with HIDETREE and set y position */
  tree[tree[tree[0].ob_head].ob_next].ob_y = cheight + 3;
  
  i = tree[tree[tree[0].ob_head].ob_next].ob_head;
  
  while(i != -1) {		
    tree[i].ob_flags |= HIDETREE;
    
    if(i == tree[tree[i].ob_next].ob_tail) {
      break;
    }
    
    i = tree[i].ob_next;
  }

  /* Calculate menu bar area */
  i = tree[tree[0].ob_head].ob_head;
  
  globals->menu_bar_rect.x =
    tree[0].ob_x + tree[tree[0].ob_head].ob_x + tree[i].ob_x;
  globals->menu_bar_rect.width =
    tree[tree[i].ob_tail].ob_x + tree[tree[i].ob_tail].ob_width;
  globals->menu_bar_rect.y = 0;
  globals->menu_bar_rect.height = cheight + 3;

  DEBUG2 ("menu_bar_rect = %d %d %d %d",
          globals->menu_bar_rect.x,
          globals->menu_bar_rect.y,
          globals->menu_bar_rect.width,
          globals->menu_bar_rect.height);

  /* Return OK */
  return 1;
}


/*
** Exported
*/
WORD
Menu_do_bar (WORD     apid,
             OBJECT * tree,
             WORD     mode) {
  C_MENU_BAR    par;
  R_MENU_BAR    ret;
  GLOBAL_APPL * globals = get_globals (apid);
  
  PUT_C_ALL(MENU_BAR, &par);

  par.tree = tree;
  par.mode = mode;

  CLIENT_SEND_RECV(&par,
                   sizeof (C_MENU_BAR),
                   &ret,
                   sizeof (R_MENU_BAR));

  /*
  ** FIXME
  ** Check retval that the operation went fine
  */
  switch (mode) {
  case MENU_REMOVE :
    globals->menu = NULL;
    break;
  case MENU_INSTALL :
    Menu_bar_install (apid, tree);
    break;
  }

  return ret.common.retval;
}


/*
** Exported
**
** 1999-01-09 CG
*/
void
Menu_bar (AES_PB *apb) {
  apb->int_out[0] = Menu_do_bar (apb->global->apid,
                                 (OBJECT *)apb->addr_in[0],
                                 apb->int_in[0]);
}


/****************************************************************************
 *  Menu_icheck                                                             *
 *   0x001f menu_icheck().                                                  *
 ****************************************************************************/
void              /*                                                        */
Menu_icheck(      /*                                                        */
AES_PB *apb)      /* Pointer to AES parameter block.                        */
/****************************************************************************/
{
	switch(apb->int_in[1]) {
	case UNCHECK:
		((OBJECT *)apb->addr_in[0])[apb->int_in[0]].ob_state &= ~CHECKED;
		apb->int_out[0] = 1;
		break;
	
	default:
		((OBJECT *)apb->addr_in[0])[apb->int_in[0]].ob_state |= CHECKED;
		apb->int_out[0] = 1;
	}
}

/****************************************************************************
 *  Menu_ienable                                                            *
 *   0x0020 menu_ienable().                                                 *
 ****************************************************************************/
void              /*                                                        */
Menu_ienable(     /*                                                        */
AES_PB *apb)      /* Pointer to AES parameter block.                        */
/****************************************************************************/
{
	switch(apb->int_in[1]) {
	case DISABLE:
		((OBJECT *)apb->addr_in[0])[apb->int_in[0]].ob_state |= DISABLED;
		apb->int_out[0] = 1;
		break;

	default:
		((OBJECT *)apb->addr_in[0])[apb->int_in[0]].ob_state &= ~DISABLED;
		apb->int_out[0] = 1;
	};
}

/****************************************************************************
 *  Menu_tnormal                                                            *
 *   0x0021 menu_tnormal().                                                 *
 ****************************************************************************/
void              /*                                                        */
Menu_tnormal(     /*                                                        */
AES_PB *apb)      /* Pointer to AES parameter block.                        */
/****************************************************************************/
{
	switch(apb->int_in[1]) {
	case UNHIGHLIGHT:
		((OBJECT *)apb->addr_in[0])[apb->int_in[0]].ob_state &= ~SELECTED;
		apb->int_out[0] = 1;
		break;

	case HIGHLIGHT:
		((OBJECT *)apb->addr_in[0])[apb->int_in[0]].ob_state |= SELECTED;
		apb->int_out[0] = 1;
		break;
	default:
		DB_printf("%s: Line %d: Menu_tnormal:\r\n"
						"Unknown mode %d!\r\n",__FILE__,__LINE__,apb->int_in[1]);
		apb->int_out[0] = 0;
	};
}


/****************************************************************************
 *  Menu_text                                                               *
 *   0x0022 menu_text().                                                    *
 ****************************************************************************/
void              /*                                                        */
Menu_text(        /*                                                        */
AES_PB *apb)      /* Pointer to AES parameter block.                        */
/****************************************************************************/
{
	OBJECT  *ob = (OBJECT *)apb->addr_in[0];
	TEDINFO *ti;
	
	if(ob->ob_flags & INDIRECT) {
		ti = ob->ob_spec.indirect->tedinfo;
	}
	else {
		ti = ob->ob_spec.tedinfo;
	};

	strcpy(ti->te_ptext,
					(BYTE *)apb->addr_in[1]);
					
	apb->int_out[0] = 1;
}

/*
** Description
** Implementation of menu_register ()
*/
static
WORD
Menu_do_register (WORD   apid,
                  BYTE * title) {
  C_MENU_REGISTER par;
  R_MENU_REGISTER ret;
  GLOBAL_APPL *   globals = get_globals(apid);
 
  PUT_C_ALL(MENU_REGISTER, &par);

  strncpy (par.title, title, sizeof (par.title) - 1);
  par.title[sizeof (par.title) - 1] = 0;

  strcpy(globals->application_name, par.title);

  CLIENT_SEND_RECV(&par,
                   sizeof (C_MENU_REGISTER),
                   &ret,
                   sizeof (R_MENU_REGISTER));

  return ret.common.retval;
}

/*
** Exported
** 0x0023 menu_register ()
**
** 1999-04-11 CG
*/
void
Menu_register (AES_PB * apb) {
  apb->int_out[0] = Menu_do_register (apb->int_in[0],
                                      (BYTE *)apb->addr_in[0]);
}

