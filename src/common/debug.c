#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_MINTBIND_H
#include <mintbind.h>
#endif

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "debug.h"

#define DEBUGPATH "u:\\dev\\null"

static char debugpath[128] = DEBUGPATH;

static
void
DEBUG (char * s) {
  FILE	*debug;
  _DTA  newdta,*olddta;
  char  searchpat[128];
  
  olddta = Fgetdta();
  Fsetdta(&newdta);
  
  sprintf(searchpat,"u:\\proc\\*.%03d",Pgetpid());
  
  if(Fsfirst(searchpat,0) == 0) {
    if((debug = fopen(debugpath,"a+"))) {
      fprintf(debug,"%s: %s\r\n",newdta.dta_name,s);
      fclose(debug);
    };
  };
  
  Fsetdta(olddta);
}

/*
** Exported
**
** 1999-03-28 CG
*/
void
DB_printf (char * fmt, ...) {
  va_list arguments;
  char    s[128];
  
  va_start(arguments, fmt);
  vsprintf(s, fmt, arguments);
  va_end(arguments);

  /*DEBUG(s);*/
  fprintf (stderr, "oaesis: %s\n", s);
}


/*
** Exported
**
** 1999-03-28 CG
*/
void
DB_setpath (char * path) {
  strcpy (debugpath, path);
}
