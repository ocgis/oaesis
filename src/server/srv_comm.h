/*
** srv_comm.h
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

#ifndef _SRV_COMM_H_
#define _SRV_COMM_H_

#include "srv_get.h"
#include "srv_interface.h"

#define SRV_GET(parameters,max_size,handle) \
{ \
  handle = Srv_get(parameters, max_size); \
  if(handle != NULL) { \
    NTOH(parameters); \
  } \
}

#define SRV_REPLY(handle,parameters,size) \
{ \
  HTON(parameters); \
  Srv_reply(handle,parameters,size); \
}

#endif /* _SRV_COMM_H_ */
