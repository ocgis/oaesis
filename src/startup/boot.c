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

#include "boot.h"
#include "debug.h"
#include "launcher.h"
#include "mintdefs.h"
#include "srv.h"
#include "types.h"

#ifdef HAVE_SYSVARS_H
#include <sysvars.h>
#endif

/*
** Description
** This function looks for something looking like a token, 
** ignoring the comments (lines beginning with '#')
*/
static
void
get_token(FILE * fp,
          char * token)
{
  /* Skip whitespace or comments */
  while(feof(fp) == 0)
  {
    char c;

    c = fgetc(fp);

    if(c == '#')
    {
      char buffer[200];

      /* Comment found: skip rest of line */
      fgets(buffer, sizeof(buffer) - 1, fp);
    }
    else if(!isspace(c))
    {
      /* Found non whitespace character */
      ungetc(c, fp);
      break;
    }
  }

  /* Get token */
  while(feof(fp) == 0)
  {
    char c;

    c = fgetc(fp);

    if((c == '#') || isspace(c))
    {
      ungetc(c, fp);
    }
    else
    {
      *token = c;
      token++;
    }
  }

  /* Terminate token */
  *token = 0;
}


/*
** Description
** Extract the value of a token, taking quotes into account
*/
static
void
get_value(FILE * fp,
          char * value)
{
  int data = 0;
  
  /* Scrap spaces after the separator */
  while(feof(fp) == 0)
  {
    data = fgetc(fp);

    if(!isspace(data))
    {
      break;
    }
  }

  if(feof(fp) == 0)
  {
    switch(data)
    {
    case '"':	/* " data is quoted */
    case '\'':	/* ' */
      while(feof(fp) == 0)
      {
        *value = fgetc(fp);
        if(*value == data)
        {
          /* Skip the quote */
          break;
        }
        else if(*value >= ' ')
        {
          value++;
        }
        else
        {
          ungetc(*value, fp);
          break;
        }
      }
      break;
      
    default:	/* value is not quoted */
      while(feof(fp) == 0)
      {
        *value = fgetc(fp);

        if(isspace(*value))
        {
          ungetc(*value, fp);
          break;
        }
        else
        {
          value++;
        }
      }
    }
  }

  *value = 0;
  DEBUG3("Got value %s",value);
}


/*
** Description
** Find the oaesis configuration file and open it
*/
static
FILE *
open_config_file(void)
{
  char   config_path[FILENAME_MAX*2];
  FILE * fp = NULL;
  char * home;
  int    i;
#ifndef MINT_TARGET
  char * filelist[] =
  {
    "usr/share/osis/oaesis.cnf",
    ".oaesisrc",
    NULL
  };
  char   bootdrive[]="/";	
#else		
  char * filelist[] =
  {
    "mint/oaesis.cnf",
    "multitos/oaesis.cnf",
    "oaesis.cnf",
    NULL
  };
  char   bootdrive[] = "x:/";

  bootdrive[0] = (get_sysvar(_bootdev) >> 16) + 'a'; 
#endif
  
  /* Is the HOME environment variable set? */
  home = getenv("HOME");
  if(home != NULL)
  {
    strcpy(config_path, home);
    strcat(config_path, "/.oaesisrc");
    fp = fopen(config_path, "r");
  }		
  
  for(i = 0; filelist[i] && (fp == NULL); i++)
  {
    if(strchr(filelist[i], '/'))
    {
      strcpy(config_path, bootdrive);
    }
    else
    {
      config_path[0] = '\0';
    }
    
    strcat(config_path, filelist[i]);
    DEBUG2("Trying to find config file in %s", config_path);
    fp = fopen(config_path, "r");
  }
  
  return fp;
}


static
void
check_srv_feature(FILE *       fp,
                  SRV_VAR_KIND kind)
{
  SRV_VAR_VALUE srv_value;
  char          value[200];
  
  get_value(fp, value);
  
  if(strcmp(value, "enabled") == 0)
  {
    srv_value.feature = SRV_VAR_ENABLED;
    Srv_set_variable(kind, &srv_value);
  }
  else if(strcmp(value, "disabled") == 0)
  {
    srv_value.feature = SRV_VAR_DISABLED;
    Srv_set_variable(kind, &srv_value);
  }
}


static
void
check_srv_integer(FILE *       fp,
                  SRV_VAR_KIND kind)
{
  char          value[200];
  SRV_VAR_VALUE srv_value;

  get_value(fp, value);
  sscanf(value, "%d", &srv_value.integer);
  Srv_set_variable(kind, &srv_value);
}


/*
** Description
** Parse oaesis.cnf
*/
void Boot_parse_cnf(void)
{
  FILE * fp;
  char   token[32];
  char   value[1000];
  BYTE   line[200];
  
  /* Find configuration file */
  fp = open_config_file();
  
  /* Didn't we find any configuration file? */
  if(fp == NULL)
  {
    DEBUG1("oAESis: couldn't find configuration file\n");
  }
  else
  {
    DEBUG2("Beginning parsing the config file");
    
    for(;;)
    {
      get_token(fp,line);
      
      if(feof(fp))
      {
        break;
      }      
      else if(!strcmp(line,"realmove"))
      {
        check_srv_feature(fp, SRV_VAR_REALMOVE);
      }
      else if(!strcmp(line,"realsize"))
      {
        check_srv_feature(fp, SRV_VAR_REALSIZE);
      }
      else if(!strcmp(line,"realslide"))
      {
        check_srv_feature(fp, SRV_VAR_REALSLIDE);
      }
      else if(!strcmp(line,"font"))
      {
        check_srv_integer(fp, SRV_VAR_FONT);
      }
      else if(!strcmp(line,"fontsize"))
      {
        check_srv_integer(fp, SRV_VAR_FONTSIZE);
      }
      else if(!strcmp(line, "growbox"))
      {
        check_srv_feature(fp, SRV_VAR_GROWBOX);
      }
      else if(!strcmp(line, "movebox"))
      {
        check_srv_feature(fp, SRV_VAR_MOVEBOX);
      }
      else if(!strcmp(line, "shrinkbox"))
      {
        check_srv_feature(fp, SRV_VAR_SHRINKBOX);
      }
      else if(!strcmp(line, "accpath"))
      {
        get_value(fp, value);
        launcher_set_accessory_path(value);
      }
      else if((strcmp(line, "run") == 0) || (strcmp(line, "shell") == 0))
      {
        char * arg;

        get_value(fp, value);

        for(arg = value; !isspace(*arg) && (*arg != 0); arg++)
        {
          ;
        }
       
        if(isspace(*arg))
        {
          *arg = 0;
          arg++;
        }

        if(strcmp(line, "run") == 0)
        {
          launcher_add_startup_application(value,
                                           arg);
        }
        else
        {
          launcher_set_shell_application(value,
                                         arg);
        }
      }
      else if(!strcmp(line, "setenv"))
      {
        get_token(fp,token);
        get_value(fp,value);
        launcher_set_environment_variable(token, value);
      }
    }
    
    fclose(fp);
  }
}
