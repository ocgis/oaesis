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

#include "appl.h"
/*#include "boot.h"*/
#include "debug.h"
#include "evnt.h"
#include "form.h"
#include "fsel.h"
#include "gemdefs.h"
#include "lib_global.h"
#include "mintdefs.h"
#include "menu.h"
#include "mesagdef.h"
#include "lib_misc.h"
#include "objc.h"
#include "resource.h"
#include "rsrc.h"
#include "srv_calls.h"
#include "types.h"
#include "wind.h"

#ifdef HAVE_SYSVARS_H
#include <sysvars.h>
#endif

/****************************************************************************
 * Macros                                                                   *
 ****************************************************************************/

#define MENU_GEN_ERR	0
#define MENU_GEN_OK		1

/****************************************************************************
 * Typedefs of module global interest                                       *
 ****************************************************************************/

/*MENUREG is used to keep track of the application menu*/
typedef struct menureg {
	WORD apid;            /*Owner of entry       */
	BYTE *name;           /*Application name     */
	
	struct menureg *mn_next; /*Pointer to next entry*/}MENUREG;

/****************************************************************************
 * Module global variables                                                  *
 ****************************************************************************/

/*
 *	Global struct for menu lib.. Created by menu_handl_init()
 */
struct {
	WORD menu_handl_apid; /* Set to the apid of the menu_handler process. */
} mglob = {
	-1
};

static BYTE progpath[500],progfile[70];

static GLOBAL_ARRAY global_array;

SRV_APPL_INFO appl_info;

/****************************************************************************
 * Public functions                                                         *
 ****************************************************************************/

/****************************************************************************
 * Menu_init_module                                                         *
 *  Initialize menu module.                                                 *
 ****************************************************************************/
WORD                   /*                                                   */
Menu_init_module(void) /*                                                   */
/****************************************************************************/
{
	strcpy(progpath,"u:\\*");
	
	strcpy(progfile,"");

	return 0;
}

/****************************************************************************
 *  Menu_exit_module                                                        *
 *   Shutdown menu module.                                                  *
 ****************************************************************************/
void                   /*                                                   */
Menu_exit_module(void) /*                                                   */
/****************************************************************************/
{
  Appl_do_exit(mglob.menu_handl_apid);
}


/****************************************************************************
 *  Menu_handler                                                            *
 *   Handle main menu events.                                               *
 ****************************************************************************/
void                   /*                                                   */
Menu_handler(          /*                                                   */
WORD   apid,
BYTE * envp[])         /* Environment string.                               */
/****************************************************************************/
{
  RECT    f;
  MENUMSG msg;
  WORD    quit = FALSE;
  WORD    i = 0;
  GLOBAL_APPL * globals = get_globals (apid);

  fprintf (stderr, "oaesis: menu.c: Entering Menu_handler\n");

  mglob.menu_handl_apid = Appl_do_init (&global_array);
  
  Srv_get_appl_info(mglob.menu_handl_apid,&appl_info);
  
  while(envp[i]) {
    Srv_shel_write(global_array.apid,SWM_ENVIRON,ENVIRON_CHANGE,0,envp[i],
		   NULL);
    i++;
  };
  
  Srv_menu_register(mglob.menu_handl_apid,"  oAESis");

  Srv_menu_bar(mglob.menu_handl_apid,globals->common->menutad,MENU_INSTALL);
  
  Wind_do_get (mglob.menu_handl_apid,
               0,
               WF_CURRXYWH,
               &globals->common->deskbgtad[0].ob_x,
               &globals->common->deskbgtad[0].ob_y,
               &globals->common->deskbgtad[0].ob_width,
               &globals->common->deskbgtad[0].ob_height,
               TRUE);
  
  Srv_wind_set(mglob.menu_handl_apid,0,WF_NEWDESK,
	       (WORD)(((LONG)globals->common->deskbgtad) >> 16),
	       (WORD)((LONG)globals->common->deskbgtad),0,0);
  
  /*  Boot_start_programs(mglob.menu_handl_apid);*/
							
  /* Start waiting for messages and rect 1 */
  while(!quit) {
    EVENTIN  ei;
    EVENTOUT eo;
    
    ei.events = MU_MESAG | MU_KEYBD;
    
    Evnt_do_multi(mglob.menu_handl_apid,
		  appl_info.eventpipe,
		  appl_info.msgpipe,
		  &ei,(COMMSG *)&msg,&eo,0);
    
    if(MU_MESAG & eo.events) {
      switch(msg.type) {
      case MN_SELECTED:
	switch(msg.title) {
	case MENU_FILE:
	  switch(msg.item) {
	  case MENU_QUIT:
	    quit = TRUE;
	    break;
	    
	  case MENU_LAUNCHAPP:
	    {
	      BYTE	execpath[128];
	      BYTE	oldpath[128];
	      BYTE *tmp;
	      WORD button;
	      
	      
	      Fsel_do_exinput(mglob.menu_handl_apid,
			      appl_info.vid,
			      appl_info.eventpipe,
			      &button,
			      "Select program to start",progpath,progfile);
	      
	      if(button == FSEL_OK) {
		LONG err;
		BYTE newpath[128];
		
		strcpy(newpath,progpath);
		
		tmp = strrchr(newpath,'\\');
		
		if(tmp) {
		  *tmp = '\0';
		  sprintf(execpath,"%s\\%s",newpath,progfile);
		}
		else {
		  strcpy(execpath,progfile);
		};
		
		Dgetpath(oldpath,0);
		
		Misc_setpath(newpath);
		
		err = Pexec(100,execpath,0L,0L);
		
		Misc_setpath(oldpath);
		
		if(err < 0) {
		  Form_do_error(mglob.menu_handl_apid,
				appl_info.eventpipe,
				(WORD) -err - 31);
		};
	      };
	    };
	    break;
	  };
	  break;
	  
	case MENU_OAESIS:
	  switch(msg.item) {
	  case MENU_INFO:
	    Form_do_center (apid, globals->common->informtad, &f);
	    Form_do_dial(mglob.menu_handl_apid,
			 FMD_START,&f,&f);
	    Objc_do_draw (apid, globals->common->informtad,0,9,&f);
	    Form_do_do(mglob.menu_handl_apid,
		       appl_info.eventpipe,
		       globals->common->informtad,0);
	    globals->common->informtad[INFOOK].ob_state &= ~SELECTED;
	    Form_do_dial(mglob.menu_handl_apid,
			 FMD_FINISH,&f,&f);
	    break;
	  };
	  break;
	};
	break;
	
      default:
      	DB_printf("Unknown message %d in Menu_handler\r\n",msg.type);
      };
    };
    
    if(MU_KEYBD & eo.events) {
      if((eo.kc & 0xff) == 'q') {
	quit = TRUE;
      };
    };
  };

  fprintf (stderr, "oaesis: menu.c: Leaving Menu_handler\n");
}

/****************************************************************************
 *  Menu_bar                                                                *
 *   0x001e menu_bar() library code.                                        *
 ****************************************************************************/
void              /*                                                        */
Menu_bar(         /*                                                        */
AES_PB *apb)      /* Pointer to AES parameter block.                        */
/****************************************************************************/
{
	apb->int_out[0] =
		Srv_menu_bar(apb->global->apid,(OBJECT *)apb->addr_in[0],apb->int_in[0]);
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

/****************************************************************************
 * Menu_register                                                            *
 *  0x0023 menu_register().                                                 *
 ****************************************************************************/
void              /*                                                        */
Menu_register(    /*                                                        */
AES_PB *apb)      /* Pointer to AES parameter block.                        */
/****************************************************************************/
{
	apb->int_out[0] = Srv_menu_register(apb->int_in[0],
													(BYTE *)apb->addr_in[0]);
}

