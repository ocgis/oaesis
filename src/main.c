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

#include <unistd.h>

#include "debug.h"
#include "evnthndl.h"
#include "graf.h"
#include "lxgemdos.h"
#include "menu.h"
#include "misc.h"
#include "mousedev.h"
#include "objc.h"
#include "resource.h"
#include "srv.h"
#include "version.h"

#ifdef HAVE_SYSVARS_H
#include	<sysvars.h>
#endif

void init_aes(WORD nocnf) {
  fprintf(stderr,"Initializing:\r\n");
  
  init_global(nocnf);

  fprintf(stderr,"Server\r\n");
  Srv_init_module();
  
#ifdef MINT_TARGET
  fprintf(stderr,"AES trap vector\r\n");
  Supexec(link_in);
#endif /* MINT_TARGET */

  fprintf(stderr,"Object calls\r\n");
  Objc_init_module();
  
  fprintf(stderr,"Menu calls\r\n");
  Menu_init_module();
  
  fprintf(stderr,"Graf calls\r\n");
  Graf_init_module();

#ifdef MINT_TARGET  
  fprintf(stderr,"Mouse device\r\n");
  Moudev_init_module();
#endif /* MINT_TARGET */

  fprintf(stderr,"Event handler\r\n");
  Evhd_init_module();
  
  fprintf(stderr,"Done.\r\n");
}

void exit_aes(void) {
  fprintf(stderr,"Exiting:\r\n");
  
  fprintf(stderr,"Event handler\r\n");
  
  Evhd_exit_module();
  
  fprintf(stderr,"Menu calls\r\n");

  Menu_exit_module();
  
#ifdef MINT_TARGET
  fprintf(stderr,"Mouse device\r\n");
  
  Moudev_exit_module();
  
  fprintf(stderr,"Aes trap vector\r\n");
  
  Supexec(link_remove);
#endif /* MINT_TARGET */
  
  fprintf(stderr,"Object calls\r\n");
  
  Objc_exit_module();
  
  fprintf(stderr,"Global\r\n");
  
  exit_global();
  
  fprintf(stderr,"Server\r\n");
  Srv_exit_module();
  
  fprintf(stderr,"Done.\r\n");
}


int main(int argc,BYTE *argv[],BYTE *envp[]) {
  WORD nocnf = 0;
  WORD i;

#ifdef MINT_TARGET
  LONG mintval;
#endif  

  /* We only need to check for mint if we've built oaesis for mint */
#ifdef MINT_TARGET
  if(!Misc_get_cookie(0x4d694e54L /*'MiNT'*/,&mintval)) {
    fprintf(stderr,"oAESis requires MiNT to work. Start MiNT and try again!\r\n");
    
    return -1;
  };
#endif
  
  fprintf(stderr,"Starting oAESis version %s.\r\n",VERSIONTEXT);
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
 
#ifdef MINT_TARGET 
  fprintf(stderr,"\r\nMiNT version %ld.%02ld detected\r\n",mintval >> 8,mintval & 0xff);
  
  globals.video = 0;
  Misc_get_cookie(0x5f56444fL/*'_VDO'*/,&globals.video);
  
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
  
  init_aes(nocnf);
  
  Misc_setpath("u:\\");
  
  Menu_handler(envp);
  
  exit_aes();
  
  return 0;
}


