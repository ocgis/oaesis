#include <mintbind.h>
#include <osbind.h>
#include <process.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <support.h>
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

#include	<sysvars.h>

void init_aes(WORD physical) {
	init_global(physical);

	printf("Initializing:\r\n");

	printf("Server\r\n");
	Srv_init_module();
	
	printf("AES trap vector\r\n");
	Supexec(link_in);

	printf("Object calls\r\n");
	init_objc();

	printf("Menu calls\r\n");
	Menu_init_module();

	printf("Graf calls\r\n");
	Graf_init_module();

	printf("Mouse device\r\n");
	Moudev_init_module();

	printf("Event handler\r\n");
	Evhd_init_module();
	
	printf("Done.\r\n");
}

void	exit_aes(void) {
	printf("Exiting:\r\n");

	printf("Event handler\r\n");

	Evhd_exit_module();
		
	printf("Menu calls\r\n");

	Menu_exit_module();

	printf("Mouse device\r\n");

	Moudev_exit_module();
	
	printf("Aes trap vector\r\n");

	Supexec(link_remove);

	printf("Object calls\r\n");

	exit_objc();

	printf("Global\r\n");

	exit_global();

	printf("Server\r\n");
	Srv_exit_module();
	
	printf("Done.\r\n");
}


int main(int argc,char *argv[],char *envp[]) {
	WORD physical = 0;
	WORD i;
	LONG mintval;

	if(!Misc_get_cookie(0x4d694e54L /*'MiNT'*/,&mintval)) {
		fprintf(stderr,"oAESis requires MiNT to work. Start MiNT and try again!\r\n");
		
		return -1;
	};

	printf("Starting oAESis version %s.\r\n",VERSIONTEXT);
	printf("Compiled on %s at %s with ",__DATE__,__TIME__);

#ifdef __TURBOC__
	printf("Pure C / Turbo C %x.%x.\r\n",__TURBOC__ >> 8,__TURBOC__ & 0xff);
#endif


	printf("The following options were used:\r\n");

#ifdef __68020__
	printf("- Main processor 68020.\r\n");
#endif

#ifdef __68881__
	printf("- Math co-processor 68881.\r\n");
#endif

	printf("\r\nMiNT version %ld.%02ld detected\r\n",mintval >> 8,mintval & 0xff);

	globals.video = 0;
	Misc_get_cookie('_VDO',&globals.video);

	switch(globals.video >> 16) {
	case 0x00:
		printf("ST video shifter detected.\r\n");
		break;
		
	case 0x01:
		printf("STe video shifter detected.\r\n");
		break;
		
	case 0x02:
		printf("TT video shifter detected.\r\n");
		break;
		
	case 0x03:
		printf("Falcon video shifter detected.\r\n");
		break;
		
	default:
		printf("Unknown video shifter!\r\n");
	}

	for(i = 1; i < argc; i++) {
		if(!strcmp("-physical",argv[i])) {
			physical = 1;
		}
		else {
			fprintf(stderr,"Unknown option %s\r\n",argv[i]);
		};
	};

	init_aes(physical);

	Misc_setpath("u:\\");

	Menu_handler(envp);
	
	exit_aes();

	return 0;
};


