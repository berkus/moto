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

   $RCSfile: intboundedqueue.h,v $   
   $Revision: 1.2 $
   $Author: dhakim $
   $Date: 2002/09/10 22:14:07 $
 
==============================================================================*/
#ifndef __INTBOUNDEDQUEUE_H
#define __INTBOUNDEDQUEUE_H

#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/*-----------------------------------------------------------------------------
        typedefs
-----------------------------------------------------------------------------*/


typedef struct intBoundedQueue{
   int start;
   int end;
   int cap;
   int data[1];
} IntBoundedQueue;

IntBoundedQueue* ibqueue_create(int cap);

inline void ibqueue_enqueue(IntBoundedQueue* q,int data);

inline void ibqueue_prequeue(IntBoundedQueue* q,int data);

void ibqueue_free(IntBoundedQueue* q);

inline char ibqueue_isEmpty(IntBoundedQueue* q);

inline int ibqueue_dequeue(IntBoundedQueue* q);

inline void ibqueue_clear(IntBoundedQueue* q);

#endif

/*=============================================================================
// end of file: $RCSfile: intboundedqueue.h,v $
==============================================================================*/
