/*
** srv_kdebug.h
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

#ifndef __SRV_KDEBUG_H__
#define __SRV_KDEBUG_H__

#ifndef DEBUGLEVEL
#define DEBUGLEVEL 0
#endif

#ifdef __GNUC__
# define KDEBUG0(fmt,args...) kdebug(fmt, ##args)
#if DEBUGLEVEL>=1
# define KDEBUG1(fmt,args...) kdebug(fmt, ##args)
#endif
#if DEBUGLEVEL>=2
# define KDEBUG2(fmt,args...) kdebug(fmt, ##args)
#endif
#if DEBUGLEVEL>=3
# define KDEBUG3(fmt,args...) kdebug(fmt, ##args)
#endif

#ifndef DEBUG1
# define KDEBUG1(fmt,args...)
#endif
#ifndef DEBUG2
# define KDEBUG2(fmt,args...)
#endif
#ifndef DEBUG3
# define KDEBUG3(fmt,args...)
#endif

#else /* Not GCC */

#if DEBUGLEVEL>=1
# define KDEBUG1 kdebug
#endif
#if DEBUGLEVEL>=2
# define KDEBUG2 kdebug
#endif
#if DEBUGLEVEL>=3
# define KDEBUG3 kdebug
#endif

#ifndef DEBUG1
# define KDEBUG1 debug_dummy
#endif
#ifndef DEBUG2
# define KDEBUG2 debug_dummy
#endif
#ifndef DEBUG3
# define KDEBUG3 debug_dummy
#endif

void debug_dummy(char *fmt, ...);

#endif /* __GNUC__ */

void kdebug(char * fmt, ...);

#endif
