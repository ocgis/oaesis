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

  960816 jps
   some new variables

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
 * Local variables                                                          *
 ****************************************************************************/

static char Boot_acc_path[128] = "\0";

/****************************************************************************
 * Local functions (use static!)                                            *
 ****************************************************************************/

static WORD get_token(FILE *fp,BYTE *token) {
  WORD err;

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
      }
			else if(!strcmp(line,"AE_TRACE")) {
				BYTE size[128];
				BYTE lineend[128];
				
				get_token(fp,size);
				get_token(fp,size);
				get_token(fp,lineend);

				sscanf(size,"%hd",&globals.aes_trace);
			}
			else if(!strcmp(line, "AE_GRAF_MBOX")) {
				BYTE size[128];
				BYTE lineend[128];
				
				get_token(fp,size);
				get_token(fp,size);
				get_token(fp,lineend);

                                sscanf(size,"%hd",&globals.graf_mbox);
			}
			else if(!strcmp(line, "AE_GRAF_GROWBOX")) {
				BYTE size[128];
				BYTE lineend[128];
				
				get_token(fp,size);
				get_token(fp,size);
				get_token(fp,lineend);

                                sscanf(size,"%hd",&globals.graf_growbox);
			}
			else if(!strcmp(line, "AE_GRAF_SHRINKBOX")) {
				BYTE size[128];
				BYTE lineend[128];
				
				get_token(fp,size);
				get_token(fp,size);
				get_token(fp,lineend);

                                sscanf(size,"%hd",&globals.graf_shrinkbox);
			}
			else if(!strcmp(line, "AE_FSEL_SORTED")) {
				BYTE size[128];
				BYTE lineend[128];
				
				get_token(fp,size);
				get_token(fp,size);
				get_token(fp,lineend);

                                sscanf(size,"%hd",&globals.fsel_sorted);
			}
/* this one I need for tests with extern fileselectors. jps */
			else if(!strcmp(line, "AE_FSEL_EXTERN")) {
				BYTE size[128];
				BYTE lineend[128];
				
				get_token(fp,size);
				get_token(fp,size);
				get_token(fp,lineend);

                                sscanf(size,"%hd",&globals.fsel_extern);
			}
			else if(!strcmp(line, "AE_ACC_PATH")) {
				BYTE path[128];
				BYTE lineend[128];

				get_token(fp,path);
				get_token(fp,path);
				get_token(fp,lineend);

				strcpy(Boot_acc_path, path);
			}
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
	

    if(!(*Boot_acc_path)) {
	Boot_acc_path[0] = (get_sysvar(_bootdev) >> 16) + 'a';
        Boot_acc_path[1] = ':';
        Boot_acc_path[2] = '\\';
    }

    Misc_setpath(Boot_acc_path);
/*
	Misc_setpath(bootpath);
*/
	
	found = Fsfirst("*.acc",0);
	
	while(!found) {
		BYTE accname[128];
		
		sprintf(accname,"%s%s",bootpath,newdta.dta_name);
		
		Srv_shel_write(apid,SWM_LAUNCHACC,0,0,accname,"");
		found = Fsnext();
	};
		
	Fsetdta(olddta);
}

