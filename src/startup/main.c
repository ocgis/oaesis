/*
** main.c
**
** Copyright 1999 - 2000 Christer Gustavsson <cg@nocrew.org>
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

#ifdef HAVE_PROCESS_H
#include <process.h>
#endif

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_SUPPORT_H
#include <support.h>
#endif

#include <sys/wait.h>
#include <unistd.h>

#include "boot.h"
#include "debug.h"
#include "lxgemdos.h"
#include "mousedev.h"
#include "srv.h"
#include "srv_misc.h"

#ifdef HAVE_SYSVARS_H
#include	<sysvars.h>
#endif

/*
** Description
** Initialize oAESis
**
** 1998-09-20 CG
** 1998-12-22 CG
** 1999-01-09 CG
** 1999-05-20 CG
*/
void
init_aes (WORD nocnf) {
  DEBUG3 ("main.c: init_aes: Initializing server");
  Srv_init_module (nocnf);
  
#ifdef MINT_TARGET
  DEBUG3 ("main.c: init_aes: AES trap vector");
  Supexec(link_in);
#endif /* MINT_TARGET */
}


/*
** Exported
*/
void
exit_aes (void) {
  DEBUG3 ("main.c: exit_aes: Enter");
  
#ifdef MINT_TARGET
  DEBUG3 ("main.c: exit_aes: AES trap vector");
  Supexec(link_remove);
#endif /* MINT_TARGET */

  Srv_stop();
}


/*
** Description
** Main routine of the oaesis server
*/
int
main (int     argc,
      char ** argv,
      char ** envp) {
  WORD nocnf = 0;
  WORD i;

#ifdef MINT_TARGET
  LONG mintval;
#endif  

  DEBUG3 ("main: Starting oAESis");

  /* We only need to check for mint if we've built oaesis for mint */
#ifdef MINT_TARGET
  if(!srv_get_cookie(0x4d694e54L /*'MiNT'*/,&mintval)) {
    fprintf(stderr,"oAESis requires MiNT to work. Start MiNT and try again!\r\n");
    
    return -1;
  };
#endif
  
  fprintf(stderr,"Starting oAESis version %s.\r\n", OAESIS_VERSION);
  fprintf(stderr,"Compiled on %s at %s with ",__DATE__,__TIME__);
  
#ifdef __TURBOC__
  fprintf(stderr,"Pure C / Turbo C %x.%x.\r\n",__TURBOC__ >> 8,__TURBOC__ & 0xff);
#endif
  
#ifdef __GNUC__
  fprintf(stderr,"Gnu C %s.\r\n",__VERSION__);
#endif
  
  
  fprintf(stderr,"The following options were used:\r\n");
  
#ifdef __68020__
  fprintf(stderr,"- Main processor 68020.\r\n");
#endif
  
#ifdef __68881__
  fprintf(stderr,"- Math co-processor 68881.\r\n");
#endif
 
#if 0 /* FIXME def MINT_TARGET */
  fprintf(stderr,"\r\nMiNT version %ld.%02ld detected\r\n",mintval >> 8,mintval & 0xff);
  
  globals.video = 0;
  srv_get_cookie(0x5f56444fL/*'_VDO'*/,&globals.video);
  
  switch(globals.video >> 16) {
  case 0x00:
    fprintf(stderr,"ST video shifter detected.\r\n");
    break;
    
  case 0x01:
    fprintf(stderr,"STe video shifter detected.\r\n");
    break;
    
  case 0x02:
    fprintf(stderr,"TT video shifter detected.\r\n");
    break;
    
  case 0x03:
    fprintf(stderr,"Falcon video shifter detected.\r\n");
    break;
    
  default:
    fprintf(stderr,"Unknown video shifter!\r\n");
  }
#endif
  
  for(i = 1; i < argc; i++) {
    if(!strcmp("--nocnf",argv[i])) {
      nocnf = 1;
    }
    else {
      fprintf(stderr,"Unknown option %s\r\n",argv[i]);
    };
  };

  Boot_parse_cnf();

  init_aes (nocnf);

  /*
  srv_setpath ("u:\\");
  */

  sleep (1);

#ifdef LAUNCHER_AS_PRG
  system ("launcher.prg");
#else

#ifdef MINT_TARGET
  launcher_main();
#else
  {
    int child;
    
    child = fork();

    if(child == 0)
    {
      launcher_main();
      exit(0);
    }
    else
    {
      waitpid(child, NULL, 0);
    }
  }
#endif

#endif

  exit_aes();
  
  return 0;
}
