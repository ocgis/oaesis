/* config.h.  Generated automatically by configure.  */
/* config.h.in.  Generated automatically from configure.in by autoheader.  */

/* Define if you have the ANSI C header files.  */
#define STDC_HEADERS 1

/* Define if your processor stores words with the most significant
   byte first (like Motorola and SPARC, unlike Intel and VAX).  */
#define WORDS_BIGENDIAN 1

/* Define if you have the <alloc.h> header file.  */
#define HAVE_ALLOC_H 1

/* Define if you have the <basepage.h> header file.  */
#define HAVE_BASEPAGE_H 1

/* Define if you have the <compiler.h> header file.  */
#define HAVE_COMPILER_H 1

/* Define if you have the <fcntl.h> header file.  */
#define HAVE_FCNTL_H 1

/* Define if you have the <ioctl.h> header file.  */
#define HAVE_IOCTL_H 1

/* Define if you have the <mintbind.h> header file.  */
#define HAVE_MINTBIND_H 1

/* Define if you have the <osbind.h> header file.  */
#define HAVE_OSBIND_H 1

/* Define if you have the <process.h> header file.  */
#define HAVE_PROCESS_H 1

/* Define if you have the <support.h> header file.  */
#define HAVE_SUPPORT_H 1

/* Define if you have the <sysvars.h> header file.  */
#define HAVE_SYSVARS_H 1

/* Define if you have the pthread library (-lpthread).  */
/* #undef HAVE_LIBPTHREAD */

/* Define if you have the socket library (-lsocket).  */
#define HAVE_LIBSOCKET 1

/* Define if <signal.h> contains SIGSTKFLT. */
/* #undef HAVE_SIGNAL_SIGSTKFLT */

/* Define if vdi calls are to be tunneled to the oAESis server */
/* #undef TUNNEL_VDI_CALLS */

/* Define if we're building for MiNT */
#define MINT_TARGET 1

/* Define to build launcher.prg not attached to the server */
#define LAUNCHER_AS_PRG 1

/* Missing POSIX types */
#define int8_t signed char
#define u_int8_t unsigned char
#define int16_t signed short
#define u_int16_t unsigned short
#define int32_t signed long
#define u_int32_t unsigned long

/* Version */
#define OAESIS_VERSION "0.91.0"

#define inline

#define strcasecmp stricmp
