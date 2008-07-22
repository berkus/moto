/*=============================================================================

   Copyright (C) 2002 David Hakim.
   This file is part of the Codex C Library.

   The Codex C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The Codex C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the Codex C Library; see the file COPYING.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   $RCSfile: intboundedqueue.c,v $   
   $Revision: 1.3 $
   $Author: dhakim $
   $Date: 2002/12/05 02:40:48 $
 
==============================================================================*/
#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>

#include "memsw.h"
#include "excpfn.h"
#include "exception.h"
#include "intboundedqueue.h"

IntBoundedQueue* 
ibqueue_create(int cap){
   IntBoundedQueue* ibq = (IntBoundedQueue*)emalloc(sizeof(IntBoundedQueue) + cap*sizeof(int));
	ibq->start = ibq->end = 0;
	ibq->cap=cap;
	return ibq;
}

inline void 
ibqueue_enqueue(IntBoundedQueue* q,int data){
	if((q->end + 1) % q->cap == q->start)
		{ THROW_D("ArrayBoundsException"); }
	
   q->data[q->end] = data;
	q->end = (q->end + 1) % q->cap; 
}

inline void 
ibqueue_prequeue(IntBoundedQueue* q,int data){
	if((q->start - 1 + q->cap) % q->cap == q->end)
		{ THROW_D("ArrayBoundsException"); }
   
   q->start = (q->start - 1 + q->cap) % q->cap;
   q->data[q->start] = data;
}

void 
ibqueue_free(IntBoundedQueue* q){
	free(q);
}

inline char 
ibqueue_isEmpty(IntBoundedQueue* q){
   return q->start==q->end;
}

inline int 
ibqueue_dequeue(IntBoundedQueue* q){
	int result;
	
	if(q->start == q->end)
		{ THROW_D("EmptyQueueException"); }
	
	result = q->data[q->start];
	q->start = (q->start + 1) % q->cap;
	
	return result;
}

inline void 
ibqueue_clear(IntBoundedQueue* q){
	q->start = q->end = 0;
}

/*=============================================================================
// end of file: $RCSfile: intboundedqueue.c,v $
==============================================================================*/
