/*
** form.c
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "aesbind.h"
#include "debug.h"
#include "evnt.h"
#include "evnthndl.h"
#include "form.h"
#include "lib_global.h"
#include "graf.h"
#include "mintdefs.h"
#include "objc.h"
#include "resource.h"
#include "types.h"
#include "wind.h"


/*form_do 0x0032*/

/*
** Exported
**
** 1998-12-19 CG
** 1999-01-09 CG
** 1999-01-11 CG
*/
WORD
Form_do_do (WORD     apid,
            OBJECT * tree,
            WORD     editobj) {
  WORD buffer[16];
  WORD object,newobj,keyout;
  WORD idx;
	
  EVENTIN	ei = 	{
    MU_BUTTON | MU_KEYBD,
    2,
    LEFT_BUTTON,
    LEFT_BUTTON,
    0,
    {0,0,0,0},
    0,
    {0,0,0,0},
    0,
    0
  };
  EVENTOUT	eo;
  GLOBAL_APPL * globals = get_globals (apid);

  if (editobj != 0) {
    Objc_do_edit (globals->vid, tree, editobj, 0, &idx, ED_INIT);
  }

  Wind_do_update (apid, BEG_MCTRL);
	
  while (TRUE) {
    Evnt_do_multi (apid, &ei, (COMMSG *)buffer, &eo, 0, DONT_HANDLE_MENU_BAR);

    if(eo.events & MU_BUTTON) {
      object = Objc_do_find(tree,0,9,eo.mx,eo.my,0);

      if(object >= 0) {
        if(!Form_do_button(apid, tree, object, eo.mc, &newobj)) {
          if(editobj != 0) {
            Objc_do_edit (globals->vid, tree,editobj,0,&idx,ED_END);
          }

          Wind_do_update (apid, END_MCTRL);

          return newobj;
        } else {
          if((newobj != 0) && (newobj != editobj)) {
            if(editobj != 0) {
              Objc_do_edit (globals->vid, tree,editobj,0,&idx,ED_END);
            }
				
            editobj = newobj;

            Objc_do_edit (globals->vid, tree,editobj,0,&idx,ED_INIT);
          }
        }
      }
    }
		
    if(eo.events & MU_KEYBD) {
      if (!Form_do_keybd (apid, tree,editobj,eo.kc,&newobj,&keyout)) {
        if(editobj != 0) {
          Objc_do_edit (globals->vid, tree,editobj,0,&idx,ED_END);
        }

        Wind_do_update (apid, END_MCTRL);
        return newobj;
      } else if(newobj != editobj) {
        if(editobj != 0) {
          Objc_do_edit (globals->vid, tree, editobj, 0, &idx, ED_END);
        }
				
        editobj = newobj;

        if(editobj != 0) {
          Objc_do_edit (globals->vid, tree, editobj, 0, &idx, ED_INIT);
        }
      } else {
        if((editobj != 0) && (keyout != 0)) {
          Objc_do_edit (globals->vid, tree, editobj, keyout, &idx, ED_CHAR);
        }
      }
    }
  }
}


/*
** Exported
*/
void
Form_do (AES_PB *apb)
{
  CHECK_APID(apb->global->apid);

  apb->int_out[0] = Form_do_do (apb->global->apid,
                                (OBJECT *)apb->addr_in[0],
                                apb->int_in[0]);		
}

/*form_dial 0x0033*/

/*
** Exported
*/
WORD
Form_do_dial (WORD   apid,
              WORD   mode,
              RECT * r1,
              RECT * r2) {
  GLOBAL_APPL * globals = get_globals (apid);
    
  switch(mode) {
  case	FMD_GROW		:	/*0x0001*/
    if(globals->common->graf_growbox)
    {
      Graf_do_grmobox(apid, r1, r2);
    }
    return 1;
					
  case	FMD_START	:	/*0x0000*/
    if((r2->width > 0) && (r2->height > 0)) {
      WORD id;
      WORD top_window,owner,dummy;
				
      Wind_do_get (apid,
                   0,
                   WF_TOP,
                   &top_window,
                   &owner,
                   &dummy,
                   &dummy,
                   TRUE);
				
      if(owner == apid) {
        WORD status;
					
        Wind_do_get (apid,
                     top_window,
                     WF_OWNER,
                     &owner,
                     &status,
                     &dummy,
                     &dummy,
                     TRUE);
					
        if(status & WIN_DIALOG) {
          Wind_do_close (apid, top_window);
					
          Wind_do_delete (apid,
                          top_window);
        }
      }

      id = Wind_do_create(apid,
                          0,
                          r2,
                          WIN_DIALOG);
				
      Wind_do_open (apid, id, r2);

      return 1;
    }
    else {
      return 0;
    };

  case	FMD_SHRINK	:	/*0x0002*/
    if(globals->common->graf_shrinkbox)
    {
      Graf_do_grmobox(apid, r2, r1);
    }

  case	FMD_FINISH	:	/*0x0003*/
  {
    WORD top_window,owner,dummy;
				
    Wind_do_get (apid,
                 0,
                 WF_TOP,
                 &top_window,
                 &owner,
                 &dummy,
                 &dummy,
                 TRUE);
				
    if(owner == apid) {
      WORD status;
					
      Wind_do_get (apid,
                   top_window,
                   WF_OWNER,
                   &owner,
                   &status,
                   &dummy,
                   &dummy,
                   TRUE);
					
      if (status & WIN_DIALOG) {
        Wind_do_close (apid, top_window);
				
        Wind_do_delete (apid,
                        top_window);
      };
    };
			
    return 1;
  };
			
  default	:
    return 0;
  };
}

void Form_dial(AES_PB *apb)
{
  CHECK_APID(apb->global->apid);

  apb->int_out[0] = Form_do_dial(apb->global->apid,
				 apb->int_in[0],
				 (RECT *)&apb->int_in[1],
				 (RECT *)&apb->int_in[5]);
}


/*
** Description
** Implementation of form_alert ()
*/
static
WORD
Form_do_alert(WORD   apid,
              WORD   def,
              BYTE * alertstring)
{
  BYTE *        s;
  WORD          i;
  WORD          no_rows;
  WORD          no_butts;
  WORD          cwidth,cheight,width,height;
  WORD          but_chosen;
	
  OBJECT *      tree;
  TEDINFO *     ti;

  RECT          clip;
	
  BYTE *        icon;
  BYTE *        text;
  BYTE *        buttons;

  WORD          textwidth;
  WORD          buttonwidth;
  GLOBAL_APPL * globals;
	
  s = (BYTE *)malloc(strlen(alertstring) + 1);
  i = 0;
  no_rows = 1;
  no_butts = 1;
  textwidth = 0;
  buttonwidth = 0;
  globals = get_globals (apid);

  Graf_do_handle(&cwidth,&cheight,&width,&height);
	
  strcpy(s,alertstring);
	
  while(s[i] != '[')
  {
    i++;
  }

  icon = &s[i + 1];
	
  while(s[i] != ']')
  {
    i++;
  }
	
  s[i] = 0;
		
  while(s[i] != '[')
  {
    i++;
  }
		
  text = &s[i + 1];
	
  while(s[i] != ']')
  {
    if(s[i] == '|')
    {
      s[i] = 0;
      no_rows++;
    }
		
    i++;
  }
	
  s[i] = 0;
	
  while(s[i] != '[')
  {
    i++;
  }
	
  buttons = &s[i + 1];
	
  while(s[i] != ']')
  {
    if(s[i] == '|')
    {
      s[i] = 0;
      no_butts++;
    }
		
    i++;
  }
	
  s[i] = 0;
	
  tree = (OBJECT *)malloc((2 + no_butts + no_rows) * sizeof(OBJECT));
	
  ti = (TEDINFO *)malloc(no_rows * sizeof(TEDINFO));
	
  memcpy(&tree[0],&globals->common->alerttad[0],sizeof(OBJECT));
	
  OB_HEAD_PUT(&tree[0], -1);
  OB_TAIL_PUT(&tree[0], -1);
	
  for(i = 0; i < no_rows; i ++)
  {
    memcpy(&tree[1 + i],
           &globals->common->alerttad[AL_TEXT],
           sizeof(OBJECT));
    memcpy(&ti[i],
           (TEDINFO *)OB_SPEC(&globals->common->alerttad[AL_TEXT]),
           sizeof(TEDINFO));
    OB_WIDTH_PUT(&tree[i + 1], (WORD)(strlen(text) * cwidth));
    OB_HEIGHT_PUT(&tree[i + 1], globals->common->clheight);
    OB_SPEC_PUT(&tree[i + 1], &ti[i]);
	
    TE_PTEXT_PUT(OB_SPEC(&tree[i + 1]), text);
	
    OB_FLAGS_CLEAR(&tree[i + 1], LASTOB);

    if(OB_WIDTH(&tree[i + 1]) > textwidth)
    {
      textwidth = OB_WIDTH(&tree[i + 1]);
    }

    while(*text)
    {
      text++;
    }
			
    text++;
	
    do_objc_add(tree,0,i + 1);
  }

  for(i = 0; i < no_butts; i ++)
  {
    memcpy(&tree[1 + i + no_rows],
           &globals->common->alerttad[AL_BUTTON],
           sizeof(OBJECT));
	
    OB_Y_PUT(&tree[i + 1 + no_rows], no_rows * globals->common->clheight + 20);
    OB_HEIGHT_PUT(&tree[i + 1 + no_rows], globals->common->clheight);
	
    OB_SPEC_PUT(&tree[i + 1 + no_rows], buttons);

    OB_FLAGS_CLEAR(&tree[i + 1 + no_rows], LASTOB);

    width = (WORD)(strlen(buttons) * cwidth);

    if(width > buttonwidth)
      buttonwidth = width;
	
    while(*buttons)
      buttons++;
			
    buttons++;
	
    do_objc_add(tree,0,i + 1 + no_rows);
  }
	
  memcpy(&tree[1 + no_butts + no_rows],
         &globals->common->alerttad[AL_ICON],
         sizeof(OBJECT));

  do_objc_add(tree,0,1 + no_butts + no_rows);

  switch(*icon)
  {
  case '1':
    OB_SPEC_PUT(&tree[1 + no_butts + no_rows],
                OB_SPEC(&globals->common->aiconstad[AIC_EXCLAMATION]));
    break;

  case '2':
    OB_SPEC_PUT(&tree[1 + no_butts + no_rows],
                OB_SPEC(&globals->common->aiconstad[AIC_QUESTION]));
    break;

  case '3':
    OB_SPEC_PUT(&tree[1 + no_butts + no_rows],
                OB_SPEC(&globals->common->aiconstad[AIC_STOP]));
    break;

  case '4':
    OB_SPEC_PUT(&tree[1 + no_butts + no_rows],
                OB_SPEC(&globals->common->aiconstad[AIC_INFO]));
    break;

  case '5':
    OB_SPEC_PUT(&tree[1 + no_butts + no_rows],
                OB_SPEC(&globals->common->aiconstad[AIC_DISK]));
    break;

  default:
    OB_FLAGS_SET(&tree[1 + no_butts + no_rows], HIDETREE);
  }
		
  buttonwidth += 2;

  if(def)
  {
    OB_FLAGS_SET(&tree[no_rows + def], DEFAULT);
  }
	
  OB_FLAGS_SET(&tree[no_rows + no_butts + 1], LASTOB);

  OB_WIDTH_PUT(&tree[0], (buttonwidth + 10) * no_butts + 10);
	
  if((textwidth + 28 + OB_WIDTH(&tree[1 + no_butts + no_rows])) >
     OB_WIDTH(&tree[0]))
  {
    OB_WIDTH_PUT(&tree[0], textwidth + 28 +
                 OB_WIDTH(&tree[1 + no_butts + no_rows]));
  }
	
  OB_HEIGHT_PUT(&tree[0], globals->common->clheight * no_rows + 45);
	
  for(i = 0; i < no_rows; i++)
  {
    OB_X_PUT(&tree[i + 1],
             (OB_WIDTH(&tree[0]) - textwidth -
              OB_WIDTH(&tree[1 + no_butts + no_rows])) / 2 +
             OB_WIDTH(&tree[1 + no_butts + no_rows]) + 8);
    OB_Y_PUT(&tree[i + 1], i * globals->common->clheight + 10);
  }
	
  for(i = 0; i < no_butts; i++)
  {
    OB_X_PUT(&tree[i + no_rows + 1],
             (buttonwidth + 10) * i +
             ((OB_WIDTH(&tree[0]) - (buttonwidth + 10) * no_butts + 10) >> 1));
    OB_WIDTH_PUT(&tree[i + 1 + no_rows], buttonwidth);
  }
	
  Form_do_center (apid,tree,&clip);
  
  Form_do_dial(apid,FMD_START,&clip,&clip);
  
  Objc_do_draw (globals->vid, tree,0,9,&clip);
  
  but_chosen = Form_do_do (apid, tree, 0) & 0x7fff;
  
  Form_do_dial(apid,FMD_FINISH,&clip,&clip);
  
  free(ti);
  free(tree);
  free(s);
	
  return but_chosen - no_rows;
}


/*
** Exported
*/
void
Form_alert (AES_PB *apb)
{
  CHECK_APID(apb->global->apid);

  apb->int_out[0] = Form_do_alert(apb->global->apid,
                                  apb->int_in[0],
                                  (BYTE *)apb->addr_in[0]);
}


/*
** Exported
**
** 1998-12-19 CG
*/
WORD
Form_do_error (WORD apid,
               WORD error) {
  BYTE	s[100];
  BYTE	*sp = s;
  GLOBAL_APPL * globals = get_globals (apid);

  switch(error)
  {
  case	FERR_FILENOTFOUND:
  case	FERR_PATHNOTFOUND:
  case	FERR_NOFILES:
    sp = globals->common->fr_string[ERROR_2_3_18];
    break;
  case	FERR_NOHANDLES:
    sp = globals->common->fr_string[ERROR_4];
    break;
  case	FERR_ACCESSDENIED:
    sp = globals->common->fr_string[ERROR_5];
    break;
  case	FERR_LOWMEM:
  case	FERR_BADENVIRON:
  case	FERR_BADFORMAT:
    sp = globals->common->fr_string[ERROR_8_10_11];
    break;
  case	FERR_BADDRIVE:
    sp = globals->common->fr_string[ERROR_15];
    break;
  case	FERR_DELETEDIR:
    sp = globals->common->fr_string[ERROR_16];
    break;
  default:
    sprintf(s,globals->common->fr_string[ERROR_GENERAL],error);
  };
	
  return Form_do_alert (apid, 1, sp);
}


/*
** Exported
*/
void
Form_error (AES_PB * apb)
{
  CHECK_APID(apb->global->apid);

  apb->int_out[0] = Form_do_error(apb->global->apid,
				  apb->int_in[0]);
}


/*form_center 0x0036*/

/*
** Exported
*/
void
Form_do_center (WORD     apid,
                OBJECT * tree,
                RECT   * clip) {
  WORD pw1, pw2, pw3, pw4;
  
  Wind_do_get (apid,
               0,
               WF_WORKXYWH,
               &pw1,
               &pw2,
               &pw3,
               &pw4,
               TRUE);
  OB_X_PUT(&tree[0], pw1 + ((pw3 - OB_WIDTH(&tree[0])) >> 1));
  OB_Y_PUT(&tree[0], pw2 + ((pw4 - OB_HEIGHT(&tree[0])) >> 1));

  Objc_area_needed (tree, 0, clip);
}


void
Form_center(AES_PB *apb)
{
  CHECK_APID(apb->global->apid);

  Form_do_center (apb->global->apid,
                  (OBJECT *)apb->addr_in[0],
                  (RECT *)&apb->int_out[1]);
}


/*
** Description
** Process key input to form
*/
WORD
Form_do_keybd(WORD     apid,
              OBJECT * tree,
              WORD     obj,
              WORD     kc,
              WORD   * newobj,
              WORD   * keyout)
{
  WORD internal_kc;

  if((kc & 0xff) != 0)
  {
    /* Don't worry about scancodes for anything but special keys */
    internal_kc = kc & 0xff;
  }
  else
  {
    internal_kc = kc;
  }

  switch(internal_kc)
  {
  case 0x0009: /* tab */
  case 0x5000: /* arrow down */
  {
    WORD i = obj + 1;
			
    *newobj = obj;
    *keyout = 0;
			
    if((obj != 0) && !(OB_FLAGS(&tree[obj]) & LASTOB))
    {
      while(TRUE)
      {
        if(OB_FLAGS(&tree[i]) & EDITABLE)
        {
          *newobj = i;

          break;
        }
	
        if(OB_FLAGS(&tree[i]) & LASTOB)
        {
          break;
        }
				
        i++;
      }
    }
  }
  return 1;

  case 0x4800: /* arrow up */
  {
    WORD i = obj - 1;
			
    *newobj = obj;
    *keyout = 0;
			
    while(i >= 0)
    {
      if(OB_FLAGS(&tree[i]) & EDITABLE)
      {
        *newobj = i;

        break;
      }

      i--;
    }
  }
  return 1;

  case 0x000d: /* return */
  {
    WORD i = 0;
			
    *newobj = -1;
			
    while(TRUE)
    {
      if(OB_FLAGS(&tree[i]) & DEFAULT)
      {
        RECT clip;
					
        *newobj = i;
        *keyout = 0;
					
        Objc_calc_clip(tree,i,&clip);
        Objc_do_change (apid, tree,i,&clip,SELECTED,REDRAW);
					
        return 0;
      }
	
      if(OB_FLAGS(&tree[i]) & LASTOB)
      {
        break;
      }
				
      i++;
    }
  }
  break;
  }	
	
  *newobj = obj;
  *keyout = kc;

  return 1;
}


/****************************************************************************
 *  Form_keybd                                                              *
 *   0x0037 form_keybd()                                                    *
 ****************************************************************************/
void              /*                                                        */
Form_keybd(       /*                                                        */
AES_PB *apb)      /* AES parameter block.                                   */
/****************************************************************************/
{
  CHECK_APID(apb->global->apid);

  apb->int_out[0] = Form_do_keybd (apb->global->apid,
                                   (OBJECT *)apb->addr_in[0],
                                   apb->int_in[0],
                                   apb->int_in[1],&apb->int_out[1],
                                   &apb->int_out[2]);
}


/*
** Exported
*/
WORD
Form_do_button (WORD     apid,
                OBJECT * tree,
                WORD     obj,
                WORD     clicks,
                WORD *newobj) {
  WORD	dummy;

  *newobj = 0;

  if(OB_FLAGS(&tree[obj]) & (EXIT | SELECTABLE))
  {
    RECT clip;
		
    if(OB_FLAGS(&tree[obj]) & RBUTTON)
    {
      if(!(OB_STATE(&tree[obj]) & SELECTED))
      {
        WORD i = obj;
				
        while(1)
        {
          if(OB_TAIL(&tree[OB_NEXT(&tree[i])]) == i)
          {
            i = OB_HEAD(&tree[OB_NEXT(&tree[i])]);
          }
          else
          {
            i = OB_NEXT(&tree[i]);
          }
					
          if(i == obj)
          {
            break;
          }
					
          if(OB_STATE(&tree[i]) & SELECTED)
          {
            Objc_calc_clip(tree,i,&clip);
            OB_STATE_CLEAR(&tree[i], SELECTED);
            Objc_do_change(apid,
                           tree,
                           i,
                           &clip,
                           OB_STATE(&tree[i]),
                           REDRAW);
          }
        }

        Objc_calc_clip(tree,obj,&clip);
        OB_STATE_SET(&tree[i], SELECTED);
        Objc_do_change(apid,
                       tree,
                       obj,
                       &clip,
                       OB_STATE(&tree[i]),
                       REDRAW);
      }
			
      Evnt_do_button (apid,
                      0,
                      LEFT_BUTTON,
                      0,
                      &dummy,
                      &dummy,
                      &dummy,
                      &dummy);
			
      if(OB_FLAGS(&tree[obj]) & (TOUCHEXIT | EXIT))
      {
        *newobj = obj;
				
        if((OB_FLAGS(&tree[obj]) & TOUCHEXIT) && (clicks >= 2))
        {
          *newobj |= 0x8000;
        }

        return 0;
      }
      else {
        return 1;
      };
    }
    else
    {
      WORD instate = OB_STATE(&tree[obj]);
      WORD outstate = instate;

      if(OB_FLAGS(&tree[obj]) & SELECTABLE)
      {
        instate ^= SELECTED;
      }
	
      if ((Graf_do_watchbox (apid, tree, obj, instate, outstate) == 1) &&
         (OB_FLAGS(&tree[obj]) & (EXIT | TOUCHEXIT)))
      {
	
        *newobj = obj;
	
        if((OB_FLAGS(&tree[obj]) & TOUCHEXIT) && (clicks >= 2))
        {
          *newobj |= 0x8000;
        }
				
        return 0;
      }
      else {
        return 1;
      };
    };
  }
  else if(OB_FLAGS(&tree[obj]) & TOUCHEXIT)
  {
    *newobj = obj;
		
    if(clicks >= 2) {
      *newobj |= 0x8000;
    }
	
    return 0;
  }
  else if(OB_FLAGS(&tree[obj]) & EDITABLE)
  {
    *newobj = obj;
		
    return 1;
  }
		
  return 1;
}


/*
** Exported
*/
void
Form_button (AES_PB *apb)
{
  CHECK_APID(apb->global->apid);

  apb->int_out[0] = Form_do_button (apb->global->apid,
                                    (OBJECT *)apb->addr_in[0],
                                    apb->int_in[0],
                                    apb->int_in[1],
                                    &apb->int_out[1]);
}
