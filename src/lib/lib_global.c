/*
** lib_global.c
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <vdibind.h>

#include "aesbind.h"
#include "cursors.h"
#include "debug.h"
#include "graf.h"
#include "lib_global.h"
#include "oaesis.h"
#include "oconfig.h"
#include "resource.h"
#include "rsrc.h"
#include "types.h"

#ifdef MINT_TARGET
#include "mintdefs.h"
#endif

/****************************************************************************
 * Global variables                                                         *
 ****************************************************************************/

GLOBAL_COMMON global_common;

/* FIXME: allocate when needed */
GLOBAL_APPL globals_appl[MAX_NUM_APPS];

char *p_fsel_extern = (char *)&global_common.fsel_extern;
#if 0
static WORD global[15];
#endif

/****************************************************************************
 * Module global variables                                                  *
 ****************************************************************************/

#ifdef MINT_TARGET
static WORD open_physical_ws; /* set in own_appl_init. jps */

static WORD oldmode,oldmodecode;

static OAESIS_PATH_MODE default_path_mode = OAESIS_PATH_MODE_MINT;
#else
static
short
(*oaesis_callback)(void *, void *) = NULL;

static OAESIS_PATH_MODE default_path_mode = OAESIS_PATH_MODE_UNIX;
#endif /* MINT_TARGET */

/****************************************************************************
 * Module local functions                                                   *
 ****************************************************************************/

/* These few functions can only be used if we run TOS/MiNT */
#if 0 /* FIXME: Remove def MINT_TARGET */
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
*/
void
init_global (WORD physical_vdi_id)
{
#if 0
  int        temp_vid;
  int        work_in[] = {1,1,1,1,1,1,1,1,1,1,2};
#endif
  int        work_out[57];
  int        dum;
  static int is_inited = FALSE;

  /* Only initialize globals if they haven't been initialized before */
  if(!is_inited)
  {
    is_inited = TRUE;
    
    DEBUG3 ("Entering init_global");
    
#if 0 /* FIXME def MINT_TARGET */
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
    
#if 0 /* FIXME : Remove? def MINT_TARGET */
    fprintf(stderr,"appl_init()\r\n");
    own_appl_init();
    fprintf(stderr,"/appl_init()\r\n");
    
    if(open_physical_ws)
    {
      printf("No other AES found. Opening own Workstation.\r\n");
      work_in[0] = 5;
      v_opnwk(work_in,&global_common.vid,work_out);
      
      if(global_common.video == 0x00030000L)
      {
        VsetScreen(NULL, NULL, global_common.vmode, global_common.vmodecode);
      }
      else
      {
        VsetScreen((void*)-1, (void *)-1, global_common.vmode, global_common.vmodecode);
      }
    }
    else
    {
      printf("Other AES detected.\r\n");
      global_common.vid = own_graf_handle();
      v_clrwk(global_common.vid);
    }
#endif
    
    global_common.physical_vdi_id = physical_vdi_id;
    DEBUG2 ("lib_global.c: init_global: calling vq_extnd");
    vq_extnd (physical_vdi_id, 0, work_out);
    
    
    global_common.screen.x = 0;
    global_common.screen.y = 0;
    global_common.screen.width = work_out[0] + 1;
    global_common.screen.height = work_out[1] + 1;
    
    global_common.num_pens = work_out[13];
    
    vq_extnd (physical_vdi_id, 1, work_out);
    global_common.num_planes = work_out[4];
    
    /* setup systemfont information */
    
    if(global_common.screen.height >= 400)
    {
      global_common.fnt_regul_id = 1;
      global_common.fnt_regul_sz = 13;
    }
    else
    {
      global_common.fnt_regul_id = 1;
      global_common.fnt_regul_sz = 9;
    }
    
    global_common.fnt_small_id = global_common.fnt_regul_id;
    global_common.fnt_small_sz = global_common.fnt_regul_sz / 2;
    
    vst_font (physical_vdi_id, global_common.fnt_regul_id);
    vst_point (physical_vdi_id,
               global_common.fnt_regul_sz,
               &dum,
               &dum,
               &dum,
               &dum);
    
    global_common.arrowrepeat = 100;
    
    DEBUG2 ("lib_global.c: init_global: calling vqt_attributes");
    vqt_attributes (physical_vdi_id, work_out);
    
    global_common.blwidth = work_out[8] + 3;
    global_common.blheight = work_out[9] + 3;
    global_common.clwidth = work_out[8];
    global_common.clheight = work_out[9];
    
    global_common.bswidth = work_out[8] / 2 + 3;
    global_common.bsheight = work_out[9] / 2 + 3;
    global_common.cswidth = work_out[8] / 2;
    global_common.csheight = work_out[9] / 2;
    
    global_common.time = 0L;
    
#ifndef MINT_TARGET
    global_common.callback_handler = oaesis_callback;
#endif
    
    DEBUG2("lib_global.c: init_global: Calling Rsrc_do_rcfix");
    Rsrc_do_rcfix (physical_vdi_id,
                   (RSHDR *)resource,
#ifdef WORDS_BIGENDIAN
                   FALSE
#else
                   TRUE
#endif
                   );
    DEBUG2("lib_global.c: init_global: Called Rsrc_do_rcfix");
    
    Rsrc_do_gaddr ((RSHDR *)resource, R_TREE, AICONS, &global_common.aiconstad);
    Rsrc_do_gaddr ((RSHDR *)resource, R_TREE, ALERT, &global_common.alerttad);
    Rsrc_do_gaddr ((RSHDR *)resource,
                   R_TREE,
                   FISEL,
                   &global_common.fiseltad);

    Rsrc_do_gaddr ((RSHDR *)resource,
                   R_TREE,
                   PMENU,
                   &global_common.pmenutad);
    Rsrc_do_gaddr ((RSHDR *)resource,
                   R_FRSTR,
                   0,
                   (OBJECT **)&global_common.fr_string);
    
    /* Initialize window elements and resource counters */
    Rsrc_do_gaddr((RSHDR *)resource,
                  R_TREE,
                  WINDOW,
                  &global_common.windowtad);
    global_common.elemnumber = -1;
 
    /* Init mouseforms */
    Rsrc_do_rcfix (physical_vdi_id,
                   (RSHDR *)cursors,
#ifdef WORDS_BIGENDIAN
                   FALSE
#else
                   TRUE
#endif
                   );
    Rsrc_do_gaddr((RSHDR *)cursors,
                  R_TREE,
                  MOUSEFORMS,
                  &global_common.mouseformstad);
    Graf_init_mouseforms();
    
#ifdef MINT_TARGET
    /* Initialize semaphore used by Shel_do_write */
    Psemaphore(SEM_CREATE, SHEL_WRITE_LOCK, 0);
    Psemaphore(SEM_UNLOCK, SHEL_WRITE_LOCK, 0);
#endif
  }
}


/*
** Description
** Initialize application specific variables
*/
void
init_global_appl (int    apid,
		  int    physical_vdi_id,
                  char * appl_name)
{
  GLOBAL_APPL * globals;
  int           work_in[] = {1,1,1,1,1,1,1,1,1,1,2};
  int           work_out[57];
  int           temp_vid;

  globals = get_globals(apid);

  globals->pid = getpid();

  work_in[0] = 5;
  DEBUG3 ("lib_global.c: init_global_appl: physical_vdi_id = %d",
             physical_vdi_id);
  /* FIXME: change vid types to int and remove temp_vid */
  temp_vid = physical_vdi_id;
  v_opnvwk (work_in, &temp_vid, work_out);

  globals->vid = temp_vid;
  DEBUG2("lib_global.c: apid = %d globals->vid = %d", apid, globals->vid);

  /* There is no default desktop background */
  globals->desktop_background = NULL;

  /* There is no default menu */
  globals->menu = NULL;

  /* Setup resource header and resource file */
  globals->rscfile = NULL;
  globals->rshdr = NULL;

  /* Setup path mode: MiNT or Unix */
  globals->path_mode = default_path_mode;

  /* Initialize application and accessory list as empty */
  globals->appl_menu.count = 0;
  globals->appl_menu.size = 10;
  globals->appl_menu.entries =
    (APPL_ENTRY *)malloc (sizeof (APPL_ENTRY) * 10);
  globals->acc_menu.count = 0;
  globals->acc_menu.size = 10;
  globals->acc_menu.entries =
    (APPL_ENTRY *)malloc (sizeof (APPL_ENTRY) * 10);
  strcpy(globals->application_name, appl_name);

  globals->common = &global_common;

  DEBUG3 ("lib_global.c: Leaving global_init");
}

void
exit_global(void)
{
#ifdef MINT_TARGET
  /* Destroy semaphore used by Shel_do_write */
  Psemaphore(SEM_LOCK, SHEL_WRITE_LOCK, -1);
  Psemaphore(SEM_DESTROY, SHEL_WRITE_LOCK, 0);
#endif

#if 0 /* FIXME def MINT_TARGET */
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


/*
** Description
** Return reference to application global AES variables
*/
GLOBAL_APPL *
get_globals (WORD apid)
{
  return &globals_appl[apid];
}


/*
** Description
** Set emulation path mode
*/
void
Oaesis_set_path_mode(OAESIS_PATH_MODE path_mode)
{
  default_path_mode = path_mode;
}


#ifndef MINT_TARGET

/*
** Description
** Install a callback handler
*/
void
Oaesis_callback_handler(void * callback_handler)
{
  oaesis_callback = callback_handler;
}

#endif MINT_TARGET

int
check_apid(int apid)
{
  int i;
  int pid = getpid();

  if((apid >= 0) && (apid < MAX_NUM_APPS) && (globals_appl[apid].pid == pid))
  {
    return apid;
  }
  else
  {
    for(i = 0; i < MAX_NUM_APPS; i++)
    {
      if(globals_appl[i].pid == pid)
      {
        return i;
      }
    }
  }

  return -1;
}
