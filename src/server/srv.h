#ifndef __SRV__
#define __SRV__

#include "types.h"

#define SRV_COMPLETE_MCTRL 0x1000


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


typedef enum
{
  SRV_VAR_REALMOVE,
  SRV_VAR_REALSIZE,
  SRV_VAR_REALSLIDE,
  SRV_VAR_FONT,
  SRV_VAR_FONTSIZE,
  SRV_VAR_GROWBOX,
  SRV_VAR_MOVEBOX,
  SRV_VAR_SHRINKBOX,
} SRV_VAR_KIND;

typedef enum
{
  SRV_VAR_ENABLED,
  SRV_VAR_DISABLED
} SRV_VAR_FEATURE;

typedef union
{
  SRV_VAR_FEATURE feature;
  int             integer;
} SRV_VAR_VALUE;

/*
** Description
** Set server variable
*/
void
Srv_set_variable(SRV_VAR_KIND    var,
                 SRV_VAR_VALUE * value);

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
