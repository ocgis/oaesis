/*
** srv_put_sockets.c
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
Client_open (void) {
  struct hostent * he;
  struct sockaddr_in their_addr; /* connector's address information */

  if(level == 0)
  {
    if ((he = gethostbyname ("localhost")) == NULL) {  /* get the host info */
      perror("oaesis: srv_put_sockets.c: Client_open: gethostbyname");
      exit (-1);
      return -1;
    }
    
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
      perror("oaesis: srv_put_sockets.c: Client_open: socket");
      return -1;
    }
    
    their_addr.sin_family = AF_INET;         /* host byte order */
    their_addr.sin_port = htons(MYPORT);     /* short, network byte order */
    their_addr.sin_addr = *((struct in_addr *)he->h_addr);
    bzero(&(their_addr.sin_zero), 8);        /* zero the rest of the struct */
    
    if (connect(sockfd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr))
        == -1) {
      perror("oaesis: srv_put_sockets.c: Client_open: connect");
      exit (-1);
      return -1;
    }
  }

  level++;

  return 0;
}

/*
** Description
** Put a message to the server and wait for a reply
**
** 1998-09-26 CG
*/
LONG
Client_send_recv (void * in,
                  int    bytes_in,
                  void * out,
                  int    max_bytes_out) {
  int numbytes;
  /*  static int count = 0;*/
  /*
  DB_printf ("srv_put_sockets.c: Sending through socket: %d", sockfd);
  */

  /*
  DB_printf ("srv_put_sockets.c: bytes_in = %d %d",
             bytes_in, count++);
  */

  if ((numbytes = send (sockfd, in, bytes_in, 0)) == -1)
  {
    perror("send");
    exit(-1);
  }

  DEBUG3("srv_put_sockets.c: sent numbytes = %d", numbytes);
  
  if ((numbytes = recv (sockfd, out, max_bytes_out, 0)) == -1) {
    perror("oaesis: srv_put_sockets.c: Client_send_recv: recv");
    exit(-1);
  }

  /*
  DB_printf ("srv_put_sockets.c: received numbytes = %d",
             numbytes);
  */

  return numbytes;
}
