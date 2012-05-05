/*
** oAESis - AES for Un*x
**
** lib_strace.c
**
** Copyright (C) 1991, 1992  Linus Torvalds, Lars Wirzenius
** Copyright 1996 Roman Hodek <Roman.Hodek@informatik.uni-erlangen.de>
** Copyright 2001 Christer Gustavsson <cg@nocrew.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**  
** Read the file COPYING for more information.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "lib_global.h"
#include "lib_strace.h"

/*
 * This global variable is set whenever the cursor is in the middle of the
 * output line, after the "Function( ... ) " part of the strace output. It is
 * used by DDEBUG and handle_trace() to print an extra newline (for output
 * sanity :-), and by strace_end() to restart the log line.
 */
int mid_of_line = 0;


/*
** Structures for printing AES structures
*/

typedef struct
{
  unsigned offset;
  char     *name;
  char     *format;
} COMPONENT;

typedef struct
{
  unsigned value;
  char *   name;
} LITERAL;

typedef enum
{
  VOID,
  STRUCT,
  ENUM
} TYPE;

typedef struct
{
  TYPE        type;
  char *      name;
  unsigned    n_components;
  COMPONENT * components;
  LITERAL *   literals;
} AESSTRUCT;

COMPONENT MENU_components[] =
{
  { 0, "mn_tree", "%08lx" },
  { 4, "mn_menu", "%hd" },
  { 6, "mn_item", "%hd" },
  { 8, "mn_scroll", "%hd" },
  { 10, "mn_keystate", "%hd" }
};

LITERAL MU_EVENT_literals[] =
{
  { 0x01, "MU_KEYBD" },
  { 0x02, "MU_BUTTON" },
  { 0x04, "MU_M1" },
  { 0x08, "MU_M2" },
  { 0x10, "MU_MESAG" },
  { 0x20, "MU_TIMER" }
};

#define	DECLARE_STRUCT(name) \
    { STRUCT, #name, sizeof(name##_components)/sizeof(COMPONENT), name##_components, NULL }
#define	DECLARE_ENUM(name) \
    { ENUM, #name, sizeof(name##_literals)/sizeof(LITERAL), NULL, name##_literals }
	
AESSTRUCT aes_structures[] =
{
  DECLARE_STRUCT(MENU),
  DECLARE_ENUM(MU_EVENT),
  { VOID, NULL, 0, NULL }
};



/***************************** Prototypes *****************************/

static int aes_vfprintf(FILE *        f,
                        const char *  fmt,
                        const void ** oargp,
                        long *        extra_arg,
                        int           ffs);
static void aes_single_arg(const char * format,
                           void **      oargpp);
static AESSTRUCT * aes_find_struct( const char *name );
static void aes_print_retval(const char *fmt, const char *argp);

/************************* End of Prototypes **************************/



/*
 * Start strace log line with function name and arguments; cursor stops after
 * ')', to be continued by strace_end().
 */
void
aes_strace_begin(const char * call_name,
                 const char * call_format,
                 const char * argp)
{
  fprintf(stderr, "%s(", call_name);
  aes_vfprintf( stderr, call_format, argp, NULL, 0 );
  fprintf( stderr, ")" );
  
  mid_of_line = 1;
}

/*
 * Finish an strace log line with the syscall's return value. If some other
 * output happened in the time between, reprint some information.
 */
void
aes_strace_end(const char * call_name,
               const char * rv_format,
               const char * argp)
{
  /* if some other output intervened, reprint the function name */
  if (!mid_of_line)
    fprintf( stderr, "%s(...)", call_name );
  
  fprintf( stderr, " = " );
  aes_print_retval(rv_format, argp);
  fprintf( stderr, "\n" );
  
  mid_of_line = 0;
}
	

/*
 * This function does more or less the same as vfprintf(), but it treats short
 * arguments ('%...h.') really as 16-bit, since this is widely used in TOS.
 * And the standard libc function always assume shorts are promoted to int,
 * which is not the case for arguments passed from TOS programs.
 */
static
int
aes_vfprintf(FILE *        f,
             const char *  fmt,
             const void ** oargp,
             long *        extra_arg,
             int           ffs)
{
  unsigned long new_args[40], *nargp;		/* 40 should be enough... */
  const char *  p, *q, *start;
  char          sfmt[64];
  int           cnt = 0;
  void *        args[6];
  int           index;
  int           arg_index;

  for(index = 0; index < 6; index++)
  {
    args[index] = oargp[index];
  }
  arg_index = 0;

  nargp = new_args;
  
  /* Parse the format to know about number and type of arguments */
  for( p = fmt; *p; ++p )
  {
    if (*p != '%') {
      fprintf(stderr, "%c", *p);
      continue;
    }
    if (!*++p) break;
    
    start = p-1;
    ++cnt;
    
    /* check for structure */
    if(strchr(p, '{') && !ffs)
    {
      AESSTRUCT *  struc;
      COMPONENT *  comp;
      const char * struct_p;
      const void * carg;
      int          i;
      int          index;
      char *       end;

      index = strtoul(p, &end, 10);
      if(*end == 'P')
      {
        arg_index = index;
        p = end + 1;
      }

      /* FIXME: Check for '{' */
      ++p;
      if (!(q = strchr( p, '}')))
      {
        fprintf( stderr, "Internal error: unterminated %%{ in "
                 "format spec\n" );
        return -1;
      }
      strncpy( sfmt, p, q-p ); sfmt[q-p] = 0;
      
      if (cnt == 1 && extra_arg)
        struct_p = *(const char **)extra_arg, extra_arg = NULL;
      else
      {
        struct_p = *((const char **)oargp);
        oargp++;
      }
      
      struct_p = (const char *)(ULONG)struct_p;
      
      if ((struc = aes_find_struct( sfmt )))
      {
        if(struc->type == STRUCT)
        {
          fprintf(stderr, "{");
          for( i = 0, comp = struc->components; i < struc->n_components;
               ++i, ++comp ) {
            fprintf(stderr, "%s=", comp->name);
            
            carg = (const void *)(struct_p + comp->offset);
            aes_single_arg( comp->format, &carg);
            
            if (i != struc->n_components-1)
            {
              fprintf(stderr, " ");
            }
          }
          fprintf(stderr, "}");
        }
        else if(struc->type == ENUM)
        {
          LITERAL *      lit;
          unsigned short value;
          int            first = 1;

          value = *((unsigned short *)args[arg_index]);
          args[arg_index]++;

          fprintf(stderr, "0x%x", value);
          for(i = 0, lit = struc->literals;
              i < struc->n_components;
              ++i, ++lit)
          {
            if(value & lit->value)
            {
              if(first)
              {
                fprintf(stderr, " (");
                first = 0;
              }
              else
              {
                fprintf(stderr, " | ");
              }
              fprintf(stderr, "%s", lit->name);
              value &= ~lit->value;
            }
          }

          if(!first)
          {
            if(value != 0)
            {
              fprintf(stderr, " | 0x%x)", value);
            }
            else
            {
              fprintf(stderr, ")");
            }
          }
        }
        else
        {
          fprintf(stderr, "Unknown type in format %s\n", fmt);
        }
      }
      else
        fprintf( stderr, "Internal error: Cannot print struct %s\n",
                 sfmt );
      p = q;
    }
    else
    {
      p += strspn( p, "-0123456789lhP" );
      if (!strchr("duxospDTS", *p))
      {
        fprintf( stderr, "Internal error: bad char '%c' in "
                 "format spec %s\n", *p, p);
        return -1;
      }
      strncpy( sfmt, start, p+1-start );
      sfmt[p+1-start] = 0;
      
      if (cnt == 1 && extra_arg) {
        /* special case for translated errno's: */
        if(ffs)
        {
          fprintf(stderr,
                  "%s",
                  (char *)*(ULONG *)extra_arg);
        }
        else
        {
          aes_single_arg(sfmt, (const void **)&extra_arg);
        }
        
        extra_arg = NULL;
      }
      else
      {
        char * real_fmt;
        int    index;

        index = strtoul(&sfmt[1], &real_fmt, 10);
        if(*real_fmt == 'P')
        {
          arg_index = index;
          *real_fmt = '%';
        }
        else
        {
          real_fmt = sfmt;
        }
        aes_single_arg(real_fmt, &args[arg_index]);
      }
    }
  }
  
  return 0;
}

/*
 * Extensions to standard printf() format specs:
 *
 *  - modifier 'q' can be used for 8-bit arguments (not possible on the
 *    stack, but in structures)
 *
 *  - format code 'S' means "skip", i.e. skip argument of width determined by
 *    modifiers, but print nothing
 *
 *  - format code 'D' stands for TOS style date (16-bit)
 *  
 *  - format code 'T' stands for TOS style time (16-bit)
 *
 *  - format code 'x' automatically prints "0x" prefix
 *  
 *  - format code 's' automatically frames string in '"'
 *
 *  - format code 'z' is like 's', but the argument isn't a pointer to the
 *    string, but the string is at the argument position itself (for
 *    structures containing char arrays)
 */

static
void
aes_single_arg(const char *  format,
               void **       oargpp)
{
  char * fmt;
  int    is_short = 0, is_quart = 0, is_signed = 0, is_date = 0, is_time = 0;
  int    is_direct_str = 0, do_skip = 0;
  char * prefix = NULL, *postfix = NULL;
  
  fmt = (char *)format;
  
  fmt++; /* Skip '%' */
  for( ; *fmt; ++fmt ) {
    if (isdigit(*fmt) || *fmt == '-')
      ;
    else if (*fmt == 'l')
      is_short = 0;
    else if (*fmt == 'h')
      is_short = 1;
    else if (*fmt == 'q') {
      is_quart = 1;
      continue; /* don't copy 'q' into nfmt */
    }
    else if (*fmt == 'D') {
      is_date = 1;
      break;
    }
    else if (*fmt == 'T') {
      is_time = 1;
      break;
    }
    else if (*fmt == 'S') {
      do_skip = 1;
      break;
    }
    else if (*fmt == 'z')
    {
      is_direct_str = 1;
      break;
    }
    else if (strchr( "duxosp", *fmt )) {
      is_signed = (*fmt == 'd');
      if (*fmt == 'x') prefix = "0x";
      if (*fmt == 's') prefix = postfix = "\"";
      break;
    }
    else
    {
      fprintf(stderr, "Internal error: bad char '%c' in format spec %s\n",
              *fmt, fmt);
      break;
    }
  }
  
  if(prefix != NULL)
  {
    fprintf(stderr, prefix);
  }
  
  if (is_date)
  {
    unsigned date;
    
    date = *((unsigned short *)(*oargpp));
    (*oargpp)++;
    
    fprintf(stderr, "%02d.%02d.%d",
            date & 0x1f, (date >> 5) & 0x0f,
            ((date >> 9) & 0x7f) + 1980 );
  }
  else if (is_time)
  {
    unsigned time;
    
    time = CW_TO_HW(*((unsigned short *)(*oargpp)));
    (*oargpp)++;
    
    fprintf(stderr, "%02d:%02d:%02d",
            (time >> 11) & 0x1f, (time >> 5) & 0x3f,
            (time & 0x1f) * 2 );
  }
  else if (do_skip) {
    /* new format empty -> print nothing */
    *oargpp += is_quart ? 1 : is_short ? 2 : 4;
  }
  else if (is_direct_str) {
    prefix = postfix = "\"";
    
    fprintf(stderr, "%s,", (char *)*oargpp++);
  }
  else
  {
    /* get the argument with correct width, do correct sign extend and
     * push it onto new_args array */
    if (is_quart)
    {
      if (is_signed)
        fprintf(stderr, "%c,", *((signed char *)*oargpp));

      else
        fprintf(stderr, "%c,", *((unsigned char *)*oargpp));

      (*oargpp)++;
    }
    
    if (is_short)
    {
      if (is_signed)
        fprintf(stderr, format, *((signed short *)(*oargpp)));
      else
        fprintf(stderr, format, *((unsigned short *)(*oargpp)));

      (*oargpp)++;
    }
    else
    {
      if (is_signed)
        fprintf(stderr,
                format,
                (long)*((signed long *)(*oargpp)));
      else
        fprintf(stderr, format, *((unsigned long *)(*oargpp)));

      (*oargpp)++;
    }
  }
  
  if(postfix != NULL)
  {
    fprintf(stderr, postfix);
  }
}


static
AESSTRUCT *
aes_find_struct(const char * name)
{
  AESSTRUCT *p;
  
  for(p = aes_structures; p->name; ++p)
  {
    if (0 == strcmp( p->name, name ))
    {
      break;
    }
  }
  
  return( p->name ? p : NULL );
}


/*
 * Special features for the return value format string:
 *
 *  - unless it starts with '#', small negative numbers are interpreted as TOS
 *    errno's, if that makes sense
 * 
 *  - if the first character is '@', only -1 is not interpreted as errno
 *
 */

static
void
aes_print_retval(const char * fmt,
                 const char * argp)
{
  aes_vfprintf(stderr, fmt, argp, NULL, 0);
}
