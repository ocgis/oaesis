/*
** srv_malloc.h
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

#ifndef __SRV_MALLOC_H__
#define __SRV_MALLOC_H__

#include "config.h"
#include "types.h"

#ifdef SERVER_AS_DEVICE
extern struct kerinfo * kerinf; /* In srv_comm_device.h */
#define MALLOC (*kerinf->kmalloc)
#define FREE   (*kerinf->kfree)
#else
#define MALLOC malloc
#define FREE   free
#endif

#endif /* __SRV_MALLOC_H__ */
