/*
** boot.c
**
** Copyright 1996-1999 Christer Gustavsson <cg@nocrew.org>
** Copyright 1996 Jan Paul Schmidt <Jan.P.Schmidt@mni.fh-giessen.de>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**  
** Read the file COPYING for more information.
*/


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

#include "misc.h"

/****************************************************************************
 * Local variables                                                          *
 ****************************************************************************/

static char Boot_acc_path[128] = "\0";

/****************************************************************************
 * Local functions (use static!)                                            *
 ****************************************************************************/

static
int
get_token(FILE * fp,
	  char * token)
{
  int err;

  fscanf(fp,"%[ \t]",token);
		
  if((err = fscanf(fp,"%[^= \t\n\r]",token)) == 0) {
    if((err = fscanf(fp,"%[=]",token)) == 0) {
      err = fscanf(fp,"%[\n\r]",token);
      strcpy(token,"\n");
    };
  };
  
  return err;
}


/****************************************************************************
** Description
** Find the oaesis configuration file and open it
**
** To be done
** Look for oaesis.cnf in <prefix>/share/osis/oaesis.cnf, <boot>/oaesis.conf,
** <boot>/mint/oaesis.cnf and <boot>/multitos/oaesis.cnf
**
** 1998-07-11 CG
*****************************************************************************/
static
FILE *
open_config_file () {
  char   config_path[256];
  FILE * fp;

  /* Is the HOME environment variable set? */
  if (getenv("HOME") != NULL) {
    /* Get value of $HOME */
    strcpy (config_path, getenv("HOME"));

    strcat (config_path, "/.oaesisrc");

    fprintf(stderr, "config path: %s", config_path);

#ifdef MINT_TARGET
    {
      char * tmp = config_path;

      /* Convert '/' to '\' */
      while (*tmp) {
	if (*tmp == '/') {
	  *tmp = '\\';
	}
	
	tmp++;
      }

      if(tmp[-1] == '\\') {
	tmp[-1] = '\0';
      }
    }

#endif

    fp = fopen(config_path, "r");

    /* Did we get lucky? */
    if (fp != NULL) {
      return fp;
    }
  }

  return NULL;
}


/****************************************************************************
 * Public functions                                                         *
 ****************************************************************************/

/****************************************************************************
 * Boot_parse_cnf                                                           *
 *  Parse oaesis.cnf.                                                       *
 ****************************************************************************/
static void Boot_parse_cnf(void) /*                                         */
/****************************************************************************/
{
  FILE *fp;
  char line[200];
  
  /* Find configuration file */
  fp = open_config_file();

  /* Didn't we find any configuration file? */
  if(fp == NULL) {
    fprintf (stderr, "oaesis: couldn't find configuration file\n");

    return;
  }
  
  while(TRUE)
  {
    get_token(fp,line);
    
    if(feof(fp)) {
      break;
    }
    
    if(line[0] == '#')
    {
      fgets(line,200,fp);
    }
    else
    {
      if(!strcmp(line,"AE_DEBUG"))
      {
	char path[128];
	char lineend[128];
	
	get_token(fp,path);
	get_token(fp,path);
	get_token(fp,lineend);
	
	/* FIXME: DB_setpath(path); */
      }
      else if(!strcmp(line,"AE_VMODE"))
      {
	char size[128];
	char lineend[128];
	
	get_token(fp,size);
	get_token(fp,size);
	get_token(fp,lineend);
	
	/* FIXME sscanf(size,"%hd",&globals.vmode); */
      }
      else if(!strcmp(line,"AE_VMODECODE"))
      {
	char size[128];
	char lineend[128];
	
	get_token(fp,size);
	get_token(fp,size);
	get_token(fp,lineend);
	
	/* FIXME sscanf(size,"%hd",&globals.vmodecode); */
      }
      else if(!strcmp(line,"AE_REALMOVE"))
      {
	char size[128];
	char lineend[128];
	
	get_token(fp,size);
	get_token(fp,size);
	get_token(fp,lineend);
	
	/* FIXME sscanf(size,"%hd",&globals.realmove); */
      }
      else if(!strcmp(line,"AE_REALSIZE"))
      {
	char size[128];
	char lineend[128];
	
	get_token(fp,size);
	get_token(fp,size);
	get_token(fp,lineend);
	
	/* FIXME sscanf(size,"%hd",&globals.realsize);*/
      }
      else if(!strcmp(line,"AE_REALSLIDE"))
      {
	char size[128];
	char lineend[128];
	
	get_token(fp,size);
	get_token(fp,size);
	get_token(fp,lineend);

	/* FIXME sscanf(size,"%hd",&globals.realslide); */
      }
      else if(!strcmp(line,"AE_FONTID"))
      {
	char size[128];
	char lineend[128];
	
	get_token(fp,size);
	get_token(fp,size);
	get_token(fp,lineend);
	
	/* FIXME sscanf(size,"%hd",&globals.fnt_regul_id); */
      }
      else if(!strcmp(line,"AE_PNTSIZE"))
      {
	char size[128];
	char lineend[128];
	
	get_token(fp,size);
	get_token(fp,size);
	get_token(fp,lineend);
	
	/* FIXME sscanf(size,"%hd",&globals.fnt_regul_sz); */
      }
      else if(!strcmp(line,"AE_WIND_APPL"))
      {
	char size[128];
	char lineend[128];
	
	get_token(fp,size);
	get_token(fp,size);
	get_token(fp,lineend);
	
	/* FIXME sscanf(size,"%hd",&globals.wind_appl); */
      }
      else if(!strcmp(line,"AE_TRACE"))
      {
	char size[128];
	char lineend[128];
	
	get_token(fp,size);
	get_token(fp,size);
	get_token(fp,lineend);
	
	/* FIXME sscanf(size,"%hd",&globals.aes_trace); */
      }
      else if(!strcmp(line, "AE_GRAF_MBOX"))
      {
	char size[128];
	char lineend[128];
	
	get_token(fp,size);
	get_token(fp,size);
	get_token(fp,lineend);
	
	/* FIXME sscanf(size,"%hd",&globals.graf_mbox); */
      }
      else if(!strcmp(line, "AE_GRAF_GROWBOX"))
      {
	char size[128];
	char lineend[128];
	
	get_token(fp,size);
	get_token(fp,size);
	get_token(fp,lineend);
	
	/* FIXME sscanf(size,"%hd",&globals.graf_growbox); */
      }
      else if(!strcmp(line, "AE_GRAF_SHRINKBOX"))
      {
	char size[128];
	char lineend[128];
	
	get_token(fp,size);
	get_token(fp,size);
	get_token(fp,lineend);
	
	/* FIXME sscanf(size,"%hd",&globals.graf_shrinkbox); */
      }
      else if(!strcmp(line, "AE_FSEL_SORTED"))
      {
	char size[128];
	char lineend[128];
	
	get_token(fp,size);
	get_token(fp,size);
	get_token(fp,lineend);
	
	/* FIXME sscanf(size,"%hd",&globals.fsel_sorted); */
      }
      /* this one I need for tests with extern fileselectors. jps */
      else if(!strcmp(line, "AE_FSEL_EXTERN"))
      {
	char size[128];
	char lineend[128];
	
	get_token(fp,size);
	get_token(fp,size);
	get_token(fp,lineend);
	
	/* FIXME sscanf(size,"%hd",&globals.fsel_extern); */
      }
      else if(!strcmp(line, "AE_ACC_PATH"))
      {
	char path[128];
	char lineend[128];
	
	get_token(fp,path);
	get_token(fp,path);
	get_token(fp,lineend);
	
	strcpy(Boot_acc_path, path);
      }
    }
  }
  
  fclose(fp);
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
  FILE *fp;
  char line[200];
  char bootpath[] = "c:\\";
  char filepath[128];
  
  char *filelist[] = {
    "mint\\oaesis.cnf",
    "multitos\\oaesis.cnf",
    "oaesis.cnf",
    NULL
  };
  
  int i = 0;
  
#ifdef MINT_TARGET /* FIXME for linux */
  bootpath[0] = (get_sysvar(_bootdev) >> 16) + 'a';
#endif
  
  misc_setpath(bootpath);

  fp = fopen("oaesis.cnf", "r");
  
  while((fp == NULL) && filelist[i])
  {
    sprintf(filepath, "%s%s", bootpath, filelist[i]);
    
    fp = fopen(filepath,"r");
    
    i++;
  }
  
  if(fp == NULL)
  {
    return;
  }
  
  while(TRUE)
  {
    get_token(fp,line);
    
    if(feof(fp))
    {
      break;
    }
		
    if(line[0] == '#')
    {
      fgets(line,200,fp);
    }
    else
    {
      if(!strcmp(line,"run")) {
	char program[128],param[128];
	
	get_token(fp,program);
	fgets(&param[1],199,fp);
	
	if(param[strlen(&param[1])] == '\n') {
	  param[strlen(&param[1])] = '\0';
	}
	
	param[0] = (char)strlen(&param[1]);
	
	shel_write(SWM_LAUNCH, 0, 0, program, param);
      }
      else if(!strcmp(line,"shell"))
      {
	char shell[128],param[128];
	
	get_token(fp,shell);
	
	fgets(&param[1],199,fp);
	
	if(param[strlen(&param[1])] == '\n') {
	  param[strlen(&param[1])] = '\0';
	}
	
	param[0] = (char)strlen(&param[1]);
	
	shel_write(SWM_LAUNCH, 0, 0, shell, param);
      }
      else if(!strcmp(line,"setenv"))
      {
	char value[300];
	
	get_token(fp,value);
	get_token(fp,&value[strlen(value)]);
	get_token(fp,&value[strlen(value)]);
	
	shel_write(SWM_ENVIRON, ENVIRON_CHANGE, 0, value, NULL);
	get_token(fp,value);
      }
    }
  }
  
  fclose(fp);
  
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
  
  misc_setpath(Boot_acc_path);
  
  found = Fsfirst("*.acc", 0);
  
  while(found == 0)
  {
    char accname[128];
    
    sprintf(accname,"%s%s", bootpath, newdta.dta_name);
    shel_write(SWM_LAUNCHACC,0,0,accname,"");

    found = Fsnext();
  }
  
  Fsetdta(olddta);
}
