/*
** aesbind.c
**
** Copyright 1998 - 2000 Christer Gustavsson <cg@nocrew.org>
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
short _global[15];
static short intin[16];
static short intout[7];
static long  addrin[3];
static long  addrout[1];
static AESPB aespb = {contrl, _global, intin, intout, addrin, addrout};

#define OPCODE     aespb.contrl[0]
#define NO_INTIN   aespb.contrl[1]
#define NO_ADDRIN  aespb.contrl[2]
#define NO_INTOUT  aespb.contrl[3]
#define NO_ADDROUT aespb.contrl[4]

int
appl_exit (void)
{
  OPCODE = 19;
  NO_INTIN = 0;
  NO_ADDRIN = 1;
  NO_INTOUT = 0;

  aes_call (&aespb);

  return aespb.intout[0];
}


int
appl_find (char * fname) {
  OPCODE = 13;
  NO_INTIN = 0;
  NO_ADDRIN = 1;
  NO_INTOUT = 1;
  NO_ADDROUT = 0;
  
  aespb.addrin[0] = (long)fname;

  aes_call (&aespb);

  return aespb.intout[0];
}


int
appl_init (void)
{
  OPCODE = 10;
  NO_INTIN = 0;
  NO_ADDRIN = 0;
  NO_INTOUT = 1;
  NO_ADDROUT = 0;

  aes_call (&aespb);

  return aespb.intout[0];
}


int
appl_write (int    ap_id,
            int    length,
            void * msg) {
  OPCODE = 12;
  NO_INTIN = 2;
  NO_ADDRIN = 1;
  NO_INTOUT = 1;
  NO_ADDROUT = 0;

  aespb.intin[0] = ap_id;
  aespb.intin[1] = length;

  aespb.addrin[0] = (long)msg;

  aes_call (&aespb);

  return aespb.intout[0];
}


int
evnt_button (int   clicks,
             int   mask,
             int   state,
             int * mx,
             int * my,
             int * button,
             int * kstate) {
  OPCODE = 21;
  NO_INTIN = 3;
  NO_ADDRIN = 0;
  NO_INTOUT = 5;
  NO_ADDROUT = 0;

  aespb.intin[0] = clicks;
  aespb.intin[1] = mask;
  aespb.intin[2] = state;

  aes_call (&aespb);

  *mx = aespb.intout[1];
  *my = aespb.intout[2];
  *button = aespb.intout[3];
  *kstate = aespb.intout[4];

  return aespb.intout[0];
}


int
evnt_dclick (int new,
             int flag) {
  OPCODE = 26;
  NO_INTIN = 2;
  NO_ADDRIN = 0;
  NO_INTOUT = 1;
  NO_ADDROUT = 0;

  aespb.intin[0] = new;
  aespb.intin[0] = flag;

  aes_call (&aespb);

  return aespb.intout[0];
}


int
evnt_mesag (short msg[])
{
  OPCODE = 23;
  NO_INTIN = 0;
  NO_ADDRIN = 1;
  NO_INTOUT = 1;
  NO_ADDROUT = 1;

  aespb.addrin[0] = (long)msg;

  aes_call (&aespb);

  return aespb.intout[0];
}


int
evnt_multi (int           Type,
            int           Clicks,
            int           WhichButton,
            int           WhichState,
            int           EnterExit1,
            int           In1X,
            int           In1Y,
            int           In1W,
            int           In1H,
            int           EnterExit2,
            int           In2X, 
            int           In2Y,
            int           In2W,
            int           In2H,
            short         MesagBuf[],
            unsigned long Interval,
            int *         OutX,
            int *         OutY,
            int *         ButtonState,
            int *         KeyState,
            int *         Key,
            int *         ReturnCount)
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
  
  aespb.intin[14] = Interval & 0x0000ffff;
  aespb.intin[15] = (Interval >> 16) & 0x0000ffff;
  
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


int
evnt_timer(unsigned long interval)
{
  OPCODE = 24;
  NO_INTIN = 2;
  NO_ADDRIN = 0;
  NO_INTOUT = 1;
  NO_ADDROUT = 0;

  aespb.intin[0] = interval & 0x0000ffff;
  aespb.intin[1] = (interval >> 16) & 0x0000ffff;
  
  aes_call (&aespb);

  return aespb.intout[0];
}


int
form_alert (int    default_button,
            char * alertstr) {
  OPCODE = 52;
  NO_INTIN = 1;
  NO_ADDRIN = 1;
  NO_INTOUT = 1;
  NO_ADDROUT = 0;
  
  aespb.intin[0] = default_button;
  aespb.addrin[0] = (long)alertstr;

  aes_call (&aespb);
  
  return aespb.intout[0];
}


int
form_button(OBJECT * tree,
            int      obj,
            int      clicks,
            int *    nxtobj)
{
  OPCODE = 56;
  NO_INTIN = 2;
  NO_ADDRIN = 1;
  NO_INTOUT = 2;
  NO_ADDROUT = 0;
  
  aespb.addrin[0] = (long)tree;
  aespb.intin[0] = obj;
  aespb.intin[0] = clicks;

  aes_call (&aespb);
  
  *nxtobj = aespb.intout[1];

  return aespb.intout[0];
}


int
form_center (OBJECT * tree,
             int *    cx,
             int *    cy,
             int *    cw,
             int *    ch) {
  OPCODE = 54;
  NO_INTIN = 0;
  NO_ADDRIN = 1;
  NO_INTOUT = 5;
  NO_ADDROUT = 0;
  
  aespb.addrin[0] = (long)tree;

  aes_call (&aespb);
  
  *cx = aespb.intout[1];
  *cy = aespb.intout[2];
  *cw = aespb.intout[3];
  *ch = aespb.intout[4];

  return aespb.intout[0];
}


int
form_dial (int flag,
           int sx,
           int sy,
           int sw,
           int sh,
           int bx,
           int by,
           int bw,
           int bh)
{
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


int
form_do (void * tree,
         int    startobj)
{
  OPCODE     = 50;
  NO_INTIN   = 1;
  NO_ADDRIN  = 1;
  NO_INTOUT  = 1;
  NO_ADDROUT = 0;
  
  aespb.intin[0] = startobj;
    
  aespb.addrin[0] = (long)tree;

  aes_call (&aespb);
  
  return aespb.intout[0];
}


int
form_error (int code)
{
  OPCODE     = 53;
  NO_INTIN   = 1;
  NO_ADDRIN  = 0;
  NO_INTOUT  = 1;
  NO_ADDROUT = 0;
  
  aespb.intin[0] = code;
    
  aes_call (&aespb);
  
  return aespb.intout[0];
}


int
fsel_exinput (char *  Path,
              char *  File,
              int *   ExitButton,
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


int
fsel_input (char *  path,
            char *  file,
            int *   button) {
  OPCODE = 90;
  NO_INTIN = 0;
  NO_ADDRIN = 2;
  NO_INTOUT = 2;
  NO_ADDROUT = 0;
  
  aespb.addrin[0] = (long)path;
  aespb.addrin[1] = (long)file;
    
  aes_call (&aespb);
  
  *button = aespb.intout[1];

  return aespb.intout[0];
}


int
graf_dragbox (int   w,
              int   h,
              int   sx,
              int   sy,
              int   bx,
              int   by,
              int   bw,
              int   bh,
              int * endx,
              int * endy) {
  OPCODE = 71;
  NO_INTIN = 8;
  NO_ADDRIN = 0;
  NO_INTOUT = 3;

  aespb.intin[0] = w;
  aespb.intin[1] = h;
  aespb.intin[2] = sx;
  aespb.intin[3] = sy;
  aespb.intin[4] = bx;
  aespb.intin[5] = by;
  aespb.intin[6] = bw;
  aespb.intin[7] = bh;

  aes_call (&aespb);

  *endx = aespb.intout[1];
  *endy = aespb.intout[2];

  return aespb.intout[0];
}


int
graf_growbox (int x1,
              int y1,
              int w1,
              int h1,
              int x2,
              int y2,
              int w2,
              int h2)
{
  OPCODE = 73;
  NO_INTIN = 8;
  NO_ADDRIN = 0;
  NO_INTOUT = 1;

  aespb.intin[0] = x1;
  aespb.intin[1] = y1;
  aespb.intin[2] = h1;
  aespb.intin[3] = w1;
  aespb.intin[4] = x2;
  aespb.intin[5] = y2;
  aespb.intin[6] = w2;
  aespb.intin[7] = h2;

  aes_call (&aespb);

  return aespb.intout[0];
}


int
graf_handle (int * Wchar,
             int * Hchar,
             int * Wbox, 
             int * Hbox)
{
  OPCODE = 77;
  NO_INTIN = 0;
  NO_ADDRIN = 0;
  NO_INTOUT = 5;

  aes_call (&aespb);

  *Wchar = aespb.intout[1];
  *Hchar = aespb.intout[2];
  *Wbox  = aespb.intout[3];
  *Hbox  = aespb.intout[4];
  return aespb.intout[0];
}


int
graf_mkstate (int * mx,
              int * my,
              int * mb,
              int * ks) {
  OPCODE = 79;
  NO_INTIN = 0;
  NO_ADDRIN = 0;
  NO_INTOUT = 5;
  NO_ADDROUT = 0;

  aes_call (&aespb);

  *mx = aespb.intout[1];
  *my = aespb.intout[2];
  *mb  = aespb.intout[3];
  *ks  = aespb.intout[4];

  return aespb.intout[0];
}


int
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


int
graf_movebox (int   bw,
              int   bh,
              int   sx,
              int   sy,
              int   ex,
              int   ey) {
  OPCODE = 72;
  NO_INTIN = 6;
  NO_ADDRIN = 0;
  NO_INTOUT = 1;
  NO_ADDROUT = 0;

  aespb.intin[0] = bw;
  aespb.intin[1] = bh;
  aespb.intin[2] = sx;
  aespb.intin[3] = sy;
  aespb.intin[4] = ex;
  aespb.intin[5] = ey;

  aes_call (&aespb);

  return aespb.intout[0];
}


int
graf_rubberbox (int   bx,
                int   by,
                int   minw,
                int   minh,
                int * endw,
                int * endh) {
  OPCODE = 70;
  NO_INTIN = 4;
  NO_ADDRIN = 0;
  NO_INTOUT = 3;
  NO_ADDROUT = 0;

  aespb.intin[0] = bx;
  aespb.intin[1] = by;
  aespb.intin[2] = minw;
  aespb.intin[3] = minh;

  aes_call (&aespb);

  *endw = aespb.intout[1];
  *endh = aespb.intout[2];

  return aespb.intout[0];
}


int
graf_shrinkbox (int x1,
                int y1,
                int w1,
                int h1,
                int x2,
                int y2,
                int w2,
                int h2)
{
  OPCODE = 74;
  NO_INTIN = 8;
  NO_ADDRIN = 0;
  NO_INTOUT = 1;

  aespb.intin[0] = x1;
  aespb.intin[1] = y1;
  aespb.intin[2] = h1;
  aespb.intin[3] = w1;
  aespb.intin[4] = x2;
  aespb.intin[5] = y2;
  aespb.intin[6] = w2;
  aespb.intin[7] = h2;

  aes_call (&aespb);

  return aespb.intout[0];
}


int
graf_slidebox (OBJECT * tree,
               int      parent,
               int      obj,
               int      orient)
{
  OPCODE = 76;
  NO_INTIN = 3;
  NO_ADDRIN = 1;
  NO_INTOUT = 1;
  NO_ADDROUT = 0;

  aespb.addrin[0] = (long)tree;

  aespb.intin[0] = parent;
  aespb.intin[1] = obj;
  aespb.intin[2] = orient;

  aes_call (&aespb);

  return aespb.intout[0];
}


int
menu_bar (void * tree,
          int    mode)
{
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


int
menu_icheck (OBJECT * tree,
             int      obj,
             int      flag)
{
  OPCODE = 31;
  NO_INTIN = 2;
  NO_ADDRIN = 1;
  NO_INTOUT = 1;
  NO_ADDROUT = 0;

  aespb.addrin[0] = (long)tree;

  aespb.intin[0] = obj;
  aespb.intin[1] = flag;

  aes_call (&aespb);

  return aespb.intout[0];
}


int
menu_ienable (OBJECT * tree,
              int      obj,
              int    flag)
{
  OPCODE = 32;
  NO_INTIN = 2;
  NO_ADDRIN = 1;
  NO_INTOUT = 1;
  NO_ADDROUT = 0;

  aespb.addrin[0] = (long)tree;

  aespb.intin[0] = obj;
  aespb.intin[1] = flag;

  aes_call (&aespb);

  return aespb.intout[0];
}


int
menu_register (int  ap_id,
               char * title)
{
  OPCODE = 35;
  NO_INTIN = 1;
  NO_ADDRIN = 1;
  NO_INTOUT = 1;
  NO_ADDROUT = 0;

  aespb.intin[0] = ap_id;

  aespb.addrin[0] = (long)title;

  aes_call (&aespb);

  return aespb.intout[0];
}


int
menu_text (OBJECT * tree,
           int      obj,
           char *   text) {
  OPCODE = 4;
  NO_INTIN = 1;
  NO_ADDRIN = 2;
  NO_INTOUT = 1;
  NO_ADDROUT = 0;

  aespb.intin[0] = obj;

  aespb.addrin[0] = (long)tree;
  aespb.addrin[0] = (long)text;

  aes_call (&aespb);

  return aespb.intout[0];
}


int
menu_tnormal (OBJECT * tree,
              int      obj,
              int      flag)
{
  OPCODE = 33;
  NO_INTIN = 2;
  NO_ADDRIN = 1;
  NO_INTOUT = 1;
  NO_ADDROUT = 0;

  aespb.addrin[0] = (long)tree;

  aespb.intin[0] = obj;
  aespb.intin[1] = flag;

  aes_call (&aespb);

  return aespb.intout[0];
}


int
objc_add (OBJECT * tree,
          int      parent,
          int      child) {
  OPCODE = 40;
  NO_INTIN = 2;
  NO_ADDRIN = 1;
  NO_INTOUT = 1;
  NO_ADDROUT = 0;

  aespb.intin[0] = parent;
  aespb.intin[1] = child;

  aespb.addrin[0] = (long)tree;

  aes_call (&aespb);

  return aespb.intout[0];
}


int
objc_change (OBJECT * tree,
             int      obj,
             int      rsvd,
             int      ox,
             int      oy,
             int      ow,
             int      oh,
             int      newstate,
             int      drawflag)
{
  OPCODE = 47;
  NO_INTIN = 8;
  NO_ADDRIN = 1;
  NO_INTOUT = 1;
  NO_ADDROUT = 0;

  aespb.intin[0] = obj;
  aespb.intin[1] = rsvd;
  aespb.intin[2] = ox;
  aespb.intin[3] = oy;
  aespb.intin[4] = ow;
  aespb.intin[5] = oh;
  aespb.intin[6] = newstate;
  aespb.intin[7] = drawflag;

  aespb.addrin[0] = (long)tree;

  aes_call (&aespb);

  return aespb.intout[0];
}


int
objc_delete(OBJECT * tree,
            int      obj)
{
  OPCODE = 41;
  NO_INTIN = 1;
  NO_ADDRIN = 1;
  NO_INTOUT = 1;
  NO_ADDROUT = 0;

  aespb.intin[0] = obj;

  aespb.addrin[0] = (long)tree;

  aes_call (&aespb);

  return aespb.intout[0];
}


int
objc_draw (OBJECT * tree,
           int      start,
           int      depth,
           int      cx,
           int      cy,
           int      cw,
           int      ch)
{
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

  aespb.addrin[0] = (long)tree;

  aes_call (&aespb);

  return aespb.intout[0];
}


int
objc_find (OBJECT * tree,
           int      obj,
           int      depth,
           int      ox,
           int      oy)
{
  OPCODE = 43;
  NO_INTIN = 4;
  NO_ADDRIN = 1;
  NO_INTOUT = 1;
  NO_ADDROUT = 0;

  aespb.intin[0] = obj;
  aespb.intin[1] = depth;
  aespb.intin[2] = ox;
  aespb.intin[3] = oy;

  aespb.addrin[0] = (long)tree;

  aes_call (&aespb);

  return aespb.intout[0];
}


int
objc_offset (OBJECT * tree,
             int      obj,
             int *    ox,
             int *    oy) {
  OPCODE = 44;
  NO_INTIN = 1;
  NO_ADDRIN = 1;
  NO_INTOUT = 3;
  NO_ADDROUT = 0;

  aespb.intin[0] = obj;

  aespb.addrin[0] = (long)tree;

  aes_call (&aespb);

  *ox = aespb.intout[1];
  *oy = aespb.intout[2];

  return aespb.intout[0];
}


int
objc_order (OBJECT * tree,
            int      obj,
            int      pos) {
  OPCODE = 45;
  NO_INTIN = 2;
  NO_ADDRIN = 1;
  NO_INTOUT = 1;
  NO_ADDROUT = 0;

  aespb.intin[0] = obj;
  aespb.intin[1] = pos;

  aespb.addrin[0] = (long)tree;

  aes_call (&aespb);

  return aespb.intout[0];
}


int
rsrc_free(void)
{
  OPCODE     = 111;
  NO_INTIN   = 0;
  NO_ADDRIN  = 0;
  NO_INTOUT  = 1;
  NO_ADDROUT = 0;

  aes_call (&aespb);

  return aespb.intout[0];
}


int
rsrc_gaddr (int     type,
            int     index,
            void  * addr) {
  OPCODE     = 112;
  NO_INTIN   = 2;
  NO_ADDRIN  = 0;
  NO_INTOUT  = 1;
  NO_ADDROUT = 1;

  aespb.intin[0] = type;
  aespb.intin[1] = index;

  aes_call (&aespb);

  *(long *)addr = aespb.addrout[0];

  return aespb.intout[0];
}


int
rsrc_load (char * fname) {
  OPCODE     = 110;
  NO_INTIN   = 0;
  NO_ADDRIN  = 1;
  NO_INTOUT  = 1;
  NO_ADDROUT = 0;

  aespb.addrin[0] = (long)fname;

  aes_call (&aespb);

  return aespb.intout[0];
}


int
rsrc_obfix (OBJECT * tree,
            int      obj) {
  OPCODE     = 114;
  NO_INTIN   = 1;
  NO_ADDRIN  = 1;
  NO_INTOUT  = 1;
  NO_ADDROUT = 0;

  aespb.addrin[0] = (long)tree;

  aespb.intin[0] = obj;

  aes_call (&aespb);

  return aespb.intout[0];
}


int
rsrc_rcfix (void * rc_header) {
  OPCODE = 115;
  NO_INTIN = 0;
  NO_ADDRIN = 1;
  NO_INTOUT = 1;

  aespb.addrin[0] = (long)rc_header;

  aes_call (&aespb);

  return aespb.intout[0];
}


int
scrp_read (char * cpath) {
  OPCODE = 80;
  NO_INTIN = 0;
  NO_ADDRIN = 1;
  NO_INTOUT = 1;
  NO_ADDROUT = 0;

  aespb.addrin[0] = (long)cpath;

  aes_call (&aespb);

  return aespb.intout[0];  
}


int
scrp_write (char * cpath) {
  OPCODE = 81;
  NO_INTIN = 0;
  NO_ADDRIN = 1;
  NO_INTOUT = 1;
  NO_ADDROUT = 0;

  aespb.addrin[0] = (long)cpath;

  aes_call (&aespb);

  return aespb.intout[0];  
}


int
shel_envrn (char ** value,
            char *  name) {
  OPCODE = 125;
  NO_INTIN = 0;
  NO_ADDRIN = 2;
  NO_INTOUT = 1;
  NO_ADDROUT = 0;

  aespb.addrin[0] = (long)value;
  aespb.addrin[1] = (long)name;

  aes_call (&aespb);

  return aespb.intout[0];
}


int
shel_find (char * buf) {
  OPCODE = 124;
  NO_INTIN = 0;
  NO_ADDRIN = 1;
  NO_INTOUT = 1;
  NO_ADDROUT = 0;

  aespb.addrin[0] = (long)buf;

  aes_call (&aespb);

  return aespb.intout[0];
}


int
shel_get (char * buf,
          int    length) {
  OPCODE = 122;
  NO_INTIN = 1;
  NO_ADDRIN = 1;
  NO_INTOUT = 1;
  NO_ADDROUT = 0;

  aespb.addrin[0] = (long)buf;
  aespb.intin[0] = length;

  aes_call (&aespb);

  return aespb.intout[0];
}


int
shel_put (char * buf,
          int    length) {
  OPCODE = 123;
  NO_INTIN = 1;
  NO_ADDRIN = 1;
  NO_INTOUT = 1;
  NO_ADDROUT = 0;

  aespb.addrin[0] = (long)buf;
  aespb.intin[0] = length;

  aes_call (&aespb);

  return aespb.intout[0];
}


int
shel_read(char * name,
          char * tail)
{
  OPCODE = 120;
  NO_INTIN = 0;
  NO_ADDRIN = 2;
  NO_INTOUT = 1;
  NO_ADDROUT = 0;

  aespb.addrin[0] = (long)name;
  aespb.addrin[1] = (long)tail;

  aes_call (&aespb);

  return aespb.intout[0];
}


int
shel_write (int    mode,
            int    wisgr,
            int    wiscr,
            char * cmd,
            char * tail) {
  OPCODE = 121;
  NO_INTIN = 3;
  NO_ADDRIN = 2;
  NO_INTOUT = 1;
  NO_ADDROUT = 0;

  aespb.intin[0] = mode;
  aespb.intin[1] = wisgr;
  aespb.intin[2] = wiscr;

  aespb.addrin[0] = (long)cmd;
  aespb.addrin[1] = (long)tail;

  aes_call (&aespb);

  return aespb.intout[0];
}


int
wind_calc (int   request,
           int   kind,
           int   x1,
           int   y1,
           int   w1,
           int   h1,
           int * x2,
           int * y2,
           int * w2,
           int * h2)
{
  OPCODE = 108;
  NO_INTIN = 6;
  NO_ADDRIN = 0;
  NO_INTOUT = 5;
  NO_ADDROUT = 0;

  aespb.intin[0] = request;
  aespb.intin[1] = kind;
  aespb.intin[2] = x1;
  aespb.intin[3] = y1;
  aespb.intin[4] = w1;
  aespb.intin[5] = h1;

  aes_call (&aespb);

  *x2 = aespb.intout[1];
  *y2 = aespb.intout[2];
  *w2 = aespb.intout[3];
  *h2 = aespb.intout[4];

  return aespb.intout[0];
}


int
wind_close (int WindowHandle)
{
  OPCODE = 102;
  NO_INTIN = 1;
  NO_ADDRIN = 1;
  NO_INTOUT = 0;

  aespb.intin[0] = WindowHandle;

  aes_call (&aespb);

  return aespb.intout[0];
}


int
wind_create (int Parts,
             int Wx,
             int Wy,
             int Ww,
             int Wh)
{
  OPCODE = 100;
  NO_INTIN = 5;
  NO_ADDRIN = 0;
  NO_INTOUT = 1;

  aespb.intin[0] = Parts;
  aespb.intin[1] = Wx;
  aespb.intin[2] = Wy;
  aespb.intin[3] = Ww;
  aespb.intin[4] = Wh;

  aes_call (&aespb);

  return aespb.intout[0];
}


int
wind_delete (int handle)
{
  OPCODE = 103;
  NO_INTIN = 1;
  NO_ADDRIN = 0;
  NO_INTOUT = 1;

  aespb.intin[0] = handle;

  aes_call (&aespb);

  return aespb.intout[0];
}


int
wind_find (int x,
           int y)
{
  OPCODE = 106;
  NO_INTIN = 2;
  NO_ADDRIN = 0;
  NO_INTOUT = 1;
  NO_ADDROUT = 0;

  aespb.intin[0] = x;
  aespb.intin[1] = y;

  aes_call (&aespb);

  return aespb.intout[0];
}


int
wind_get (int   WindowHandle,
          int   What,
          int * W1,
          int * W2,
          int * W3,
          int * W4)
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


int
wind_open (int WindowHandle,
           int Wx,
           int Wy,
           int Ww,
           int Wh)
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


int
wind_set (int WindowHandle,
          int What,
          int parm1,
          int parm2,
          int parm3,
          int parm4)
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


int
wind_update (int Code)
{
  OPCODE = 107;
  NO_INTIN = 1;
  NO_ADDRIN = 1;
  NO_INTOUT = 0;

  aespb.intin[0] = Code;

  aes_call (&aespb);

  return aespb.intout[0];
}


void
r_get (GRECT * r,
       int   * x,
       int   * y,
       int   * width,
       int   * height) {
  *x = r->g_x;
  *y = r->g_y;
  *width = r->g_w;
  *height = r->g_h;
}


void
r_set (GRECT * r,
       int     x,
       int     y,
       int     width,
       int     height) {
  r->g_x = x;
  r->g_y = y;
  r->g_w = width;
  r->g_h = height;
}


void
rc_copy (GRECT * src,
         GRECT * dest) {
  *dest = *src;
}


int
rc_equal (GRECT * src,
          GRECT * dest) {
  return ((src->g_x == dest->g_x) &&
          (src->g_y == dest->g_y) &&
          (src->g_w == dest->g_w) &&
          (src->g_h == dest->g_h));
}


int
rc_intersect (GRECT * r1,
              GRECT * r2) {
#define max(a,b) ((a) > (b) ? a : b)
#define min(a,b) ((a) < (b) ? a : b)
  int tx, ty, tw, th, ret;
  
  tx = max (r2->g_x, r1->g_x);
  tw = min (r2->g_x + r2->g_w, r1->g_x + r1->g_w) - tx;

  if ((ret = (0 < tw))) {
    ty = max (r2->g_y, r1->g_y);
    th = min (r2->g_y + r2->g_h, r1->g_y + r1->g_h) - ty;
    if ((ret = (0 < th))) {
      r2->g_x = tx;
      r2->g_y = ty;
      r2->g_w = tw;
      r2->g_h = th;
    }
  }

  return ret;
#undef max
#undef min
}
