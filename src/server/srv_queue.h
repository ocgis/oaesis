/*
** srv_queue.h
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

#ifndef _SRV_QUEUE_H_
#define _SRV_QUEUE_H_

typedef struct queue_s * QUEUE;
#define QUEUE_NIL ((QUEUE)NULL);

typedef void * INFO_REF;
#define INFO_REF_NIL ((INFO_REF)NULL);

/*
** Description
** Allocate a new queue
**
** 1999-02-06 CG
*/
QUEUE
allocate_queue (void);

/*
** Description
** Free the memory held by a queue
**
** 1999-02-06 CG
*/
void
free_queue (QUEUE q);

/*
** Description
** Queue an element
**
** 1999-02-06 CG
*/
void
insert_last (QUEUE    q,
             INFO_REF i);

/*
** Description
** Get the first element out of the queue
**
** 1999-02-06 CG
*/
INFO_REF
pop_first (QUEUE q);

#endif /* _SRV_QUEUE_H_ */
