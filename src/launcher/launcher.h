/*
** launcher.h
**
** Copyright 2000 Christer Gustavsson <cg@nocrew.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**  
** Read the file COPYING for more information.
*/

#ifndef __LAUNCHER_H__
#define __LAUNCHER_H__

/*
** Description
** Set accessories path
*/
void
launcher_set_accessory_path(char * accpath);

/*
** Description
** Add startup application
*/
void
launcher_add_startup_application(char * application,
                                 char * arg);

/*
** Description
** Set shell application
*/
void
launcher_set_shell_application(char * shell,
                               char * arg);

/*
** Description
** Set environment variable
*/
void
launcher_set_environment_variable(char * variable,
                                  char * value);

/*
** Description
** Main routine of the launcher
*/
int
launcher_main(void);

#endif /* __LAUNCHER_H__ */
