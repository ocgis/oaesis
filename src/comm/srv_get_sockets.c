#include <stdio.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "srv_get.h"
#include "srv_sockets.h"

#define BACKLOG 256

/*
** Module type definitions
*/
typedef struct appl_handle{
  int    fd;
  struct appl_handle * next;
}APPL_HANDLE;

/*
** Module global variables
*/
static int sockfd;  /* Server socket descriptor */

static APPL_HANDLE * handles = NULL;

/* This is used when trying to wake a Srv_get call */
static int wake_fd [2];

/*
** Description
** Open the server connection
**
** 1998-09-25 CG
*/
void
Srv_open (void) {
  struct sockaddr_in my_addr;  /* Address information for server */

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("oaesis: Srv_open: socket");
    return;
  }
  
  my_addr.sin_family = AF_INET;         /* host byte order */
  my_addr.sin_port = htons(MYPORT);     /* short, network byte order */
  my_addr.sin_addr.s_addr = INADDR_ANY; /* automatically fill with my IP */
  bzero(&(my_addr.sin_zero), 8);        /* zero the rest of the struct */
  
  if (bind(sockfd,
           (struct sockaddr *)&my_addr,
           sizeof(struct sockaddr)) == -1) {
    perror("oaesis: Srv_open: bind");
    exit (-1);
  }

  if (listen(sockfd, BACKLOG) == -1) {
    perror("oaesis: Srv_open: listen");
    return;
  }

  if (pipe (wake_fd) == -1) {
    perror ("oaesis: Srv_open: pipe");
  }
}


/*
** Description
** Wait for a message from a client
**
** 1998-09-25 CG
*/
void *
Srv_get (void * in,
         int    max_bytes_in) {
  int                sin_size = sizeof (struct sockaddr_in);
  struct sockaddr_in their_addr; /* Client address information */
  fd_set             handle_set;
  APPL_HANDLE *      handle_walk;
  int                new_fd;
  int                highest_fd;

  FD_ZERO (&handle_set);
  
  /* Specify which handles to wait for */
  FD_SET (sockfd, &handle_set);
  FD_SET (wake_fd [0], &handle_set);
  highest_fd = wake_fd[0];

  handle_walk = handles;
  while (handle_walk) {
    /*
    DB_printf ("srv_get_sockets.c: Waiting for handle %d", handle_walk->fd); 
    */

    FD_SET (handle_walk->fd, &handle_set);
    if (handle_walk->fd > highest_fd) {
      highest_fd = handle_walk->fd;
    }
    handle_walk = handle_walk->next;
  }

  /*
  DB_printf ("highest_fd = %d", highest_fd);
  */

  /* Wait for input */
  select (highest_fd + 1, &handle_set, NULL, NULL, NULL);

  if (FD_ISSET (sockfd, &handle_set)) {
    /* A new application has called the server*/

    if ((new_fd = accept (sockfd, (struct sockaddr *)&their_addr, &sin_size)) == -1) {
      perror ("oaesis: Srv_get: accept");
      return NULL;
    }

    /* Allocate a new handle and put it in the list */
    handle_walk = (APPL_HANDLE *)malloc (sizeof (APPL_HANDLE));
    handle_walk->fd = new_fd;
    handle_walk->next = handles;
    handles = handle_walk;
  } else if (FD_ISSET (wake_fd [0], &handle_set)) {
    char dum;

    read (wake_fd [0], &dum, 1);

    return NULL;
  } else {
    /* An existing application has sent a command. Now find its handle */
    handle_walk = handles;

    while (handle_walk != NULL) {
      if (FD_ISSET (handle_walk->fd, &handle_set)) {
        /* The correct handle has been found */
        break;
      }

      handle_walk = handle_walk->next;
    }

    /* Something strange has happened */
    if (handle_walk == NULL) {
      return NULL;
    }
  }

  if (recv (handle_walk->fd, in, max_bytes_in, 0) == -1) {
    perror ("oaesis: Srv_get: recv");
    return NULL;
  }

  return handle_walk;
}


/*
** Description
** Wake a waiting Srv_get
**
** 1998-12-06 CG
*/
void
Srv_wake (void) {
  if (write (wake_fd [1], "o", 1) == -1) {
    perror ("oaesis: Srv_wake: write");
  }
}


/*
** Description
** Reply with a message to a client
**
** 1998-09-25 CG
*/
void
Srv_reply (void * handle,
           void * out,
           WORD   bytes_out) {
 /*
   DB_printf ("srv_get_sockets.c: Srv_reply: bytes_out=%d\n",
   bytes_out);
   */

  /*
  DB_printf ("srv_get_sockets.c: Sending reply through fd %d", ((APPL_HANDLE *)handle)->fd);
  */

  if (send (((APPL_HANDLE *)handle)->fd, out, bytes_out, 0) == -1) {
    perror ("oaesis: Srv_reply: send");
    return;
  }
}

