/*
** boot.c
**
** Copyright 1996-2000 Christer Gustavsson <cg@nocrew.org>
** Copyright 1996 Jan Paul Schmidt <Jan.P.Schmidt@mni.fh-giessen.de>
** Copyright 2000 Vincent Barrilliot <vbarr@nist.gov>
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

#include <aesbind.h>

#ifdef HAVE_SYSVARS_H
#include <sysvars.h>
#endif

#include "boot.h"
#include "debug.h"
#include "launcher.h"
#include "misc.h"

typedef struct _boot_program
{
  char	                 name[FILENAME_MAX];
  char	                 cmdline[256];
  struct _boot_program * next;
} BOOT_PROGRAM;

static BOOT_PROGRAM * Boot_runlist = NULL;
static BOOT_PROGRAM * Boot_shell = NULL;

static char *         Boot_acc_path = NULL;


/*
** Description
** Set accessories path
*/
void
launcher_set_accessory_path(char * accpath)
{
  if(Boot_acc_path != NULL)
  {
    free(Boot_acc_path);
  }

  Boot_acc_path = (char *)malloc(strlen(accpath) + 1);

  strcpy(Boot_acc_path, accpath);
}


/*
** Description
** Add startup application
*/
void
launcher_add_startup_application(char * application,
                                 char * arg)
{
  BOOT_PROGRAM * bp;

  bp = malloc(sizeof(BOOT_PROGRAM));

  /* Copy data */
  strcpy(bp->name, application);
  bp->cmdline[0] = strlen(arg);
  strcpy(&bp->cmdline[1], arg);

  /* Insert into list */
  bp->next = Boot_runlist;
  Boot_runlist = bp;
}


/*
** Description
** Set shell application
*/
void
launcher_set_shell_application(char * shell,
                               char * arg)
{
  if(Boot_shell != NULL)
  {
    free(Boot_shell);
  }

  Boot_shell = (BOOT_PROGRAM *)malloc(sizeof(BOOT_PROGRAM));

  strcpy(Boot_shell->name, shell);
  Boot_shell->cmdline[0] = strlen(arg);
  strcpy(&Boot_shell->cmdline[1], arg);
  Boot_shell->next = NULL;
}


/*
** Description
** Set environment variable
*/
void
launcher_set_environment_variable(char * variable,
                                  char * value)
{
  /* FIXME: Implement */
}


/*
** Description
** Start programs mentioned in oaesis.cnf
*/
void
start_programs(void)
{
  _DTA newdta, *olddta;
  int  found;
  char bootpath[] = "c:\\";
  
  BOOT_PROGRAM * run_walk;
  
#ifdef MINT_TARGET /* FIXME for linux */
  bootpath[0] = (get_sysvar(_bootdev) >> 16) + 'a';
#endif
  
  /* Start shell if it was specified */
  if(Boot_shell != NULL)
  {
    DEBUG2("Starting shell: %s %d %s",
           Boot_shell->name,
           Boot_shell->cmdline[0],
           &Boot_shell->cmdline[1]);
    shel_write(SWM_LAUNCH, 0, 0, Boot_shell->name, Boot_shell->cmdline);
  }

  /* Start applications specified in the configuration file */
  for(run_walk = Boot_runlist; run_walk != NULL; run_walk = run_walk->next)
  {
    DEBUG2("Starting application: %s %d %s",
           run_walk->name,
           run_walk->cmdline[0],
           &run_walk->cmdline[1]);
    shel_write(SWM_LAUNCH, 0, 0, run_walk->name, run_walk->cmdline);
  }
  
  /* Start accessories */
  olddta = Fgetdta();
  Fsetdta(&newdta);
  
  
  if(Boot_acc_path[0] == '\0')
  {
#ifdef MINT_TARGET
    Boot_acc_path[0] = (get_sysvar(_bootdev) >> 16) + 'a';
    Boot_acc_path[1] = ':';
    Boot_acc_path[2] = '\\';
#else
    /* FIXME */
#endif
  }

  DEBUG2("Starting accessories in %s", Boot_acc_path);

  misc_setpath(Boot_acc_path);
  
  found = Fsfirst("*.acc", 0);
  
  while(found == 0)
  {
    char accname[128];
    
    sprintf(accname, "%s%s", bootpath, newdta.dta_name);
    shel_write(SWM_LAUNCHACC, 0, 0, accname, "");

    found = Fsnext();
  }
  
  Fsetdta(olddta);
}
