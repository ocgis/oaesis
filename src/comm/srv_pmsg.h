/*
** srv_pmsg.h
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

#ifndef _SRV_PMSG_H_
#define _SRV_PMSG_H_

#include "types.h"

#define SRVBOX 0x6f535256l /*'oSRV'*/

typedef struct
{
  void * in;
  void * out;
}DATA;

typedef struct
{
  DATA * par;
  WORD   bytes_in;
  WORD   bytes_out;
  WORD   pid;
}PMSG;

#endif /* _SRV_PMSG_H_ */
