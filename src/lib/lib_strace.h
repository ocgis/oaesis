/*
** oAESis - AES for Un*x
**
** lib_strace.h
**
** Copyright 2001 Christer Gustavsson <cg@nocrew.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**  
** Read the file COPYING for more information.
*/

void
aes_strace_begin(const char * call_name,
                 const char * call_format,
                 const char * argp);

void
aes_strace_end(const char * call_name,
               const char * rv_format,
               const char * argp);
