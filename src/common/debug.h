#ifndef __DEBUG__
#define __DEBUG__

#ifndef DEBUGLEVEL
#define DEBUGLEVEL 0
#endif

#if DEBUGLEVEL>=1
# define DEBUG1(fmt,args...) DB_printf (fmt, ##args)
#endif
#if DEBUGLEVEL>=2
# define DEBUG2(fmt,args...) DB_printf (fmt, ##args)
#endif
#if DEBUGLEVEL>=3
# define DEBUG3(fmt,args...) DB_printf (fmt, ##args)
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

void DB_printf (char * fmt, ...);

void DB_setpath (char * path);

#endif
