/****************************************************************************

 Module
  appl.c
  
 Description
  Application handling routines in oAESis.
  
 Author(s)
 	cg (Christer Gustavsson <d2cg@dtek.chalmers.se>)
 	kkp (Klaus Pedersen) <kkp@gamma.dou.dk>)

 Revision history
 
  960101 cg
   Added standard header.
   Basic version of appl_getinfo implemented.

  960102 cg
   AES_WINDOW mode of appl_getinfo implemented.

  960115 cg
   Appl_get_vid() implemented.
      
  960228 kkp
   Changed the behaviour of TOPAPPL

  980622 cg
   Added check for macros defined by autoconf.

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

#ifdef HAVE_ALLOC_H
#include <alloc.h>
#endif

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#ifdef HAVE_MINTBIND_H
#include <mintbind.h>
#endif

#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef HAVE_SUPPORT_H
#include <support.h>
#endif

#include <unistd.h>

#include "appl.h"
#include "debug.h"
#include "gemdefs.h"
#include "lib_global.h"
#include "srv_calls.h"
#include "srv_put.h"
#include "srv_interface.h"
#include "types.h"

/****************************************************************************
 * Local functions (use static!)                                            *
 ****************************************************************************/

/*
** Description
** Implementation of appl_read ()
**
** 1998-10-04 CG
*/
static
WORD
Appl_do_read (WORD   apid,
              WORD   msgpipe,
              WORD   length,
              void * m) {
  if((apid == APR_NOWAIT) && (Finstat(msgpipe) < length)) {
    return 0;
  };
	
  if(Fread(msgpipe,length,m) < 0) {
    return 0;
  };
		
  return 1;
}

/****************************************************************************
 * Public functions                                                         *
 ****************************************************************************/

/* 0x000a appl_init */

/****************************************************************************
 * Appl_do_init                                                             *
 *  Implementation of appl_init().                                          *
 ****************************************************************************/
WORD                   /* Application id, or -1.                            */
Appl_do_init(          /*                                                   */
GLOBAL_ARRAY * global) /* Global array.                                     */
/****************************************************************************/
{
  WORD        pid = Pgetpid();
  C_APPL_INIT par;
  R_APPL_INIT ret;
  LONG        fnr;
  WORD        work_in[] = {1,7,1,1,1,1,1,1,1,1,2};
  WORD        work_out[57];
  int         count;

  /*
  sprintf(par.msgname,"u:\\pipe\\applmsg.%03d",pid);		
  sprintf(par.eventname,"u:\\pipe\\applevnt.%03d",pid);		

  par.msghandle = par.eventhandle = 0;

  if((fnr = Fopen(par.msgname,0)) >= 0)
  {
    Fclose((WORD)fnr);
  }
  else if((par.msghandle = (WORD)Fcreate(par.msgname,0)) < 0)
  {
    Fclose(par.msghandle);
		
    return -1;
  }
	
  if((fnr = Fopen(par.eventname,0)) >= 0)
  {
    Fclose((WORD)fnr);
  }
  else if((par.eventhandle = (WORD)Fcreate(par.eventname,0)) < 0)
  {
    Fclose(par.msghandle);
    Fclose(par.eventhandle);
		
    return -1;
  }
	

  */

  /* Open connection */
  if (Client_open () == -1) {
    exit (-1);
  }

  par.common.call = SRV_APPL_INIT;
  par.common.pid = getpid ();
  count = Client_send_recv (&par,
                            sizeof (C_APPL_INIT),
                            &ret,
                            sizeof (R_APPL_INIT));

  DB_printf ("srv_calls.c: Srv_appl_init: apid = %d count = %d\n",
             (int)ret.apid, count);
  
  global->apid = ret.apid;	
  global->version = 0x0410;
  global->numapps = -1;
  global->appglobal = 0L;
  global->rscfile = 0L;
  global->rshdr = 0L;
  global->resvd1 = 0;
  global->resvd2 = 0;
  global->int_info = 0L;
  global->maxchar = 0;
  global->minchar = 0;

  if(global->apid >= 0) {
    DB_printf ("appl.c: Appl_do_init: Calling init_global");

    init_global (1, ret.physical_vdi_id);

    return global->apid;
  } else {
  /*
  Vdi_v_clsvwk(par.vid);
  Fclose(par.msghandle);
  Fclose(par.eventhandle);
  */
    return -1;
  }
}


void
Appl_init (AES_PB *apb) {
  apb->int_out[0] = Appl_do_init(apb->global);
}

/* 0x000b appl_read */

void Appl_read(AES_PB *apb) {
  SRV_APPL_INFO appl_info;
	
  Srv_get_appl_info(apb->global->apid,&appl_info);

  apb->int_out[0] = Appl_do_read(apb->int_in[0],
                                 appl_info.msgpipe,
                                 apb->int_in[1],
                                 (void *)apb->addr_in[0]);
}

/* 0x000c appl_write */

void	Appl_write(AES_PB *apb) {
  apb->int_out[0] =	
    Srv_appl_write(apb->int_in[0],apb->int_in[1],(void *)apb->addr_in[0]);
}


/****************************************************************************
 * Appl_find                                                                *
 *  0x000d appl_find().                                                     *
 ****************************************************************************/
void              /*                                                        */
Appl_find(        /*                                                        */
          AES_PB *apb)      /* AES parameter block.                                   */
     /****************************************************************************/
{
  if(apb->addr_in[0] == 0) {
    /* return our own pid */
    apb->int_out[0] = apb->global->apid;
  }
  else {
    apb->int_out[0] = Srv_appl_find((BYTE *)apb->addr_in[0]);
  };
}

/****************************************************************************
 * Appl_search                                                              *
 *  0x0012 appl_search().                                                   *
 ****************************************************************************/
void              /*                                                        */
Appl_search(      /*                                                        */
            AES_PB *apb)      /* AES parameter block.                                   */
     /****************************************************************************/
{
  apb->int_out[ 0] = Srv_appl_search(apb->global->apid, apb->int_in[0], 
                                     (BYTE *)apb->addr_in[0],
                                     &apb->int_out[1],
                                     &apb->int_out[2]);
}

/* 0x0013 appl_exit */

/****************************************************************************
 * Appl_do_exit                                                             *
 *  Implementation of appl_exit().                                          *
 ****************************************************************************/
WORD            /* 0 if error, or 1.                                        */
Appl_do_exit (  /*                                                          */
WORD apid)      /* Application id.                                          */
/****************************************************************************/
{
  C_APPL_EXIT   par;
  R_APPL_EXIT   ret;
  /*  SRV_APPL_INFO apinf;*/
  WORD          code;
  int           count;

  /*
  Srv_get_appl_info(apid, &apinf);
  */

  par.common.call = SRV_APPL_EXIT;
  par.common.apid = apid;
  par.common.pid = getpid ();

  count = Client_send_recv (&par,
                            sizeof (C_APPL_EXIT),
                            &ret,
                            sizeof (R_APPL_EXIT));
  
  DB_printf ("srv_calls.c: Srv_appl_exit: count = %d\n",
             count);
  
  /*
  Fclose(apinf.msgpipe);
  Fclose(apinf.eventpipe);
  Vdi_v_clsvwk(apinf.vid);
  */

  return ret.common.retval;
}

void	Appl_exit(AES_PB *apb) {
  apb->int_out[0] = Appl_do_exit(apb->global->apid);
}

/****************************************************************************
 * Appl_getinfo                                                             *
 *  0x0082 appl_getinfo().                                                  *
 ****************************************************************************/
void                      /*                                                */
Appl_getinfo(AES_PB *apb) /* AES parameter block.                           */
     /***********************************************************************/
{
  GLOBAL_APPL * globals;

  apb->int_out[0] = 1; /* default: return OK ( ret != 0) */

  switch(apb->int_in[0]) {
  case AES_LARGEFONT: /* 0 */
    globals = get_globals (apb->global->apid);
    apb->int_out[ 1] = globals->common->fnt_regul_sz;
    apb->int_out[ 2] = globals->common->fnt_regul_id;
    apb->int_out[ 3] = 0;
    break;
  case AES_SMALLFONT: /* 1 */
    globals = get_globals (apb->global->apid);
    apb->int_out[ 1] = globals->common->fnt_small_sz;
    apb->int_out[ 2] = globals->common->fnt_small_id;
    apb->int_out[ 3] = 0; /* system font */
    break;
  case AES_SYSTEM:    /* 2 */
    apb->int_out[ 1] = 0;  /* rez nr. */
    apb->int_out[ 2] = 16; /* n_colors */
    apb->int_out[ 3] = 1;  /* new .RCS (?) */
    break;
  case AES_LANGUAGE:  /* 3 */
    apb->int_out[ 1] = 0;  /* english */	
    break;
    /* We don't get these unless vAES>=4.1 - hopefully... */
  case AES_PROCESS:   /* 4 */
    apb->int_out[ 1] = 1;  /* real multitask */
    apb->int_out[ 2] = 1;  /* appl_find can mintid->aespid */
    apb->int_out[ 3] = 0;  /* no appl_search */
    apb->int_out[ 4] = 0;  /* no rsrc_rsfix */
    break;
  case AES_PCGEM:     /* 5 */
    apb->int_out[ 1] = 0;  /* objc_xfind */
    apb->int_out[ 2] = 0;  /* 0 */
    apb->int_out[ 3] = 0;  /* menu_click */
    apb->int_out[ 4] = 0;  /* shel_r/wdef */
    break;
  case AES_INQUIRE:   /* 6 */
    apb->int_out[ 1] = 1;  /* appl_read(-1) */
    apb->int_out[ 2] = 0;  /* shel_get(-1) */
    apb->int_out[ 3] = 1;  /* menu_bar(-1) */
    apb->int_out[ 4] = 0;  /* menu_bar(MENU_INSTL) */
    break;
  case AES_MOUSE:     /* 8 */
    apb->int_out[1] = 1;  /* graf_mouse modes 258-260 supported */
    apb->int_out[2] = 0;  /* mouse form maintained by OS */

  case AES_MENU:      /* 9 */
    apb->int_out[ 1] = 0;  /* MultiTOS style submenus */
    apb->int_out[ 2] = 0;  /* MultiTOS style popup menus */
    apb->int_out[ 3] = 0;  /* MultiTOS style scrollable menus */
    apb->int_out[ 4] = 0;  /* words 5/6/7 in MN_SELECTED message give extra info */
    break;
 
  case AES_SHELL:   /* 10 */
    apb->int_out[1] = SWM_AESMSG;
    apb->int_out[2] = 0;
    apb->int_out[3] = 0;
    apb->int_out[4] = 0;
    break;

  case AES_WINDOW:    /* 11 */
    apb->int_out[1] = 0x1a1;
    /* 0x001: WF_TOP returns window below current one
       0x002: wind_get(WF_NEWDESK) supported
       0x004: WF_COLOR get/set supported
       0x008: WF_DCOLOR get/set supported
       0x010: WF_OWNER supported in wind_get
       0x020: WF_BEVENT get/set supported
       0x040: WF_BOTTOM supported
       0x080: WF_ICONIFY supported
       0x100: WF_UNICONIFY supported      */
    apb->int_out[2] = 0;  /* resv */
    apb->int_out[3] = 0x1;  /* 0x1: iconifier
                               0x2: explicit "bottomer" gadget
                               0x4: shift+click to send window to bottom
                               0x8: "hot" close box            */
    apb->int_out[ 4] = 1;  /* wind_update check and set allowed  */
    break;
  case AES_MESSAGES:  /* 12 */
    apb->int_out[ 1] = 0;  /* bit 0: WM_NEWTOP message meaningful
                              bit 1: WM_UNTOPPED message sent
                              bit 2: WM_ONTOP message sent
                              bit 3: AP_TERM message sent
                              bit 4: MultiTOS shutdown and resolution change
                              messages supported
                              bit 5: AES sends CH_EXIT
                              bit 6: WM_BOTTOM message sent
                              bit 7: WM_ICONIFY message sent
                              bit 8: WM_UNICONIFY message sent
                              bit 9: WM_ALLICONIFY message sent */
    apb->int_out[ 2] = 0;  /* 0 */
    apb->int_out[ 3] = 0;  /* 0 */
    break;
  case AES_OBJECTS:   /* 13 object information */
    apb->int_out[ 1] = 0;  /* 3D objects supported via objc_flags */
    apb->int_out[ 2] = 2;  /* 1=objc_sysvar, 2=extended sysvar */
    apb->int_out[ 3] = 0;  /* SPEEDO and GDOS fonts allowed in TEDINFO */
    apb->int_out[ 4] = 0;  /* 0 */
    break;
  case AES_FORM:      /* 14 form library information */
    apb->int_out[ 1] = 0;  /* flying dialogs */
    apb->int_out[ 2] = 0;  /* Mag!X style keyboard tables */
    apb->int_out[ 3] = 0;  /* last cursor position returned */
    apb->int_out[ 4] = 0;  /* 0 */
    break;

  default:
    DB_printf("%s: Line %d: Appl_getinfo:\r\n"
              "Unknown type %d\r\n",__FILE__,__LINE__,apb->int_in[0]);
    apb->int_out[0] = 0;
  };
}
