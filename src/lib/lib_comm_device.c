/*
** lib_comm_device.c
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

#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include "debug.h"
#include "srv_put.h"
#include "srv_sockets.h"

static int level = 0;
static int sockfd;

LONG
Client_open (void)
{
  DEBUG1("In Client_open");
  if(level == 0)
  {
    sockfd = Fopen("u:/dev/oaes_ker", O_RDWR);
    fprintf(stderr, "sockfd = %d\n", sockfd);
  }

  fprintf(stderr, "level = %d\n", level);

  level++;

  return 0;
}

/*
** Description
** Put a message to the server and wait for a reply
*/
LONG
Client_send_recv (void * in,
                  int    bytes_in,
                  void * out,
                  int    max_bytes_out) {
  int numbytes;

  if ((numbytes = Fwrite(sockfd, bytes_in, in)) < 0)
  {
    DEBUG1("Couldn't write to oaes_ker");
    exit(-1);
  }

  DEBUG3("srv_put_sockets.c: sent numbytes = %d", numbytes);
  
  if ((numbytes = Fread(sockfd, max_bytes_out, out)) < 0)
  {
    DEBUG1("Couldn't read from oaes_ker");
    exit(-1);
  }

  return numbytes;
}
