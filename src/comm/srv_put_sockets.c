#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include "srv_put.h"
#include "srv_sockets.h"


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
  int sockfd, numbytes;
  struct hostent * he;
  struct sockaddr_in their_addr; /* connector's address information */
  
  if ((he = gethostbyname ("localhost")) == NULL) {  /* get the host info */
    perror("oaesis: srv_put_sockets.c: Client_send_recv: gethostbyname");
    return -1;
  }
  
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("oaesis: srv_put_sockets.c: Client_send_recv: socket");
    return -1;
  }
  
  their_addr.sin_family = AF_INET;         /* host byte order */
  their_addr.sin_port = htons(MYPORT);     /* short, network byte order */
  their_addr.sin_addr = *((struct in_addr *)he->h_addr);
  bzero(&(their_addr.sin_zero), 8);        /* zero the rest of the struct */
  
  if (connect(sockfd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr))
      == -1) {
    perror("oaesis: srv_put_sockets.c: Client_send_recv: connect");
    return -1;
  }
  
  if ((numbytes = send (sockfd, in, bytes_in, 0)) == -1) {
    perror("send");
    return -1;
  }

  fprintf (stderr, "oaesis: srv_put_sockets.c: max_bytes_out=%d\n",
           max_bytes_out);
  if ((numbytes = recv (sockfd, out, max_bytes_out, 0)) == -1) {
    perror("oaesis: srv_put_sockets.c: Client_send_recv: recv");
    return -1;
  }
  
  close(sockfd);
  
  return numbytes;
}


#define MAXDATASIZE 100

/*
** Description
** Put a message to the server and wait for a reply
**
** 1998-09-25 CG
*/
LONG
Srv_put (WORD   apid,
         WORD   call,
         void * spec) {
  int sockfd, numbytes;  
  char buf[MAXDATASIZE];
  struct hostent * he;
  struct sockaddr_in their_addr; /* connector's address information */
  
  if ((he = gethostbyname ("localhost")) == NULL) {  /* get the host info */
    perror("gethostbyname");
    exit(1);
  }
  
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("socket");
    exit(1);
  }
  
  their_addr.sin_family = AF_INET;         /* host byte order */
  their_addr.sin_port = htons(MYPORT);     /* short, network byte order */
  their_addr.sin_addr = *((struct in_addr *)he->h_addr);
  bzero(&(their_addr.sin_zero), 8);        /* zero the rest of the struct */
  
  if (connect(sockfd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr))
      == -1) {
    perror("connect");
    exit(1);
  }
  
  if ((numbytes = send (sockfd, buf, MAXDATASIZE, 0)) == -1) {
    perror("send");
    exit(1);
  }
  
  if ((numbytes = recv (sockfd, buf, MAXDATASIZE, 0)) == -1) {
    perror("recv");
    exit(1);
  }
  
  buf[numbytes] = '\0';
  
  printf("Received: %s",buf);
  
  close(sockfd);
  
  return 0;
}
