/*
** lib_comm.h
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

#ifndef _LIB_COMM_H_
#define _LIB_COMM_H_

#include "srv_interface.h"
#include "srv_put.h"

#define CLIENT_SEND_RECV(par_in,size_in,par_out,size_out) \
{ \
  HTON(par_in); \
  Client_send_recv(par_in, size_in, par_out, size_out); \
  NTOH(par_out); \
}

#endif /* _LIB_COMM_H_ */
