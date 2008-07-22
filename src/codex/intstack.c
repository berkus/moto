/*=============================================================================

   Copyright (C) 2000 Kenneth Kocienda and David Hakim.
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

   $RCSfile: intstack.c,v $   
   $Revision: 1.14 $
   $Author: dhakim $
   $Date: 2003/07/07 00:28:04 $
 
==============================================================================*/
#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>

#include "memsw.h"

#include "intstack.h"
#include "stringbuffer.h"
#include "util.h"
#include "exception.h"
#include "excpfn.h"
#include "log.h"

static IntStackIterator *createIterator(IntStack *v);
static int next_fn(IntStackIterator *isi);
static char has_next_fn(IntStackIterator *isi);
static int istack_resize(IntStack *p, int newcap);

enum {
   /* the factor by which the intstack grows */
   STACK_RESIZE_FACTOR = 2,
   /* the capacity of the intstack when created by istack_createDefault()*/
   STACK_DEFAULT_CAPACITY  = 10,   
};

IntStack *istack_create(int cap, short flags) {
	IntStack *p;
	int i;
	
   if (cap < STACK_DEFAULT_CAPACITY) {
		cap = STACK_DEFAULT_CAPACITY;
	}

   /* create intstack */
	p = (IntStack *)emalloc(sizeof(IntStack));
	
   /* create element array */
	p->elements = (int *)emalloc(cap * sizeof(int));
	for (i = 0; i < cap; i++) {
		p->elements[i] = 0;
	}
	
   /* set other fields */
   p->flags = flags;

   p->cap = cap;
	p->size = 0;
	
   return p;
}

IntStack *istack_createDefault() {
   return istack_create(STACK_DEFAULT_CAPACITY, 0);
}

void istack_free(IntStack *p) {
   free(p->elements);
   free(p);
}

void istack_clear(IntStack *p) {
	int i;
	for (i = 0; i < p->cap; i++) {
		p->elements[i] = 0;
	}
}

int istack_contains(IntStack *p, int data) {
	int result = -1;
	int i;
	for (i = 0; i < p->cap; i++) {
		if (p->elements[i] == data) {
			result = i;
			break;
		}
	}
	return result;
}

IntEnumeration *istack_elements(IntStack *p){
   IntStackIterator *isi;
   IntEnumeration *e;

   isi = createIterator(p);
   e = ienum_create(
      (void *)isi,
      (int(*)(void *))next_fn,
      (char(*)(void *))has_next_fn
   );

   return e;
}

int istack_equals(IntStack *s1, IntStack *s2) {
	/* equal if same IntStack */
	if (s1 == s2) {
		return 1;
	}
	/* could be equal if same size */
	else if (s1->size == s2->size) {
		/* check up to the nth element of the intstack with the smaller cap */
		/* if stacks have equal elements up to that point, they are equal */
		int i;
		for (i = 0; i < s1->size; i++) {
			if (s1->elements[i] != s2->elements[i]) {
				return 0;		
			}
		}
		return 1;			
	}
	return 0;
}

int istack_notequals(IntStack *s1, IntStack *s2) {
	return !istack_equals(s1, s2);
}

int istack_peek(IntStack *p) {
	int data;
   excp_assert(p->size > 0, THROW_D("EmptyStackException"));	
   data = p->elements[p->size - 1];
   return data;
}

int istack_peekAt(IntStack *p, int nth) {
	int data;
   int index;
   int size;
   size = p->size;
   excp_assert(size > 0, THROW_D("EmptyStackException"));	
   excp_assert(nth > 0 && nth <= size, THROW_D("ArrayBoundsException"));	
   index = abs(size - nth);
   data = p->elements[index];
   return data;
}

int istack_pop(IntStack *p) {
	int data;
   excp_assert(p->size > 0, THROW_D("EmptyStackException"));	
   p->size--;
   data = p->elements[p->size];
	p->elements[p->size] = 0;
   return data;
}

void istack_push(IntStack *p, int data) {
	if (p->size >= p->cap) {
		istack_resize(p, p->cap * STACK_RESIZE_FACTOR);
	}
	p->elements[p->size] = data;
	p->size++;
}

int istack_size(IntStack *p) {
   int result = 0;
	result = p->size;
   return result;
}

char *istack_toString(IntStack *p) {
	char *result = NULL;
	StringBuffer *buf = buf_createDefault();   
   IntEnumeration *e;  
   int i = 0;
   
   for(e = istack_elements(p); ienum_hasNext(e); ) { 
      int data = ienum_next(e);
		if (i > 0) {
      	buf_puts(buf, ", ");
		}
      buf_putc(buf, '{');
      buf_printf(buf, "%d", data);
      buf_putc(buf, '}');
   	i++;
   }
   result = buf_toString(buf);
	free(e);
   
   return result;
}

static int istack_resize(IntStack *p, int newcap) {
   excp_assert(newcap > p->cap, THROW_D("ArrayBoundsException"));	
	p->elements = (int *)realloc(p->elements, newcap * sizeof(int));
   excp_assert(p->elements != NULL, THROW_D("AllocationFailureException"));	
	p->cap = newcap;
	return 0;
}

static IntStackIterator *createIterator(IntStack *p) {
   IntStackIterator *isi = emalloc(sizeof(IntStackIterator));
   isi->p = p;
   isi->index = 0;
   isi->size = p->size;
   return isi;
}

static int next_fn(IntStackIterator *isi){
   int data;
   data = isi->p->elements[isi->index];
   isi->index++;
   return data;
}

static char has_next_fn(IntStackIterator *isi){
   return (isi->index < isi->size);
}

/*=============================================================================
// end of file: $RCSfile: intstack.c,v $
==============================================================================*/

