/*
** shel.c
**
** Copyright 1999 Christer Gustavsson <cg@nocrew.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**  
** Read the file COPYING for more information.
*/

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
#endif

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
#include "debug.h"
/*#include "lxgemdos.h"*/
#include "lib_global.h"
#include "lib_misc.h"
#include "shel.h"
#include "srv_calls.h"
#include "types.h"

#define TOSAPP 0
#define GEMAPP 1

WORD Shel_do_read(BYTE *name,BYTE *tail) {
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
			struct __ploadinfo li;
			
			li.fnamelen = 128;
			li.cmdlin = tail;
			li.fname = name;
			
			Fcntl((WORD)fd,&li,PLOADINFO);
			Fclose((WORD)fd);
			retval = 1;
		}
		else {
			retval = 0;
		};
		
	}
	else {
		retval = 0;
	};

	Fsetdta(olddta);
	
	return retval;
}

/*0x0078 shel_read*/

void	Shel_read(AES_PB *apb) {
	apb->int_out[0] = Shel_do_read((BYTE *)apb->addr_in[0],(BYTE *)apb->addr_in[1]);
}


/****************************************************************************
 * Shel_write                                                               *
 *  0x0079 shel_write().                                                    *
 ****************************************************************************/
void              /*                                                        */
Shel_write(       /*                                                        */
AES_PB *apb)      /* AES parameter block.                                   */
/****************************************************************************/
{
	apb->int_out[0] = 
	Srv_shel_write(apb->global->apid,apb->int_in[0], apb->int_in[1], apb->int_in[2],
		(BYTE *)apb->addr_in[0], (BYTE *)apb->addr_in[1]);
		
	if(apb->int_out[0] == 0) {
		DB_printf("shel_write(0x%04x,0x%04x,0x%04x,\r\n"
																"%s,\r\n"
																"%s)", 
		        apb->int_in[0],apb->int_in[1],apb->int_in[2],
		        (BYTE *)apb->addr_in[0],(BYTE *)apb->addr_in[1]+1);
	}
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
**
** 1999-03-14 CG
*/
WORD
Shel_do_find (WORD   apid,
              BYTE * buf) {
  GLOBAL_APPL * globals = get_globals (apid);

  if (globals->use_mint_paths) {
    return Shel_do_find_mint (buf);
  } else {
    return Shel_do_find_unix (buf);
  }
}


/*
** Exported
** 0x007c shel_find()
**
** 1999-03-14 CG
*/
void
Shel_find (AES_PB * apb) {
	apb->int_out[0] =
          Shel_do_find (apb->global->apid, (BYTE *)apb->addr_in[0]);
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
  apb->int_out[0] = Srv_shel_envrn((BYTE **)apb->addr_in[0],
                                   (BYTE *)apb->addr_in[1]);
}
