/*
** srv_call.h
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

/*
** Description
** Handle incoming server request
*/

#ifndef __SRV_CALL_H__
#define __SRV_CALL_H__

#include "srv_get.h"
#include "srv_interface.h"

void
srv_call(COMM_HANDLE handle,
         C_SRV *     par);

#endif /* __SRV_CALL_H__ */
