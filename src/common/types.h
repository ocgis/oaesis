#ifndef __TYPES__
#define __TYPES__

#ifdef __GNUC__
#define PACKED __attribute__ ((packed))
#else
#define PACKED
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_MINTBIND_H
#include <mintbind.h> /* for XATTR */
#endif

#include <fcntl.h>
#include <sys/types.h>

#include "aesbind.h" /* FIXME */


/* Use POSIX types */
#ifndef BYTE
#define BYTE  int8_t
#define UBYTE u_int8_t
#endif /* BYTE */
#ifndef WORD
#define WORD  int16_t
#define UWORD u_int16_t
#endif /* WORD */
#ifndef LONG
#define LONG  int32_t
#define ULONG u_int32_t
#endif /* LONG */

#ifdef  PUREC
#define CDECL cdecl
#else
#define CDECL
#endif

#define NOT_USED(c) (void)c

typedef struct {
  WORD	type;
  WORD	sid;
  WORD	length;
  WORD	msg0;
  WORD	msg1;
  WORD	msg2;
  WORD	msg3;
  WORD	msg4;
}COMMSG;

/* Cookie structure */

typedef struct {
  LONG cookie;
  LONG value;
}COOKIE;

/* VDI Memory Form Definition Block */

#if 0
/* FIXME: check type in ovdisis and remove from here */
typedef struct {
  void            *fd_addr;   /* Addrerss of upper left corner of first*/
  /* plane of raster area. If NULL then    */
  /* MFDB is for a physical device         */
  WORD            fd_w;         /* Form Width in Pixels                  */
  WORD            fd_h;       /* Form Height in Pixels                 */
  WORD            fd_wdwidth; /* Form Width in shorts(fd_w/sizeof(int) */
  WORD            fd_stand;   /* Form format 0= device spec 1=standard */
  WORD            fd_nplanes; /* Number of memory planes               */
  WORD            fd_r1;      /* Reserved                              */
  WORD            fd_r2;      /* Reserved                              */
  WORD            fd_r3;      /* Reserved                              */
} MFDB;
#endif

typedef struct mouse_event_type {
  int     *x;
  int *y;
  int     *b;
  int     *k;
} Mouse;

#if 0
/* FIXME: check type in ovdisis and remove from here */
typedef struct vdi_rectangle {
  int v_x1;
  int v_y1;
  int v_x2;
  int v_y2;
} VRECT;
#endif

typedef struct orect {
  struct orect  *o_link;
  int     o_x;
  int     o_y;
  int     o_w;
  int     o_h;
} ORECT;


typedef struct {
  LONG    msg1;
  LONG    msg2;
  WORD    pid;
}MSG;

typedef struct {
  WORD    x;
  WORD    y;
  WORD    width;
  WORD    height;
}RECT;

typedef enum {
  FALSE = 0,
  TRUE  = 1
}BOOLEAN;

#ifdef MINT_TARGET
typedef struct  {
  UWORD mode;
  LONG  index;
  UWORD dev;
  UWORD reserved1;
  UWORD nlink;
  UWORD uid;
  UWORD gid;
  LONG  size;
  LONG  blksize;
  LONG  nblocks;
  WORD  mtime;
  WORD  mdate;
  WORD  atime;
  WORD  adate;
  WORD  ctime;
  WORD  cdate;
  WORD  attr;
  WORD  reserved2;
  LONG  reserved3;
  LONG  reserved4;
}XATTR;
#endif

struct filesys;         /* forward declaration */
struct devdrv;          /* ditto */

typedef struct f_cookie {
  struct filesys *fs; /* filesystem that knows about this cookie */
  UWORD dev;          /* device info (e.g. Rwabs device number) */
  UWORD aux;          /* extra data that the file system may want */
  LONG  index;        /* this+dev uniquely identifies a file */
} fcookie;

typedef struct fileptr {
  WORD  links;          /* number of copies of this descriptor */
  UWORD flags;          /* file open mode and other file flags */
  LONG  pos;            /* position in file */
  LONG  devinfo;        /* device driver specific info */
  fcookie fc;           /* file system cookie for this file */
  struct devdrv *dev;   /* device driver that knows how to deal with this */
  struct fileptr *next; /* link to next fileptr for this file */
} FILEPTR;


#define TOS_SEARCH      0x01

/* structure for opendir/readdir/closedir */
typedef struct dirstruct {
  fcookie fc;             /* cookie for this directory */
  UWORD   index;          /* index of the current entry */
  UWORD   flags;          /* flags (e.g. tos or not) */
  BYTE    fsstuff[60];    /* anything else the file system wants */
  /* NOTE: this must be at least 45 bytes */
  struct dirstruct *next; /* linked together so we can close them
                             on process termination */
} MINTDIR;

typedef struct devdrv {
  LONG CDECL (*open)(FILEPTR *f);
  LONG CDECL (*write)(FILEPTR *f, const BYTE *buf, LONG bytes);
  LONG CDECL (*read)(FILEPTR *f, BYTE *buf, LONG bytes);
  LONG CDECL (*lseek)(FILEPTR *f, LONG where, WORD whence);
  LONG CDECL (*ioctl)(FILEPTR *f, WORD mode, void *buf);
  LONG CDECL (*datime)(FILEPTR *f, WORD *timeptr, WORD rwflag);
  LONG CDECL (*close)(FILEPTR *f, WORD pid);
  LONG CDECL (*select)(FILEPTR *f, LONG proc, WORD mode);
  void CDECL (*unselect)(FILEPTR *f, LONG proc, WORD mode);

  /* extensions, check dev_descr.drvsize (size of DEVDRV struct) before calling:
   * fast RAW tty byte io  */

  LONG CDECL (*writeb)(FILEPTR *f, const BYTE *buf, LONG bytes);
  LONG CDECL (*readb)(FILEPTR *f, BYTE *buf, LONG bytes);

  /* what about: scatter/gather io for DMA devices...
   *      LONG CDECL (*writev)    P_((FILEPTR *f, const struct iovec *iov, LONG cnt));
   *      LONG CDECL (*readv)     P_((FILEPTR *f, const struct iovec *iov, LONG cnt));
   */

} DEVDRV;


#define FS_KNOPARSE      0x01 /* kernel shouldn't do parsing */
#define FS_CASESENSITIVE 0x02 /* file names are case sensitive */
#define FS_NOXBIT        0x04 /* if a file can be read, it can be executed */
#define FS_LONGPATH      0x08 /* file system understands "size" argument to
                                 "getname" */

typedef struct filesys {
  struct filesys  *next;  /* link to next file system on chain */
  LONG   fsflags;
  LONG   CDECL (*root)(WORD drv,fcookie *fc);
  LONG   CDECL (*lookup)(fcookie *dir, const BYTE *name, fcookie *fc);
  LONG   CDECL (*creat)(fcookie *dir, const BYTE *name, UWORD mode,
                        WORD attrib, fcookie *fc);
  DEVDRV * CDECL (*getdev)(fcookie *fc, LONG *devspecial);
  LONG    CDECL (*getxattr)(fcookie *file, XATTR *xattr);
  LONG    CDECL (*chattr)(fcookie *file, WORD attr);
  LONG    CDECL (*chown)(fcookie *file, WORD uid, WORD gid);
  LONG    CDECL (*chmode)(fcookie *file, WORD mode);
  LONG    CDECL (*mkdir)(fcookie *dir, const BYTE *name, UWORD mode);
  LONG    CDECL (*rmdir)(fcookie *dir, const BYTE *name);
  LONG    CDECL (*remove)(fcookie *dir, const BYTE *name);
  LONG    CDECL (*getname)(fcookie *relto, fcookie *dir,
                           BYTE *pathname, WORD size);
  LONG    CDECL (*rename)(fcookie *olddir, BYTE *oldname,
                          fcookie *newdir, const BYTE *newname);
  LONG    CDECL (*opendir)(MINTDIR *dirh, WORD tosflag);
  LONG    CDECL (*readdir)(MINTDIR *dirh, BYTE *name, WORD namelen, fcookie *fc);
  LONG    CDECL (*rewinddir)(MINTDIR *dirh);
  LONG    CDECL (*closedir)(MINTDIR *dirh);
  LONG    CDECL (*pathconf)(fcookie *dir, WORD which);
  LONG    CDECL (*dfree)(fcookie *dir, LONG *buf);
  LONG    CDECL (*writelabel)(fcookie *dir, const BYTE *name);
  LONG    CDECL (*readlabel)(fcookie *dir, BYTE *name, WORD namelen);
  LONG    CDECL (*symlink)(fcookie *dir, const BYTE *name, const BYTE *to);
  LONG    CDECL (*readlink)(fcookie *dir, BYTE *buf, WORD len);
  LONG    CDECL (*hardlink)(fcookie *fromdir, const BYTE *fromname,
                            fcookie *todir, const BYTE *toname);
  LONG    CDECL (*fscntl)(fcookie *dir, const BYTE *name, WORD cmd, LONG arg);
  LONG    CDECL (*dskchng)(WORD drv);
  LONG    CDECL (*release)(fcookie *);
  LONG    CDECL (*dupcookie)(fcookie *new, fcookie *old);
} FILESYS;


/* structure for internal kernel locks */
typedef struct ilock {
  struct flock l;         /* the actual lock */
  struct ilock *next;     /* next lock in the list */
  LONG    reserved[4];    /* reserved for future expansion */
} LOCK;

/* different process queues */

#define CURPROC_Q 0
#define READY_Q   1
#define WAIT_Q    2
#define IO_Q      3
#define ZOMBIE_Q  4
#define TSR_Q     5
#define STOP_Q    6
#define SELECT_Q  7

#define NUM_QUEUES      8

typedef LONG CDECL (*Func)();

struct kerinfo {
  WORD  maj_version;      /* kernel version number */
  WORD  min_version;      /* minor kernel version number */
  UWORD default_perm;     /* default file permissions */
  WORD  reserved1;        /* room for expansion */

  /* OS functions */
  Func    *bios_tab;      /* pointer to the BIOS entry points */
  Func    *dos_tab;       /* pointer to the GEMDOS entry points */

  /* media change vector */
  void    CDECL (*drvchng)(UWORD dev);

  /* Debugging stuff */
  void    CDECL (*trace)(const BYTE *, ...);
  void    CDECL (*debug)(const BYTE *, ...);
  void    CDECL (*alert)(const BYTE *, ...);
  void CDECL (*fatal)(const BYTE *, ...);

  /* memory allocation functions */
  void *  CDECL (*kmalloc)(LONG);
  void    CDECL (*kfree)(void *);
  void *  CDECL (*umalloc)(LONG);
  void    CDECL (*ufree)(void *);

  /* utility functions for string manipulation */
  WORD    CDECL (*strnicmp)(const BYTE *, const BYTE *, WORD);
  WORD    CDECL (*stricmp)(const BYTE *, const BYTE *);
  BYTE *  CDECL (*strlwr)(BYTE *);
  BYTE *  CDECL (*strupr)(BYTE *);
  WORD    CDECL (*sprintf)(BYTE *, const BYTE *, ...);

  /* utility functions for manipulating time */
  void    CDECL (*millis_time)(ULONG ms, WORD *td);
  LONG    CDECL (*unixtim)(UWORD time, UWORD date);
  LONG    CDECL (*dostim)(LONG unixtime);

  /* utility functions for dealing with pauses, or for putting processes
   * to sleep
   */
  void    CDECL (*nap)(UWORD n);
  WORD    CDECL (*sleep)(WORD que, LONG cond);
  void    CDECL (*wake)(WORD que, LONG cond);
  void    CDECL (*wakeselect)(LONG param);

  /* file system utility functions */
  WORD    CDECL (*denyshare)(FILEPTR *, FILEPTR *);
  LOCK *  CDECL (*denylock)(LOCK *, LOCK *);

  /* reserved for future use */
  LONG    res2[9];
};

#define DEV_INSTALL 0xde02

struct dev_descr {
  DEVDRV *driver;
  WORD   dinfo;
  WORD   flags;
  void   *tty;
  LONG   drvsize;        /* size of DEVDRV struct */
  LONG   reserved[3];
};


#define MAX_MSG 100

typedef struct global_array {
  WORD    version;
  WORD    numapps;
  WORD    apid;
  LONG    appglobal;
  OBJECT  **rscfile;
  RSHDR   *rshdr;
  WORD    resvd1;
  WORD    resvd2;
  void *  int_info;
  WORD    maxchar;
  WORD    minchar;
} PACKED GLOBAL_ARRAY;

typedef struct aes_pb {
  WORD         *control;
  GLOBAL_ARRAY *global;
  WORD         *int_in;
  WORD         *int_out;
  LONG         *addr_in;
  LONG         *addr_out;
}AES_PB;

typedef struct rlist {
  RECT  r;
  
  struct rlist *next;
}RLIST;

typedef struct winstruct {
  WORD   id;       /*window id*/
  
  WORD   status;   /*window status*/
  WORD   elements; /*window elements*/
  
  OBJECT *tree; /*object tree of the window elements*/
  
  RECT   worksize; /*current worksize*/
  RECT   totsize;  /*current total size*/
  RECT   lastsize; /*previous total size*/
  RECT   maxsize;  /*maximal total size*/
  RECT   origsize; /*original, uniconified, size*/
  
  WORD hslidepos;     /*position of the horizontal slider*/
  WORD vslidepos;     /*position of the vertical slider*/
  WORD hslidesize;    /*size of the horizontal slider*/
  WORD vslidesize;   /*size of the vertical slider*/
  
  RLIST *rlist;         /*rectangle list of the window*/
  RLIST *rpos;          /*pointer to help wind_get to traverse the
                          rectangle list*/
  
  WORD  owner;          /*application id of the owner*/
  
  OBJC_COLORWORD top_colour[20];
  OBJC_COLORWORD untop_colour[20];
  WORD           own_colour;
}WINSTRUCT;

typedef struct winlist
{
  WINSTRUCT       *win;

  struct winlist  *next;
}WINLIST;

typedef struct
{
  WORD events;
  WORD bclicks;
  WORD bmask;
  WORD bstate;
  WORD m1flag;
  RECT m1r;
  WORD m2flag;
  RECT m2r;
  WORD locount;
  WORD hicount;
  RECT menu_bar;
}EVENTIN;

typedef struct
{
  WORD	events;
  WORD	mx;
  WORD	my;
  WORD	mb;
  WORD	ks;
  WORD	kc;
  WORD	mc;
}EVENTOUT;
#endif
