/* Fopen() modes */

#define FO_READ  0
#define FO_WRITE 1
#define FO_RW    2

/* defines for Mxalloc() */

#define	PRIVATEMEM 0x0018
#define	GLOBALMEM  0x4028
#define	SUPERMEM   0x0038
#define	READMEM    0x0048

/* Pmsg() modes */

#define MSG_READ      0
#define MSG_WRITE     1
#define MSG_READWRITE 2
#define MSG_NOWAIT    0x8000

/* Psemaphore() modes */

#define SEM_CREATE  0
#define SEM_DESTROY 1
#define SEM_LOCK    2
#define SEM_UNLOCK  3
