/*
** shel.c
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

#include <sys/stat.h>
#include <unistd.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_BASEPAGE_H
#include <basepage.h>
#endif

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#ifdef HAVE_IOCTL_H
#include <ioctl.h>
#ifdef MINT_TARGET
#define ploadinfo __ploadinfo
#endif /* MINT_TARGET */
#endif /* HAVE_IOCTL_H */

#ifdef HAVE_MINT_DCNTL_H
#include <mint/dcntl.h>
#endif /* HAVE_MINT_DCNTL_H */

#include <limits.h>

#ifdef HAVE_MINTBIND_H
#include <mintbind.h>
#endif

#ifdef HAVE_OSBIND_H
#include <osbind.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "aesbind.h"
#include "appl.h"
#include "debug.h"
/*#include "lxgemdos.h"*/
#include "lib_global.h"
#include "lib_misc.h"
#include "shel.h"
#include "types.h"

#ifdef MINT_TARGET
#include "mintdefs.h"
#endif

#define TOSAPP 0
#define GEMAPP 1

#ifdef MINT_TARGET
extern void prgstart(); /* In gcc.s or purec.s FIXME: make .h-file */
#endif


/*
** Description
** Implementation of shel_read
*/
#ifdef MINT_TARGET
static
WORD
Shel_do_read(BYTE * name,
             BYTE * tail)
{
  BYTE pname[30];
  _DTA *olddta,newdta;
  WORD retval;
  
  olddta = Fgetdta();
  Fsetdta(&newdta);
  
  sprintf(pname,"u:\\proc\\*.%03d",Pgetpid());
  if(Fsfirst(pname,0) == 0) {
    LONG fd;
    
    sprintf(pname,"u:\\proc\\%s",newdta.dta_name);
    
    if((fd = Fopen(pname,O_RDONLY)) >= 0) {
      struct ploadinfo li;
      
      li.fnamelen = 128;
      li.cmdlin = tail;
      li.fname = name;
      
      Fcntl((WORD)fd, (LONG)&li, PLOADINFO);
      Fclose((WORD)fd);
      retval = 1;
    }
    else
    {
      retval = 0;
    }
  }
  else
  {
    retval = 0;
  }
  
  Fsetdta(olddta);
  
  return retval;
}
#else /* not MINT_TARGET */
static
WORD
Shel_do_read(BYTE * name,
             BYTE * tail)
{
  /* FIXME: Implement */
  return 1;
}
#endif /* MINT_TARGET */


/*0x0078 shel_read*/
void
Shel_read(AES_PB *apb)
{
  apb->int_out[0] = Shel_do_read((BYTE *)apb->addr_in[0],
                                 (BYTE *)apb->addr_in[1]);
}


/*
** Description
** Start a GEM program under MiNT
*/
static
WORD
start_gem_program_mint(WORD   apid,
                       WORD   type,
                       BYTE * envir,
                       BYTE * ddir,
                       BYTE * cmd,
                       BYTE * tail)
{
  BYTE *     tmp;
  BYTE       oldpath[128];
  BYTE       exepath[128];
  WORD       pid = -1;
  WORD       ap_id;
  BASEPAGE * b;


  Dgetpath(oldpath,0);
  
  strcpy(exepath, cmd);
  
  if(ddir)
  {
    Misc_setpath(ddir);
    tmp = exepath;
  }
  else
  {
    tmp = strrchr(exepath,'\\');
    if(tmp != NULL)
    {
      char c = tmp[1];

      tmp[1] = 0;
      Misc_setpath(exepath);
      tmp[1] = c;
      tmp++;
    }
    else
    {
      tmp = exepath;
    }
  }

#ifdef MINT_TARGET /* FIXME: Linux version */
  /* Get semaphore */
  Psemaphore(SEM_LOCK, SHEL_WRITE_LOCK, -1);

  if(type == APP_ACCESSORY)
  {
    /* Load accessory or application into memory but don't start it */
    b = (BASEPAGE *)Pexec(3, tmp, tail, envir);
    
    if(((LONG)b) > 0)
    {
      Mshrink(b, 256 + b->p_tlen + b->p_dlen + b->p_blen);
    
      b->p_dbase = b->p_tbase;
      b->p_tbase = (BYTE *)prgstart;
      
      /* Start accessory */
      pid = (WORD)Pexec(106, tmp, (BYTE *)b, NULL);

      DEBUG3("pid = %d", pid);
    }
    else
    {
      DEBUG1("Pexec failed: code %ld",(LONG)b);
    }
  }
  else
  {
    /* Start application */
    pid = Pexec(100, tmp, tail, envir);
  }
  
  Misc_setpath(oldpath);
  
  if(pid < 0)
  {
    ap_id = 0;
  }
  else
  {
    ap_id = Appl_do_reserve(apid, type, pid);
  }

  /* Release the semaphore now that the application id is reserved */
  Psemaphore(SEM_UNLOCK, SHEL_WRITE_LOCK, 0);
#endif

  return ap_id;
}


/*
** Description
** Start a GEM program under Unix
*/
static
WORD
start_gem_program_unix(WORD   apid,
                       WORD   type,
                       BYTE * envir,
                       BYTE * ddir,
                       BYTE * cmd,
                       BYTE * tail)
{
  BYTE *     tmp;
  BYTE       oldpath[128];
  BYTE       exepath[128];
  WORD       pid = -1;
  WORD       ap_id;

  getcwd(oldpath, 128);
  
  strcpy(exepath, cmd);
  
  if(ddir)
  {
    chdir(ddir);
    tmp = exepath;
  }
  else
  {
    tmp = strrchr(exepath, '/');
    if(tmp != NULL)
    {
      char c = tmp[1];

      tmp[1] = 0;
      chdir(exepath);
      tmp[1] = c;
      tmp++;
    }
    else
    {
      tmp = exepath;
    }
  }

  /* Get semaphore */
  /* FIXME: Psemaphore(SEM_LOCK, SHEL_WRITE_LOCK, -1); */

  pid = fork();

  if(pid == 0)
  {
    /* Child */

    if(execl(tmp, tmp, NULL) == -1)
    {
      perror("execl failed");
    }

    exit(-1);
  }

  chdir(oldpath);
  
  if(pid < 0)
  {
    ap_id = 0;
  }
  else
  {
    ap_id = Appl_do_reserve(apid, type, pid);
  }

  /* Release the semaphore now that the application id is reserved */
  /* FIXME: Psemaphore(SEM_UNLOCK, SHEL_WRITE_LOCK, 0); */

  return ap_id;
}


/*
** Description
** Start a GEM program or accessory
*/
static
WORD
start_gem_program(WORD   apid,
                  WORD   type,
                  BYTE * envir,
                  BYTE * ddir,
                  BYTE * cmd,
                  BYTE * tail)
{
  GLOBAL_APPL * globals = get_globals (apid);

  if(globals->path_mode == OAESIS_PATH_MODE_MINT)
  {
    return start_gem_program_mint(apid,
                                  type,
                                  envir,
                                  ddir,
                                  cmd,
                                  tail);
  }
  else
  {
    return start_gem_program_unix(apid,
                                  type,
                                  envir,
                                  ddir,
                                  cmd,
                                  tail);
  }
}


/*
** Description
** Implementation of shel_write ()
*/
static
inline
WORD
Shel_do_write(WORD   apid,
              WORD   mode,
              WORD   wisgr,
              WORD   wiscr,
              BYTE * cmd,
              BYTE * tail)
{
  WORD    r = 0;
  BYTE *  tmp;
  BYTE *  ddir = NULL;
  BYTE *  envir = NULL;
  BYTE    oldpath[128];
  BYTE    exepath[128];			
  SHELW * shelw;
  
  shelw = (SHELW *)cmd;
  ddir = NULL;
  envir = NULL;
  
  if (mode & 0xff00) /* should we use extended info? */
  {
    cmd = shelw->newcmd;
    
    /*	
        if(mode & SW_PSETLIMIT) {
        v_Psetlimit = shelw->psetlimit;
        };
        
        if(mode & SW_PRENICE) {
        v_Prenice = shelw->prenice;
        };
    */
    
    if(mode & SW_DEFDIR) {
      ddir = shelw->defdir;
    }
      
    if(mode & SW_ENVIRON) {
      envir = shelw->env;
    }
  }
  
  mode &= 0xff;
  
  if (mode == SWM_LAUNCH)	/* - run application */ 
  {
    tmp = strrchr (cmd, '.');
    if(!tmp) {
      tmp = "";
    }
      
    /* use enviroment GEMEXT, TOSEXT, and ACCEXT. */
    if((strcasecmp(tmp,".app") == 0) || (strcasecmp(tmp,".prg") == 0)) {
      mode = SWM_LAUNCHNOW;
      wisgr = 1;
    } else if (strcasecmp(tmp,".acc") == 0) {
      mode = SWM_LAUNCHACC;
      wisgr = 3;
    } else {
      mode = SWM_LAUNCHNOW;
      wisgr = 0;
    }
  }
  
  switch (mode) {
  case SWM_LAUNCH: 	/* - run application */
    /* we just did take care of this case */
    break;
    
  case SWM_LAUNCHNOW: /* - run another application in GEM or TOS mode */
    if(wisgr == TOSAPP)
    {
      WORD fd;
      BYTE new_cmd[300];
      WORD t;
      
      Dgetpath(oldpath,0);
      
      strcpy (exepath, cmd);
      tmp = exepath;
      
      if(!ddir)
      {
	ddir = oldpath;
      }
      
      sprintf (new_cmd, "%s %s %s", ddir, cmd, tail + 1);
      
      fd = (int)Fopen("U:\\PIPE\\TOSRUN", 2);
      t = (short)strlen(new_cmd) + 1;
      
      Fwrite(fd, t, new_cmd);
      
      Fclose(fd);
      
      r = 1;
    }
    else /* GEMAPP */
    {
      r = start_gem_program(apid, APP_APPLICATION, envir, ddir, cmd, tail);
    }
    break;
    
  case SWM_LAUNCHACC: /* - run an accessory */
    r = start_gem_program(apid, APP_ACCESSORY, envir, ddir, cmd, tail);
    break;
    
  case SWM_SHUTDOWN: /* - set shutdown mode */
  case SWM_REZCHANGE: /* - resolution change */
  case SWM_BROADCAST: /* - send message to all processes */
    break;
    
  case SWM_ENVIRON: /* - AES environment */
    switch (wisgr) {
    case ENVIRON_CHANGE:
      putenv (cmd);
      r = 1;
      break;
      
    default:
      DB_printf("shel_write(SWM_ENVIRON,%d,...) not implemented.", wisgr);
      r = 0;
    }
    break;
    
  case SWM_NEWMSG: /* - I know about: bit 0: AP_TERM */
    /* FIXME
    if(apps[apid].id != -1) {
      apps[apid].newmsg = wisgr;
      r = 1;
    }
    else
    */
  {
    r = 0;
  }
    
  break;
    
  case SWM_AESMSG: /* - send message to the AES */
  default:
    ;
  };
  
  return r;
}

/*
** Exported
*/
void
Shel_write (AES_PB *apb)
{
  CHECK_APID(apb->global->apid);

  apb->int_out[0] =  Shel_do_write (apb->global->apid,
                                    apb->int_in[0],
                                    apb->int_in[1],
                                    apb->int_in[2],
                                    (BYTE *)apb->addr_in[0],
                                    (BYTE *)apb->addr_in[1]);
}


/*
** Description
** Implementation of shel_find () for MiNT
**
** 1999-03-14 CG
*/
static
WORD
Shel_do_find_mint (BYTE * buf) {
  BYTE name[128];
  BYTE tail[128], *p, *q;
  XATTR xa;
  
  strcpy(name,buf);
  
  /* we start by checking if the file is in our path */
  
  if(Fxattr(0,buf,&xa) == 0)  {
    /* check if we were passed an absolute path (rather simplistic)
     * if this was the case, then _don't_ concat name on the path */
    if (!((name[0] == '\\') ||
          (name[0] && (name[ 1] == ':')))) {
      Dgetpath(buf,0);
      strcat(buf, "\\");
      strcat(buf, name);
    };
    
    return SHEL_FIND_OK;
  }
  
  /*	strlwr(buf);  <-- Add equivalent */
  
  if(Fxattr(0,buf,&xa) == 0)  {
    /* check if we were passed an absolute path (rather simplistic)
     * if this was the case, then _don't_ concat name on the path */
    if (!((name[0] == '\\') ||
          (name[0] && (name[ 1] == ':')))) {
      Dgetpath(buf,0);
      strcat(buf, "\\");
      strcat(buf, name);
    };
    
    return SHEL_FIND_OK;
  }
  
  Shel_do_read(buf, tail);
  p = strrchr( buf, '\\');
  if(p) {
    strcpy( p+1, name);
    if(Fxattr(0, buf, &xa) == 0) 
      return SHEL_FIND_OK;
    
    q = strrchr( name, '\\');
    
    if(q) {
      strcpy(p, q);
      if(Fxattr(0, buf, &xa) == 0) {
        return SHEL_FIND_OK;
      }
      else {
        (void)0; /* <--search the PATH enviroment */
      }
    }
  }
  
  return SHEL_FIND_ERROR;
}


/*
** Description
** Implementation of shel_find () for Unix
**
** 1999-03-14 CG
*/
static
WORD
Shel_do_find_unix (BYTE * buf) {
  BYTE        name[128];
  BYTE        tail[128];
  BYTE *      p;
  BYTE *      q;
  struct stat st;

  strcpy (name, buf);
  
  /* we start by checking if the file is in our current working directory */
  
  if (stat (buf, &st) == 0)  {
    /* check if we were passed an absolute path (rather simplistic)
     * if this was the case, then _don't_ concat name on the path */
    if (!(name[0] == '/')) {
      char buf2[200];

      if (getcwd (buf2, 200) == NULL) {
        return SHEL_FIND_ERROR;
      }

      sprintf (buf, "%s/%s", buf2, name);
    }
    
    return SHEL_FIND_OK;
  }
  
  Shel_do_read (buf, tail);
  p = strrchr( buf, '/');

  if (p) {
    strcpy (p + 1, name);

    if (stat (buf, &st) == 0) {
      return SHEL_FIND_OK;
    }
    
    q = strrchr( name, '/');
    
    if (q) {
      strcpy (p, q);
      if (stat (buf, &st) == 0) {
        return SHEL_FIND_OK;
      }
      else {
        (void)0; /* <--search the PATH enviroment */
      }
    }
  }
  
  return SHEL_FIND_ERROR;
}


/*
** Exported
*/
WORD
Shel_do_find (WORD   apid,
              BYTE * buf)
{
  GLOBAL_APPL * globals = get_globals (apid);

  if(globals->path_mode == OAESIS_PATH_MODE_MINT)
  {
    return Shel_do_find_mint(buf);
  }
  else
  {
    return Shel_do_find_unix(buf);
  }
}


/*
** Exported
** 0x007c shel_find()
*/
void
Shel_find (AES_PB * apb)
{
  CHECK_APID(apb->global->apid);

  apb->int_out[0] = Shel_do_find (apb->global->apid, (BYTE *)apb->addr_in[0]);
}


/*
** Description
** Implementation of shel_envrn ()
*/
static
inline
WORD
Shel_do_envrn (BYTE ** value,
               BYTE *  name)
{
  *value = getenv (name);
  
  return 1;
}


/****************************************************************************
 * Shel_envrn                                                               *
 *  0x007d shel_envrn().                                                    *
 ****************************************************************************/
void              /*                                                        */
Shel_envrn(       /*                                                        */
AES_PB *apb)      /* AES parameter block.                                   */
/****************************************************************************/
{
  apb->int_out[0] = Shel_do_envrn((BYTE **)apb->addr_in[0],
                                  (BYTE *)apb->addr_in[1]);
}
