/*
** objc.h
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

#ifndef	__OBJC__
#define	__OBJC__

#include "types.h"

#include "lib_global.h"

#define BUTTONFRAME   2
#define DEFBUTFRAME   3
#define OUTLINESIZE   3
#define D3DSIZE       2
#define D3DOFFS       1

#define	LINETYPE		1
#define	FRAMECOLOR	1
#define	MARKTYPE		1
#define	MARKCOLOR	1
#define	TEXTSTYLE	1
#define	TEXTCOLOR	1
#define	FILLTYPE	1
#define	FILLNUMBER	1
#define	FILLCOLOR	1

/* Object handling macros */
#define OB_TYPE(ob)              CW_TO_HW((ob)->ob_type)
#define OB_TYPE_PUT(ob,val)      (ob)->ob_type = HW_TO_CW(val)

#define OB_FLAGS(ob)             CW_TO_HW((ob)->ob_flags)
#define OB_FLAGS_SET(ob,flags)   (ob)->ob_flags |= HW_TO_CW(flags)
#define OB_FLAGS_CLEAR(ob,flags) (ob)->ob_flags &= ~(HW_TO_CW(flags))

#define OB_NEXT(ob)              CW_TO_HW((ob)->ob_next)
#define OB_NEXT_PUT(ob,val)      (ob)->ob_next = HW_TO_CW(val)

#define OB_HEAD(ob)              CW_TO_HW((ob)->ob_head)
#define OB_HEAD_PUT(ob,val)      (ob)->ob_head = HW_TO_CW(val)

#define OB_TAIL(ob)              CW_TO_HW((ob)->ob_tail)
#define OB_TAIL_PUT(ob,val)      (ob)->ob_tail = HW_TO_CW(val)

#define OB_STATE(ob)             CW_TO_HW((ob)->ob_state)
#define OB_STATE_SET(ob,state)   (ob)->ob_state |= HW_TO_CW(state)
#define OB_STATE_CLEAR(ob,state) (ob)->ob_state &= ~(HW_TO_CW(state))
#define OB_STATE_PUT(ob,state)   (ob)->ob_state = HW_TO_CW(state)

#define OB_SPEC(ob)              CL_TO_HL((ob)->ob_spec.index)
#define OB_SPEC_PUT(ob,val)      (ob)->ob_spec.index = HL_TO_CL(val)

#define OB_X(ob)                 CW_TO_HW((ob)->ob_x)
#define OB_X_PUT(ob,val)         (ob)->ob_x = HW_TO_CW(val)

#define OB_Y(ob)                 CW_TO_HW((ob)->ob_y)
#define OB_Y_PUT(ob,val)         (ob)->ob_y = HW_TO_CW(val)

#define OB_WIDTH(ob)             CW_TO_HW((ob)->ob_width)
#define OB_WIDTH_PUT(ob,val)     (ob)->ob_width = HW_TO_CW(val)

#define OB_HEIGHT(ob)            CW_TO_HW((ob)->ob_height)
#define OB_HEIGHT_PUT(ob,val)    (ob)->ob_height = HW_TO_CW(val)

void do_objc_add(OBJECT *t,WORD p,WORD c);

/****************************************************************************
 * Objc_do_draw                                                             *
 *  Implementation of objc_draw().                                          *
 ****************************************************************************/
WORD              /* 0 if an error occured, or 1.                           */
Objc_do_draw(     /*                                                        */
WORD     vid,
OBJECT * t,       /* Resource tree.                                         */
WORD     start,   /* Start object.                                          */
WORD     depth,   /* Maximum draw depth.                                    */
RECT   * xywh);   /* Clipping rectangle.                                    */
/****************************************************************************/

/****************************************************************************
 * Objc_do_offset                                                           *
 *  Implementation of objc_offset().                                        *
 ****************************************************************************/
WORD              /* 0 if error, or 1.                                      */
Objc_do_offset(   /*                                                        */
OBJECT *t,        /* Resource tree.                                         */
WORD o,           /* Object index.                                          */
WORD *xy);        /* X and Y coordinates of object if successfull.          */
/****************************************************************************/

/****************************************************************************
 * Objc_do_find                                                             *
 *  Implementation of objc_find().                                          *
 ****************************************************************************/
WORD              /* Object index, or -1.                                   */
Objc_do_find(     /*                                                        */
OBJECT *t,        /* Resource tree to search.                               */
WORD startobject, /* Start object.                                          */
WORD depth,       /* Maximum depth.                                         */
WORD x,           /* X offset.                                              */
WORD y,           /* Y offset.                                              */
WORD level);      /* Current depth of search.                               */
/****************************************************************************/

/****************************************************************************
 * Objc_do_edit                                                             *
 *  Implementation of objc_edit().                                          *
 ****************************************************************************/
WORD              /* 0 if an error occured, or 1.                           */
Objc_do_edit(     /*                                                        */
WORD     vid,
OBJECT * tree,    /* Resource tree.                                         */
WORD     obj,     /* Object index.                                          */
WORD     kc,      /* Key code to process.                                   */
WORD   * idx,     /* Character index.                                       */
WORD     mode);   /* Edit mode.                                             */
/****************************************************************************/

/****************************************************************************
 * Objc_do_change                                                           *
 *  Implementation of objc_change().                                        *
 ****************************************************************************/
WORD                /* 0 if an error occured, or 1.                         */
Objc_do_change(     /*                                                      */
WORD     vid,
OBJECT * tree,      /* Resource tree.                                       */
WORD     obj,       /* Object index.                                        */
RECT   * clip,      /* Clipping rectangle.                                  */
WORD     newstate,  /* New object state.                                    */
WORD     drawflag); /* Drawing flag.                                        */
/****************************************************************************/

/****************************************************************************
 *  Objc_area_needed                                                        *
 *   Calculate how large area an object covers.                             *
 ****************************************************************************/
void              /*                                                        */
Objc_area_needed( /*                                                        */
OBJECT *tree,     /* Pointer to the root of the resource tree.              */
WORD   object,    /* Index of interesting object.                           */
RECT   *rect);    /* Buffer where the requested area size will be placed.   */
/****************************************************************************/

/****************************************************************************
 *  Objc_calc_clip                                                          *
 *   Calculate required clip area for object.                               *
 ****************************************************************************/
void              /*                                                        */
Objc_calc_clip(   /*                                                        */
OBJECT *tree,     /* Pointer to the root of the resource tree.              */
WORD   object,    /* Index of interesting object.                           */
RECT   *rect);    /* Buffer where the requested area size will be placed.   */
/****************************************************************************/

void Objc_add(AES_PB *apb);    /*0x0028*/
void Objc_delete(AES_PB *apb); /*0x0029*/
void Objc_draw(AES_PB *apb);   /*0x002a*/

/****************************************************************************
 * Objc_edit                                                                *
 *   0x002e objc_edit().                                                    *
 ****************************************************************************/
void              /*                                                        */
Objc_edit(        /*                                                        */
AES_PB *apb);     /* AES parameter block.                                   */
/****************************************************************************/

/****************************************************************************
 * Objc_change                                                              *
 *   0x002f objc_change().                                                  *
 ****************************************************************************/
void              /*                                                        */
Objc_change(      /*                                                        */
AES_PB *apb);     /* AES parameter block.                                   */
/****************************************************************************/

void	Objc_find(AES_PB *apb);	/*0x002b*/
void	Objc_offset(AES_PB *apb);/*0x002c*/

/****************************************************************************
 * Objc_sysvar                                                              *
 *  0x0030 objc_sysvar().                                                   *
 ****************************************************************************/
void              /*                                                        */
Objc_sysvar(      /*                                                        */
AES_PB *apb);     /* AES parameter block.                                   */
/****************************************************************************/

#endif
