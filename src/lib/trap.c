/*
** trap.c
**
** Copyright 1998 Christer Gustavsson <cg@nocrew.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**  
** Read the file COPYING for more information.
**
*/


/*
** Exported
**
** 1999-08-01 CG
*/
void
aes_call (void * aespb) {
  __asm__ volatile
  ("  movl %0,    d1\n"
   "  movw #0xc8, d0\n"
   "  trap #2\n"
   :                                     /* No output */
   : "g"(aespb)                          /* Input */
   : "d0", "d1", "d2", "a0", "a1", "a2"  /* Clobbered registers */
   );
}
