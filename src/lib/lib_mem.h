/*
** lib_mem.h
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

#ifndef __LIB_MEM_H__
#define __LIB_MEM_H__

typedef struct memory_handle_s * MEMORY_HANDLE;
#define MEMORY_HANDLE_NIL ((MEMORY_HANDLE)NULL)

typedef enum
{
  MEMORY_READ      = 1, /* The server is only allowed to read the memory     */
  MEMORY_WRITE     = 2, /* The server is only allowed to write the memory    */
  MEMORY_READWRITE = 3  /* The server is allowed to read or write the memory */
} MEMORY_MODE;

/*
** Description
** Register a memory area for use in the server
*/
MEMORY_HANDLE
Mem_register(void *      address,
             LONG        amount,
             MEMORY_MODE mode);

/*
** Description
** Unregister a memory area
*/
void
Mem_unregister(MEMORY_HANDLE mem);


/*
** Description
** Get server address from memory handle
*/
void *
Mem_server_addr(MEMORY_HANDLE mem);

#endif /* __LIB_MEM_H__ */
