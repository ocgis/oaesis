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

#include "aesbind.h"

static short contrl[5];
static short global[15];
static short intin[16];
static short intout[7];
static long  addrin[2];
static long  addrout[1];
static AESPB aespb = {contrl, global, intin, intout, addrin, addrout};


short
appl_exit (void)
{
  aespb.contrl[0] = 19;
  aespb.contrl[1] = 0;
  aespb.contrl[2] = 1;
  aespb.contrl[3] = 0;

  aes_call (&aespb);

  return aespb.intout[0];
}


short
appl_init (void)
{
  aespb.contrl[0] = 10;
  aespb.contrl[1] = 0;
  aespb.contrl[2] = 1;
  aespb.contrl[3] = 0;

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
  aespb.contrl[0] = 25;
  aespb.contrl[1] = 16;
  aespb.contrl[2] = 7;
  aespb.contrl[3] = 1;
  
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
graf_handle (short * Wchar,
             short * Hchar,
             short * Wbox, 
             short * Hbox)
{
  aespb.contrl[0] = 77;
  aespb.contrl[1] = 0;
  aespb.contrl[2] = 5;
  aespb.contrl[3] = 0;

  aes_call (&aespb);

  *Wchar = aespb.intout[1];
  *Hchar = aespb.intout[2];
  *Wbox  = aespb.intout[3];
  *Hbox  = aespb.intout[4];
  return aespb.intout[0];
}


short
wind_close (short WindowHandle)
{
  aespb.contrl[0] = 102;
  aespb.contrl[1] = 1;
  aespb.contrl[2] = 1;
  aespb.contrl[3] = 0;

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
  aespb.contrl[0] = 100;
  aespb.contrl[1] = 5;
  aespb.contrl[2] = 1;
  aespb.contrl[3] = 0;

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
  aespb.contrl[0] = 104;
  aespb.contrl[1] = 2;
  aespb.contrl[2] = 5;
  aespb.contrl[3] = 0;

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
  aespb.contrl[0] = 101;
  aespb.contrl[1] = 5;
  aespb.contrl[2] = 1;
  aespb.contrl[3] = 0;

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
  aespb.contrl[0] = 105;
  aespb.contrl[1] = 6;
  aespb.contrl[2] = 1;
  aespb.contrl[3] = 0;

	
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
  aespb.contrl[0] = 107;
  aespb.contrl[1] = 1;
  aespb.contrl[2] = 1;
  aespb.contrl[3] = 0;

  aespb.intin[0] = Code;

  aes_call (&aespb);

  return aespb.intout[0];
}
