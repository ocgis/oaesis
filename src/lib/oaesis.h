/*
** aesbind.h
**
** Copyright 1998-1999 Christer Gustavsson <cg@nocrew.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**  
** Read the file COPYING for more information.
**
*/

#ifndef _OAESIS_H_
#define _OAESIS_H_

typedef enum
{
  OAESIS_PATH_MODE_UNIX,
  OAESIS_PATH_MODE_MINT
} OAESIS_PATH_MODE;

/*
** Description
** Set emulation path mode
*/
void
Oaesis_set_path_mode(OAESIS_PATH_MODE path_mode);

typedef enum
{
  OAESIS_ENDIAN_HOST,
  OAESIS_ENDIAN_BIG,
  OAESIS_ENDIAN_LOW
} OAESIS_ENDIAN;

/*
** Description
** Set client endian. Default is the host endian.
*/
void
Oaesis_set_client_endian(OAESIS_ENDIAN client_endian);

/*
** Description
** Install a callback handler
*/
void
Oaesis_callback_handler(void * callback_handler);

#endif /* _OAESIS_H_ */
