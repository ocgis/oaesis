/*
** lib_mem.c
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

#define DEBUGLEVEL 0

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "debug.h"
#include "lib_comm.h"
#include "lib_mem.h"
#include "srv_interface.h"


typedef struct memory_handle_s
{
  void *      server_addr;
  void *      client_addr;
  LONG        amount;
  MEMORY_MODE mode;
} MEMORY_HANDLE_S;


/*
** Description
** Reserve a memory block on the server side
*/
static
inline
void *
Mem_alloc(size_t amount)
{
  C_MEMORY_ALLOC par;
  R_MEMORY_ALLOC ret;
  WORD           apid = 0;

  PUT_C_ALL(MEMORY_ALLOC, &par);

  par.amount = (ULONG)amount;

  /* Pass the call to the server */
  CLIENT_SEND_RECV(&par,
                   sizeof (C_MEMORY_ALLOC),
                   &ret,
                   sizeof (R_MEMORY_ALLOC));
  
  return (void *)ret.address;
}


/*
** Description
** Free a memory block on the server side
*/
static
inline
void
Mem_free(void * address)
{
  C_MEMORY_FREE par;
  R_MEMORY_FREE ret;
  WORD          apid = 0;

  PUT_C_ALL(MEMORY_FREE, &par);

  par.address = (ULONG)address;

  /* Pass the call to the server */
  CLIENT_SEND_RECV(&par,
                   sizeof (C_MEMORY_FREE),
                   &ret,
                   sizeof (R_MEMORY_FREE));  
}


/*
** Description
** Copy a memory block from client to server.
** Max size is MEMORY_BLOCK_SIZE.
*/
static
inline
void
Mem_set_block(void * dst,
              void * src,
              WORD   amount)
{
  C_MEMORY_SET par;
  R_MEMORY_SET ret;
  WORD         apid = 0;

  PUT_C_ALL(MEMORY_SET, &par);

  par.amount = amount;
  par.address = (ULONG)dst;
  memcpy(&par.data, src, amount);

  /* Pass the call to the server */
  CLIENT_SEND_RECV(&par,
                   sizeof (C_MEMORY_SET),
                   &ret,
                   sizeof (R_MEMORY_SET));  
}


/*
** Description
** Copy a memory block from server to client.
** Max size is MEMORY_BLOCK_SIZE.
*/
static
inline
void
Mem_get_block(void * dst,
              void * src,
              WORD   amount)
{
  C_MEMORY_GET par;
  R_MEMORY_GET ret;
  WORD         apid = 0;

  PUT_C_ALL(MEMORY_GET, &par);

  par.amount = amount;
  par.address = (ULONG)src;

  /* Pass the call to the server */
  CLIENT_SEND_RECV(&par,
                   sizeof (C_MEMORY_GET),
                   &ret,
                   sizeof (R_MEMORY_GET));  

  memcpy(dst, &ret.data, amount);
}


/*
** Description
** Copy memory from client to server.
*/
static
inline
void
Mem_set(void * dst,
        void * src,
        LONG   amount)
{
  while(amount > 0)
  {
    Mem_set_block(dst, src,
                  (amount > MEMORY_BLOCK_SIZE) ? MEMORY_BLOCK_SIZE : amount);

    amount -= MEMORY_BLOCK_SIZE;
    src = (BYTE *)src + MEMORY_BLOCK_SIZE;
    dst = (BYTE *)dst + MEMORY_BLOCK_SIZE;
  }
}


/*
** Description
** Copy memory from server to client.
*/
static
inline
void
Mem_get(void * dst,
        void * src,
        LONG   amount)
{
  while(amount > 0)
  {
    Mem_get_block(dst, src,
                  (amount > MEMORY_BLOCK_SIZE) ? MEMORY_BLOCK_SIZE : amount);

    amount -= MEMORY_BLOCK_SIZE;
    src = (BYTE *)src + MEMORY_BLOCK_SIZE;
    dst = (BYTE *)dst + MEMORY_BLOCK_SIZE;
  }
}


/*
** Description
** Register a memory area for use in the server
*/
MEMORY_HANDLE
Mem_register(void *      address,
             LONG        amount,
             MEMORY_MODE mode)
{
  MEMORY_HANDLE handle = (MEMORY_HANDLE)malloc(sizeof(MEMORY_HANDLE_S));

  handle->server_addr = Mem_alloc(amount);
  handle->client_addr = address;
  handle->amount = amount;
  handle->mode = mode;

  /* Don't care about the memory mode for now: always sync */
  Mem_set(handle->server_addr, handle->client_addr, handle->amount);

  return handle;
}


/*
** Description
** Unregister a memory area
*/
void
Mem_unregister(MEMORY_HANDLE mem)
{
  /* Don't care about the memory mode for now: always sync */
  Mem_get(mem->client_addr, mem->server_addr, mem->amount);

  Mem_free(mem->server_addr);
  /*free(mem);*/
}


/*
** Description
** Get server address from memory handle
*/
void *
Mem_server_addr(MEMORY_HANDLE mem)
{
  return (mem == MEMORY_HANDLE_NIL) ? NULL : mem->server_addr;
}
