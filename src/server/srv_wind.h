/*
** srv_wind.h
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

#ifndef _SRV_WIND_H_
#define _SRV_WIND_H_

#include "srv_appl_info.h"
#include "srv_get.h"
#include "srv_interface.h"
#include "types.h"

/* appl_* related */ /* FIXME */
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


/* Global variables. These should be removed from here FIXME */
extern WINLIST * win_vis;
extern WINLIST * win_list;
extern WINLIST * win_free;
extern WORD	 win_next;


/*
** Description
** Implementation of wind_close ()
*/
void
srv_wind_close (C_WIND_CLOSE * par,
                R_WIND_CLOSE * ret);

/*
** Description
** Implementation of wind_create()
*/
void
srv_wind_create(C_WIND_CREATE * msg,
                R_WIND_CREATE * ret);

/*
** Description
** Implementation of wind_delete()
*/
void
srv_wind_delete (C_WIND_DELETE * msg,
                 R_WIND_DELETE * ret);

/*
** Description
** Find window on known coordinates
**
** 1999-02-05 CG
*/
void
srv_wind_find (C_WIND_FIND * par,
               R_WIND_FIND * ret);

/*
** Description
** Implementation of wind_get()
**
** 1999-02-05 CG
*/
void
srv_wind_get (C_WIND_GET * msg,
              R_WIND_GET * ret);

/*
** Description
** Implementation of wind_new()
*/
WORD
srv_wind_new(WORD apid);

/*
** Description
** Implementation of wind_open ()
*/
void
srv_wind_open (C_WIND_OPEN * msg,
               R_WIND_OPEN * ret);

/*
** Description
** Server part of wind_set ()
*/
void
srv_wind_set(C_WIND_SET * msg,
             R_WIND_SET * ret);

/*
** Description
** Server part of wind_update
**
** 1999-02-05 CG
*/
void
srv_wind_update (COMM_HANDLE     handle,
                 C_WIND_UPDATE * msg);

/*
** Description
** Free locks that an exited application has
*/
void
srv_wind_update_clear(WORD apid);

/*
** Description
** Find out which application that "owns" mouse clicks
**
** 1999-02-05 CG
*/
WORD
srv_click_owner (WORD mouse_x,
                 WORD mouse_y);

/*
** Description
** Output debugging information about windowing functions
*/
void
srv_wind_debug(int mouse_x,
               int mouse_y);

/* Description
** Create/fetch a free window structure/id
*/
WINSTRUCT *
winalloc(void);

/*
** Description
** Initialize windows
*/
void
srv_init_windows(void);

/*
** Description
** Try to top window
*/
WORD
top_window (WORD winid);

WINSTRUCT *
find_wind_description(WORD id);

/*
** Description
** Update the owner of the desktop window
*/
void
update_desktop_owner(void);

#endif /* _SRV_WIND_H_ */
