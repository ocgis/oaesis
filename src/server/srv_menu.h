/*
** srv_menu.h
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


/*
** Description
** Get the owner of the topped menu
*/
WORD
get_top_menu_owner(void);

/*
** Description
** Redraw the menu bar
*/
void
redraw_menu_bar(void);

/*
** Description
** Server part of 0x001e menu_bar ()
*/
void
srv_menu_bar(C_MENU_BAR * msg,
             R_MENU_BAR * ret);

/*
** Description
** Implementation of menu_register ()
*/
void
srv_menu_register (C_MENU_REGISTER * par,
                   R_MENU_REGISTER * ret);
