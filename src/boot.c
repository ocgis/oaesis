/****************************************************************************

 Module
  boot.c
  
 Description
  Bootup routines used in oAESis.
  
 Author(s)
  cg (Christer Gustavsson <d2cg@dtek.chalmers.se>)
  jps (Jan Paul Schmidt <Jan.P.Schmidt@mni.fh-giessen.de>)

 Revision history
 
  960421 cg
   Created module.

  960507 jps
   added AE_REALSLIDE variable

 Copyright notice
  The copyright to the program code herein belongs to the authors. It may
  be freely duplicated and distributed without fee, but not charged for.
 
 ****************************************************************************/

/****************************************************************************
 * Used interfaces                                                          *
 ****************************************************************************/

#include <basepage.h>
#include <ctype.h>
#include <mintbind.h>
#include <process.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "gemdefs.h"
#include "global.h"
#include "mintdefs.h"
#include "misc.h"
#include "srv.h"
#include "types.h"

#include <sysvars.h>

/****************************************************************************
 * Local functions (use static!)                                            *
 ****************************************************************************/

static void get_token(FILE *fp,BYTE *token) {
	fscanf(fp,"%[ \t]",token);
		
	if(fscanf(fp,"%[^= \t\n\r]",token) == 0) {
		if(fscanf(fp,"%[=]",token) == 0) {
			fscanf(fp,"%[\n\r]",token);
			strcpy(token,"\n");
		};
	};
}

/****************************************************************************
 * Public functions                                                         *
 ****************************************************************************/

/****************************************************************************
 * Boot_parse_cnf                                                           *
 *  Parse oaesis.cnf.                                                       *
 ****************************************************************************/
void Boot_parse_cnf(void) /*                                                */
/****************************************************************************/
{
  FILE *fp;
  BYTE line[200];
  BYTE bootpath[] = "u:\\c\\";
  BYTE filepath[128];
  
  BYTE *filelist[] = {
    "mint\\oaesis.cnf",
    "multitos\\oaesis.cnf",
    "oaesis.cnf",
    NULL
    };
  
  WORD i = 0;
  
  bootpath[3] = (get_sysvar(_bootdev) >> 16) + 'a';
  
  fp = fopen("oaesis.cnf","r");

  printf("A\n");
 
  while(!fp && filelist[i]) {
    sprintf(filepath,"%s%s",bootpath,filelist[i]);
    
    fp = fopen(filepath,"r");
    
    i++;
  };
  
  printf("B\n");

  if(!fp) {
    return;
  };
  
  printf("C\n");

  while(1) {
    get_token(fp,line);

    printf("D: %s\n",line);

    
    if(feof(fp)) {
      break;
    };
    
    if(line[0] == '#') {
      fgets(line,200,fp);
    }
    else {
      if(!strcmp(line,"AE_DEBUG")) {
	BYTE path[128];
	BYTE lineend[128];
	
	get_token(fp,path);
	get_token(fp,path);
	get_token(fp,lineend);
	
	DB_setpath(path);				
      }
      else if(!strcmp(line,"AE_VMODE")) {
	BYTE size[128];
	BYTE lineend[128];
	
	get_token(fp,size);
	get_token(fp,size);
	get_token(fp,lineend);
	
	sscanf(size,"%hd",&globals.vmode);
      }
      else if(!strcmp(line,"AE_VMODECODE")) {
	BYTE size[128];
	BYTE lineend[128];
	
	get_token(fp,size);
	get_token(fp,size);
	get_token(fp,lineend);
	
	sscanf(size,"%hd",&globals.vmodecode);
      }
      else if(!strcmp(line,"AE_REALMOVE")) {
	BYTE size[128];
	BYTE lineend[128];
	
	get_token(fp,size);
	get_token(fp,size);
	get_token(fp,lineend);
	
	sscanf(size,"%hd",&globals.realmove);
      }
      else if(!strcmp(line,"AE_REALSIZE")) {
	BYTE size[128];
	BYTE lineend[128];
	
	get_token(fp,size);
	get_token(fp,size);
	get_token(fp,lineend);
	
	sscanf(size,"%hd",&globals.realsize);
      }
      else if(!strcmp(line,"AE_REALSLIDE")) {
	BYTE size[128];
	BYTE lineend[128];
	
	get_token(fp,size);
				get_token(fp,size);
	get_token(fp,lineend);

	sscanf(size,"%hd",&globals.realslide);
      }
      else if(!strcmp(line,"AE_FONTID")) {
	BYTE size[128];
	BYTE lineend[128];
	
	get_token(fp,size);
	get_token(fp,size);
	get_token(fp,lineend);
	
	sscanf(size,"%hd",&globals.fnt_regul_id);
      }
      else if(!strcmp(line,"AE_PNTSIZE")) {
	BYTE size[128];
	BYTE lineend[128];
	
	get_token(fp,size);
	get_token(fp,size);
	get_token(fp,lineend);
	
	sscanf(size,"%hd",&globals.fnt_regul_sz);
      }
      else if(!strcmp(line,"AE_WIND_APPL")) {
	BYTE size[128];
	BYTE lineend[128];
	
	get_token(fp,size);
	get_token(fp,size);
	get_token(fp,lineend);
	
	sscanf(size,"%hd",&globals.wind_appl);
      };
    }
  }
  
  fclose(fp);
}

/****************************************************************************
 * Boot_start_programs                                                      *
 *  Start programs mentioned in oaesis.cnf.                                 *
 ****************************************************************************/
void Boot_start_programs(      /*                                           */
WORD apid)                     /*                                           */
/****************************************************************************/
{
	_DTA newdta, *olddta;
	WORD found;
	FILE *fp;
	BYTE line[200];
	BYTE bootpath[] = "c:\\";
	BYTE filepath[128];
	
	BYTE *filelist[] = {
		"mint\\oaesis.cnf",
		"multitos\\oaesis.cnf",
		"oaesis.cnf",
		NULL
	};
	
	WORD i = 0;
	
	bootpath[0] = (get_sysvar(_bootdev) >> 16) + 'a';

	fp = fopen("oaesis.cnf","r");

	while(!fp && filelist[i]) {
		sprintf(filepath,"%s%s",bootpath,filelist[i]);
		
		fp = fopen(filepath,"r");
		
		i++;
	};
	
	if(!fp) {
		return;
	};

	while(1) {
		get_token(fp,line);
		
		if(feof(fp)) {
			break;
		};
		
		if(line[0] == '#') {
			fgets(line,200,fp);
		}
		else {
			if(!strcmp(line,"run")) {
				BYTE program[128],param[128];
				
				get_token(fp,program);
				fgets(&param[1],199,fp);
				
				if(param[strlen(&param[1])] == '\n') {
					param[strlen(&param[1])] = '\0';
				}
				
				param[0] = (BYTE)strlen(&param[1]);
				
				Srv_shel_write(apid,SWM_LAUNCH,0,0,program,param);
			}
			else if(!strcmp(line,"shell")) {
				BYTE shell[128],param[128];
				
				get_token(fp,shell);

				fgets(&param[1],199,fp);
				
				if(param[strlen(&param[1])] == '\n') {
					param[strlen(&param[1])] = '\0';
				}

				param[0] = (BYTE)strlen(&param[1]);
				
				Srv_shel_write(apid,SWM_LAUNCH,0,0,shell,param);
			}
			else if(!strcmp(line,"setenv")) {
				BYTE value[300];
				
				get_token(fp,value);
				get_token(fp,&value[strlen(value)]);
				get_token(fp,&value[strlen(value)]);

				Srv_shel_write(apid,SWM_ENVIRON,ENVIRON_CHANGE,0,value,NULL);
				
				get_token(fp,value);
			};
		}
	}
	
	fclose(fp);

	olddta = Fgetdta();
	Fsetdta(&newdta);
	
	Misc_setpath(bootpath);
	
	found = Fsfirst("*.acc",0);
	
	while(!found) {
		BYTE accname[128];
		
		sprintf(accname,"%s%s",bootpath,newdta.dta_name);
		
		Srv_shel_write(apid,SWM_LAUNCHACC,0,0,accname,"");
		found = Fsnext();
	};
		
	Fsetdta(olddta);
}

