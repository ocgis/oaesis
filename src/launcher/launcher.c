/*
** launcher.c
**
** Copyright 1998 Christer Gustavsson <cg@nocrew.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**  
** Read the file COPYING for more information.
*/

#include <stdio.h>

#include <aesbind.h>
#include <vdibind.h>

#include "launch.h"

#define WORD short
#define LONG long

#ifndef TRUE
#define TRUE  1
#define FALSE 0
#endif /* TRUE */

#define HI_WORD(l) ((WORD)((LONG)l >> 16))
#define LO_WORD(l) ((WORD)((LONG)l & 0xffff))

static int  vid;
static WORD num_colors;
static char title[] = "Lines";

static
WORD
max (WORD a,
     WORD b) {
  if(a > b) 
    return a;
  else
    return b;
}

static
WORD
min (WORD a,
     WORD b) {
  if(a < b) 
    return a;
  else
    return b;
}

#define NUM_LINES       10


/*
** Description
** Show information about oAESis
**
** 1999-01-10 CG
*/
static
void
show_information (void) {
  OBJECT * information;
  WORD     x;
  WORD     y;
  WORD     w;
  WORD     h;

  /* Get address if information resource */
  rsrc_gaddr (R_TREE, INFORM, &information);

  /* Calculate area of resource */
  form_center (information, &x, &y, &w, &h);

  fprintf (stderr, "launcher.c: form_center returned: %d %d %d %d\n",
           x, y, w, h);
  /* Reserve area for dialog */
  form_dial (FMD_START, x, y, w, h, x, y, w, h);

  /* Draw dialog */
  objc_draw (information, 0, 9, x, y, w, h);

  /* Let the user handle the dialog */
  form_do (information, 0);

  /* Free area used by dialog */
  form_dial (FMD_FINISH, x, y, w, h, x, y, w, h);
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
**
** 1999-01-10 CG
*/
static
void
updatewait (int wid) {
  WORD  quit = FALSE;
  WORD  ant_klick;
  WORD  buffert[16];
  WORD  happ;
  WORD  knapplage;
  WORD  lastline = 0;
  WORD  num_lines = 1;
  WORD  tangent,tanglage;
  WORD  winx,winy,winw,winh;
  WORD  x,y,w,h;
  VRECT lines[NUM_LINES];
  
  WORD  sx1 = 5,sy1 = 10,sx2 = 15,sy2 = 5;

  wind_get (wid, WF_WORKXYWH, &winx, &winy, &winw, &winh);

  /*
  fprintf (stderr, "wind_get (WF_WORKXYWH...): x=%d y=%d w=%d h=%d\n",
           winx, winy, winw, winh);
           */

  lines[0].v_x1 = winx;
  lines[0].v_y1 = winy;
  lines[0].v_x2 = winx + 100;
  lines[0].v_y2 = winy + 100;

  while (!quit) {
    happ = evnt_multi(MU_KEYBD | MU_MESAG | MU_TIMER | MU_BUTTON,
                      0,0,0,0,0,0,0,0,0,0,0,0,0,
                      buffert,0,&x,&y,&knapplage,&tanglage,
                      &tangent,&ant_klick);

    if (happ & MU_BUTTON) {
      fprintf (stderr,
               "lines.prg: evnt_multi returned MU_BUTTON x = %d y = %d buttons = %d\n",
               x, y, knapplage);
    }

    if (happ & MU_MESAG) {
      if (buffert[0] == MN_SELECTED) {
        quit = handle_menu (buffert);
      } else if (buffert[0] == WM_REDRAW) {
        WORD      x,y,w,h;
        wind_update(BEG_UPDATE);
        
        wind_get (wid, WF_FIRSTXYWH, &x, &y, &w, &h);
        
        /*
          fprintf (stderr, "wind_get (WF_FIRSTXYWH...): x=%d y=%d w=%d h=%d\n",
          x, y, w, h);
        */
        
        while((w > 0) && (h > 0))
        {
          WORD    xn,yn,wn,hn;
          
          xn = max(x,buffert[4]);
          wn = min(x + w, buffert[4] + buffert[6]) - xn;
          yn = max(y,buffert[5]);
          hn = min(y + h, buffert[5] + buffert[7]) - yn;
          
          if((wn > 0) && (hn > 0))
          {
            WORD  i;
            int   xyxy[4];
            
            xyxy[0] = xn;
            xyxy[1] = yn;
            xyxy[2] = xn+wn-1;
            xyxy[3] = yn+hn-1;
            
            vs_clip(vid,1,xyxy);
            
            graf_mouse (M_OFF, NULL);
            
            vr_recfl(vid,xyxy);
            
            for(i = 0; i < num_lines; i++) {
              vsl_color(vid,i % (num_colors - 1));
              
              v_pline(vid,2,&lines[i]);
            }
            
            graf_mouse (M_ON, NULL);
            
            vs_clip(vid,0,xyxy);
          }
          
          wind_get(wid,WF_NEXTXYWH,&x,&y,&w,&h);
        }                         
        wind_update(END_UPDATE);
      } else if (buffert[0] == WM_TOPPED) {
        wind_set (wid, WF_TOP, 0, 0, 0, 0);
      } else if (buffert[0] == WM_CLOSED) {
        quit = TRUE;
      } else if (buffert[0] == WM_SIZED) {          
        WORD      i;
        WORD      newx,newy,neww,newh;
        
        wind_set(wid,WF_CURRXYWH,buffert[4],buffert[5]
                 ,buffert[6],buffert[7]);
        
        wind_get(wid,WF_WORKXYWH,&newx,&newy,&neww,&newh);
        
        for(i = 0; i < num_lines; i++) {
          if(lines[i].v_x1 >= (newx + neww)) {
            lines[i].v_x1 = newx + neww - 1;
          }
          
          if(lines[i].v_y1 >= (newy + newh)) {
            lines[i].v_y1 = newy + newh - 1;
          }
          
          if(lines[i].v_x2 >= (newx + neww)) {
            lines[i].v_x2 = newx + neww - 1;
          }
          
          if(lines[i].v_y2 >= (newy + newh)) {
            lines[i].v_y2 = newy + newh - 1;
          }
        }
      
        winx = newx;
        winy = newy;
        winw = neww;
        winh = newh;
      } else if (buffert[0] == WM_MOVED) {
        WORD      i;
        WORD      newx,newy,neww,newh;
        
        wind_set (wid,
                  WF_CURRXYWH,
                  buffert[4],
                  buffert[5],
                  buffert[6],
                  buffert[7]);
        
        wind_get(wid,WF_WORKXYWH,&newx,&newy,&neww,&newh);
        
        for(i = 0; i < num_lines; i++) {
          lines[i].v_x1 += newx - winx;
          lines[i].v_y1 += newy - winy;
          lines[i].v_x2 += newx - winx;
          lines[i].v_y2 += newy - winy;
        }
        
        winx = newx;
        winy = newy;
        winw = neww;
        winh = newh;
      }
      /* happ & MU_MESAG */
    } else if((happ & MU_KEYBD) && ((tangent & 0xff) == 'q')) {
      break;
    } else if(happ & MU_TIMER) {
      VRECT     delete;

      delete = lines[(lastline + 1) % NUM_LINES];
      lines[(lastline + 1) % NUM_LINES] = lines[lastline];
      lastline = (lastline + 1) % NUM_LINES;
      
      lines[lastline].v_x1 += sx1;
      if((lines[lastline].v_x1 >= (winx + winw)) ||
         (lines[lastline].v_x1 < winx)) {
        sx1 = -sx1;
        
        lines[lastline].v_x1 += sx1;
      };
      
      lines[lastline].v_y1 += sy1;
      if((lines[lastline].v_y1 >= (winy + winh)) ||
         (lines[lastline].v_y1 < winy)) {
        sy1 = -sy1;
        
        lines[lastline].v_y1 += sy1;
      };
      
      lines[lastline].v_x2 += sx2;
      if((lines[lastline].v_x2 >= (winx + winw)) ||
         (lines[lastline].v_x2 < winx)) {
        sx2 = -sx2;
        
        lines[lastline].v_x2 += sx2;
      };
      
      lines[lastline].v_y2 += sy2;
      if((lines[lastline].v_y2 >= (winy + winh)) ||
         (lines[lastline].v_y2 < winy)) {
        sy2 = -sy2;
        
        lines[lastline].v_y2 += sy2;
      };

      wind_update(BEG_UPDATE);
      
      wind_get(wid,WF_FIRSTXYWH,&x,&y,&w,&h);

      while((w > 0) && (h > 0)) {
        int     xyxy[4];
        
        xyxy[0] = x;
        xyxy[1] = y;
        xyxy[2] = x + w - 1;
        xyxy[3] = y + h - 1;

        vs_clip(vid,1,xyxy);

        graf_mouse (M_OFF, NULL);

        vsl_color(vid,lastline % (num_colors - 1));
        v_pline(vid,2,&lines[lastline]);
        
        if(num_lines == NUM_LINES) {
          vsl_color(vid,BLACK);
          v_pline(vid,2,&delete);
        }

        graf_mouse (M_ON, NULL);

        vs_clip(vid,0,xyxy);

        wind_get(wid,WF_NEXTXYWH,&x,&y,&w,&h);
      }
      
      wind_update(END_UPDATE);

      if(num_lines < NUM_LINES) {
        num_lines++;
      }
    }
  }
}

void testwin(void)
{
  WORD  xoff,yoff,woff,hoff;
  
  WORD  wid;
  
  wind_get(0,WF_WORKXYWH,&xoff,&yoff,&woff,&hoff);

  wid = wind_create(NAME|MOVER|FULLER|SIZER|CLOSER, xoff, yoff, woff, hoff);

  wind_set (wid, WF_NAME, (long)title >> 16, (long)title & 0xffff, 0, 0);
  
  wind_open (wid, xoff, yoff, woff / 2, hoff / 2);

  updatewait(wid);

  wind_close (wid);
  
  /*
  wind_delete(wid);
  */
}


/*
** Description
** The launcher takes care of program and accessory launching for oaesis
**
** 1998-12-28 CG
** 1999-01-01 CG
** 1999-01-03 CG
** 1999-01-06 CG
** 1999-01-09 CG
*/
int
main ()
{
  int      work_in[] = {1,1,1,1,1,1,1,1,1,1,2,0};
  
  int      work_out[57];
  OBJECT * desktop_bg;
  OBJECT * menu;
  char     path[] = "/usr/dum";
  char     file[] = "djuro";
  short    exit_button;
  
  WORD     wc, hc, wb, hb;

  /* Get application id */
  appl_init();

  /* Fix resource data */
  rsrc_rcfix (launch);

  /* Get address of desktop background */
  rsrc_gaddr (R_TREE, DESKBG, &desktop_bg);

  /* Set desktop background */
  wind_set (0,
            WF_NEWDESK,
            HI_WORD(desktop_bg),
            LO_WORD(desktop_bg),
            0,
            0);

  /* Get address of the menu */
  rsrc_gaddr (R_TREE, MENU, &menu);

  /* Install menu */
  menu_bar (menu, MENU_INSTALL);

  vid = graf_handle (&wc, &hc, &wb, &hb);
  v_opnvwk(work_in,&vid,work_out);
  num_colors = work_out[39];
  num_colors = 256;

  vsf_interior(vid,1);

  graf_mouse (ARROW, 0L);

  /*
  fsel_exinput (path, file, &exit_button, "Hulabopp");
  */

  testwin();

  v_clsvwk(vid);

  appl_exit();
        
  return 0;
}
