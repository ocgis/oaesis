/*
** objc.c
**
** Copyright 1995 - 2001 Christer Gustavsson <cg@nocrew.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**  
** Read the file COPYING for more information.
**
*/

#define DEBUGLEVEL 0


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_MINTBIND_H
#include <mintbind.h>
#endif

#include <stdio.h>
#include <string.h>
#include <vdibind.h>

#include "aesbind.h"
#include "debug.h"
#include "lib_global.h"
#include "mintdefs.h"
#include "lib_misc.h"
#include "objc.h"
#include "types.h"


#define	SWRM_TRANS    0
#define	SWRM_REPLACE  1
#define	SWRM_INVTRANS 4

#define FLD3DANY      0x0600


typedef struct tiext
{
  unsigned framecol    : 4;
  unsigned textcol     : 4;
  unsigned textmode    : 1;
  unsigned fillpattern : 3;
  unsigned interiorcol : 4;
} TIEXT;


static WORD numplanes;

static WORD textyoffs = 8;

static struct
{
  WORD br;        /* Colour used for bottom right 3d edge */
  WORD tl;        /* Colour used for top left 3d edge */
  WORD border;    /* Colour used for edging */
  WORD highlight; /* Colour used for highlighting */
  
  WORD colour_ind; /* Default indicator colour */
  WORD move_ind;   /* Indicates whether indicator text should be moved */
  WORD alter_ind;  /* Indicates whether indicator colour should be changed */
  
  WORD colour_act; /* Default activator colour */
  WORD move_act;   /* Indicates whether activator text should be moved */
  WORD alter_act;  /* Indicates whether activator colour should be changed */
  
  WORD colour_bkg; /* Default background colour */
} ocolours =
{
  LBLACK,
  WHITE,
  BLACK,
  BLUE,
  LWHITE, 1, 1,
  LWHITE, 1, 0,
  LWHITE
};


/* FIXME: Use table instead */
static
WORD
invertcolor(WORD color)
{
  switch(color)
  {
  case BLACK:
    return WHITE;
  case WHITE:
    return BLACK;
  case BLUE:
    return LYELLOW;
  case LYELLOW:
    return BLUE;
  case RED:
    return LCYAN;
  case LCYAN:
    return RED;
  case GREEN:
    return LMAGENTA;
  case LMAGENTA:
    return GREEN;
  case CYAN:
    return LRED;
  case LRED:
    return CYAN;
  case YELLOW:
    return LBLUE;
  case LBLUE:
    return YELLOW;
  case MAGENTA:
    return LGREEN;
  case LGREEN:
    return MAGENTA;
  case LWHITE:
    return LBLACK;
  case LBLACK:
    return LWHITE;
  default:
    return BLACK;
  }
}


static
void
setfilltype(WORD vid,
            WORD fill)
{
  switch(fill)
  {
  case 0:
    vsf_interior(vid,0);
    break;
  case 7:
    vsf_interior(vid,1);
    break;
  default:
    vsf_interior(vid,2);
    vsf_style(vid,fill);
    break;
  }
}


static
void
settxtalign(WORD vid,
            WORD alignment)
{
  int dum;
  
  vst_alignment (vid, alignment, 3, &dum, &dum);
}


static
void
set_write_mode(WORD vid,
               WORD mode)
{
  switch(mode)
  {
  case SWRM_TRANS: /*transparent*/
    vswr_mode(vid,MD_TRANS);
    break;
    
  case SWRM_REPLACE: /*replace*/
    vswr_mode(vid,MD_REPLACE);
    break;
    
  case SWRM_INVTRANS: /*0x0004*/
    vswr_mode(vid,MD_ERASE);
    break;
  }
}


static
void
settextsize (WORD vid,
             WORD size)
{
  GLOBAL_COMMON * globals = get_global_common ();
  int             dum;
  
  switch(size)
  {
  case	3:	/*large*/
    vst_font( vid, globals->fnt_regul_id);
    vst_point(vid,globals->fnt_regul_sz,&dum,&dum,&dum,&dum);
    
    textyoffs = globals->clheight >> 1;
    break;
    
  case	5:	/*small*/
    vst_font( vid, globals->fnt_small_id);
    vst_point(vid,globals->fnt_small_sz,&dum,&dum,&dum,&dum);
    
    textyoffs = globals->csheight >> 1;	
  }
}


static
void
drawtext(WORD   vid,
         BYTE * text,
         RECT * size,
         WORD   alignment,
         WORD   color,
         WORD   textsize,
         WORD   writemode,
         WORD   state)
{
  WORD x;
  WORD y;
  WORD txteff = 0;

  settextsize (vid, textsize);
  
  y = size->y + (size->height >> 1) + textyoffs - 1;
  
  switch(alignment)
  {
  case 0: /* left */
    x = size->x;
    settxtalign(vid,0);
    break;
  case 1: /*right*/
    x = size->x + size->width - 1;
    settxtalign(vid,2);
    break;
  case 2: /*center*/
    x = size->x + (size->width >> 1);
    settxtalign(vid,1);
    break;
    
  default:
    x = size->x;   /* Just in case */
  }
	
  if(state & DISABLED)
  {
    txteff |= LIGHT;
  }
  
  vst_effects(vid,txteff);
  
  if(state & SELECTED)
  {
    set_write_mode(vid,writemode);
    
    vst_color(vid,invertcolor(color));
    v_gtext(vid,x,y,text);
    
    if(writemode == SWRM_REPLACE)
    {
      set_write_mode(vid,SWRM_INVTRANS);
      vst_color(vid,BLACK);
      v_gtext(vid,x,y,text);
    }
  }
  else
  {
    set_write_mode(vid,writemode);
    vst_color(vid,color);
    v_gtext(vid,x,y,text);
  }
}


/*
** Description
** Draw text portion of an object
*/
static
void
draw_text(WORD     vid,
          OBJECT * ob,
          WORD     par_x,
          WORD     par_y,
          WORD     is_top)
{
  WORD            txteff = 0;
  WORD            type = OB_TYPE(ob) & 0xff;
  BYTE            ctext[100];
  BYTE *          text = NULL;
  int             temp;
  WORD            tx = par_x + OB_X(ob);
  WORD            ty = par_y + OB_Y(ob);
  WORD            tcolour = BLACK,bcolour = WHITE;
  WORD            bfill = 0;
  WORD            writemode = SWRM_TRANS;
  WORD            draw3d = OB_FLAGS(ob) & FLD3DANY;
  WORD            invertcolour = 0;
  U_OB_SPEC       obspec;
  GLOBAL_COMMON * globals = get_global_common ();

  if(OB_FLAGS(ob) & INDIRECT)
  {
    obspec = *((U_OB_SPEC *)OB_SPEC(ob));
  }
  else
  {
    obspec.index = OB_SPEC(ob);
  }

  /* find the text string to draw */
	
  switch(type)
  {
  case G_FTEXT:
  case G_FBOXTEXT:
  {
    BYTE * ptmplt = TE_PTMPLT(obspec.tedinfo);
    BYTE * ptext = TE_PTEXT(obspec.tedinfo);
    BYTE * pctext = ctext;

    if(*ptext == '@')
    {
      *ptext = '\0';
    }
							
    while(*ptmplt)
    {
      switch(*ptmplt)
      {
      case '_':
        if(*ptext)
        {
          *(pctext++) = *(ptext++);
        }
        else
        {
          *(pctext++) = '_';
        }
	
        ptmplt++;
        break;
					
      default:
        *(pctext++) = *(ptmplt++);
      }
    }
			
    *pctext = '\0';
    text = ctext;
			
    bfill = GET_TE_COLOR_PATTERN(TE_COLOR(obspec.tedinfo));
    bcolour = GET_TE_COLOR_FILLC(TE_COLOR(obspec.tedinfo));
  }
  break;
		
  case G_TEXT:
  case G_BOXTEXT:
    text = TE_PTEXT(obspec.tedinfo);

    bfill = GET_TE_COLOR_PATTERN(TE_COLOR(obspec.tedinfo));
    bcolour = GET_TE_COLOR_FILLC(TE_COLOR(obspec.tedinfo));
    break;
		
  case G_TITLE:
  case G_STRING:
  case G_BUTTON:
    text = obspec.free_string;
    break;

  case G_BOXCHAR:
    ctext[0] = GET_OBSPEC_CHARACTER(obspec.index);
    ctext[1] = 0;
    text = ctext;

    bfill = GET_OBSPEC_FILLPATTERN(obspec.index);
    bfill = GET_OBSPEC_INTERIORCOL(obspec.index);
  }

  /* set font, alignment, color and writemode */

  if(text)
  {		
    switch(type)
    {
    case G_TEXT:
    case G_BOXTEXT:
    case G_FTEXT:
    case G_FBOXTEXT:
    {
      TEDINFO *textblk = (TEDINFO *)obspec.tedinfo;

      switch(TE_FONT(textblk)) /* Set the correct text size & font */
      {
      case GDOS_PROP: /* Use a proportional SPEEDOGDOS font (AES4.1 style) */
      case GDOS_MONO: /* Use a monospaced SPEEDOGDOS font (AES4.1 style) */
      case GDOS_BITM: /* Use a GDOS bitmap font (AES4.1 style) */
        vst_font(vid, TE_FONTID(textblk));
        vst_point(vid, TE_FONTSIZE(textblk), &temp, &temp, &temp, &temp);
        ty = par_y + OB_Y(ob) + ((OB_HEIGHT(ob) - globals->clheight) / 2);
        break;
        
      case IBM: /* Use the standard system font (probably 10 point) */
        vst_font(vid, globals->fnt_regul_id);
        vst_point(vid, globals->fnt_regul_sz, &temp, &temp, &temp, &temp);
        ty = par_y + OB_Y(ob) + ((OB_HEIGHT(ob) - globals->clheight) / 2);
        break;

      case SMALL: /* Use the small system font (probably 8 point) */
        vst_font(vid, globals->fnt_small_id);
        vst_point(vid, globals->fnt_small_sz, &temp, &temp, &temp, &temp);
						
        ty = par_y + OB_Y(ob) + ((OB_HEIGHT(ob) - globals->csheight) / 2);
      }
				
      switch(TE_JUST(textblk))
      {
        /*Set text alignment - why on earth did */
        /* atari use a different horizontal alignment */
        /* code for GEM to the one the VDI uses? */
      case 0:
        tx = par_x + OB_X(ob);
        vst_alignment(vid, 0, 5, &temp, &temp);
        break;
      case 1:
        tx = par_x + OB_X(ob) + OB_WIDTH(ob) - 1;
        vst_alignment(vid, 2, 5, &temp, &temp);
        break;
      case 2:
        tx = par_x + OB_X(ob) + (OB_WIDTH(ob) >> 1);
        vst_alignment(vid, 1, 5, &temp, &temp);
      }
				
      tcolour = GET_TE_COLOR_TEXTC(TE_COLOR(obspec.tedinfo));

      if(draw3d)
      {
        writemode = SWRM_TRANS;
      }
      else
      {
        writemode = GET_TE_COLOR_OPAQUE(TE_COLOR(obspec.tedinfo));
      }
      break;
				
    case G_STRING:
    case G_TITLE:
      vst_font(vid, globals->fnt_regul_id);
      vst_point(vid, globals->fnt_regul_sz, &temp, &temp, &temp, &temp);
      ty = par_y + OB_Y(ob) + ((OB_HEIGHT(ob) - globals->clheight) / 2);

      tx = par_x + OB_X(ob);
      vst_alignment(vid, 0, 5, &temp, &temp);

      tcolour = BLACK;
		
      if(is_top)
      {
        writemode = SWRM_REPLACE;
      }
      else
      {
        writemode = SWRM_TRANS;
      }
      break;

    case G_BOXCHAR:
      vst_font(vid, globals->fnt_regul_id);
      vst_point(vid, globals->fnt_regul_sz, &temp, &temp, &temp, &temp);
      ty = par_y + OB_Y(ob) + ((OB_HEIGHT(ob) - globals->clheight) / 2);

      tx = par_x + OB_X(ob) + (OB_WIDTH(ob) >> 1);
      vst_alignment(vid, 1, 5, &temp, &temp);

      tcolour = GET_OBSPEC_TEXTCOL(obspec.index);
				
      if(draw3d)
      {
        writemode = SWRM_TRANS;
      }
      else
      {
        writemode = GET_OBSPEC_TEXTMODE(obspec.index);
      }
      break;			
				
    case G_BUTTON:
      vst_font(vid, globals->fnt_regul_id);
      vst_point(vid, globals->fnt_regul_sz, &temp, &temp, &temp, &temp);
      ty = par_y + OB_Y(ob) + ((OB_HEIGHT(ob) - globals->clheight) / 2);
      
      tx = par_x + OB_X(ob) + (OB_WIDTH(ob) >> 1);
      vst_alignment(vid, 1, 5, &temp, &temp);
      
      tcolour = BLACK;
      
      writemode = SWRM_TRANS;
    }
    }
    
			
    if(OB_STATE(ob) & DISABLED)
    {
      txteff |= LIGHT;
    }
		
    vst_effects(vid,txteff);

    if(draw3d)
    {
      if(OB_STATE(ob) & SELECTED)
      {
        if(((draw3d == FL3DIND) && ocolours.move_ind) ||
           ((draw3d == FL3DACT) && ocolours.move_act))
        {
          tx += D3DOFFS;
          ty += D3DOFFS;
        }
				
        if(((draw3d == FL3DIND) && ocolours.alter_ind) ||
           ((draw3d == FL3DACT) && ocolours.alter_act))
        {
          invertcolour = TRUE;
        }
      }
			
      if((bcolour == WHITE) && (bfill == 0))
      {
        switch(draw3d)
        {
        case FL3DACT:
          bcolour = ocolours.colour_act;
          break;

        case FL3DIND:
          bcolour = ocolours.colour_ind;
          break;
					
        case FL3DBAK:
          bcolour = ocolours.colour_bkg;
          break;
        }
				
        bfill = 7;
      }
    }
    else
    {
      if(OB_STATE(ob) & SELECTED)
      {
        invertcolour = TRUE;
      }
			
      if(bfill == 0) {
        bcolour = WHITE;
      }
    }
		
    if(!(bcolour < globals->num_pens))
    {
      bcolour = BLACK;
    }
		
    if(!(tcolour < globals->num_pens))
    {
      tcolour = BLACK;
    }
		
    if((tcolour == bcolour) && (bfill == 7))
    {
      tcolour = invertcolor(tcolour);
    }
		
    if(invertcolour)
    {
      tcolour = invertcolor(tcolour);
      bcolour = invertcolor(bcolour);
    }

    if((writemode == SWRM_REPLACE) && (bcolour != WHITE))
    {
      set_write_mode(vid, SWRM_INVTRANS);
      vst_color(vid, bcolour);
      v_gtext(vid, tx, ty, text);
			
      set_write_mode(vid, SWRM_TRANS);
    }
    else
    {
      set_write_mode(vid, writemode);
    }

    vst_color(vid, tcolour);
    v_gtext(vid, tx, ty, text);
  }
}


static
void
drawframe(WORD   vid,
          RECT * r,
          WORD   framesize)
{
  WORD incr;
  int  size[10];
  WORD thick;
  
  DEBUG3("objc.c: drawframe: framesize = %d", framesize);
  if(framesize)
  {
    if(framesize > 0)
    {
      thick = framesize;
      incr = 1;
      size[0] = size[6] = size[8] = r->x;
      size[1] = size[3] = size[9] = r->y;
      size[2] = size[4] = size[0] + r->width - 1;
      size[5] = size[7] = size[1] + r->height - 1;
    }
    else
    {
      thick =  -framesize;
      incr = -1;
      size[0] = size[6] = size[8] = r->x - 1;
      size[1] = size[3] = size[9] = r->y - 1;
      size[2] = size[4] = r->x + r->width;
      size[5] = size[7] = r->y + r->height;
    }
    
    while(thick > 0)
    {
      v_pline(vid, 5, size);
      thick--;
      if(!thick)
      {
        break;
      }
      
      
      size[0] += incr;
      size[1] += incr;
      size[2] -= incr;
      size[3] += incr;
      size[4] -= incr;
      size[5] -= incr;
      size[6] += incr;
      size[7] -= incr;
      size[8] += incr;
      size[9] += incr;		
    }
  }
}


static
void
draw_filled_rect(WORD   vid,
                 RECT * r)
{
  int size[4];
  
  size[0] = r->x;
  size[1] = r->y;
  size[2] = r->width + size[0] - 1;
  size[3] = r->height + size[1] - 1;
  vr_recfl(vid, size);
}


static
void
draw_bg(WORD     vid,
        OBJECT * ob,
        WORD     par_x,
        WORD     par_y)
{
  WORD            type = OB_TYPE(ob) & 0xff;
  RECT            r;
  WORD            draw = 1;
  WORD            filltype = 0;
  WORD            fillcolour = WHITE;
  WORD            bcolour = WHITE;
  WORD            mode3d;
  WORD            invertcolour = 0;
  U_OB_SPEC       obspec;
  GLOBAL_COMMON * globals = get_global_common ();

  if(OB_FLAGS(ob) & INDIRECT)
  {
    obspec = *((U_OB_SPEC *)OB_SPEC(ob));
  }
  else
  {
    obspec.index = OB_SPEC(ob);
  }

  switch(type)
  {
  case G_BOXCHAR:
  case G_BOX:
    r.x      = OB_X(ob) + par_x;
    r.y      = OB_Y(ob) + par_y;
    r.width  = OB_WIDTH(ob);
    r.height = OB_HEIGHT(ob);
						
    fillcolour = GET_OBSPEC_INTERIORCOL(obspec.index);
    filltype = GET_OBSPEC_FILLPATTERN(obspec.index);
    break;
		
  case G_BOXTEXT:
  case G_FBOXTEXT:
  {
    r.x      = OB_X(ob) + par_x;
    r.y      = OB_Y(ob) + par_y;
    r.width  = OB_WIDTH(ob);
    r.height = OB_HEIGHT(ob);

    fillcolour = GET_TE_COLOR_FILLC(TE_COLOR(obspec.tedinfo));
    filltype = GET_TE_COLOR_PATTERN(TE_COLOR(obspec.tedinfo));
  }
  break;

  case G_STRING:
  case G_TITLE:
    if(OB_STATE(ob) & SELECTED)
    {
      r.x      = OB_X(ob) + par_x;
      r.y      = OB_Y(ob) + par_y;
      r.width  = OB_WIDTH(ob);
      r.height = OB_HEIGHT(ob);
			
      fillcolour = WHITE;

      if(OB_FLAGS(ob) & FLD3DANY)
      {
        filltype = 0;
      }
      else
      {
        filltype = 7;
      }
    }
    else
    {
      draw = 0;
    }
    break;

  case G_BUTTON:
    r.x      = OB_X(ob) + par_x;
    r.y      = OB_Y(ob) + par_y;
    r.width  = OB_WIDTH(ob);
    r.height = OB_HEIGHT(ob);
		
    fillcolour = WHITE;

    if(OB_FLAGS(ob) & FLD3DANY)
    {
      filltype = 0;
    }
    else
    {
      filltype = 7;
    }
    break;	
		
  default:
    draw = 0;
  }

  if(draw)
  {
    mode3d = OB_FLAGS(ob) & FLD3DANY;
		
    if(mode3d)
    {
      if((mode3d == FL3DACT) || (mode3d == FL3DIND))
      {
        r.x--;
        r.y--;
        r.width += 2;
        r.height += 2;
      }
			
      if((fillcolour == WHITE) && (filltype == 0))
      {
        switch(mode3d)
        {
        case FL3DACT:
          fillcolour = ocolours.colour_act;
          break;

        case FL3DIND:
          fillcolour = ocolours.colour_ind;
          break;
					
        case FL3DBAK:
          fillcolour = ocolours.colour_bkg;
          break;
        }

        filltype = 7;
      }

      if(OB_STATE(ob) & SELECTED)
      {
        if(((mode3d == FL3DIND) && ocolours.alter_ind) ||
           ((mode3d == FL3DACT) && ocolours.alter_act))
        {
          invertcolour = TRUE;
        }
      }
    }
    else if(OB_STATE(ob) & SELECTED)
    {
      invertcolour = TRUE;
    }

    set_write_mode(vid, SWRM_REPLACE);
		
    if(!(fillcolour < globals->num_pens))
    {
      fillcolour = BLACK;
    }
		
    if(!(bcolour < globals->num_pens))
    {
      bcolour = BLACK;
    }
		
    if(invertcolour)
    {
      fillcolour = invertcolor(fillcolour);
      bcolour = invertcolor(bcolour);
    }
		
    if(filltype != 7)
    {
      vsf_color(vid, bcolour);
      setfilltype(vid, 7);
	
      draw_filled_rect(vid, &r);
    }
		
    if(filltype != 0)
    {
      set_write_mode(vid, SWRM_TRANS);
      vsf_color(vid, fillcolour);
      setfilltype(vid, filltype);
      draw_filled_rect(vid, &r);
    }
  }
}


static
void
draw_3dshadow(WORD   vid,
              RECT * r,
              WORD   selected)
{
  int pnt[6];
  
  if (selected)
  {
    vsl_color(vid, ocolours.br);
  }
  else
  {
    vsl_color(vid, ocolours.tl);
  }

  pnt[0] = r->x;
  pnt[1] = r->y + r->height - 1;
  pnt[2] = r->x;
  pnt[3] = r->y;
  pnt[4] = r->x + r->width - 1;
  pnt[5] = r->y;
  v_pline(vid, 3, pnt);
  
  if (selected)
  {
    vsl_color(vid, ocolours.tl);
  }
  else
  {
    vsl_color(vid, ocolours.br);
  }

  pnt[0] = r->x + 1;
  pnt[1] = r->y + r->height - 1;
  pnt[2] = r->x + r->width - 1;
  pnt[3] = r->y + r->height - 1;
  pnt[4] = r->x + r->width - 1;
  pnt[5] = r->y + 1;
  v_pline(vid, 3, pnt);
}


/*
** Description
** Draw a frame around a resource object
*/
static
void
draw_frame(WORD     vid,
           OBJECT * ob,
           WORD     par_x,
           WORD     par_y)
{
  WORD      type = OB_TYPE(ob) & 0xff;
  RECT      r;
  WORD      draw = 1;
  BYTE      framesize = 0;
  U_OB_SPEC obspec;

  if(OB_FLAGS(ob) & INDIRECT)
  {
    obspec = *((U_OB_SPEC *)OB_SPEC(ob));
  }
  else
  {
    obspec.index = OB_SPEC(ob);
  }

  DEBUG3("objc.c: draw_frame: type = %d (0x%x)", type, type);
  switch(type)
  {
  case G_IBOX :
  case G_BOX  :	/*0x14*/
  case G_BOXCHAR:
  {
    r.x      = OB_X(ob) + par_x;
    r.y      = OB_Y(ob) + par_y;
    r.width  = OB_WIDTH(ob);
    r.height = OB_HEIGHT(ob);
			
    framesize = GET_OBSPEC_FRAMESIZE(obspec.index);
    DEBUG3("objc.c: draw_frame: framesize = %d obspec = 0x%x",
           framesize, obspec.index);

    vsl_color(vid, GET_OBSPEC_FRAMECOL(obspec.index));
  }
  break;

  case G_BOXTEXT:
  case G_FBOXTEXT:
  {
    r.x      = OB_X(ob) + par_x;
    r.y      = OB_Y(ob) + par_y;
    r.width  = OB_WIDTH(ob);
    r.height = OB_HEIGHT(ob);
			
    framesize = TE_THICKNESS(obspec.tedinfo);

    vsl_color(vid, GET_TE_COLOR_BORDERC(TE_COLOR(obspec.tedinfo)));
  }
  break;

  case G_STRING:
  {
    r.x      = OB_X(ob) + par_x;
    r.y      = OB_Y(ob) + par_y;
    r.width  = OB_WIDTH(ob);
    r.height = OB_HEIGHT(ob);
			
    framesize = 0;
  }
  break;

  case G_BUTTON:
  {
    r.x      = OB_X(ob) + par_x;
    r.y      = OB_Y(ob) + par_y;
    r.width  = OB_WIDTH(ob);
    r.height = OB_HEIGHT(ob);
			
    if(OB_FLAGS(ob) & DEFAULT)
    {
      framesize = -DEFBUTFRAME;
    }
    else
    {
      framesize = -BUTTONFRAME;
    }

    vsl_color(vid, BLACK);
  }
  break;
			
  default:
    draw = 0;
  }

  if(draw)
  {
    WORD mode3d = OB_FLAGS(ob) & FLD3DANY;

    vsl_type(vid, 1);
		
    set_write_mode(vid, SWRM_REPLACE);

    if((mode3d == FL3DIND) || (mode3d == FL3DACT))
    {
      r.x -= D3DSIZE;
      r.y -= D3DSIZE;
      r.width += D3DSIZE << 1;
      r.height += D3DSIZE << 1;
			
      drawframe(vid, &r, framesize);

      if(framesize > 0)
      {
        r.x += framesize;
        r.y += framesize;
        r.width -= framesize << 1;
        r.height -= framesize << 1;			
      }

      draw_3dshadow(vid, &r, OB_STATE(ob) & SELECTED);
    }
    else
    {
      drawframe(vid,&r,framesize);

      if(OB_STATE(ob) & OUTLINED)
      {
        r.x -= OUTLINESIZE;	
        r.y -= OUTLINESIZE;	
        r.width += OUTLINESIZE << 1;	
        r.height += OUTLINESIZE << 1;
				
				
        if(mode3d == FL3DBAK)
        {
          WORD i = OUTLINESIZE - 1;
					
          vsl_color(vid, ocolours.br);
          drawframe(vid, &r, 1);
					
          while(i--)
          {
            r.x++;
            r.y++;
            r.width -= 2;
            r.height -= 2;
            draw_3dshadow(vid, &r, 0);
          }
        }
        else
        {
          drawframe(vid, &r, 1);

          vsl_color(vid, WHITE);
			
          r.x += 1;
          r.y += 1;
          r.width -= 2;
          r.height -= 2;
          
          drawframe(vid, &r, OUTLINESIZE - 1);
        }
      }
    }
  }
}


/*
** Description
** Draw bitblock image
*/
static
void
drawimage(WORD     vid,
          WORD     x,
          WORD     y,
          BITBLK * bb,
           WORD    state)
{
  MFDB	d,s;
  
  WORD	xyarray[8];
  WORD	colour[2];
  
  GLOBAL_COMMON * globals = get_global_common ();

  xyarray[0] = 0;
  xyarray[1] = 0;
  xyarray[2] = (BI_WB(bb) << 3) - 1;
  xyarray[3] = BI_HL(bb) - 1;
  xyarray[4] = x + BI_X(bb);
  xyarray[5] = y + BI_Y(bb);
  xyarray[6] = x + BI_X(bb) + (BI_WB(bb) << 3) - 1;
  xyarray[7] = y + BI_Y(bb) + BI_HL(bb) - 1;

  (LONG)s.fd_addr = (LONG)BI_PDATA(bb);
  s.fd_w = (BI_WB(bb) << 3);
  s.fd_h = BI_HL(bb);
  s.fd_wdwidth = (BI_WB(bb) >> 1);
  s.fd_stand = 0;
  s.fd_nplanes = 1;

  (LONG)d.fd_addr = 0L;

  colour[0] = BI_COLOR(bb);
	
  if(!(colour[0] < globals->num_pens))
  {
    colour[0] = BLACK;
  }
	
  if(state & SELECTED)
  {
    colour[0] = invertcolor(colour[0]);
  }

  /*
  ** FIXME
  vrt_cpyfm(vid,MD_TRANS,xyarray,&s,&d,colour);
  */
}


static
void
drawicon(WORD      vid,
         WORD      x,
         WORD      y,
         ICONBLK * ib,
         WORD      state)
{
  BYTE	ch[2];
	
  MFDB	d,s;

  int  xyarray[8];
  int  color[2];
  WORD icon_colour = IB_CHAR(ib) >> 12;
  WORD mask_colour = (IB_CHAR(ib) & 0x0f00) >> 8;
	
  RECT	r;
  GLOBAL_COMMON * globals = get_global_common ();
	
  if(!(mask_colour < globals->num_pens))
  {
    mask_colour = BLACK;
  }
	
  if(!(icon_colour < globals->num_pens))
  {
    icon_colour = BLACK;
  }
	
  xyarray[0] = 0;
  xyarray[1] = 0;
  xyarray[2] = IB_WICON(ib) - 1;
  xyarray[3] = IB_HICON(ib) - 1;
  xyarray[4] = x + IB_XICON(ib);
  xyarray[5] = y + IB_YICON(ib);
  xyarray[6] = x + IB_XICON(ib) + IB_WICON(ib) - 1;
  xyarray[7] = y + IB_YICON(ib) + IB_HICON(ib) - 1;
  (LONG)s.fd_addr = (LONG)IB_PMASK(ib);
  s.fd_w = IB_WICON(ib);
  s.fd_h = IB_HICON(ib);
  s.fd_wdwidth = ((IB_WICON(ib) + 15) >> 4);
  s.fd_stand = 0;
  s.fd_nplanes = 1;

  (LONG)d.fd_addr = 0L;

  if(state & SELECTED)
  {
    color[0] = icon_colour;
  }
  else
  {
    color[0] = mask_colour;
  }

  vrt_cpyfm(vid, MD_TRANS, xyarray, &s, &d, color);

  if(state & SELECTED)
  {
    color[0] = mask_colour;
  }
  else
  {
    color[0] = icon_colour;
  }
	
  (LONG)s.fd_addr = (LONG)IB_PDATA(ib);

  vrt_cpyfm(vid, MD_TRANS, xyarray, &s, &d, color);

  r.x = x + IB_XTEXT(ib);
  r.y = y + IB_YTEXT(ib);
  r.width = IB_WTEXT(ib);
  r.height = IB_HTEXT(ib);

  drawtext(vid, IB_PTEXT(ib), &r, 2, IB_CHAR(ib) >> 12, 5, MD_REPLACE, state);

  r.x = x + IB_XCHAR(ib);
  r.y = y + IB_YCHAR(ib);
  r.width = 0;
  r.height = 0;
	
  ch[0] = IB_CHAR(ib) & 0x00ff;
  ch[1] = 0;

  drawtext(vid, ch, &r, 0, IB_CHAR(ib) >> 12, 5, MD_REPLACE, state);
}


static
void
drawcicon(WORD       vid,
          WORD       x,
          WORD       y,
          CICONBLK * cib,
          WORD       state)
{
  BYTE	  ch[2];
  MFDB	  d,s;
  int	  xyarray[8];
  int	  color[] = { WHITE,BLACK };
  RECT	  r;
  CICON * best = NULL;
  CICON * ciwalk = MAINLIST(cib);
  WORD    bestplanes = 0;

  while(ciwalk)
  {
    if((NUM_PLANES(ciwalk) <= numplanes) &&
       (NUM_PLANES(ciwalk) > bestplanes))
    {
      best = ciwalk;
      bestplanes = NUM_PLANES(best);
			
      if(bestplanes == numplanes)
      {
        break;
      }
    }
		
    ciwalk = NEXT_RES(ciwalk);
  }

  if(!best)
  {
    drawicon(vid, x, y, &cib->monoblk, state);
  }
  else
  {
    xyarray[0] = 0;
    xyarray[1] = 0;
    xyarray[2] = IB_WICON(&cib->monoblk) - 1;
    xyarray[3] = IB_HICON(&cib->monoblk) - 1;
    xyarray[4] = x + IB_XICON(&cib->monoblk);
    xyarray[5] = y + IB_YICON(&cib->monoblk);
    xyarray[6] = x + IB_XICON(&cib->monoblk) + IB_WICON(&cib->monoblk) - 1;
    xyarray[7] = y + IB_YICON(&cib->monoblk) + IB_HICON(&cib->monoblk) - 1;
	
    (LONG)s.fd_addr = (LONG)COL_MASK(best);
    s.fd_w = IB_WICON(&cib->monoblk);
    s.fd_h = IB_HICON(&cib->monoblk);
    s.fd_wdwidth = ((IB_WICON(&cib->monoblk) + 15) >> 4);
    s.fd_stand = 0;
    s.fd_nplanes = 1;
	
    (LONG)d.fd_addr = 0L;
	
    vrt_cpyfm(vid, MD_TRANS, xyarray, &s, &d, color);

    s.fd_nplanes = NUM_PLANES(best);	
    (LONG)s.fd_addr = (LONG)COL_DATA(best);
		
    vro_cpyfm(vid, S_OR_D, xyarray, &s, &d);
	
    r.x = x + IB_XTEXT(&cib->monoblk);
    r.y = y + IB_YTEXT(&cib->monoblk);
    r.width = IB_WTEXT(&cib->monoblk);
    r.height = IB_HTEXT(&cib->monoblk);
	
    drawtext(vid,
             IB_PTEXT(&cib->monoblk),
             &r,
             2,
             IB_CHAR(&cib->monoblk) >> 12,
             5,
             MD_REPLACE,
             state);
	
    r.x = x + IB_XCHAR(&cib->monoblk);
    r.y = y + IB_YCHAR(&cib->monoblk);
    r.width = 0;
    r.height = 0;
		
    ch[0] = IB_CHAR(&cib->monoblk) & 0x00ff;
    ch[1] = 0;
	
    drawtext(vid,
             ch,
             &r,
             0,
             IB_CHAR(&cib->monoblk) >> 12,
             5,
             MD_REPLACE,
             state);
  }
}


/*
** Description
** Draw one resource object to the display
*/
static
void
draw_object(WORD     vid,
            OBJECT * tree,
            WORD     object,
            RECT *   clip,
            WORD     par_x,
            WORD     par_y,
            WORD     is_top)
{
  WORD      type = OB_TYPE(&tree[object]) & 0xff;
  U_OB_SPEC obspec;
  RECT      ci = *clip;
  RECT      cu;
  RECT      area;
	
  ci.x -= par_x;
  ci.y -= par_y;

  area.x      = OB_X(&tree[object]);
  area.y      = OB_Y(&tree[object]);
  area.width  = OB_WIDTH(&tree[object]);
  area.height = OB_HEIGHT(&tree[object]);

  if(!Misc_intersect(&area, &ci, &cu))
  {
    return;
  }

  if(OB_FLAGS(&tree[object]) & INDIRECT)
  {
    obspec = *((U_OB_SPEC *)OB_SPEC(&tree[object]));
  }
  else
  {
    obspec.index = OB_SPEC(&tree[object]);
  }

  switch(type)
  {
  case G_IMAGE: /*0x17*/
    drawimage (vid,
               OB_X(&tree[object]) + par_x,
               OB_Y(&tree[object]) + par_y,
               (BITBLK *)obspec.index,
               OB_STATE(&tree[object]));
    break;

  case G_PROGDEF: /*0x18*/
  {
    PARMBLK pb;
    WORD    remainstate;
    RECT    oclip;
    RECT    nclip;
    WORD    xy[2];

    PB_TREE_PUT(&pb, tree);
    PB_OBJ_PUT(&pb, object);
    PB_PREVSTATE_PUT(&pb, OB_STATE(&tree[object]));
    PB_CURRSTATE_PUT(&pb, OB_STATE(&tree[object]));
				
    Objc_do_offset(tree, object, xy);
    PB_X_PUT(&pb, xy[0]);
    PB_Y_PUT(&pb, xy[1]);
    PB_W_PUT(&pb, OB_WIDTH(&tree[object]));
    PB_H_PUT(&pb, OB_HEIGHT(&tree[object]));
				
    Objc_calc_clip(tree, object, &oclip);
				
    if(Misc_intersect(&oclip, clip, &nclip))
    {
      PB_XC_PUT(&pb, nclip.x);
      PB_YC_PUT(&pb, nclip.y);
      PB_WC_PUT(&pb, nclip.width);
      PB_HC_PUT(&pb, nclip.height);

      PB_PARM_PUT(&pb, UB_PARM(obspec.userblk));

#ifdef MINT_TARGET
      remainstate = UB_CODE(obspec)(&pb);
#else
      {
        GLOBAL_COMMON * globals = get_global_common();

        if(globals->callback_handler == NULL)
        {
          remainstate = UB_CODE(obspec.userblk)(&pb);
        }
        else
        {
          remainstate = globals->callback_handler(UB_CODE(obspec.userblk),
                                                  &pb);
        }
      }
#endif
					
      NOT_USED(remainstate);
    }
  }
  break;

  case	G_ICON:
    drawicon (vid,
              OB_X(&tree[object]) + par_x,
              OB_Y(&tree[object]) + par_y,
              obspec.iconblk,
              OB_STATE(&tree[object]));
    break;
		
  case	G_CICON:
    drawcicon (vid,
               OB_X(&tree[object]) + par_x,
               OB_Y(&tree[object]) + par_y,
               (CICONBLK *)obspec.index,
               OB_STATE(&tree[object]));
    break;
			
  default:
    draw_bg(vid, &tree[object], par_x, par_y);
    draw_text(vid, &tree[object], par_x, par_y, is_top);
    draw_frame(vid, &tree[object], par_x, par_y);
  }

  if(OB_STATE(&tree[object]) & CHECKED)
  {
    int dum;
		
    vst_alignment(vid, 0, 5, &dum, &dum);
    v_gtext(vid,
            OB_X(&tree[object]) + par_x,
            OB_Y(&tree[object]) + par_y,
            "\x8");
  }
}


static
void
get_char_bound(OBJECT * tree,
               WORD     obj,
               WORD     object,
               WORD     last,
               RECT *   r)
{
  GLOBAL_COMMON * globals = get_global_common ();
  BYTE *          ptmplt;
  WORD            firstreal = 0;
  WORD            lastreal = 0;
  U_OB_SPEC       obspec;
	
  if(OB_FLAGS(&tree[object]) & INDIRECT)
  {
    obspec = *((U_OB_SPEC *)OB_SPEC(&tree[object]));
  }
  else
  {
    obspec.index = OB_SPEC(&tree[object]);
  }

  ptmplt = TE_PTMPLT(obspec.tedinfo);
	
  while(*ptmplt)
  {
    if(*ptmplt == '_')
    {
      object--;
      last--;
    }

    if(object >= 0)
    {
      firstreal++;
    }
		
    if(last >= 0)
    {
      lastreal++;
    }
		
    ptmplt++;
  }

  Objc_do_offset(tree, obj, (WORD *)r);

  r->y += (OB_HEIGHT(&tree[obj]) - globals->clheight) >> 1;
  r->height = globals->clheight;
  r->width = globals->clwidth * (lastreal - firstreal);
	
  switch(TE_JUST(obspec.tedinfo))
  {
  case 0:
    break;

  case 1:
    r->x += (OB_WIDTH(&tree[obj]) - 
             (globals->clwidth * (WORD)strlen(TE_PTMPLT(obspec.tedinfo))));
    break;
  case 2:
    r->x += ((OB_WIDTH(&tree[obj]) -
              (globals->clwidth *
               (WORD)strlen(TE_PTMPLT(obspec.tedinfo)))) >> 1);
    break;
  }

  r->x = r->x + (firstreal * globals->clwidth);
}


static
void
draw_cursor(WORD    vid,
            WORD     pos,
            OBJECT * tree,
            WORD     obj)
{
  RECT r;
  int  xy[4];
  
  get_char_bound(tree, obj, pos, pos, &r);
  
  xy[0] = r.x;
  xy[1] = r.y;
  xy[2] = xy[0];
  xy[3] = xy[1] + r.height;

  vswr_mode(vid, MD_XOR);
  v_pline(vid, 2, xy);
}


static
WORD
handle_ed_char(WORD     vid,
               WORD     idx,
               OBJECT * tree,
               WORD     obj,
               WORD     kc)
{
  RECT      clip;
  U_OB_SPEC obspec;
  WORD      internal_kc;
	
  if(OB_FLAGS(&tree[obj]) & INDIRECT)
  {
    obspec = *((U_OB_SPEC *)OB_SPEC(&tree[obj]));
  }
  else
  {
    obspec.index = OB_SPEC(&tree[obj]);
  }

  /* Ignore scancode for standard characters */
  if((kc & 0xff) == 0)
  {
    internal_kc = kc;
  }
  else
  {
    internal_kc = kc & 0xff;
  }

  switch(internal_kc)
  {
  case 0x001b: /* escape */
    if(idx > 0)
    {
      draw_cursor(vid, idx, tree, obj);
      get_char_bound(tree, obj, 0,
                     (WORD)strlen(TE_PTEXT(obspec.tedinfo)), &clip);
      TE_PTEXT(obspec.tedinfo)[0] = '\0';
      Objc_do_draw(vid, tree, 0, 9, &clip);
      draw_cursor(vid, 0, tree, obj);
    }
    return(0);

  case 0x0008: /* backspace */
    if(idx > 0)
    {
      draw_cursor (vid, idx, tree, obj);
			
      strcpy(&TE_PTEXT(obspec.tedinfo)[idx - 1],
             &TE_PTEXT(obspec.tedinfo)[idx]);
	    
      get_char_bound (tree,
                      obj,
                      idx - 1,
                      (WORD)strlen(TE_PTEXT(obspec.tedinfo)) + 2,
                      &clip);

      Objc_do_draw(vid, tree, 0, 9, &clip);
			
      draw_cursor(vid, idx - 1, tree, obj);
      return idx - 1;
    }
    return idx;
		
  case 0x4b00: /* left */
    if(idx > 0)
    {
      draw_cursor(vid, idx, tree, obj);
      draw_cursor(vid, idx - 1, tree, obj);
      return idx - 1;
    }
    return idx;
	
  case 0x4b34: /* shift left */
    if(idx > 0)
    {
      draw_cursor(vid, idx, tree, obj);
      draw_cursor(vid, 0, tree, obj);
      return 0;
    }
    return(idx);

  case 0x4d00: /* right */
    if(idx < (WORD)strlen(TE_PTEXT(obspec.tedinfo)))
    {
      draw_cursor(vid, idx, tree, obj);
      draw_cursor(vid, idx + 1, tree, obj);
      return idx + 1;
    }
    return idx;

  case 0x4d36: /* shift right */
    if(idx < (WORD)strlen(TE_PTEXT(obspec.tedinfo)))
    {
      draw_cursor(vid, idx, tree, obj);
      idx = (WORD)strlen(TE_PTEXT(obspec.tedinfo));
      draw_cursor(vid, idx, tree, obj);
      return idx;
    }
    return idx;

  case 0x007f: /* delete */
    if(idx < (WORD)strlen(TE_PTEXT(obspec.tedinfo)))
    {
      draw_cursor(vid, idx, tree, obj);
			
      strcpy(&TE_PTEXT(obspec.tedinfo)[idx],
             &TE_PTEXT(obspec.tedinfo)[idx + 1]);

      get_char_bound(tree, obj, idx,
                     (WORD)strlen(TE_PTEXT(obspec.tedinfo)) + 1, &clip);
      Objc_do_draw(vid, tree, 0, 9, &clip);
			
      draw_cursor(vid, idx, tree, obj);
    }
    return idx;
		
  default:
    if(TE_PTEXT(obspec.tedinfo)[idx] != '\0')
    {
      BYTE i = strlen(TE_PTEXT(obspec.tedinfo));
			
      while(i >= idx)
      {
        /* FIXME: ALL TE_PTEXT */
        TE_PTEXT(obspec.tedinfo)[i + 1] = TE_PTEXT(obspec.tedinfo)[i];
				
        i--;
      }
    }
    else
    {
      TE_PTEXT(obspec.tedinfo)[idx + 1] = '\0';
    }
		
    TE_PTEXT(obspec.tedinfo)[idx] = (BYTE)internal_kc;

    draw_cursor(vid, idx, tree, obj);
    get_char_bound(tree, obj, idx,
                   (WORD)strlen(TE_PTEXT(obspec.tedinfo)), &clip);
    Objc_do_draw (vid, tree, 0, 9, &clip);
    draw_cursor (vid, idx + 1, tree, obj);
		
    return idx + 1;
  }
}


void
do_objc_add(OBJECT * t,
            WORD     p,
            WORD     c)
{
  if(OB_TAIL(&t[p]) < 0)
  {
    OB_HEAD_PUT(&t[p], c);
    OB_TAIL_PUT(&t[p], c);
    OB_NEXT_PUT(&t[c], p);
  }
  else
  {
    OB_NEXT_PUT(&t[c], p);
    OB_NEXT_PUT(&t[OB_TAIL(&t[p])], c);
    OB_TAIL_PUT(&t[p], c);
  }
}


/* objc_add 0x0028 */

void
Objc_add(AES_PB *apb)
{
  do_objc_add((OBJECT *)apb->addr_in[0], apb->int_in[0], apb->int_in[1]);
  
  apb->int_out[0] = 1;
}


/* objc_delete	0x0029 */
static
inline
void
Objc_do_delete(OBJECT * tree,
               WORD     object)
{
  WORD i;
  WORD prev = -1;
  WORD next;
  
  i = 0;
  
  next = OB_NEXT(&tree[object]);
  
  if(next != -1)
  {
    if(OB_TAIL(&tree[next]) == object)
    {
      next = -1;
    }
  }
  
  while(TRUE)
  {	
    if((OB_NEXT(&tree[i]) == object) && (OB_TAIL(&tree[object]) != i))
    {
      prev = i;
      OB_NEXT_PUT(&tree[i], OB_NEXT(&tree[object]));
      
      break;
    }
    
    if(OB_FLAGS(&tree[i]) & LASTOB)
    {
      break;
    }
    
    i++;
  }
  
  i = 0;
  
  while(TRUE)
  {	
    if(OB_HEAD(&tree[i]) == object)
    {
      OB_HEAD_PUT(&tree[i], next);
    }
    
    if(OB_TAIL(&tree[i]) == object)
    {
      OB_TAIL_PUT(&tree[i], prev);
    }
    
    if(OB_FLAGS(&tree[i]) & LASTOB)
    {
      break;
    }
    
    i++;
  }
}


void
Objc_delete(AES_PB *apb)
{
  Objc_do_delete((OBJECT *)apb->addr_in[0], apb->int_in[0]);
  
  apb->int_out[0] = 1;
}


/*objc_draw	0x002a*/

/*
** Exported
*/
WORD
Objc_do_draw(WORD     vid,
             OBJECT * tree,
             WORD     object,
             WORD     depth,
             RECT   * clip)
{
  WORD current = object;
  WORD next = -1;
  
  WORD xy[2];
  int  xyxy[4];
  WORD x,y;
  
  if((tree == NULL) || (object < 0) || (depth < 0) || (clip == NULL))
  {
    return 0;
  }
  
  if (Objc_do_offset(tree, object, xy) == 0)
  {
    return 0;
  }

  x = xy[0] - OB_X(&tree[object]);
  y = xy[1] - OB_Y(&tree[object]);
  
  xyxy[0] = clip->x;
  xyxy[1] = clip->y;
  xyxy[2] = xyxy[0] + clip->width - 1;
  xyxy[3] = xyxy[1] + clip->height - 1;
  
  vs_clip(vid, 1, xyxy);

  v_hide_c(vid);

  do
  {
    if(!(OB_FLAGS(&tree[current]) & HIDETREE))
    {
      draw_object (vid,
                   tree,
                   current,
                   clip,
                   x,
                   y,
                   object == current);

      next = OB_HEAD(&tree[current]);
      
      if(next != -1)
      {
	x += OB_X(&tree[current]);
	y += OB_Y(&tree[current]);
      }
    }
    
    if(((next == -1) || (depth <= 0)) && (current == object))
    {
      break;
    }
    
    if((depth < 0) || (next == -1) || (OB_FLAGS(&tree[current]) & HIDETREE))
    {
      next = OB_NEXT(&tree[current]);
      
      if(next == -1)
      {
	break;
      }
      
      while((OB_TAIL(&tree[next]) == current) && (current != object))
      {
	depth++;
	
	if(current == next)
        {
	  break;
	}
	
	current = next;
	
	x -= OB_X(&tree[current]);
	y -= OB_Y(&tree[current]);
	
	next = OB_NEXT(&tree[current]);
	
	if(next == -1)
        {
	  break;
	}
      }
      
      if(current == next)
      {
	break;
      }
      
      if(current != object)
      {
	current = next;
      }
    }
    else
    {
      depth--;
      
      current = next;
    }
  } while(current != object);

  v_show_c(vid, 1);

  vs_clip(vid, 0, xyxy);

  return 1;
}


/*
** Description
** objc_draw ()
*/
void
Objc_draw(AES_PB *apb)
{
  GLOBAL_APPL * globals;

  CHECK_APID(apb->global->apid);

  globals = get_globals(apb->global->apid);

  apb->int_out[0] = Objc_do_draw(globals->vid,
                                 (OBJECT *)apb->addr_in[0],
                                 apb->int_in[0],
                                 apb->int_in[1],
                                 (RECT *)&apb->int_in[2]);
}


/*objc_find 0x002b*/

/*
** Exported
*/
WORD
Objc_do_find(OBJECT * t,
              WORD     startobject,
              WORD     depth,
              WORD     x,
              WORD     y,
              WORD     level)
{
  /* Avoid crash if someone passes a NULL pointer */
  if(t == NULL)
  {
    return -1;
  }

  if (OB_FLAGS(&t[startobject]) & HIDETREE)
  {
    return -1;
  }
  
  if (level == 0)
  {
    WORD lxy[2];

    Objc_do_offset(t, startobject, lxy);
    
    x -= lxy[0];
    y -= lxy[1];
  }
	
  if ((x >= 0) && (x < OB_WIDTH(&t[startobject])) &&
      (y >= 0) && (y < OB_HEIGHT(&t[startobject])))
  {
    WORD deeper;
    WORD bestobj = startobject;
    
    if((depth > 0) && (OB_HEAD(&t[startobject]) >= 0))
    {
      WORD i = OB_HEAD(&t[startobject]);
      
      while(i != startobject)
      {
        deeper = Objc_do_find(t,
                              i,
                              depth - 1,
                              x - OB_X(&t[i]),
                              y - OB_Y(&t[i]),
                              level + 1);
        
        if(deeper >= 0)
        {
          bestobj = deeper;
        }
        
        i = OB_NEXT(&t[i]);
      }
    }
    
    return bestobj;
  }
  
  return -1;
}


void
Objc_find(AES_PB *apb)
{
  apb->int_out[0] = Objc_do_find((OBJECT *)apb->addr_in[0],
                                 apb->int_in[0],
                                 apb->int_in[1],
                                 apb->int_in[2],
                                 apb->int_in[3],
                                 0);
}


/*objc_offset 0x002c*/

/*
** Description
** Implementation of objc_offset()
*/
WORD
Objc_do_offset(OBJECT * tree,
               WORD     object,
               WORD   * xy)
{
  if((tree == NULL))
  {
    return 0;
  }
  
  xy[0] = 0;
  xy[1] = 0;
  
  do
  {
    WORD last;
    
    xy[0] += OB_X(&tree[object]);
    xy[1] += OB_Y(&tree[object]);
    
    if((OB_NEXT(&tree[object]) < 0) || (object == 0))
    {
      break;
    }
		
    do
    {
      last = object;
      object = OB_NEXT(&tree[object]);
    } while(last != OB_TAIL(&tree[object]));	
  } while(1);
	
  if(object == 0)
  {
    return 1;
  }
  else
  {
    return 0;
  }
}


void
Objc_offset(AES_PB *apb)
{
  apb->int_out[0] = Objc_do_offset((OBJECT *)apb->addr_in[0],
                                   apb->int_in[0],
                                   &apb->int_out[1]);
}


/*
** Description
** Implementation of objc_edit()
*/
WORD
Objc_do_edit(WORD     vid,
             OBJECT * tree,
             WORD     obj,
             WORD     kc,
             WORD   * idx,
             WORD     mode)
{
  WORD      type;
  U_OB_SPEC obspec;

  if(OB_FLAGS(&tree[obj]) & INDIRECT)
  {
    obspec = *((U_OB_SPEC *)OB_SPEC(&tree[obj]));
  }
  else
  {
    obspec.index = OB_SPEC(&tree[obj]);
  }
	
  if((obj < 0) || (tree == NULL) || (idx == NULL))
  {
    return 0;
  }
	
  if(((type = OB_TYPE(&tree[obj]) & 0xff) != G_FTEXT) &&
     (type != G_FBOXTEXT))
  {
    return 0;
  }
	
  switch(mode)
  {
  case ED_INIT:
    *idx = (WORD)strlen(TE_PTEXT(obspec.tedinfo));
    draw_cursor(vid, *idx, tree, obj);
    break;
	
  case ED_CHAR:
    *idx = handle_ed_char(vid, *idx, tree, obj, kc);
    break;
		
  case ED_END:
    draw_cursor(vid, *idx, tree, obj);
    break;
		
  default:
    DEBUG0("%s: Line %d: Objc_do_edit:\r\n"
           "Unknown mode %d\r\n",__FILE__,__LINE__,mode);
    return 0;
  }

  return 1;
}


/*
** Description
** 0x002e objc_edit()
*/
void
Objc_edit(AES_PB * apb)
{
  CHECK_APID(apb->global->apid);

  apb->int_out[1] = apb->int_in[2];
  apb->int_out[0] = Objc_do_edit(apb->global->apid,
                                 (OBJECT *)apb->addr_in[0],
                                 apb->int_in[0],
                                 apb->int_in[1],
                                 &apb->int_out[1],
                                 apb->int_in[3]);
}


/*
** Description
** Implementation of objc_change()
*/
WORD
Objc_do_change(WORD     vid,
               OBJECT * tree,
               WORD     obj,
               RECT   * clip,
               WORD     newstate,
               WORD     drawflag)
{
  OB_STATE_PUT(&tree[obj], newstate);
	
  if(drawflag == REDRAW)
  {
    Objc_do_draw(vid, tree, obj, 9, clip);
  }
  
  return 1;
}


/*
** Description
**  0x002f objc_change()
*/
void
Objc_change(AES_PB * apb)
{
  GLOBAL_APPL * globals;

  CHECK_APID(apb->global->apid);

  globals = get_globals(apb->global->apid);

  apb->int_out[0] = Objc_do_change(globals->vid,
                                   (OBJECT *)apb->addr_in[0],
                                   apb->int_in[0],
                                   (RECT *)&apb->int_in[2],
                                   apb->int_in[6],
                                   apb->int_in[7]);
}


/*
** Description
** Implementation of objc_sysvar()
*/
static
inline
WORD
Objc_do_sysvar(WORD   mode,
               WORD   which,
               WORD   in1,
               WORD   in2,
               WORD * out1,
               WORD * out2)
{
  if(mode == SV_INQUIRE)
  {
    switch(which)
    {
    case LK3DIND:
      *out1 = ocolours.move_ind;
      *out2 = ocolours.alter_ind;
      return 1;
      
    case LK3DACT:
      *out1 = ocolours.move_act;
      *out2 = ocolours.alter_act;
      return 1;
      
    case INDBUTCOL:
      *out1 = ocolours.colour_ind;
      return 1;
      
    case ACTBUTCOL:
      *out1 = ocolours.colour_act;
      return 1;
      
    case BACKGRCOL:
      *out1 = ocolours.colour_bkg;
      return 1;
      
    case AD3DVAL:
      *out1 = D3DSIZE;
      *out2 = D3DSIZE;
      return 1;
    }
  }
  else if(mode == SV_SET)
  {
    switch(which)
    {
    case LK3DIND:
      ocolours.move_ind = in1;
      ocolours.alter_ind = in2;
      return 1;
      
    case LK3DACT:
      ocolours.move_act = in1;
      ocolours.alter_act = in2;
      return 1;
      
    case INDBUTCOL:
      ocolours.colour_ind = in1;
      return 1;
      
    case ACTBUTCOL:
      ocolours.colour_act = in1;
      return 1;
      
    case BACKGRCOL:
      ocolours.colour_bkg = in1;
      return 1;
    }
  }
  
  DEBUG0("Objc_do_sysvar: mode=%d which=%d",mode,which);
  
  return 0;
}


/*
** Description
** 0x0030 objc_sysvar()
*/
void
Objc_sysvar(AES_PB * apb)
{
  apb->int_out[0] = Objc_do_sysvar(apb->int_in[0],
                                   apb->int_in[1],
				   apb->int_in[2],
                                   apb->int_in[3],
				   &apb->int_out[1],
                                   &apb->int_out[2]);
}


/*
** Description
** Calculate how large area an object covers
*/
void
Objc_area_needed(OBJECT * tree,
                 WORD     object,
                 RECT   * rect)
{
  WORD      mode3d = OB_FLAGS(&tree[object]) & FLD3DANY;
  U_OB_SPEC obspec;
  WORD      framesize;
  
  if(OB_FLAGS(&tree[object]) & INDIRECT)
  {
    obspec = *((U_OB_SPEC *)OB_SPEC(&tree[object]));
  }
  else
  {
    obspec.index = OB_SPEC(&tree[object]);
  }

  Objc_do_offset(tree, object, (WORD *)rect);
  
  rect->width = OB_WIDTH(&tree[object]);
  rect->height = OB_HEIGHT(&tree[object]);
  
  switch(OB_TYPE(&tree[object]))
  {
  case	G_BOX:
  case	G_IBOX:
  case	G_BOXCHAR:
    framesize = (BYTE)GET_OBSPEC_FRAMESIZE(obspec.index);

    if(framesize < 0)
    {
      rect->x += framesize;
      rect->y += framesize;
      rect->width -= (framesize << 1);
      rect->height -= (framesize << 1);
    }
    break;
  case	G_BUTTON:
    if(OB_FLAGS(&tree[object]) & DEFAULT)
    {
      rect->x -= DEFBUTFRAME;
      rect->y -= DEFBUTFRAME;
      rect->width += (DEFBUTFRAME << 1);
      rect->height += (DEFBUTFRAME << 1);
    }
    else
    {
      rect->x -= BUTTONFRAME;
      rect->y -= BUTTONFRAME;
      rect->width += (BUTTONFRAME << 1);
      rect->height += (BUTTONFRAME << 1);
    }
    break;
  }
  
  if(OB_STATE(&tree[object]) & OUTLINED)
  {
    rect->x -= OUTLINESIZE;
    rect->y -= OUTLINESIZE;
    rect->width += (OUTLINESIZE << 1);
    rect->height += (OUTLINESIZE << 1);
  }
  
  if((mode3d == FL3DIND) || (mode3d == FL3DACT))
  {
    rect->x -= D3DSIZE;
    rect->y -= D3DSIZE;
    rect->width += (D3DSIZE << 1);
    rect->height += (D3DSIZE << 1);
  }
}


/*
** Description
** Calculate required clip area for object
*/
void
Objc_calc_clip(OBJECT * tree,
               WORD     object,
               RECT   * rect)
{
  WORD owalk = object;
  
  while(1)
  {
    if(OB_NEXT(&tree[owalk]) == -1)
    {
      owalk = -1;
      break;
    }
    
    if(OB_TAIL(&tree[OB_NEXT(&tree[owalk])]) == owalk)
    {
      owalk = OB_NEXT(&tree[owalk]);
      break;
    }
    
    owalk = OB_NEXT(&tree[owalk]);
  }
  
  if(owalk == -1)
  {
    Objc_area_needed(tree, object, rect);
  }
  else
  {
    Objc_do_offset(tree, owalk, (WORD *)rect);
    
    rect->width = OB_WIDTH(&tree[owalk]);
    rect->height = OB_HEIGHT(&tree[owalk]);
  }
}
