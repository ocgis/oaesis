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
#include <unistd.h>

#include "boot.h"
#include "debug.h"
#include "gemdefs.h"
#include "global.h"
#include "lxgemdos.h"
#include "resource.h"
#include "rsrc.h"
#include "types.h"
#include "vdi.h"
#include "version.h"

/****************************************************************************
 * Global variables                                                         *
 ****************************************************************************/

GLOBALVARS	globals;
char *p_fsel_extern = (char *)&globals.fsel_extern;
WORD global[15];

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

/* These few functions can only be used if we run TOS/MiNT */
#ifdef MINT_TARGET
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

void init_global(WORD nocnf) {
  WORD work_in[] = {1,1,1,1,1,1,1,1,1,1,2};
  WORD work_out[57];
  WORD dum;
  

  /* Only mess with videomodes if running under MiNT */
#ifdef MINT_TARGET
  fprintf(stderr,"init_global\r\n");

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
  
  globals.mouse_owner = -1;
  globals.realmove = 0;
  globals.realsize = 0;
  globals.realslide = 0;
  globals.fnt_regul_id = -1;
  globals.fnt_regul_sz = -1;
  globals.icon_width = 48;
  globals.icon_height = 56;
  globals.wind_appl = 1;
  globals.graf_mbox = 1;
  globals.graf_growbox = 1;
  globals.graf_shrinkbox = 1;
  globals.fsel_sorted = 1;
  globals.fsel_extern = 0;
  
  if(!nocnf) {
    Boot_parse_cnf();
  };

#ifdef MINT_TARGET
  fprintf(stderr,"appl_init()\r\n");
  own_appl_init();
  fprintf(stderr,"/appl_init()\r\n");

  if(open_physical_ws) {	
    printf("No other AES found. Opening own Workstation.\r\n");
    work_in[0] = 5;
    Vdi_v_opnwk(work_in,&globals.vid,work_out);

    if(globals.video == 0x00030000L) {
      VsetScreen(NULL, NULL, globals.vmode, globals.vmodecode);
    }
    else {
      VsetScreen((void*)-1, (void *)-1, globals.vmode, globals.vmodecode);
    };
  }
  else {
    printf("Other AES detected.\r\n");
    globals.vid = own_graf_handle();
    Vdi_v_clrwk(globals.vid);
  }
#else  /* ! MINT_TARGET */
  work_in[0] = 5;
  Vdi_v_opnwk(work_in,&globals.vid,work_out);
#endif /* MINT_TARGET */

  Vdi_vq_extnd(globals.vid,0,work_out);
  
  globals.screen.x = 0;
  globals.screen.y = 0;
  globals.screen.width = work_out[0] + 1;
  globals.screen.height = work_out[1] + 1;
  
  globals.num_pens = work_out[13];
  
  /* setup systemfont information */
  
  if(globals.screen.height >= 400) {
    if(globals.fnt_regul_id == -1) {
      globals.fnt_regul_id = 1;
    };
    
    if(globals.fnt_regul_sz == -1) {
      globals.fnt_regul_sz = 13;
    };
  }
  else {
    if(globals.fnt_regul_id == -1) {
      globals.fnt_regul_id = 1;
    };
    
    if(globals.fnt_regul_sz == -1) {
      globals.fnt_regul_sz = 9;
    };
  };
  
  globals.fnt_small_id = globals.fnt_regul_id;
  globals.fnt_small_sz = globals.fnt_regul_sz / 2;
  
  Vdi_vst_font(globals.vid, globals.fnt_regul_id);
  Vdi_vst_point(globals.vid,globals.fnt_regul_sz,&dum,&dum,&dum,&dum);
  
  globals.applmenu = NULL;
  globals.accmenu = NULL;
  
  globals.mouse_x = 0;
  globals.mouse_y = 0;
  globals.mouse_button = 0;
  
  globals.arrowrepeat = 100;
  
  Vdi_vqt_attributes(globals.vid,work_out);
  
  globals.blwidth = work_out[8] + 3;
  globals.blheight = work_out[9] + 3;
  globals.clwidth = work_out[8];
  globals.clheight = work_out[9];
  
  globals.bswidth = work_out[8] / 2 + 3;
  globals.bsheight = work_out[9] / 2 + 3;
  globals.cswidth = work_out[8] / 2;
  globals.csheight = work_out[9] / 2;
  
  globals.time = 0L;
  
  sprintf(globals.mousename,"u:\\dev\\aesmouse.%03d",Pgetpid());
  
  Rsrc_do_rcfix(globals.vid,(RSHDR *)RESOURCE);
  
  Rsrc_do_gaddr((RSHDR *)RESOURCE,R_TREE,AICONS,&globals.aiconstad);
  Rsrc_do_gaddr((RSHDR *)RESOURCE,R_TREE,ALERT,&globals.alerttad);
  Rsrc_do_gaddr((RSHDR *)RESOURCE,R_TREE,DESKBG,&globals.deskbgtad);
  Rsrc_do_gaddr((RSHDR *)RESOURCE,R_TREE,FISEL,&globals.fiseltad);
  Rsrc_do_gaddr((RSHDR *)RESOURCE,R_TREE,INFORM,&globals.informtad);
  Rsrc_do_gaddr((RSHDR *)RESOURCE,R_TREE,MENU,&globals.menutad);
  Rsrc_do_gaddr((RSHDR *)RESOURCE,R_TREE,MOUSEFORMS,&globals.mouseformstad);
  Rsrc_do_gaddr((RSHDR *)RESOURCE,R_TREE,PMENU,&globals.pmenutad);
  Rsrc_do_gaddr((RSHDR *)RESOURCE,R_TREE,WINDOW,&globals.windowtad);
  Rsrc_do_gaddr((RSHDR *)RESOURCE,R_FRSTR,0,(OBJECT **)&globals.fr_string);
  
  sprintf(versionstring,"Version %s",VERSIONTEXT);
  globals.informtad[INFOVERSION].ob_spec.tedinfo->te_ptext = versionstring;
  
  globals.applpid = Pgetpid();
}

void	exit_global(void) {
#ifdef MINT_TARGET
  if(open_physical_ws) {
    if(globals.video == 0x00030000L) {
      VsetScreen(NULL, NULL, oldmode, oldmodecode);
    }
    else {
      VsetScreen((void *)-1, (void *)-1, oldmode, oldmodecode);
    };
    
    Vdi_v_clswk(globals.vid);
    own_appl_exit();
  };
#endif
}
