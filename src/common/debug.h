#ifndef __DEBUG__
#define __DEBUG__

#ifndef DEBUGLEVEL
#define DEBUGLEVEL 0
#endif

#ifdef __GNUC__
# define DEBUG0(fmt,args...) ndebug(fmt, ##args)
#if DEBUGLEVEL>=1
# define DEBUG1(fmt,args...) ndebug(fmt, ##args)
#endif
#if DEBUGLEVEL>=2
# define DEBUG2(fmt,args...) ndebug(fmt, ##args)
#endif
#if DEBUGLEVEL>=3
# define DEBUG3(fmt,args...) ndebug(fmt, ##args)
#endif

#ifndef DEBUG1
# define DEBUG1(fmt,args...)
#endif
#ifndef DEBUG2
# define DEBUG2(fmt,args...)
#endif
#ifndef DEBUG3
# define DEBUG3(fmt,args...)
#endif

#else /* Not GCC */

#if DEBUGLEVEL>=1
# define DEBUG1 ndebug
#endif
#if DEBUGLEVEL>=2
# define DEBUG2 ndebug
#endif
#if DEBUGLEVEL>=3
# define DEBUG3 ndebug
#endif

#ifndef DEBUG1
# define DEBUG1 debug_dummy
#endif
#ifndef DEBUG2
# define DEBUG2 debug_dummy
#endif
#ifndef DEBUG3
# define DEBUG3 debug_dummy
#endif

void debug_dummy(char *fmt, ...);

#endif /* __GNUC__ */

void ndebug(char * fmt, ...);

void DB_setpath (char * path);

#endif
