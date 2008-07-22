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

	$RCSfile: vector.c,v $	 
	$Revision: 1.26 $
	$Author: dhakim $
	$Date: 2003/07/07 00:28:04 $
 
==============================================================================*/
#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memsw.h"

#include "exception.h"
#include "excpfn.h"
#include "log.h"
#include "sort.h"
#include "stringbuffer.h"
#include "util.h"
#include "vector.h"
#include "cdx_function.h"

static VectorIterator *createIterator(Vector *p);
static void *next_fn(VectorIterator *vi);
static char has_next_fn(VectorIterator *vi);

enum {
	/* the factor by which the stack grows */
	VEC_RESIZE_FACTOR = 2,
	/* the capacity of the stack when created by stack_new_def()*/
	VEC_DEFAULT_CAPACITY	 = 10,	
};

Vector *
vec_create(int cap, int (*cfn)(const void *, const void *)) {
	Vector *p;
	int i;
	
	if (cap < VEC_DEFAULT_CAPACITY) {
		cap = VEC_DEFAULT_CAPACITY;
	}

	/* create vector */
	p = (Vector *)emalloc(sizeof(Vector));
	
	/* create element array */
	p->elements = (void **)emalloc(cap * sizeof(void *));
	
	for (i = 0; i < cap; i++) {
		p->elements[i] = NULL;
	}

	/* set other fields */
	if (cfn != NULL) {
		p->comp_fn = cfn;
	}
	else {
		p->comp_fn = void_comp;
	}
	
	p->cap = cap;
	p->size = 0;
	
	return p;
}

Vector *
vec_createDefault() {
	return vec_create(VEC_DEFAULT_CAPACITY, NULL );
}

Vector *
vec_clone(Vector* p) {
	Vector* r = vec_create(p->cap,NULL );
	int i=0;
	r->size=p->size;
	r->comp_fn = p->comp_fn;
	for(i=0;i<p->size;i++)
		r->elements[i] = p->elements[i];
	return r;
}

Vector *
vec_createFromArray(UnArray* x) {
	Vector* v = vec_create(x->meta.length + VEC_DEFAULT_CAPACITY, NULL );
	int i;
	v->size = x->meta.length;
	for (i = 0; i < v->size; i++) 
		v->elements[i] = x->ra.data[i];
	return v;
}


void 
vec_free(Vector *p) {
	free(p->elements);
	free(p);
}

int 
vec_add(Vector *p, void *data) {
	if (p->size >= p->cap) 
		vec_resize(p, p->cap * VEC_RESIZE_FACTOR);
	
	p->elements[p->size] = data;
	p->size++;
	return 0;
}

int 
vec_clear(Vector *p) {
	int i;
	for (i = 0; i < p->cap; i++) {
		p->elements[i] = NULL;
	}
	p->size = 0;
	return 0;
}

void 
vec_shuffle(Vector *p) {
	int j,k,t = p->size;
	void *temp;
	for (j = 0; j < t; j++) {
		k = random()%t;
		temp = p->elements[j];
		p->elements[j] = p->elements[k];
		p->elements[k] = temp;
	}
}

int 
vec_contains(Vector *p, void *data) {
	int result = 0;
	int i;
	for (i = 0; i < p->cap; i++) {
		if (p->elements[i] != NULL && ((*p->comp_fn)(p->elements[i], data) == 0)) {
			result = 1;
			break;
		}
	}
	return result;
}

Enumeration *
vec_elements(Vector *p){
	VectorIterator *vi;
	Enumeration *e;

	vi = createIterator(p);
	e = enum_create(
		(void *)vi,
		(void *(*)(void *))next_fn,
		(char(*)(void *))has_next_fn
	);

	return e;
}

void** 
vec_data(Vector *v1){
	return v1->elements;
}

int 
vec_equals(Vector *v1, Vector *v2) {
	/* equal if same Vector */
	if (v1 == v2) {
		return 1;
	}
	/* can't be equal if different comparators */
	else if (v1->comp_fn != v2->comp_fn) {
		return 0;
	}
	/* could be equal if same size */
	else if (v1->size == v2->size) {
		int i;
		for (i = 0; i < v1->size; i++) {
			if ((*v1->comp_fn)(v1->elements[i], v2->elements[i]) != 0) 
				return 0;		
		}
		return 1;			
	}
	return 0;
}

int 
vec_notequals(Vector *v1, Vector *v2) {
	return ! vec_equals(v1,v2); 
}

void *
vec_get(Vector *p, int index) {
	if (index >= p->size || index < 0) 
		THROW_D("ArrayBoundsException");
	
	return p->elements[index];
}

int 
vec_insert(Vector *p, void *data, int index) {
	if (index > p->size || index < 0) 
		THROW_D("ArrayBoundsException");		
	
	/* Grow the vectory if needed */
	if (p->size == p->cap) 
		vec_resize(p, p->cap * VEC_RESIZE_FACTOR);
	
	/*	Shift all data at positions >= index up one position. */
	if(index != p->size)
		memmove(p->elements + index + 1, p->elements + index, 
			(p->size - index) * sizeof(void*));

	p->elements[index] = data;
	p->size++;

	return 0;
}

int 
vec_remove(Vector *p, void *data) {
	int result = 0;
	int i;
	for (i = 0; i < p->size; i++) {
		if (p->elements[i] != NULL && ((*p->comp_fn)(p->elements[i], data) == 0)) {
			if (i == p->cap - 1) {
				p->elements[i] = NULL;
			}
			else {
				memmove(p->elements + i, p->elements + i + 1, 
					(p->size - i - 1) * sizeof(void *));
				p->elements[p->size - 1] = NULL;
			}
			p->size--;
			result++;
			break;
		}
	}
	return result;
}


void *
vec_removeAt(Vector *p, int index) {
	void *result = NULL;
	if (index >= p->size || index < 0) 
		THROW_D("ArrayBoundsException");
	
	result = p->elements[index];
	
	/*	Shift all data at positions >= index down one position. */
	if (index < p->size - 1) 
		memmove(p->elements + index, p->elements + index + 1, 
			(p->size - index - 1) * sizeof(void *));
	
	p->size--;
	
	return result;
}

void *
vec_setAt(Vector *p, void *data, int index) {
	void *old = NULL;
	if (index >= p->size || index < 0) 
		THROW_D("ArrayBoundsException");
		
	old = p->elements[index];
	p->elements[index] = data;

	return old;
}

void *
vec_setAtOverloaded(Vector *p, int index, void* data){
	vec_setAt(p,data,index);
	return data;
}

int 
vec_size(Vector *p) {
	return p->size;
}

void 
vec_sort_cmp(Vector *p, Function* cmp) {
	quick_sort(p->elements, p->size, cmp);
}

char *
vec_toString(Vector *p) {
	char *result = NULL;
	StringBuffer *buf = buf_createDefault();	 
	Enumeration *e;  
	int i = 0;
	
	for(e = vec_elements(p); enum_hasNext(e); ) { 
		void *data = (void *)enum_next(e);
		if (i > 0) {
			buf_puts(buf, ", ");
		}
		buf_putc(buf, '{');
		buf_puts(buf, data);
		buf_putc(buf, '}');
		i++;
	}
	result = buf_toString(buf);
	enum_free(e);
	
	return result;
}

UnArray* 
vec_toArray(Vector* p) {
	int i,length = p->size;
	UnArray* arr = arr_create(1,0,0,&length,ARR_REF_TYPE);
	for (i=0;i<p->size;i++){
		*rsub(arr,i) = p->elements[i];
	}
	return arr;
}

int 
vec_resize(Vector *p, int newcap) {
	excp_assert(newcap > p->cap, THROW_D("ArrayBoundsException"));
	p->elements = (void **)realloc(p->elements, newcap * sizeof(void *));
	excp_assert(p->elements != NULL, THROW_D("AllocationFailureException"));
	p->cap = newcap;
	return 0;
}

static VectorIterator *
createIterator(Vector *p) {
	VectorIterator *vi = (VectorIterator*)emalloc(sizeof(VectorIterator));
	vi->p = p;
	vi->index = 0;
	vi->size = p->size;
	return vi;
}

static void 
*next_fn(VectorIterator *vi){
	return vi->p->elements[vi->index++];
}

static char 
has_next_fn(VectorIterator *vi){
	return (vi->index < vi->size);
}

void
vec_each(Vector *p, Function* f){
	int i;
	for (i=0;i<p->size;i++){
		f->flags & F_INTERPRETED ? 
			ifunc_vcall(f,"O", p->elements[i]) : 
			((void(*)(Function*,void*))f->fn)(f, p->elements[i]) ;
	}
}

/*=============================================================================
// end of file: $RCSfile: vector.c,v $
==============================================================================*/

