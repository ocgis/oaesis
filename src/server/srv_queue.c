/*
** srv_queue.c
**
** Copyright 1999 Christer Gustavsson <cg@nocrew.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**  
** Read the file COPYING for more information.
*/

#include <stdlib.h>

#include "srv_queue.h"

typedef struct element_s * ELEMENT;
#define ELEMENT_NIL ((ELEMENT)NULL)

typedef struct element_s {
  INFO_REF info;
  ELEMENT  next;
} ELEMENT_S;

typedef struct queue_s {
  ELEMENT first;
  ELEMENT last;
  ELEMENT free;
} QUEUE_S;


/*
** Exported
**
** 1999-02-06 CG
*/
QUEUE
allocate_queue (void) {
  QUEUE q;

  q = (QUEUE)malloc (sizeof (QUEUE_S));

  q->first = ELEMENT_NIL;
  q->last = ELEMENT_NIL;
  q->free = ELEMENT_NIL;

  return q;
}


/*
** Exported
**
** 1999-02-06 CG
*/
void
free_queue (QUEUE q) {
  while (q->free != ELEMENT_NIL) {
    ELEMENT e;
    
    e = q->free;
    q->free = e->next;
    free (e);
  }

  free (q);
}


/*
** Description
** Allocate a new element
**
** 1999-02-06 CG
*/
inline
ELEMENT
ealloc (QUEUE q) {
  if (q->free == ELEMENT_NIL) {
    return (ELEMENT)malloc (sizeof (ELEMENT_S));
  } else {
    ELEMENT e;

    e = q->free;
    q->free = q->free->next;

    return e;
  }
}


/*
** Description
** Free an element
**
** 1999-02-06 CG
*/
inline
void
efree (QUEUE   q,
       ELEMENT e) {
  e->next = q->free;
  q->free = e;
}


/*
** Exported
**
** 1999-02-06 CG
** 1999-02-13 CG
*/
void
insert_last (QUEUE    q,
             INFO_REF i) {
  ELEMENT e = ealloc (q);

  e->info = i;
  e->next = ELEMENT_NIL;

  if (q->last == ELEMENT_NIL) {
    /* No elements in queue */
    q->first = q->last = e;
  } else {
    q->last->next = e;
    q->last = e;
  }
}

/*
** Exported
**
** 1999-02-06 CG
*/
INFO_REF
pop_first (QUEUE q) {
  if (q->first == ELEMENT_NIL) {
    return INFO_REF_NIL;
  } else {
    ELEMENT  e;
    INFO_REF i;

    e = q->first;
    i = e->info;

    if (q->first == q->last) {
      q->first = q->last = ELEMENT_NIL;
    } else {
      q->first = q->first->next;
    }

    efree (q, e);

    return i;
  }
}
