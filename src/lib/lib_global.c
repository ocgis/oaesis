/****************************************************************************

 Module
  global.c
  
 Description
  Variables of global interest in oAESis.
  
 Author(s)
  cg (Christer Gustavsson <d2cg@dtek.chalmers.se>)
  jps (Jan Paul Schmidt <Jan.P.Schmidt@mni.fh-giessen.de>)

 Revision history
 
  960103 cg
   Added standard header. 

  960507 jps
   Added globals.realslide initialisation.

  960816 jps
   Initialisation of some new variables

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

#ifdef HAVE_MINTBIND_H
#include <mintbind.h>
#endif

#ifdef HAVE_OSBIND_H
#include <osbind.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <vdibind.h>

#include "aesbind.h"
/*#include "boot.h"*/
#include "debug.h"
#include "lib_global.h"
/*#include "lxgemdos.h"*/
#include "resource.h"
#include "rsrc.h"
#include "types.h"
#include "version.h"

/****************************************************************************
 * Global variables                                                         *
 ****************************************************************************/

GLOBAL_COMMON global_common;
GLOBAL_APPL   global_appl;

char *p_fsel_extern = (char *)&global_common.fsel_extern;
static WORD global[15];

/****************************************************************************
 * Module global variables                                                  *
 ****************************************************************************/

#ifdef MINT_TARGET
static WORD open_physical_ws; /* set in own_appl_init. jps */

static WORD oldmode,oldmodecode;
#endif /* MINT_TARGET */

static BYTE versionstring[50];

/****************************************************************************
 * Module local functions                                                   *
 ****************************************************************************/

/* TheBse few functions can only be used if we run TOS/MiNT */
#ifdef BOBMINT_TARGET
WORD own_appl_init(void) {
  LONG addr_in[3],
  addr_out[1];
  WORD contrl[5] = { 10, 0, 1 , 0, 0 },
  int_in[16],
  int_out[7];

  void *aespb[6];
  
  aespb[0] = contrl;
  aespb[1] = global;
  aespb[2] = int_in;
  aespb[3] = int_out;
  aespb[4] = addr_in;
  aespb[5] = addr_out;
  
  global[0] = 0;                   /* clear AES version number  */
  aescall(aespb); 
  open_physical_ws = !(global[0]); /* if AES version still cleared,
				      no AES installed */
  
  return(int_out[0]);
}

WORD own_appl_exit(void) {
  LONG addr_in[3],
  addr_out[1];
  WORD contrl[5] = { 19, 0, 1 , 0, 0 },
  int_in[16],
  int_out[7];

  void *aespb[6];
  
  aespb[0] = contrl;
  aespb[1] = global;
  aespb[2] = int_in;
  aespb[3] = int_out;
  aespb[4] = addr_in;
  aespb[5] = addr_out;
  
  aescall(aespb);

  return(int_out[0]);
}

WORD own_graf_handle(void) {
  LONG addr_in[3],
  addr_out[1];
  WORD contrl[5] = { 77, 0, 5 , 0, 0 },
  int_in[16],
  int_out[7];
  
  void *aespb[6];
  
  aespb[0] = contrl;
  aespb[1] = global;
  aespb[2] = int_in;
  aespb[3] = int_out;
  aespb[4] = addr_in;
  aespb[5] = addr_out;
  
  aescall(aespb);
  
  return(int_out[0]);
}
#endif /* MINT_TARGET */


/****************************************************************************
 * Public functions                                                         *
 ****************************************************************************/

/*
** Description
** Initialize global variables, open vdi workstation etc
**
** 1998-11-15 CG
** 1999-01-01 CG
** 1999-01-06 CG
** 1999-01-09 CG
** 1999-01-13 CG
** 1999-03-11 CG
** 1999-04-13 CG
** 1999-04-18 CG
** 1999-04-24 CG
** 1999-05-16 CG
*/
void
init_global (WORD nocnf,
             WORD physical_vdi_id) {
  int work_in[] = {1,1,1,1,1,1,1,1,1,1,2};
  int work_out[57];
  int dum;
  int temp_vid;

  DEBUG3 ("Entering init_global");

#ifdef MINT_TARGET
  /* Only mess with videomodes if running under MiNT */
  if(globals.video == 0x00030000L) {
    fprintf(stderr,"VsetMode\r\n");
    oldmode = globals.vmode = 3;
    oldmodecode = globals.vmodecode = VsetMode(-1);
    fprintf(stderr,"/VsetMode\r\n");
  }
  else {
    oldmode = globals.vmode = Getrez();
  };
#endif /* MINT_TARGET */
  
  global_common.mouse_owner = -1;
  global_common.realmove = 0;
  global_common.realsize = 0;
  global_common.realslide = 0;
  global_common.fnt_regul_id = -1;
  global_common.fnt_regul_sz = -1;
  global_common.icon_width = 48;
  global_common.icon_height = 56;
  global_common.wind_appl = 1;
  global_common.graf_mbox = 1;
  global_common.graf_growbox = 1;
  global_common.graf_shrinkbox = 1;
  global_common.fsel_sorted = 1;
  global_common.fsel_extern = 0;
  
  DEBUG3 ("init_global: 2");

  if(!nocnf) {
    /*
    Boot_parse_cnf();
    */
  };

#ifdef MINT_TARGET
  fprintf(stderr,"appl_init()\r\n");
  own_appl_init();
  fprintf(stderr,"/appl_init()\r\n");

  if(open_physical_ws) {	
    printf("No other AES found. Opening own Workstation.\r\n");
    work_in[0] = 5;
    v_opnwk(work_in,&global_common.vid,work_out);

    if(global_common.video == 0x00030000L) {
      VsetScreen(NULL, NULL, global_common.vmode, global_common.vmodecode);
    }
    else {
      VsetScreen((void*)-1, (void *)-1, global_common.vmode, global_common.vmodecode);
    };
  }
  else {
    printf("Other AES detected.\r\n");
    global_common.vid = own_graf_handle();
    v_clrwk(global_common.vid);
  }
#else  /* ! MINT_TARGET */
  work_in[0] = 5;
  DEBUG3 ("lib_global.c: init_global: physical_vdi_id = %d",
             physical_vdi_id);
  /* FIXME: change vid types to int and remove temp_vid */
  temp_vid = physical_vdi_id;
  v_opnvwk (work_in, &temp_vid, work_out);
  global_appl.vid = temp_vid;
  DEBUG3 ("init_global: 3");
  global_common.vid = global_appl.vid; /* Remove global_common.vid */
  DEBUG3 ("lib_global.c: init_global: vid=%d", global_appl.vid);
#endif /* MINT_TARGET */
  DB_printf ("lib_global.c: init_global: calling vq_extnd");
  vq_extnd(global_common.vid,0,work_out);

  
  global_common.screen.x = 0;
  global_common.screen.y = 0;
  global_common.screen.width = work_out[0] + 1;
  global_common.screen.height = work_out[1] + 1;
  
  global_common.num_pens = work_out[13];
  
  /* setup systemfont information */
  
  if(global_common.screen.height >= 400) {
    global_common.fnt_regul_id = 1;
    global_common.fnt_regul_sz = 13;
  } else {
    global_common.fnt_regul_id = 1;
    global_common.fnt_regul_sz = 9;
  }
  
  global_common.fnt_small_id = global_common.fnt_regul_id;
  global_common.fnt_small_sz = global_common.fnt_regul_sz / 2;
  
  vst_font(global_common.vid, global_common.fnt_regul_id);
  vst_point(global_common.vid,global_common.fnt_regul_sz,&dum,&dum,&dum,&dum);
  
  /*
  global_common.applmenu = NULL;
  global_common.accmenu = NULL;
  */

  global_common.mouse_x = 0;
  global_common.mouse_y = 0;
  global_common.mouse_button = 0;
  
  global_common.arrowrepeat = 100;
  
  DEBUG3 ("lib_global.c: init_global: calling vqt_attributes");
  vqt_attributes(global_common.vid,work_out);
  
  global_common.blwidth = work_out[8] + 3;
  global_common.blheight = work_out[9] + 3;
  global_common.clwidth = work_out[8];
  global_common.clheight = work_out[9];
  
  global_common.bswidth = work_out[8] / 2 + 3;
  global_common.bsheight = work_out[9] / 2 + 3;
  global_common.cswidth = work_out[8] / 2;
  global_common.csheight = work_out[9] / 2;
  
  global_common.time = 0L;

#ifdef MINT_TARGET
  sprintf(global_common.mousename,"u:\\dev\\aesmouse.%03d",Pgetpid());
#else
  strcpy (global_common.mousename, "/dev/mouse");
#endif  

  Rsrc_do_rcfix (global_common.vid,
                 (RSHDR *)resource,
                 FALSE /* FIXME: Swap if low_endian*/);

  Rsrc_do_gaddr ((RSHDR *)resource, R_TREE, AICONS, &global_common.aiconstad);
  Rsrc_do_gaddr ((RSHDR *)resource, R_TREE, ALERT, &global_common.alerttad);
  Rsrc_do_gaddr ((RSHDR *)resource,
                 R_TREE,
                 FISEL,
                 &global_common.fiseltad);
  /*
  Rsrc_do_gaddr((RSHDR *)RESOURCE,R_TREE,MOUSEFORMS,&global_common.mouseformstad);
  */

  Rsrc_do_gaddr ((RSHDR *)resource,
                 R_TREE,
                 PMENU,
                 &global_common.pmenutad);
  Rsrc_do_gaddr ((RSHDR *)resource,
                 R_FRSTR,
                 0,
                 (OBJECT **)&global_common.fr_string);
  
  sprintf(versionstring,"Version %s",VERSIONTEXT);
  /*  global_common.informtad[INFOVERSION].ob_spec.tedinfo->te_ptext = versionstring; */

  /* Initialize window elements and resource counters */
  Rsrc_do_gaddr((RSHDR *)resource,
                R_TREE,
                WINDOW,
                &global_common.windowtad);
  global_common.elemnumber = -1;

  global_common.applpid = Pgetpid();

  /* There is no default desktop background */
  global_appl.desktop_background = NULL;

  /* There is no default menu */
  global_appl.menu = NULL;

  /* Setup resource header and resource file */
  global_appl.rscfile = NULL;
  global_appl.rshdr = NULL;

  /* Default is not to use MiNT pathnames, at least under unix */
  global_appl.use_mint_paths = FALSE;

  /* Initialize application and accessory list as empty */
  global_appl.appl_menu.count = 0;
  global_appl.appl_menu.size = 10;
  global_appl.appl_menu.entries =
    (APPL_ENTRY *)malloc (sizeof (APPL_ENTRY) * 10);
  global_appl.acc_menu.count = 0;
  global_appl.acc_menu.size = 10;
  global_appl.acc_menu.entries =
    (APPL_ENTRY *)malloc (sizeof (APPL_ENTRY) * 10);

  global_appl.common = &global_common;

  DEBUG3 ("lib_global.c: Leaving global_init");
}

void	exit_global(void) {
#ifdef MINT_TARGET
  if(open_physical_ws) {
    if(global_common.video == 0x00030000L) {
      VsetScreen(NULL, NULL, oldmode, oldmodecode);
    }
    else {
      VsetScreen((void *)-1, (void *)-1, oldmode, oldmodecode);
    };
    
    v_clsvwk(global_common.vid);
    own_appl_exit();
  };
#endif
}
