/*
** srv_comm_device.c
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

#define DEBUGLEVEL 0

#include <errno.h>
#include <stdio.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "debug.h"
#include "srv_call.h"
#include "srv_comm_device.h"
#include "srv_get.h"
#include "srv_trace.h"
#include "types.h"

#define COMM_DEVICE_NAME "u:\\dev\\oaes_ker"

#define C_CONOUT   (*kerinf->dos_tab[0x2])
#define C_CONWS    (*kerinf->dos_tab[0x9])
#define SPRINTF    (*kerinf->sprintf)
#define KMALLOC    (*kerinf->kmalloc)
#define KFREE      (*kerinf->kfree)
#define SLEEP      (*kerinf->sleep)
#define WAKE       (*kerinf->wake)
#define WAKESELECT (*kerinf->wakeselect)

#define FH(fptr) ((FileHandle *)((FILEPTR *)fptr)->devinfo)

typedef struct
{
  char buf_out[200];
  int  bytes_out;      /* Bytes waiting to be read                      */
  int  selected;       /* True if a process is selecting for the result */
  int  queued;         /* True if a process is queued in the IO_Q       */
  int  selecting_proc; /* Id of waiting process                         */
} FileHandle;

struct kerinfo * kerinf;

static DEVDRV comm_device;

static int    devd = -1;

static
LONG
CDECL
comm_open(FILEPTR * f)
{
  char deb[100];

  NOT_USED(f);


  /* Create and initialize internal file handle */
  f->devinfo = (LONG)KMALLOC(sizeof(FileHandle));
  
  FH(f)->bytes_out = 0;
  FH(f)->selected = 0;
  FH(f)->queued = 0;

  SPRINTF(deb,
          "srv_comm_device: open: devinfo = 0x%lx, f = 0x%lx",
          f->devinfo,
          f);
  TRACE(deb);

  return 0;
}

static
LONG
CDECL
comm_write(FILEPTR *    f,
           const BYTE * buf,
           LONG         nbytes)
{
  char deb[100];

  SPRINTF(deb,
          "srv_comm_device: write: %ld bytes, devinfo = 0x%lx, f = 0x%lx, flags = 0x%lx",
          nbytes,
          f->devinfo,
          (LONG)f,
          f->flags);
  TRACE(deb);

  if((f->flags & O_ACCMODE) == O_RDWR)
  {
    srv_call((COMM_HANDLE)f,
             (C_SRV *)buf);
    TRACE("Returned from srv_call");
  }
  else
  {
    TRACE("Poll events!");
    srv_handle_events();
  }

  return nbytes;
}

static
LONG
CDECL
comm_read(FILEPTR * f,
          BYTE *    buf,
          LONG      nbytes)
{
  int actual_bytes;

  TRACE("srv_comm_device: read");

  if(FH(f)->bytes_out == 0)
  {
    TRACE("Put in IO_Q");
    SLEEP(IO_Q, (LONG)f);
  }

  TRACE("srv_comm_device: read: 2");

  actual_bytes = (FH(f)->bytes_out < nbytes) ? FH(f)->bytes_out : nbytes;

  TRACE("srv_comm_device: read: 3");

  memcpy(buf, FH(f)->buf_out, actual_bytes);

  TRACE("srv_comm_device: read: 4");
  /* Truncate data that wouldn't fit */
  FH(f)->bytes_out = 0;

  return actual_bytes;
}

static
LONG
CDECL
comm_lseek(FILEPTR * f,
           LONG      where,
           WORD      whence)
{
  NOT_USED(f); NOT_USED(where); NOT_USED(whence);
  
  TRACE("srv_comm_device: lseek");
  return -EUKCMD;
}

static
LONG
CDECL
comm_ioctl(FILEPTR * f,
           WORD      mode,
           void *    buf)
{
  NOT_USED(f);
  
  TRACE("srv_comm_device: ioctl");
  return 0;
}

static
LONG
CDECL
comm_datime(FILEPTR * f,
            WORD *    timeptr,
            WORD      rwflag)
{
  NOT_USED(f); NOT_USED(timeptr); NOT_USED(rwflag);
  TRACE("srv_comm_device: datime");

  return -EUKCMD;
}

static
LONG
CDECL
comm_close(FILEPTR * f,
           WORD      pid)
{
  char deb[100];

  NOT_USED(pid);

  SPRINTF(deb, "srv_comm_device: close: devinfo = 0x%lx", f->devinfo);
  TRACE(deb);

  if(f->devinfo != 0)
  {
    KFREE((void *)f->devinfo);
    f->devinfo = 0;
  }

  return 0;
}

static
LONG
CDECL
comm_select(FILEPTR * f,
            LONG      p,
            WORD      mode)
{
  NOT_USED(f);

  TRACE("srv_comm_device: select");
  return 0;
}

static
LONG
CDECL
comm_unselect(FILEPTR * f,
              LONG      p,
              WORD      mode)
{
  NOT_USED(f);
  TRACE("srv_comm_device: unselect");

  return 0;
}


/*
** Description
** Reply to a client that has sent a message to the server
*/
void
Srv_reply(COMM_HANDLE handle,
	  void *      out,
	  WORD        bytes_out)
{
  TRACE("Srv_reply: 1");
  memcpy(FH(handle)->buf_out, out, bytes_out);
  TRACE("Srv_reply: 2");
  FH(handle)->bytes_out = bytes_out;

  TRACE("Srv_reply: 3");
  /* Wake waiting process */
  if(FH(handle)->queued)
  {
    TRACE("Srv_reply: 4");
    WAKE(IO_Q, (LONG)handle);
    TRACE("Srv_reply: 5");
  }
  else if(FH(handle)->selected)
  {
    TRACE("Srv_reply: 6");
    WAKESELECT(FH(handle)->selecting_proc);
    TRACE("Srv_reply: 7");
  }
  TRACE("Srv_reply: 8");
}


/*
** Handle events
*/
void
Srv_poll_events(void)
{

  if(devd == -1)
  {
    /* Open a file handle to use for communication */
    devd = Fopen(COMM_DEVICE_NAME, O_WRONLY);
    
    DEBUG3("devd = %d\n", devd);
  }

  DEBUG3("Writing 1 byte to %d", devd);
  Fwrite(devd, 1, "p");
}


void
comm_init(void)
{
  struct dev_descr dd =
  {
    &comm_device,
    0,
    0,
    0L,
    sizeof(DEVDRV),
    {0L,0L,0L}
  };

  comm_device.open = comm_open;
  comm_device.write = comm_write;
  comm_device.read = comm_read;
  comm_device.lseek = comm_lseek;
  comm_device.ioctl = comm_ioctl;
  comm_device.datime =	comm_datime;
  comm_device.close = comm_close;
  comm_device.select = comm_select;
  comm_device.unselect = comm_unselect;
  
  (LONG)kerinf = (LONG)Dcntl(DEV_INSTALL, COMM_DEVICE_NAME, (LONG)&dd);
}


void
comm_exit(void)
{
  Fdelete(COMM_DEVICE_NAME);
}
