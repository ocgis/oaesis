/*
** evnthndl.c
**
** Copyright 1996 - 2000 Christer Gustavsson <cg@nocrew.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**  
** Read the file COPYING for more information.
*/

#ifndef __EVHD__
#define __EVHD__

#include "types.h"

/*
** Description
** Handle button click
*/
WORD
Evhd_handle_button (WORD   apid,
                    WORD   events,
                    WORD   newbutton,
                    WORD   x,
                    WORD   y,
		    WORD   bclicks,
                    WORD   bmask,
                    WORD   bstate,
		    WORD * mc,
                    WORD   handle_user_click_only);


/*
** Description
** Handle menu
**
** 1999-01-09 CG
** 1999-05-24 CG
*/
void
Evhd_handle_menu (WORD apid,
                  WORD mouse_x,
                  WORD mouse_y);

#endif
