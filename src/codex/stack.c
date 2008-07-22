/*=============================================================================

	Copyright (C) 2000 Kenneth Kocienda and David Hakim.
	This file is part of the Codex C Library.

	The Codex C Library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Library General Public License as
	published by the Free Software Foundation; either version 2 of the
	License, or (at your option) any later version.

	The Codex C Library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the GNU
	Library General Public License for more details.

	You should have received a copy of the GNU Library General Public
	License along with the Codex C Library; see the file COPYING.	If not,
	write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
	Boston, MA 02111-1307, USA.

	$RCSfile: stack.c,v $	
	$Revision: 1.16 $
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

#include "stack.h"
#include "stringbuffer.h"
#include "util.h"
#include "exception.h"
#include "excpfn.h"
#include "log.h"

static int stack_resize(Stack *p, int newcap);

enum {
	/* the factor by which the stack grows */
	STACK_RESIZE_FACTOR = 2,
	/* the capacity of the stack when created by stack_new_def()*/
	STACK_DEFAULT_CAPACITY	= 10,	  
};

Stack *
stack_create(int cap, int (*cfn)(const void *, const void *)) {
	return vec_create(cap,cfn);
}

Stack *
stack_createDefault() {
	return vec_createDefault();
}

Stack*
stack_clone(Stack* p) {
	return vec_clone(p);
}

void 
stack_free(Stack *p) {
	vec_free(p);
}

void 
stack_clear(Stack *p) {
	vec_clear(p);
}

int 
stack_contains(Stack *p, void *data) {
	int result = -1;
	int i;
	for (i = 0; i < p->cap; i++) {
		if ((*p->comp_fn)(p->elements[i], data)) {
			result = i;
			break;
		}
	}
	return result;
}

Enumeration *
stack_elements(Stack *p){
	return vec_elements(p);
}

int 
stack_equals(Stack *s1, Stack *s2) {
	return vec_equals(s1,s2);
}

int 
stack_notequals(Stack *s1, Stack *s2) {
	return vec_notequals(s1,s2);
}

void *
stack_peek(Stack *p) {
	void *data;
	excp_assert(p->size > 0, THROW_D("EmptyStackException"));	
	data = p->elements[p->size - 1];
	return data;
}

void *
stack_peekAt(Stack *p, int nth) {
	void *data;
	int index;
	int size;
	size = p->size;
	excp_assert(size > 0, THROW_D("EmptyStackException"));	
	excp_assert(nth > 0 && nth <= size, THROW_D("ArrayBoundsException"));	
	index = abs(size - nth);
	data = p->elements[index];
	return data;
}

void *
stack_pop(Stack *p) {
	void *data;
	excp_assert(p->size > 0, THROW_D("EmptyStackException"));	
	p->size--;
	data = p->elements[p->size];
	p->elements[p->size] = NULL;
	return data;
}

void 
stack_push(Stack *p, void *data) {
	if (p->size >= p->cap) {
		stack_resize(p, p->cap * STACK_RESIZE_FACTOR);
	}
	p->elements[p->size] = data;
	p->size++;
}

int 
stack_size(Stack *p) {
	return vec_size(p);
}

char *
stack_toString(Stack *p) {
	return vec_toString(p);
}

static int 
stack_resize(Stack *p, int newcap) {
	return vec_resize(p,newcap);
}

/*=============================================================================
// end of file: $RCSfile: stack.c,v $
==============================================================================*/

