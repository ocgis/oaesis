/*
** aesbind.h
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

#include <stdarg.h>
#include <stdio.h>

#include "aesbind.h"

static short contrl[5];
short global[15];
static short intin[16];
static short intout[7];
static long  addrin[3];
static long  addrout[1];
static AESPB aespb = {contrl, global, intin, intout, addrin, addrout};

#define OPCODE     aespb.contrl[0]
#define NO_INTIN   aespb.contrl[1]
#define NO_ADDRIN  aespb.contrl[2]
#define NO_INTOUT  aespb.contrl[3]
#define NO_ADDROUT aespb.contrl[4]

short
appl_exit (void)
{
  OPCODE = 19;
  NO_INTIN = 0;
  NO_ADDRIN = 1;
  NO_INTOUT = 0;

  aes_call (&aespb);

  return aespb.intout[0];
}


short
appl_init (void)
{
  OPCODE = 10;
  NO_INTIN = 0;
  NO_ADDRIN = 1;
  NO_INTOUT = 0;

  aes_call (&aespb);

  return aespb.intout[0];
}


short
evnt_multi (short         Type,
            short         Clicks,
            short         WhichButton,
            short         WhichState,
            short         EnterExit1,
            short         In1X,
            short         In1Y,
            short         In1W,
            short         In1H,
            short         EnterExit2,
            short         In2X, 
            short         In2Y,
            short         In2W,
            short         In2H,
            short         MesagBuf[],
            unsigned long Interval,
            short *       OutX,
            short *       OutY,
            short *       ButtonState,
            short *       KeyState,
            short *       Key,
            short *       ReturnCount)
{
  OPCODE = 25;
  NO_INTIN = 16;
  NO_ADDRIN = 7;
  NO_INTOUT = 1;
  
  aespb.intin[0] = Type;
  aespb.intin[1] = Clicks;
  aespb.intin[2] = WhichButton;
  aespb.intin[3] = WhichState;
  
  aespb.intin[4] = EnterExit1;
  aespb.intin[5] = In1X;
  aespb.intin[6] = In1Y;
  aespb.intin[7] = In1W;
  aespb.intin[8] = In1H;
  
  aespb.intin[9] = EnterExit2;
  aespb.intin[10] = In2X;
  aespb.intin[11] = In2Y;
  aespb.intin[12] = In2W;
  aespb.intin[13] = In2H;
  
  aespb.intin[14] = ((short *)&Interval)[1];
  aespb.intin[15] = ((short *)&Interval)[0];
  
  aespb.addrin[0] = (long)MesagBuf;
  
  aes_call (&aespb);
  
  *OutX        = aespb.intout[1];
  *OutY        = aespb.intout[2];
  *ButtonState = aespb.intout[3];
  *KeyState    = aespb.intout[4];
  *Key	       = aespb.intout[5];
  *ReturnCount = aespb.intout[6];
  
  return aespb.intout[0];
}


short
form_center (void *  tree,
             short * cx,
             short * cy,
             short * cw,
             short * ch) {
  OPCODE = 54;
  NO_INTIN = 0;
  NO_ADDRIN = 1;
  NO_INTOUT = 5;
  NO_ADDROUT = 0;
  
  aespb.addrin[0] = tree;

  aes_call (&aespb);
  
  *cx = aespb.intout[1];
  *cy = aespb.intout[2];
  *cw = aespb.intout[3];
  *ch = aespb.intout[4];

  return aespb.intout[0];
}


short
form_dial (short flag,
           short sx,
           short sy,
           short sw,
           short sh,
           short bx,
           short by,
           short bw,
           short bh) {
  OPCODE = 51;
  NO_INTIN = 9;
  NO_ADDRIN = 0;
  NO_INTOUT = 1;
  NO_ADDROUT = 0;
  
  aespb.intin[0] = flag;
    
  aespb.intin[1] = sx;
  aespb.intin[2] = sy;
  aespb.intin[3] = sw;
  aespb.intin[4] = sh;

  aespb.intin[5] = bx;
  aespb.intin[6] = by;
  aespb.intin[7] = bw;
  aespb.intin[8] = bh;

  aes_call (&aespb);
  
  return aespb.intout[0];
}


short
form_do (void * tree,
         short  startobj) {
  OPCODE     = 50;
  NO_INTIN   = 1;
  NO_ADDRIN  = 1;
  NO_INTOUT  = 1;
  NO_ADDROUT = 0;
  
  aespb.intin[0] = startobj;
    
  aespb.addrin[0] = tree;

  aes_call (&aespb);
  
  return aespb.intout[0];
}


short
form_error (short code) {
  OPCODE     = 53;
  NO_INTIN   = 1;
  NO_ADDRIN  = 0;
  NO_INTOUT  = 1;
  NO_ADDROUT = 0;
  
  aespb.intin[0] = code;
    
  aes_call (&aespb);
  
  return aespb.intout[0];
}


short
fsel_exinput (char *  Path,
              char *  File,
              short * ExitButton,
              char *  Prompt) {
  OPCODE = 91;
  NO_INTIN = 0;
  NO_ADDRIN = 3;
  NO_INTOUT = 2;
  NO_ADDROUT = 0;
  
  aespb.addrin[0] = (long)Path;
  aespb.addrin[1] = (long)File;
  aespb.addrin[2] = (long)Prompt;
    
  aes_call (&aespb);
  
  *ExitButton = aespb.intout[1];

  return aespb.intout[0];
}


short
graf_handle (short * Wchar,
             short * Hchar,
             short * Wbox, 
             short * Hbox)
{
  OPCODE = 77;
  NO_INTIN = 0;
  NO_ADDRIN = 5;
  NO_INTOUT = 0;

  aes_call (&aespb);

  *Wchar = aespb.intout[1];
  *Hchar = aespb.intout[2];
  *Wbox  = aespb.intout[3];
  *Hbox  = aespb.intout[4];
  return aespb.intout[0];
}


short
graf_mouse (int    Form,
            void * FormAddress) {
  OPCODE = 78;
  NO_INTIN = 1;
  NO_ADDRIN = 1;
  NO_INTOUT = 1;

  aespb.intin[0] = Form;
  
  aespb.addrin[0] = (long)FormAddress;

  aes_call (&aespb);

  return aespb.intout[0];
}


short
menu_bar (void * tree,
          short  mode) {
  OPCODE = 30;
  NO_INTIN = 1;
  NO_ADDRIN = 1;
  NO_INTOUT = 1;
  NO_ADDROUT = 0;

  aespb.addrin[0] = (long)tree;
  aespb.intin[0] = mode;

  aes_call (&aespb);

  return aespb.intout[0];
}


short
objc_draw (void * tree,
           short  start,
           short  depth,
           short  cx,
           short  cy,
           short  cw,
           short  ch) {
  OPCODE = 42;
  NO_INTIN = 6;
  NO_ADDRIN = 1;
  NO_INTOUT = 1;
  NO_ADDROUT = 0;

  aespb.intin[0] = start;
  aespb.intin[1] = depth;
  aespb.intin[2] = cx;
  aespb.intin[3] = cy;
  aespb.intin[4] = cw;
  aespb.intin[5] = ch;

  aespb.addrin[0] = tree;

  aes_call (&aespb);

  return aespb.intout[0];
}


short
rsrc_gaddr (short  Type,
            short  Index,
            void * Address) {
  OPCODE     = 112;
  NO_INTIN   = 2;
  NO_ADDRIN  = 1;
  NO_INTOUT  = 0;
  NO_ADDROUT = 1;

  aespb.intin[0] = Type;
  aespb.intin[1] = Index;

  aes_call (&aespb);

  *(long *)Address = aespb.addrout[0];

  return aespb.intout[0];
}


short
rsrc_rcfix (void * rc_header) {
  OPCODE = 115;
  NO_INTIN = 0;
  NO_ADDRIN = 1;
  NO_INTOUT = 1;

  aespb.addrin[0] = (long)rc_header;

  aes_call (&aespb);

  return aespb.intout[0];
}


short
wind_close (short WindowHandle)
{
  OPCODE = 102;
  NO_INTIN = 1;
  NO_ADDRIN = 1;
  NO_INTOUT = 0;

  aespb.intin[0] = WindowHandle;

  aes_call (&aespb);

  return aespb.intout[0];
}


short
wind_create (short Parts,
             short Wx,
             short Wy,
             short Ww,
             short Wh)
{
  OPCODE = 100;
  NO_INTIN = 5;
  NO_ADDRIN = 1;
  NO_INTOUT = 0;

  aespb.intin[0] = Parts;
  aespb.intin[1] = Wx;
  aespb.intin[2] = Wy;
  aespb.intin[3] = Ww;
  aespb.intin[4] = Wh;

  aes_call (&aespb);

  return aespb.intout[0];
}


short
wind_get (short   WindowHandle,
          short   What,
          short * W1,
          short * W2,
          short * W3,
          short * W4)
{
  OPCODE = 104;
  NO_INTIN = 2;
  NO_ADDRIN = 5;
  NO_INTOUT = 0;

  aespb.intin[0] = WindowHandle;
  aespb.intin[1] = What;

  aes_call (&aespb);

  *W1 = aespb.intout[1];
  *W2 = aespb.intout[2];
  *W3 = aespb.intout[3];
  *W4 = aespb.intout[4];
  return aespb.intout[0];
}


short
wind_open (short WindowHandle,
           short Wx,
           short Wy,
           short Ww,
           short Wh)
{
  OPCODE = 101;
  NO_INTIN = 5;
  NO_ADDRIN = 1;
  NO_INTOUT = 0;

  aespb.intin[0] = WindowHandle;
  aespb.intin[1] = Wx;
  aespb.intin[2] = Wy;
  aespb.intin[3] = Ww;
  aespb.intin[4] = Wh;

  aes_call (&aespb);

  return aespb.intout[0];
}


short
wind_set (short WindowHandle,
          short What,
          short parm1,
          short parm2,
          short parm3,
          short parm4)
{
  OPCODE = 105;
  NO_INTIN = 6;
  NO_ADDRIN = 1;
  NO_INTOUT = 0;

	
  aespb.intin[0] = WindowHandle;
  aespb.intin[1] = What;

  aespb.intin[2] = parm1;
  aespb.intin[3] = parm2;
  aespb.intin[4] = parm3;
  aespb.intin[5] = parm4;
	    
  aes_call (&aespb);

  return aespb.intout[0];
}


short
wind_update (short Code)
{
  OPCODE = 107;
  NO_INTIN = 1;
  NO_ADDRIN = 1;
  NO_INTOUT = 0;

  aespb.intin[0] = Code;

  aes_call (&aespb);

  return aespb.intout[0];
}
