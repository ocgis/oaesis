#include <stdio.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "srv_get.h"
#include "srv_sockets.h"

#define BACKLOG 10

/*
** Module global variables
*/
static int sockfd;  /* Server socket descriptor */

static int new_fd;  /* Client socket descriptor */


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
    return;
  }

  if (listen(sockfd, BACKLOG) == -1) {
    perror("oaesis: Srv_opem: listen");
    return;
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
  int sin_size = sizeof (struct sockaddr_in);
  struct sockaddr_in their_addr; /* Client address information */

  if ((new_fd = accept (sockfd, (struct sockaddr *)&their_addr, &sin_size)) == -1) {
    perror ("oaesis: Srv_get: accept");
    return NULL;
  }

  if (recv (new_fd, in, max_bytes_in, 0) == -1) {
    perror ("oaesis: Srv_get: recv");
    return NULL;
  }

  return NULL;
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

  if (send (new_fd, out, bytes_out, 0) == -1) {
    perror ("oaesis: Srv_reply: send");
    return;
  }

  close (new_fd);
}

