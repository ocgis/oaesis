/****************************************************************************

 Module
  form.c
  
 Description
  Form handling routines in oAESis.
  
 Author(s)
 	cg (Christer Gustavsson <d2cg@dtek.chalmers.se>)

 Revision history
 
  960101 cg
   Added standard header.
   Basic form_keybd() implemented with Form_keybd() and Form_do_keybd().
 
  960419 cg
   Fixed error in form_keybd(). The Compendium was wrong (again!).
   
 Copyright notice
  The copyright to the program code herein belongs to the authors. It may
  be freely duplicated and distributed without fee, but not charged for.
 
 ****************************************************************************/

/****************************************************************************
 * Used interfaces                                                          *
 ****************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_MINTBIND_H
#include <mintbind.h>
#endif

#include <stdio.h>
#include <string.h>

#include "debug.h"
#include "evnt.h"
#include "evnthndl.h"
#include "form.h"
#include "gemdefs.h"
#include "lib_global.h"
#include "graf.h"
#include "mintdefs.h"
#include "objc.h"
#include "resource.h"
#include "srv_calls.h"
#include "types.h"
#include "wind.h"

/****************************************************************************
 * Local functions (use static!)                                            *
 ****************************************************************************/

/****************************************************************************
 * Public functions                                                         *
 ****************************************************************************/

/*form_do 0x0032*/

/*
** Exported
**
** 1998-12-19 CG
*/
WORD
Form_do_do(WORD     apid,
           OBJECT * tree,
           WORD editobj) {
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

  if(editobj != 0) {
    Objc_do_edit (apid, tree,editobj,0,&idx,ED_INIT);
  };

  Evhd_wind_update(apid,BEG_MCTRL);
	
  while(1) {
    Evnt_do_multi (apid, &ei, (COMMSG *)buffer, &eo, 0);

    if(eo.events & MU_BUTTON) {
      object = Objc_do_find(tree,0,9,eo.mx,eo.my,0);

      if(object >= 0) {
        if(!Form_do_button(apid, tree, object, eo.mc, &newobj)) {
          if(editobj != 0) {
            Objc_do_edit (apid, tree,editobj,0,&idx,ED_END);
          };

          Evhd_wind_update(apid,END_MCTRL);
          return newobj;
        }
        else {
          if((newobj != 0) && (newobj != editobj)) {
            if(editobj != 0) {
              Objc_do_edit (apid, tree,editobj,0,&idx,ED_END);
            };
				
            editobj = newobj;

            Objc_do_edit (apid, tree,editobj,0,&idx,ED_INIT);
          };
        };
      };
    };
		
    if(eo.events & MU_KEYBD) {
      if(!Form_do_keybd (apid, tree,editobj,eo.kc,&newobj,&keyout)) {
        if(editobj != 0) {
          Objc_do_edit (apid, tree,editobj,0,&idx,ED_END);
        };

        Evhd_wind_update(apid,END_MCTRL);
        return newobj;
      }
      else if(newobj != editobj) {
        if(editobj != 0) {
          Objc_do_edit (apid, tree,editobj,0,&idx,ED_END);
        };
				
        editobj = newobj;

        if(editobj != 0) {
          Objc_do_edit (apid, tree,editobj,0,&idx,ED_INIT);
        };
      }
      else {
        if((editobj != 0) && (keyout != 0)) {
          Objc_do_edit (apid, tree,editobj,keyout,&idx,ED_CHAR);
        };
      };
    };
  };
}


/*
** Exported
**
** 1998-12-20 CG
*/
void
Form_do (AES_PB *apb) {
  apb->int_out[0] = Form_do_do (apb->global->apid,
                                (OBJECT *)apb->addr_in[0],
                                apb->int_in[0]);		
}

/*form_dial 0x0033*/

/*
** Exported
**
** 1998-12-20 CG
*/
WORD
Form_do_dial (WORD   apid,
              WORD   mode,
              RECT * r1,
              RECT * r2) {
  GLOBAL_APPL * globals = get_globals (apid);
    
  switch(mode) {
  case	FMD_GROW		:	/*0x0001*/
    if(globals->common->graf_growbox) {
      Evhd_wind_update(Pgetpid(),BEG_UPDATE);
      Graf_do_grmobox(r1,r2);
      Evhd_wind_update(Pgetpid(),END_UPDATE);
    };
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
        };
      };

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
    if(globals->common->graf_shrinkbox) {
      Evhd_wind_update(Pgetpid(),BEG_UPDATE);
      Graf_do_grmobox(r2,r1);
      Evhd_wind_update(Pgetpid(),END_UPDATE);
    };

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

void	Form_dial(AES_PB *apb) {
  apb->int_out[0] = Form_do_dial(apb->global->apid,
				 apb->int_in[0],
				 (RECT *)&apb->int_in[1],
				 (RECT *)&apb->int_in[5]);
};

/*
** Description
** Implementation of form_alert ()
**
** 1998-12-19 CG
*/
static
WORD
Form_do_alert (WORD   apid,
               WORD   def,
               BYTE * alertstring) {
  BYTE	*s = (BYTE *)Mxalloc(strlen(alertstring) + 1,PRIVATEMEM);
	
  WORD	i = 0;
	
  WORD	no_rows = 1,no_butts = 1;
	
  WORD	cwidth,cheight,width,height;
	
  WORD	but_chosen;
	
  OBJECT	*tree;
  TEDINFO	*ti;
	
  RECT	 clip;
	
  BYTE	 *icon,*text,*buttons;

  WORD	 textwidth = 0,buttonwidth = 0;
  GLOBAL_APPL * globals = get_globals (apid);
	
  Graf_do_handle(&cwidth,&cheight,&width,&height);
	
  strcpy(s,alertstring);
	
  while(s[i] != '[')
    i++;
	
  icon = &s[i + 1];
	
  while(s[i] != ']')
    i++;
	
  s[i] = 0;
		
  while(s[i] != '[')
    i++;
		
  text = &s[i + 1];
	
  while(s[i] != ']')
  {
    if(s[i] == '|')
    {
      s[i] = 0;
      no_rows++;
    };
		
    i++;
  };
	
  s[i] = 0;
	
  while(s[i] != '[')
    i++;
		
  buttons = &s[i + 1];
	
  while(s[i] != ']')
  {
    if(s[i] == '|')
    {
      s[i] = 0;
      no_butts++;
    };
		
    i++;
  };
		
  s[i] = 0;
	
  tree = (OBJECT *)Mxalloc((2 + no_butts + no_rows) * sizeof(OBJECT)
                           ,PRIVATEMEM);
	
  ti = (TEDINFO *)Mxalloc(no_rows * sizeof(TEDINFO),PRIVATEMEM);
	
  memcpy(&tree[0],&globals->common->alerttad[0],sizeof(OBJECT));
	
  tree[0].ob_head = -1;
  tree[0].ob_tail = -1;
	
  for(i = 0; i < no_rows; i ++) {
    memcpy(&tree[1 + i],&globals->common->alerttad[AL_TEXT],sizeof(OBJECT));
    memcpy(&ti[i],globals->common->alerttad[AL_TEXT].ob_spec.tedinfo
           ,sizeof(TEDINFO));
    tree[i + 1].ob_width = (WORD)(strlen(text) * cwidth);
    tree[i + 1].ob_height = globals->common->clheight;
    tree[i + 1].ob_spec.tedinfo = &ti[i];
	
    tree[i + 1].ob_spec.tedinfo->te_ptext = text;
	
    tree[i + 1].ob_flags &= ~LASTOB;

    if(tree[i + 1].ob_width > textwidth)
      textwidth = tree[i + 1].ob_width;

    while(*text)
      text++;
			
    text++;
	
    do_objc_add(tree,0,i + 1);
  };

  for(i = 0; i < no_butts; i ++) {
    memcpy(&tree[1 + i + no_rows],&globals->common->alerttad[AL_BUTTON],sizeof(OBJECT));
	
    tree[i + 1 + no_rows].ob_y = no_rows * globals->common->clheight + 20;

    tree[i + 1 + no_rows].ob_height = globals->common->clheight;
	
    tree[i + 1 + no_rows].ob_spec.free_string = buttons;

    tree[i + 1 + no_rows].ob_flags &= ~LASTOB;

    width = (WORD)(strlen(buttons) * cwidth);

    if(width > buttonwidth)
      buttonwidth = width;
	
    while(*buttons)
      buttons++;
			
    buttons++;
	
    do_objc_add(tree,0,i + 1 + no_rows);
  };
	
  memcpy(&tree[1 + no_butts + no_rows],&globals->common->alerttad[AL_ICON],sizeof(OBJECT));

  do_objc_add(tree,0,1 + no_butts + no_rows);

  switch(*icon) {
  case '1':
    tree[1 + no_butts + no_rows].ob_spec.index = globals->common->aiconstad[AIC_EXCLAMATION].ob_spec.index;
    break;

  case '2':
    tree[1 + no_butts + no_rows].ob_spec.index = globals->common->aiconstad[AIC_QUESTION].ob_spec.index;
    break;

  case '3':
    tree[1 + no_butts + no_rows].ob_spec.index = globals->common->aiconstad[AIC_STOP].ob_spec.index;
    break;

  case '4':
    tree[1 + no_butts + no_rows].ob_spec.index = globals->common->aiconstad[AIC_INFO].ob_spec.index;
    break;

  case '5':
    tree[1 + no_butts + no_rows].ob_spec.index = globals->common->aiconstad[AIC_DISK].ob_spec.index;
    break;

  default:
    tree[1 + no_butts + no_rows].ob_flags |= HIDETREE;		
  };
		
  buttonwidth += 2;

  if(def) {
    tree[no_rows + def].ob_flags |= DEFAULT;
  };
	
  tree[no_rows + no_butts + 1].ob_flags |= LASTOB;

  tree[0].ob_width = (buttonwidth + 10) * no_butts + 10;
	
  if(textwidth + 28 + tree[1 + no_butts + no_rows].ob_width
     > tree[0].ob_width) {
    tree[0].ob_width = textwidth + 28 +
      tree[1 + no_butts + no_rows].ob_width;
  };
	
  tree[0].ob_height = globals->common->clheight * no_rows + 45;
	
  for(i = 0; i < no_rows; i++) {
    tree[i + 1].ob_x = (tree[0].ob_width - textwidth - 
			tree[1 + no_butts + no_rows].ob_width) / 2 +
      tree[1 + no_butts + no_rows].ob_width + 8;
    tree[i + 1].ob_y = i * globals->common->clheight + 10;
  };
	
  for(i = 0; i < no_butts; i++) {
    tree[i + no_rows + 1].ob_x = (buttonwidth + 10) * i
      + ((tree[0].ob_width - (buttonwidth + 10) * no_butts
          +10) >> 1);
    tree[i + 1 + no_rows].ob_width = buttonwidth;
  };
	
  Form_do_center (apid,tree,&clip);

  Form_do_dial(apid,FMD_START,&clip,&clip);

  Objc_do_draw (apid, tree,0,9,&clip);

  but_chosen = Form_do_do (apid, tree, 0) & 0x7fff;
	
  Form_do_dial(apid,FMD_FINISH,&clip,&clip);

  Mfree(ti);
  Mfree(tree);
  Mfree(s);
	
  return but_chosen - no_rows;
}


/*
** Exported
**
** 1998-12-20 CG
*/
void
Form_alert (AES_PB *apb) {
  apb->int_out[0] = Form_do_alert (apb->global->apid,
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
**
** 1998-12-19 CG
*/
void
Form_error (AES_PB * apb) {
  apb->int_out[0] = Form_do_error(apb->global->apid,
				  apb->int_in[0]);
}

/*form_center 0x0036*/

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
    tree[0].ob_x = pw1 + ((pw3 - tree[0].ob_width) >> 1);
    tree[0].ob_y = pw2 + ((pw4 - tree[0].ob_height) >> 1);
	
	Objc_area_needed(tree,0,clip);
}


void
Form_center (AES_PB *apb) {
  Form_do_center (apb->global->apid,
                  (OBJECT *)apb->addr_in[0],
                  (RECT *)&apb->int_out[1]);
};

/****************************************************************************
 *  Form_do_keybd                                                           *
 *   Process key input to form.                                             *
 ****************************************************************************/
WORD              /* 0 if an exit object was selected, or 1.                */
Form_do_keybd(    /*                                                        */
WORD     apid,
OBJECT * tree,    /* Resource tree of form.                                 */
WORD     obj,     /* Object with edit focus (0 => none).                    */
WORD     kc,      /* Keypress to process.                                   */
WORD   * newobj,  /* New object with edit focus.                            */
WORD   * keyout)  /* Keypress that couldn't be processed.                   */
/****************************************************************************/
{
  switch(kc) {
  case 0x0f09: /* tab */
  case 0x5000: /* arrow down */
  {
    WORD i = obj + 1;
			
    *newobj = obj;
    *keyout = 0;
			
    if((obj != 0) && !(tree[obj].ob_flags & LASTOB)) {
      while(1) {
        if(tree[i].ob_flags & EDITABLE) {
          *newobj = i;

          break;
        };
	
        if(tree[i].ob_flags & LASTOB) {
          break;
        };
				
        i++;
      };
    };
  };
  return 1;
  case 0x4800: /* arrow up */
  {
    WORD i = obj - 1;
			
    *newobj = obj;
    *keyout = 0;
			
    while(i >= 0) {
      if(tree[i].ob_flags & EDITABLE) {
        *newobj = i;

        break;
      };

      i--;
    };
  };
  return 1;
  case 0x1c0d: /* return */
  {
    WORD i = 0;
			
    *newobj = -1;
			
    while(1) {
      if(tree[i].ob_flags & DEFAULT) {
        RECT clip;
					
        *newobj = i;
        *keyout = 0;
					
        Objc_calc_clip(tree,i,&clip);
        Objc_do_change (apid, tree,i,&clip,SELECTED,REDRAW);
					
        return 0;
      };
	
      if(tree[i].ob_flags & LASTOB) {
        break;
      };
				
      i++;
    };
  };
  break;
  };	
	
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
  apb->int_out[0] = Form_do_keybd (apb->global->apid,
                                   (OBJECT *)apb->addr_in[0],
                                   apb->int_in[0],
                                   apb->int_in[1],&apb->int_out[1],
                                   &apb->int_out[2]);
}


/*
** Exported
**
** 1998-12-19 CG
*/
WORD
Form_do_button (WORD     apid,
                OBJECT * tree,
                WORD     obj,
                WORD     clicks,
                WORD *newobj) {
  WORD	dummy;

  *newobj = 0;

  if(tree[obj].ob_flags & (EXIT | SELECTABLE)) {
    RECT clip;
		
    if(tree[obj].ob_flags & RBUTTON) {
      if(!(tree[obj].ob_state & SELECTED)) {
        WORD i = obj;
				
        while(1) {
          if(tree[tree[i].ob_next].ob_tail == i) {
            i = tree[tree[i].ob_next].ob_head;
          }
          else {
            i = tree[i].ob_next;
          };
					
          if(i == obj) {
            break;
          };
					
          if(tree[i].ob_state & SELECTED) {
            Objc_calc_clip(tree,i,&clip);
            Objc_do_change (apid, tree,i,&clip,
                           tree[i].ob_state &= ~SELECTED,REDRAW);
          };					
        };				

        Objc_calc_clip(tree,obj,&clip);
        Objc_do_change (apid, tree,obj,&clip,
                       tree[i].ob_state |= SELECTED,REDRAW);
      };
			
      Evnt_do_button (apid,
                      0,
                      LEFT_BUTTON,
                      0,
                      &dummy,
                      &dummy,
                      &dummy,
                      &dummy);
			
      if(tree[obj].ob_flags & (TOUCHEXIT | EXIT)) {
        *newobj = obj;
				
        if((tree[obj].ob_flags & TOUCHEXIT) && (clicks >= 2)) {
          *newobj |= 0x8000;
        };

        return 0;
      }
      else {
        return 1;
      };
    }
    else {
      WORD instate = tree[obj].ob_state;
      WORD outstate = instate;

      if(tree[obj].ob_flags & SELECTABLE) {
        instate ^= SELECTED;
      };
	
      if ((Graf_do_watchbox (apid, tree, obj, instate, outstate) == 1) &&
         (tree[obj].ob_flags & (EXIT | TOUCHEXIT))) {
	
        *newobj = obj;
	
        if((tree[obj].ob_flags & TOUCHEXIT) && (clicks >= 2)) {
          *newobj |= 0x8000;
        };
				
        return 0;
      }
      else {
        return 1;
      };
    };
  }
  else if(tree[obj].ob_flags & TOUCHEXIT) {
    *newobj = obj;
		
    if(clicks >= 2) {
      *newobj |= 0x8000;
    };
	
    return 0;
  }
  else if(tree[obj].ob_flags & EDITABLE) {
    *newobj = obj;
		
    return 1;
  };
		
  return 1;
}


/*
** Exported
**
** 1998-12-19 CG
*/
void
Form_button (AES_PB *apb) {
  apb->int_out[0] = Form_do_button (apb->global->apid,
                                    (OBJECT *)apb->addr_in[0],
                                    apb->int_in[0],
                                    apb->int_in[1],
                                    &apb->int_out[1]);
}
