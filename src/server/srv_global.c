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

#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <vdibind.h>

#include "boot.h"
#include "debug.h"
#include "srv_global.h"
#include "lxgemdos.h"
#include "types.h"
#include "version.h"

#ifdef MINT_TARGET
#include "lib_global.h"
#endif

#ifdef __GNUC__
# define OAESIS_CDECL
#else
# define OAESIS_CDECL cdecl
#endif

/****************************************************************************
 * Global variables                                                         *
 ****************************************************************************/

GLOBALVARS	globals;
/* FIXME char *p_fsel_extern = (char *)&globals.fsel_extern; */
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


/*
** Description
** Close vdi workstation if we get a segmentation fault
*/
static
void
OAESIS_CDECL
handle_signal (int s) {
  v_clswk (globals.vid);

  DB_printf ("srv_global.c: handle_signal: Got signal %d", s);

  signal (s, SIG_DFL);
}


/****************************************************************************
 * Public functions                                                         *
 ****************************************************************************/

/*
** Exported
**
** 1999-01-09 CG
** 1999-01-16 CG
** 1999-03-28 CG
** 1999-05-22 CG
** 1999-08-08 CG
** 1999-08-25 CG
*/
void
srv_init_global (WORD no_configuration_file) {
  int work_in[] = {1,1,1,1,1,1,1,1,1,1,2};
  int work_out[57];
  int dum;

  /* Install segmentation fault handler */
  signal (SIGSEGV, handle_signal);
  signal (SIGILL, handle_signal);
  signal (SIGTRAP, handle_signal);
  signal (SIGBUS, handle_signal);
  signal (SIGFPE, handle_signal);
  /* Sparc Linux doesn't have SIGSTKFLT */
#ifdef HAVE_SIGNAL_SIGSTKFLT
  signal (SIGSTKFLT, handle_signal);
#endif /* HAVE_SIGNAL_SIGSTKFLT */
  signal (SIGPIPE, handle_signal);
  signal (SIGQUIT, handle_signal);
  signal (SIGTERM, handle_signal);

  v_opnwk (work_in, &globals.vid, work_out);

#ifdef MINT_TARGET
  init_global (globals.vid);
#endif

  vq_extnd(globals.vid,0,work_out);
  
  globals.screen.x = 0;
  globals.screen.y = 0;
  globals.screen.width = work_out[0] + 1;
  globals.screen.height = work_out[1] + 1;
  
  globals.num_pens = work_out[13];
  
  /* setup systemfont information */
  
  if(globals.screen.height >= 400) {
    globals.fnt_regul_id = 1;
    globals.fnt_regul_sz = 13;
  } else {
    globals.fnt_regul_id = 1;
    globals.fnt_regul_sz = 9;
  }
  
  globals.fnt_small_id = globals.fnt_regul_id;
  globals.fnt_small_sz = globals.fnt_regul_sz / 2;
  
  vst_font(globals.vid, globals.fnt_regul_id);
  vst_point(globals.vid,globals.fnt_regul_sz,&dum,&dum,&dum,&dum);
  
  globals.applmenu = NULL;
  globals.accmenu = NULL;
  
  globals.arrowrepeat = 100;
  
  vqt_attributes(globals.vid,work_out);
  
  globals.blwidth = work_out[8] + 3;
  globals.blheight = work_out[9] + 3;
  globals.clwidth = work_out[8];
  globals.clheight = work_out[9];

  globals.bswidth = work_out[8] / 2 + 3;
  globals.bsheight = work_out[9] / 2 + 3;
  globals.cswidth = work_out[8] / 2;
  globals.csheight = work_out[9] / 2;
  
  sprintf(versionstring,"Version %s",VERSIONTEXT);

  if (!no_configuration_file) {
    Boot_parse_cnf ();
  }
}


/*
** Exported
**
** 1999-01-09 CG
** 1999-05-22 CG
*/
void
srv_exit_global (void) {
#if 0 /* FIXME def MINT_TARGET */
  if(open_physical_ws) {
    if(globals.video == 0x00030000L) {
      VsetScreen(NULL, NULL, oldmode, oldmodecode);
    }
    else {
      VsetScreen((void *)-1, (void *)-1, oldmode, oldmodecode);
    };
    
    v_clswk (globals.vid);
    own_appl_exit();
  };
#else
  v_clswk (globals.vid);
#endif
}
