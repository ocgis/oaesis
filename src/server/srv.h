#ifndef __SRV__
#define __SRV__

#define SRV_COMPLETE_MCTRL 0x1000

/* Internal appl_control() modes */

#define APC_TOPNEXT 0
#define APC_KILL    1


#define TOP_APPL       (-1)
#define DESK_OWNER     (-2)
#define TOP_MENU_OWNER (-3)

/*window status codes*/

#define	WIN_OPEN       0x0001
#define	WIN_UNTOPPABLE 0x0002
#define	WIN_DESKTOP    0x0004
#define	WIN_TOPPED     0x0008
#define	WIN_DIALOG     0x0010
#define	WIN_MENU       0x0020
#define WIN_ICONIFIED  0x0040

typedef struct {
	WORD eventpipe;
	WORD msgpipe;
	WORD vid;
}SRV_APPL_INFO;

/*
** Description
** Initialize server module
**
** 1999-05-20 CG
*/
void
Srv_init_module (WORD no_config);

/*
** Description
** Request the server to stop
**
** 1999-07-27 CG
*/
void
Srv_stop (void);

#endif
