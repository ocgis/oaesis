/*
** launcher.c
**
** Copyright 1998 - 2000 Christer Gustavsson <cg@nocrew.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**  
** Read the file COPYING for more information.
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef MINT_TARGET
#include <mintbind.h>
#endif

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <aesbind.h>
#include <vdibind.h>

#include "boot.h"
#include "launch.h"
#include "launcher.h"
#include "misc.h"


#ifdef MINT_TARGET
static char progpath[500] = "u:\\d\\nytto\\*";
#else
static char progpath[500] = "/usr/local/src/osis/ppc-linux/*";
#endif

static char progfile[70] = "toswin_w.prg";



/*
** Description
** Let the user select an application and start it
*/
static
void
launch_application (void)
{
  char   execpath[128]; /*FIXME length of string */
  char   oldpath[128];
  char * tmp;
  int    button;

  /* Open file selector */
  fsel_exinput(progpath, progfile, &button, "Select program to start");

  /* Handle request if OK was selected */
  if(button == FSEL_OK)
  {
    int  err;
    char newpath[128];
    
    strcpy(newpath,progpath);
    
    tmp = strrchr (newpath, PATH_SEPARATOR);
    
    if(tmp)
    {
      *tmp = 0;
      sprintf (execpath, "%s%c%s", newpath, PATH_SEPARATOR, progfile);
    }
    else
    {
      strcpy (execpath, progfile);
    }


#ifdef MINT_TARGET
    /* FIXME: Use shel_write instead */
    Dgetpath(oldpath, 0);
    misc_setpath(newpath);

    err = Pexec(100, execpath, 0L, 0L);

    misc_setpath(oldpath);
#else
    getcwd (oldpath, 128);
    chdir (newpath);

    if (fork () == 0)
    {
      execlp (execpath, NULL);

      exit (0);
    }

    chdir (oldpath);

    err = 0;
#endif

    if (err < 0) {
      form_error ((WORD) -err - 31);
    }
  }
}


/*
** Description
** Show information about oAESis
**
** 1999-01-10 CG
** 1999-01-11 CG
*/
static
void
show_information (void) {
  OBJECT * information;
  int      x;
  int      y;
  int      w;
  int      h;

  /* Get address if information resource */
  rsrc_gaddr (R_TREE, INFORM, &information);

  /* Fix version number :-) */
  sprintf(information[INFOVERSION].ob_spec.tedinfo->te_ptext,
          OAESIS_VERSION);

  /* Calculate area of resource */
  form_center (information, &x, &y, &w, &h);

  /* Reserve area for dialog */
  form_dial (FMD_START, x, y, w, h, x, y, w, h);

  /* Draw dialog */
  objc_draw (information, 0, 9, x, y, w, h);

  /* Let the user handle the dialog */
  form_do (information, 0);

  /* Free area used by dialog */
  form_dial (FMD_FINISH, x, y, w, h, x, y, w, h);

  /* Restore ok button */
  information[INFOOK].ob_state &= ~SELECTED;
}


/*
** Description
** Handle selected menu entry
**
** 1999-01-10 CG
*/
static
WORD
handle_menu (WORD * buffert) {
  switch (buffert[3]) {
  case MENU_FILE :
    switch (buffert[4]) {
    case MENU_LAUNCHAPP :
      launch_application ();
      break;

    case MENU_QUIT :
      return TRUE;

    default :
      fprintf (stderr,
               "launcher.c: handle_menu: unknown MENU_FILE entry %d\n",
               buffert[4]);
    }
    break;
    /* MENU_FILE */

  case MENU_OAESIS :
    switch (buffert[4]) {
    case MENU_INFO :
      show_information ();
      break;

    default :
      fprintf (stderr,
               "launcher.c: handle_menu: unknown MENU_OAESIS entry %d\n",
               buffert[4]);
    }
    break;
    /* MENU_OAESIS */
    
  default :
    fprintf (stderr,
             "launcher.c: handle_menu: unknown menu title %d\n",
             buffert[3]);
  }

  return FALSE;
}


/*
** Description
** Wait for events and update windows
*/
static
void
updatewait(void)
{
  WORD  quit = FALSE;
  int   ant_klick;
  WORD  buffert[16];
  WORD  happ;
  int   knapplage;
  int   tangent,tanglage;
  int   x,y;
  
  while(!quit)
  {
    happ = evnt_multi(MU_KEYBD | MU_MESAG,
                      0,0,0,0,0,0,0,0,0,0,0,0,0,
                      buffert,0,&x,&y,&knapplage,&tanglage,
                      &tangent,&ant_klick);
    
    if (happ & MU_MESAG)
    {
      if (buffert[0] == MN_SELECTED)
      {
        quit = handle_menu(buffert);
      }
    }
    else if((happ & MU_KEYBD) && ((tangent & 0xff) == 'q'))
    {
      quit = TRUE;
    }
  }
}


/*
** Description
** The launcher takes care of program and accessory launching for oaesis
*/
int
#ifdef LAUNCHER_AS_PRG
main(void)
#else
launcher_main(void)
#endif
{
  OBJECT * desktop_bg;
  OBJECT * menu;
  
  int      ap_id;  

  /* Pdomain (1); FIXME decide where to put this */
  /* Get application id */
  ap_id = appl_init();

  /* Fix resource data */
  rsrc_rcfix(launch);

  /* Get address of desktop background */
  rsrc_gaddr(R_TREE, DESKBG, &desktop_bg);

  /* Set desktop background */
  wind_set(0,
           WF_NEWDESK,
           HI_WORD(desktop_bg),
           LO_WORD(desktop_bg),
           0,
           0);

  /* Get address of the menu */
  rsrc_gaddr(R_TREE, MENU, &menu);

  /* Install menu */
  menu_bar(menu, MENU_INSTALL);

  /* Register launcher as "oAESis" */
  menu_register(ap_id, "  oAESis");

  graf_mouse(ARROW, 0L);

  start_programs();

  updatewait();

  appl_exit();
        
  return 0;
}
