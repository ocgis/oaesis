/*
** srv_wind.h
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
** Server part of wind_update
**
** 1999-02-05 CG
*/
void
srv_wind_update (COMM_HANDLE     handle,
                 C_WIND_UPDATE * msg);

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

#endif /* _SRV_WIND_H_ */
