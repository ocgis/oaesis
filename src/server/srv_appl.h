/*
** srv_appl.h
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

#include "srv_interface.h"

/* Internal appl_control() modes */
#define APC_TOPNEXT 0
#define APC_KILL    1


/*
** Description
** Server part of srv_appl_control
*/
void
srv_appl_control(C_APPL_CONTROL * msg,
                 R_APPL_CONTROL * ret);

/*
** Description
** Implementation of appl_exit()
*/
void
srv_appl_exit(C_APPL_EXIT * par,
              R_APPL_EXIT * ret);

/*
** Description
** Implementation of appl_find()
*/
void
srv_appl_find(C_APPL_FIND * msg,
              R_APPL_FIND * ret);

/*
** Description
** appl_init help call
*/
void
srv_appl_init(C_APPL_INIT * par,
              R_APPL_INIT * ret);

/*
** Description
** Reserve an application id
*/
void
srv_appl_reserve(C_APPL_RESERVE * par,
                 R_APPL_RESERVE * ret);

/*
** Description
** Implementation of appl_search ()
*/
void
srv_appl_search(C_APPL_SEARCH * msg,
                R_APPL_SEARCH * ret);

/*
** Description
** Implementation of appl_write()
*/
void
srv_appl_write (C_APPL_WRITE * msg,
                R_APPL_WRITE * ret);

/*
** Description
** Get the id of the application that owns the desktop
*/
WORD
get_desktop_owner_id (void);


/*
** Description
** Update all of the desk background
*/
void
update_desktop_background (void);
