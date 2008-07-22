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

   $RCSfile: intqueue.h,v $   
   $Revision: 1.2 $
   $Author: dhakim $
   $Date: 2002/09/10 22:14:07 $
 
==============================================================================*/
#ifndef __INTQUEUE_H
#define __INTQUEUE_H

#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/*-----------------------------------------------------------------------------
        typedefs
-----------------------------------------------------------------------------*/

typedef struct intQueueNode{
   struct intQueueNode* prev;
   struct intQueueNode* next;
   int data;
} IntQueueNode;

typedef struct intQueue{
   IntQueueNode* front;
   IntQueueNode* rear;
} IntQueue;

IntQueue* iqueue_createDefault();

void iqueue_enqueue(IntQueue* q,int data);

void iqueue_prequeue(IntQueue* q,int data);

void iqueue_free(IntQueue* q);

char iqueue_isEmpty(IntQueue* q);

int iqueue_dequeue(IntQueue* q);

#endif

/*=============================================================================
// end of file: $RCSfile: intqueue.h,v $
==============================================================================*/
