/*
** fsel.c
**
** Copyright 1996 - 2001 Christer Gustavsson <cg@nocrew.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**  
** Read the file COPYING for more information.
**
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_MINTBIND_H
#include <mintbind.h>
#endif

#ifdef HAVE_OSBIND_H
#include <osbind.h>
#endif

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "aesbind.h"
#include "debug.h"
#include "evnt.h"
#include "evnthndl.h"
#include "form.h"
#include "fsel.h"
#include "graf.h"
#include "lib_global.h"
#include "lib_misc.h"
#include "mintdefs.h"
#include "objc.h"
#include "resource.h"
#include "rsrc.h"
#include "types.h"
#include "wind.h"


typedef struct direntry {
	UWORD type;
	BYTE  *name;
	LONG  length;
	
	struct direntry *next;
}DIRENTRY;

typedef struct dirdesc {
	DIRENTRY	*dent;
	WORD	num_files;
	WORD	pos;
}DIRDESC;


static BYTE	nullstr[] = "";


static void directory_sort(DIRDESC *dd, WORD fsel_sorted) {
  if(fsel_sorted) {
    DIRENTRY *p1, *p2, *p3, *p4;

    int change;

    if(dd->dent != NULL) {
      do {
        p1 = dd->dent;
        p3 = NULL;
        change = 0;

        while(p1 != NULL) {
          p2 = p1->next;
          if(p2 != NULL) {
            if(strcmp(p1->name, p2->name) > 0) {
              if(p3 == NULL) {
                p4 = dd->dent;
                dd->dent = p1->next;
                p1->next = p2->next;
                p2->next = p4;
              }
              else {
                p4 = p3->next;
                p3->next = p1->next;
                p1->next = p2->next;
                p2->next = p4;
              }
              change = 1;
            }
          }
          p3 = p1;
          p1 = p2;
        }
      }while(change);
    }
  }
}

static WORD	globcmp(BYTE *pattern,BYTE *str) {
	while(1) {
		switch(*pattern) {
		case '\0':
			if(*str == 0) {
				return 0;
			}
			else {
				return -1;
			};
			
		case '*':
			pattern++;
			
			if(*pattern == '\0') {
				return 0;
			};
			
			do {
				if(!globcmp(pattern,str)) {
					return 0;
				};
				
				if(*str == '\0') {
					break;
				};
				
				str++;
			}while(*str);

			return -1;
			
		default:
			if(*str == '\0') {
				return -1;
			};
			
			if(*(str++) != *(pattern++)) {
				return -1;
			};
		}
	}
}


/*
** Description
** Set path when using MiNT pathnames
**
** 1999-03-11 CG
*/
static
WORD
mint_set_path (BYTE *    pattern,
               DIRDESC * dd) {
  BYTE	path[128];
  BYTE	pat[30];
  BYTE	*tmp;
  LONG  d;
  
  strcpy(path,pattern);
  
  tmp = strrchr(path,'\\');
  
  if(tmp) {
    tmp++;
    
    strcpy(pat,tmp);
    *tmp = '\0';
  } else {
    strcpy(pat,path);
    strcpy(path,".\\");
  }
  
  d = Dopendir(path,0);
  
  if((d & 0xff000000L) != 0xff000000L) {
    BYTE name[50];
    
    while(!Dreaddir(50,d,name)) {
      if(strcmp("..",&name[4]) && strcmp(".",&name[4])) {
        WORD  fa = 0;
        BYTE  filepath[128];
        
        sprintf(filepath,"%s%s",path,&name[4]);
        
        fa = Fattrib(filepath,0,0);
        if((fa & 0x10) || (!globcmp(pat,&name[4]))) {
          DIRENTRY *detmp = (DIRENTRY *)malloc(sizeof(DIRENTRY));
          DIRENTRY **dwalk = &dd->dent;
          
          if(fa & 0x10) {
            detmp->type = S_IFDIR;
          } else {
            detmp->type = S_IFREG;
          }
          
          detmp->name = (BYTE *)malloc(strlen(&name[4]) + 4);
          sprintf(detmp->name,"   %s",&name[4]);
          
          if(detmp->type == S_IFDIR) {
            detmp->name[1] = 0x7;
          }
          
          if(detmp->type == S_IFLNK) {
            detmp->name[0] = '=';
          }
          
          detmp->next = NULL;
          dd->num_files++;
          
          while(*dwalk) {
            dwalk = &(*dwalk)->next;
          }
          
          *dwalk = detmp;
        }
      }
    }
    
    Dclosedir(d);
    
    return 0;
  } else {
    return -1;
  }
}


/*
** Description
** Set path when using Unix pathnames
**
** 1999-03-11 CG
*/
static
WORD
unix_set_path (BYTE *    pattern,
               DIRDESC * dd) {
  BYTE	path[128];
  BYTE	pat[30];
  BYTE * tmp;
  DIR *  d;
  
  strcpy (path, pattern);
  
  tmp = strrchr (path,'/');
  
  if(tmp) {
    tmp++;
    
    strcpy(pat,tmp);
    *tmp = 0;
  } else {
    strcpy(pat,path);
    strcpy(path,"./");
  }
  
  d = opendir (path);
  
  if (d != NULL) {
    struct dirent * dir_entry;
    
    while ((dir_entry = readdir (d)) != NULL) {
      if (strcmp ("..", dir_entry->d_name) && strcmp(".", dir_entry->d_name)) {
        struct stat st_buf;
        BYTE        filepath[128];
        
        sprintf (filepath,"%s%s", path, dir_entry->d_name);
        
        if (stat (filepath, &st_buf) != 0) {
          return -1;
        }

        if((S_ISDIR (st_buf.st_mode)) || (!globcmp (pat, dir_entry->d_name))) {
          DIRENTRY *detmp = (DIRENTRY *)malloc(sizeof(DIRENTRY));
          DIRENTRY **dwalk = &dd->dent;
          
          if (S_ISDIR (st_buf.st_mode)) {
            detmp->type = S_IFDIR;
          } else {
            detmp->type = S_IFREG;
          }
          
          detmp->name =
            (BYTE *)malloc (strlen (dir_entry->d_name) + 4);
          sprintf (detmp->name, "   %s", dir_entry->d_name);
          
          if(detmp->type == S_IFDIR) {
            detmp->name[1] = 0x7;
          }
          
          if(detmp->type == S_IFLNK) {
            detmp->name[0] = '=';
          }
          
          detmp->next = NULL;
          dd->num_files++;
          
          while (*dwalk) {
            dwalk = &(*dwalk)->next;
          }
          
          *dwalk = detmp;
        }
      }
    }
    
    closedir (d);
    
    return 0;
  } else {
    return -1;
  }
}


/*
** Description
** FIXME
*/
static
void
reset_dirdesc (DIRDESC *dd)
{
  DIRENTRY *dwalk = dd->dent;
  
  while(dwalk)
  {
    DIRENTRY *tmp = dwalk;
    
    dwalk = dwalk->next;
    
    free(tmp->name);
    free(tmp);
  }
  
  dd->dent = NULL;
  dd->pos = 0;
  dd->num_files = 0;
}


static
void
get_files(OBJECT *  t,
          DIRDESC * dd)
{
  WORD i = dd->pos;
  DIRENTRY *dwalk = dd->dent;
  
  while((i--) && dwalk)
  {
    dwalk = dwalk->next;
  }
  
  i = FISEL_FIRST;
  
  while(i <= FISEL_LAST)
  {
    if(dwalk)
    {
      TE_PTEXT_PUT(OB_SPEC(&t[i]), dwalk->name);
      dwalk = dwalk->next;
    }
    else
    {
      TE_PTEXT_PUT(OB_SPEC(&t[i]), nullstr);
    }
    
    i++;
  }
	
  if(dd->num_files <= (FISEL_LAST - FISEL_FIRST + 1))
  {
    OB_Y_PUT(&t[FISEL_SLIDER], 0);
    OB_HEIGHT_PUT(&t[FISEL_SLIDER], OB_HEIGHT(&t[FISEL_SB]));
  }
  else
  {
    OB_HEIGHT_PUT(&t[FISEL_SLIDER],
                  (WORD)(((LONG)OB_HEIGHT(&t[FISEL_SB]) *
                          (LONG)(FISEL_LAST - FISEL_FIRST + 1)) /
                         (LONG)dd->num_files));
    OB_Y_PUT(&t[FISEL_SLIDER],
             (WORD)((((LONG)OB_HEIGHT(&t[FISEL_SB]) -
                      (LONG)OB_HEIGHT(&t[FISEL_SLIDER])) * (LONG)dd->pos) /
                    ((LONG)dd->num_files - FISEL_LAST + FISEL_FIRST - 1)));
  }
}

static DIRENTRY *find_entry(DIRDESC *dd,WORD pos) {
	DIRENTRY *walk = dd->dent;
	
	while(walk && (pos > 0)) {
		walk = walk->next;
		pos--;
	};
	
	return walk;
}


/*
** Description
** Handle file list slider
*/
static
void
slider_handle (WORD      apid,
               OBJECT  * tree,
               RECT    * clip,
               DIRDESC * dd)
{
  if(OB_HEIGHT(&tree[FISEL_SLIDER]) != OB_HEIGHT(&tree[FISEL_SB]))
  {
    WORD newpos;
    WORD oldpos = dd->pos;
    WORD slidpos;
    WORD oldslidpos = OB_Y(&tree[FISEL_SLIDER]);
    WORD slidmax = OB_HEIGHT(&tree[FISEL_SB]) - OB_HEIGHT(&tree[FISEL_SLIDER]);
    WORD buffer[16];

    EVENTIN ei = { MU_BUTTON | MU_M1,
                   2,           /* bclicks */
                   LEFT_BUTTON, /* bmask   */
                   0,           /* bstate  */
                   1,
                   { 0, 0, 1, 1},
                   0,
                   {0, 0, 0, 0},
                   0,
                   0 };
    EVENTOUT      eo;
    GLOBAL_APPL * globals = get_globals (apid);
    WORD          mx, my, mb, ks;
  
    Graf_do_mkstate (apid, &mx, &my, &mb, &ks);

    ei.m1r.x = mx;
    ei.m1r.y = my;

    Wind_do_update (apid, BEG_MCTRL);
    Graf_do_mouse (apid, FLAT_HAND, NULL);
    Objc_do_change (globals->vid, tree, FISEL_SLIDER, clip, SELECTED, REDRAW);

    while (TRUE) {
      Evnt_do_multi (apid,
                     &ei,
                     (COMMSG *)buffer,
                     &eo,
                     0,
                     DONT_HANDLE_MENU_BAR);

      if(eo.events & MU_BUTTON) {

        Objc_do_change (globals->vid, tree, FISEL_SLIDER, clip, 0, REDRAW);
        Graf_do_mouse (apid, M_RESTORE, NULL);

        if(eo.mb & RIGHT_BUTTON) {
          dd->pos = oldpos;
          get_files(tree, dd);
          Objc_do_draw (globals->vid, tree, FISEL_ENTBG, 9, clip);
          Objc_do_draw (globals->vid, tree, FISEL_SB, 9, clip);

          if(eo.mb & LEFT_BUTTON) {
            WORD dummy;

            Evnt_do_button (apid,
                            1,
                            LEFT_BUTTON,
                            0,
                            &dummy,
                            &dummy,
                            &dummy,
                            &dummy);
          }
        }
        break;
      }

      if(eo.events & MU_M1) {
        slidpos = oldslidpos - (ei.m1r.y - eo.my);
        if(slidpos < 0) slidpos = 0;
        if(slidpos > slidmax) slidpos = slidmax;
        if(slidpos != oldslidpos) {
          newpos = (WORD)(((LONG)(dd->num_files - FISEL_LAST + FISEL_FIRST - 1) * ((LONG)slidpos * 1000L / (LONG)slidmax)) / 1000L);
          if(newpos != dd->pos) {
            dd->pos = newpos;
            get_files(tree, dd);
            Objc_do_draw (globals->vid, tree, FISEL_ENTBG, 9, clip);
            Objc_do_draw (globals->vid, tree, FISEL_SB, 9, clip);
          }
          oldslidpos = slidpos;
        }
        ei.m1r.x = eo.mx;
        ei.m1r.y = eo.my;
      }
    }
    Wind_do_update (apid, END_MCTRL);
  }
}

/****************************************************************************
 * Public functions                                                         *
 ****************************************************************************/

/*
** Exported
*/
WORD
Fsel_do_exinput (WORD   apid,
                 WORD * button,
                 BYTE * description,
                 BYTE * path,
                 BYTE * file)
{
  WORD cwidth,cheight,width,height;
  WORD selected = -1;
  BYTE oldpath[128];
		
  OBJECT	*tree;
	
  RECT	area,clip;
	
  WORD but_chosen;
  DIRDESC dd = { NULL,0,0 };
  RECT src,dst;
  GLOBAL_APPL * globals = get_globals (apid);
  BYTE          path_separator =
    (globals->path_mode == OAESIS_PATH_MODE_MINT) ? '\\' : '/';

  Graf_do_mouse (apid, BUSY_BEE, NULL);

  tree = Rsrc_duplicate(globals->common->fiseltad);	
	
  Graf_do_handle(&cwidth,&cheight,&width,&height);

  if(globals->path_mode == OAESIS_PATH_MODE_MINT)
  {
    mint_set_path (path, &dd);
  }
  else
  {
    unix_set_path (path, &dd);
  }

  directory_sort(&dd, globals->common->fsel_sorted);

  get_files(tree,&dd);

  TE_PTEXT_PUT(OB_SPEC(&tree[FISEL_DESCR]), description);
  TE_PTEXT_PUT(OB_SPEC(&tree[FISEL_DIRECTORY]), path);
  TE_PTEXT_PUT(OB_SPEC(&tree[FISEL_SELECTION]), file);

  strcpy(oldpath,path);

  Form_do_center (apid, tree, &clip);

  Objc_do_offset(tree,FISEL_FIRST,(WORD *)&dst);
  dst.width = OB_WIDTH(&tree[FISEL_FIRST]);
  dst.height = OB_HEIGHT(&tree[FISEL_FIRST]) * (FISEL_LAST - FISEL_FIRST);

  src = dst;
  src.y = dst.y + OB_HEIGHT(&tree[FISEL_FIRST]);

  Form_do_dial (apid, FMD_START, &clip, &clip);

  Objc_do_draw (globals->vid, tree, 0, 9, &clip);
  
  Graf_do_mouse (apid, ARROW, NULL);

  while (TRUE) {
    but_chosen = Form_do_do (apid, tree, FISEL_DIRECTORY);
	
    switch(but_chosen & 0x7fff) {
    case FISEL_OK:
      if (strcmp (oldpath, path)) {
        Graf_do_mouse (apid, BUSY_BEE, NULL);

        OB_STATE_CLEAR(&tree[FISEL_OK], SELECTED);
        Objc_do_draw (globals->vid, tree,FISEL_OK,9,&clip);
				
        reset_dirdesc(&dd);

        if(globals->path_mode == OAESIS_PATH_MODE_MINT)
        {
          mint_set_path (path, &dd);
        }
        else
        {
          unix_set_path (path, &dd);
        }

        directory_sort(&dd, globals->common->fsel_sorted);

        get_files(tree,&dd);

        Objc_do_draw (globals->vid, tree,FISEL_ENTBG,9,&clip);	
        Objc_do_draw (globals->vid, tree,FISEL_SB,9,&clip);	
				
        strcpy(oldpath,path);
        Graf_do_mouse (apid, M_RESTORE, NULL);
        break;
      }
			
      /* Fall through... */
			
    case FISEL_CANCEL:
      Form_do_dial(apid,FMD_FINISH,&clip,&clip);
		
      Rsrc_free_tree(tree);
		
      if(but_chosen == FISEL_OK) {
        *button = FSEL_OK;
      }
      else {
        *button = FSEL_CANCEL;
      };
				
      reset_dirdesc(&dd);
	
      return 1;
				
    case FISEL_UP:
      if(dd.pos > 0)
      {
        OB_STATE_SET(&tree[FISEL_UP],  SELECTED);
        Objc_do_draw (globals->vid, tree,FISEL_UP,9,&clip);

        if(((selected - dd.pos) >= 0) &&
           ((selected - dd.pos) <= (FISEL_LAST - FISEL_FIRST)))
        {
          WORD oldobj = selected - dd.pos + FISEL_FIRST;

          OB_STATE_CLEAR(&tree[oldobj], SELECTED);
          Objc_do_change (globals->vid,
                          tree,oldobj,
                          &clip,
                          OB_STATE(&tree[oldobj]),
                          NO_DRAW);
        }
						
        dd.pos--;

        if(((selected - dd.pos) >= 0) &&
           ((selected - dd.pos) <= (FISEL_LAST - FISEL_FIRST))) {
          WORD oldobj = selected - dd.pos + FISEL_FIRST;

          OB_STATE_SET(&tree[oldobj], SELECTED);
          Objc_do_change(globals->vid,
                         tree,oldobj,
                         &clip,
                         OB_STATE(&tree[oldobj]),
                         NO_DRAW);
        }
					
        get_files(tree,&dd);
        Misc_copy_area (globals->vid, &src, &dst);

        Objc_do_draw (globals->vid, tree,FISEL_FIRST,9,&clip);
        OB_STATE_CLEAR(&tree[FISEL_UP], SELECTED);
        Objc_do_draw (globals->vid, tree,FISEL_UP,9,&clip);
        Objc_do_draw (globals->vid, tree,FISEL_SB,9,&clip);
      }
      break;
			
    case FISEL_DOWN:
      if(dd.pos < (dd.num_files - FISEL_LAST + FISEL_FIRST - 1)) {
        OB_STATE_SET(&tree[FISEL_DOWN], SELECTED);
        Objc_do_draw (globals->vid, tree,FISEL_DOWN,9,&clip);					

        if(((selected - dd.pos) >= 0) &&
           ((selected - dd.pos) <= (FISEL_LAST - FISEL_FIRST)))
        {
          WORD oldobj = selected - dd.pos + FISEL_FIRST;

          OB_STATE_CLEAR(&tree[oldobj], SELECTED);
          Objc_do_change (globals->vid,
                          tree,
                          oldobj,
                          &clip,
                          OB_STATE(&tree[oldobj]),
                          NO_DRAW);
        }
					
        dd.pos++;

        if(((selected - dd.pos) >= 0) &&
           ((selected - dd.pos) <= (FISEL_LAST - FISEL_FIRST)))
        {
          WORD oldobj = selected - dd.pos + FISEL_FIRST;

          OB_STATE_SET(&tree[oldobj], SELECTED);
          Objc_do_change (globals->vid,
                          tree,
                          oldobj,
                          &clip,
                          OB_STATE(&tree[oldobj]),
                          NO_DRAW);
        }
					
        get_files(tree,&dd);
        Misc_copy_area (globals->vid, &dst, &src);
        Objc_do_draw (globals->vid, tree,FISEL_LAST,9,&clip);
        OB_STATE_CLEAR(&tree[FISEL_DOWN], SELECTED);
        Objc_do_draw (globals->vid, tree,FISEL_DOWN,9,&clip);
        Objc_do_draw (globals->vid, tree,FISEL_SB,9,&clip);
      }
      break;
			
    case FISEL_SB:
    {
      WORD mx, my, mb, ks;
      WORD xy[2];
				
      Objc_do_offset(tree,FISEL_SLIDER,xy);
				
      if(((selected - dd.pos) >= 0) &&
         ((selected - dd.pos) <= (FISEL_LAST - FISEL_FIRST)))
      {
        WORD oldobj = selected - dd.pos + FISEL_FIRST;

        OB_STATE_CLEAR(&tree[oldobj], SELECTED);
        Objc_do_change(globals->vid,
                       tree,
                       oldobj,
                       &clip,
                       OB_STATE(&tree[oldobj]),
                       NO_DRAW);
      }

      Graf_do_mkstate (apid, &mx, &my, &mb, &ks);

      if (my > xy[1]) {
        dd.pos += FISEL_LAST - FISEL_FIRST + 1;
        if(dd.pos >= (dd.num_files - FISEL_LAST + FISEL_FIRST)) {
          dd.pos = dd.num_files - FISEL_LAST + FISEL_FIRST - 1;
        }
      } else {
        dd.pos -= FISEL_LAST - FISEL_FIRST + 1;
        if(dd.pos < 0) {
          dd.pos = 0;
        }
      }
			

      if(((selected - dd.pos) >= 0) &&
         ((selected - dd.pos) <= (FISEL_LAST - FISEL_FIRST))) {
        WORD oldobj = selected - dd.pos + FISEL_FIRST;

        OB_STATE_SET(&tree[oldobj], SELECTED);
        Objc_do_change(globals->vid,
                       tree,
                       oldobj,
                       &clip,
                       OB_STATE(&tree[oldobj]),
                       NO_DRAW);
      }

      get_files(tree,&dd);
      Objc_do_draw (globals->vid, tree,FISEL_ENTBG,9,&clip);
      Objc_do_draw (globals->vid, tree,FISEL_SB,9,&clip);
    }
    break;
		
    case FISEL_SLIDER:
      slider_handle (apid,
                     tree,
                     &clip,
                     &dd);
      break;
		
    case FISEL_BACK:
    {
      BYTE newpath[128],*tmp;
				
      OB_STATE_CLEAR(&tree[FISEL_BACK], SELECTED);
      Objc_do_draw (globals->vid, tree,FISEL_BACK,9,&clip);	

      strcpy(newpath,path);
								
      tmp = strrchr (newpath, path_separator);

      if(!tmp)
        break;
					
      *tmp = 0;
      tmp = strrchr(newpath, path_separator);
				
      if(!tmp)
        break;
					
      *tmp = '\0';
							
      tmp = strrchr(path, path_separator);
      strcat(newpath,tmp);
      strcpy(path,newpath);

      Graf_do_mouse (apid, BUSY_BEE, NULL);


      reset_dirdesc(&dd);

      if(globals->path_mode == OAESIS_PATH_MODE_MINT)
      {
        mint_set_path (path, &dd);
      }
      else
      {
        unix_set_path (path, &dd);
      }
      
      directory_sort(&dd, globals->common->fsel_sorted);

      Objc_area_needed(tree,FISEL_DIRECTORY,&area);	
      Objc_do_draw (globals->vid, tree,0,9,&area);	

      get_files(tree,&dd);

      Objc_do_draw (globals->vid, tree,FISEL_ENTBG,9,&clip);	
      Objc_do_draw (globals->vid, tree,FISEL_SB,9,&clip);	
				
      strcpy(oldpath,path);
      Graf_do_mouse (apid, M_RESTORE, NULL);
    }
    break;
			
    default:
      if(((but_chosen & 0x7fff) >= FISEL_FIRST) &&
         ((but_chosen & 0x7fff) <= FISEL_LAST)) {
        WORD     obj = but_chosen & 0x7fff;
        DIRENTRY *dent;
				
        dent = find_entry(&dd,dd.pos + (but_chosen & 0x7fff) - FISEL_FIRST);

        if(dent) {

          if(dent->type & S_IFDIR) {
            BYTE newpath[128];
            BYTE *tmp;

            Graf_do_mouse (apid, BUSY_BEE, NULL);
		
            strcpy(newpath,path);
							
            tmp = strrchr (newpath, path_separator);
							
            sprintf (tmp, "%c%s%c",
                     path_separator, &dent->name[3], path_separator);
							
            tmp = strrchr (path, path_separator);
            tmp++;
            strcat(newpath,tmp);
            strcpy(path,newpath);

            OB_STATE_SET(&tree[obj], SELECTED);
            Objc_do_change (globals->vid,
                            tree,
                            obj,
                            &clip,
                            OB_STATE(&tree[obj]),
                            REDRAW);

            OB_STATE_CLEAR(&tree[obj], SELECTED);
            Objc_do_change (globals->vid,
                            tree,
                            obj,
                            &clip,
                            OB_STATE(&tree[obj]),
                            REDRAW);		
            reset_dirdesc(&dd);

            if(globals->path_mode == OAESIS_PATH_MODE_MINT)
            {
              mint_set_path (path, &dd);
            }
            else
            {
              unix_set_path (path, &dd);
            }
            
            directory_sort(&dd, globals->common->fsel_sorted);

            Objc_area_needed(tree,FISEL_DIRECTORY,&area);	
            Objc_do_draw (globals->vid, tree,0,9,&area);	

            get_files(tree,&dd);

            Objc_do_draw (globals->vid, tree,FISEL_ENTBG,9,&clip);	
            Objc_do_draw (globals->vid, tree,FISEL_SB,9,&clip);	
						
            strcpy(oldpath,path);

            Graf_do_mouse (apid, M_RESTORE, NULL);
          } else {
            strcpy(file,&dent->name[3]);
						
            if(((selected - dd.pos) >= 0) &&
               ((selected - dd.pos) <= (FISEL_LAST - FISEL_FIRST)))
            {
              WORD oldobj = selected - dd.pos + FISEL_FIRST;

              OB_STATE_CLEAR(&tree[oldobj], SELECTED);
              Objc_do_change(globals->vid,
                             tree,
                             oldobj,
                             &clip,
                             OB_STATE(&tree[oldobj]),
                             REDRAW);
            }
            
            OB_STATE_SET(&tree[obj], SELECTED);
            Objc_do_change(globals->vid,
                           tree,
                           obj,
                           &clip,
                           OB_STATE(&tree[obj]),
                           REDRAW);
					
            selected = obj - FISEL_FIRST + dd.pos;
					
            Objc_area_needed(tree,FISEL_SELECTION,&area);	
            Objc_do_draw (globals->vid, tree,0,9,&area);	

            if(but_chosen & 0x8000) {
              Form_do_dial(apid,FMD_FINISH,&clip,&clip);

              Rsrc_free_tree(tree);
	
              *button = FSEL_OK;
              reset_dirdesc(&dd);

              return 1;
            }						
          }
        }
      }
    }
  }
}


/*
** Exported
*/
void
Fsel_input (AES_PB *apb)
{
  CHECK_APID(apb->global->apid);

  apb->int_out[0] = Fsel_do_exinput (apb->global->apid,
                                     &apb->int_out[1],
                                     "Select a file",
                                     (BYTE *)apb->addr_in[0],
                                     (BYTE *)apb->addr_in[1]);
}


/*
** Exported
*/
void
Fsel_exinput (AES_PB *apb)
{
  CHECK_APID(apb->global->apid);

  apb->int_out[0] = Fsel_do_exinput (apb->global->apid,
                                     &apb->int_out[1],
                                     (BYTE *)apb->addr_in[2],
                                     (BYTE *)apb->addr_in[0],
                                     (BYTE *)apb->addr_in[1]);
}
