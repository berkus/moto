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

   $RCSfile: intqueue.c,v $   
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
#include "intqueue.h"

IntQueue* 
iqueue_createDefault(){
   return (IntQueue*)ecalloc(sizeof(IntQueue),1);
}

void 
iqueue_enqueue(IntQueue* q,int data){
   IntQueueNode* qn = emalloc(sizeof(IntQueueNode));
   qn->data = data;
   qn->next = NULL;
   qn->prev = q->rear;
   
   if(q->rear == NULL){
      q->front = q->rear = qn;
   } else {
      q->rear->next = qn;
      q->rear = qn;
   } 
}

void 
iqueue_prequeue(IntQueue* q,int data){
   IntQueueNode* qn = emalloc(sizeof(IntQueueNode));
   qn->data = data;
   qn->next = q->front;
   qn->prev = NULL;
   
   if(q->front == NULL){
      q->front = q->rear = qn;
   } else {
      q->front->prev= qn;
      q->front = qn;
   } 
}

void 
iqueue_free(IntQueue* q){
	IntQueueNode* qn = q->front;
	while(qn){
		IntQueueNode* tqn = qn->next;
		free(qn);
		qn=tqn;
	}
	free(q);
}

char 
iqueue_isEmpty(IntQueue* q){
   return q->front==NULL;
}

int 
iqueue_dequeue(IntQueue* q){
	IntQueueNode* qn;
	int result;
	
	if(!(qn=q->front)) { THROW_D("EmptyQueueException"); }
	
	if(qn->next != NULL ){
		qn->next->prev = NULL;
		q->front = qn->next;
	} else {
	   q->front = q->rear = NULL;   
	}
	
	result = qn->data;
	free(qn);
	return result;
}

/*=============================================================================
// end of file: $RCSfile: intqueue.c,v $
==============================================================================*/
