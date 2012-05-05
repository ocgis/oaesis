/*
** srv_global.c
**
** Copyright 1996 - 2000 Christer Gustavsson <cg@nocrew.org>
** Copyright 1996 Jan Paul Schmidt <Jan.P.Schmidt@mni.fh-giessen.de>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**  
** Read the file COPYING for more information.
*/


#define DEBUGLEVEL 0

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

#include "debug.h"
#include "srv.h"
#include "srv_global.h"
#include "lxgemdos.h"
#include "types.h"

#ifdef MINT_TARGET
#include "lib_global.h"
#endif

#ifdef __GNUC__
# define OAESIS_CDECL
#else
# define OAESIS_CDECL cdecl
#endif

GLOBALVARS	globals;
/* FIXME char *p_fsel_extern = (char *)&globals.fsel_extern; */
WORD global[15];

#ifdef MINT_TARGET
static WORD open_physical_ws; /* set in own_appl_init. jps */

static WORD oldmode,oldmodecode;
#endif /* MINT_TARGET */

#ifdef MINT_TARGET

/* These few functions can only be used if we run TOS/MiNT */

static
inline
WORD
own_appl_init(void)
{
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
  DEBUG3("own_appl_init: Calling aes_call");
  aes_call(aespb); 
  DEBUG3("own_appl_init: Returned from aes_call");
  open_physical_ws = !(global[0]); /* if AES version still cleared,
				      no AES installed */
  
  return(int_out[0]);
}


static
inline
WORD
own_appl_exit(void)
{
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
  
  aes_call(aespb);

  return(int_out[0]);
}


static
inline
WORD
own_graf_handle(void)
{
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
  
  aes_call(aespb);
  
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
handle_signal(int s)
{
  DEBUG0("srv_global.c: handle_signal: Got signal %d", s);

  v_clswk (globals.vid);

  signal(s, SIG_DFL);
}


/****************************************************************************
 * Public functions                                                         *
 ****************************************************************************/

/*
** Exported
*/
void
srv_init_global (WORD no_configuration_file)
{
  int work_in[] = {1,1,1,1,1,1,1,1,1,1,2};
  int work_out[57];
  int dum;

  DEBUG3("In srv_init_global");
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

  DEBUG3("In srv_init_global: 2");
#ifdef MINT_TARGET
  own_appl_init();

  DEBUG3("In srv_init_global: 2.1");
  if(open_physical_ws)
  {
    DEBUG3("In srv_init_global: 2.2");
    v_opnwk(work_in, &globals.vid, work_out);
  }
  else
  {
    DEBUG3("In srv_init_global: 2.3");
    globals.vid = own_graf_handle();
    DEBUG3("In srv_init_global: 2.4");
    v_clrwk(globals.vid);
  }

  DEBUG3("In srv_init_global: 2.5");
  init_global(globals.vid);
#else
  v_opnwk (work_in, &globals.vid, work_out);
#endif

  DEBUG3("In srv_init_global: 4");
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
}


/*
** Exported
*/
void
srv_exit_global (void)
{
#ifdef MINT_TARGET
  if(open_physical_ws)
  {
    /*
    if(globals.video == 0x00030000L) {
      VsetScreen(NULL, NULL, oldmode, oldmodecode);
    }
    else {
      VsetScreen((void *)-1, (void *)-1, oldmode, oldmodecode);
    }
    */
    
    v_clswk(globals.vid);
  }
  else
  {
    own_appl_exit();
  }
#else
  DEBUG0("srv_exit_global");
  v_clswk (globals.vid);
#endif
}


/*
** Description
** Set server variable
*/
void
Srv_set_variable(SRV_VAR_KIND    var,
                 SRV_VAR_VALUE * value)
{
  /* FIXME: implement */
}
