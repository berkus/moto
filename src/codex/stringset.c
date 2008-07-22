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

	$RCSfile: stringset.c,v $	 
	$Revision: 1.15 $
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

#include "stringset.h"
#include "stringbuffer.h"
#include "util.h"
#include "exception.h"
#include "excpfn.h"
#include "log.h"

#define SSET_MAX_DEPTH 4
#define SSET_DEFAULT_SIZE 101

static StringSetIterator *createIterator(StringSet *stringset);
static void *next_key_fn(StringSetIterator *hi);
static char has_more_keys_fn(StringSetIterator *hi);
static void sset_resize(StringSet *p);

StringSet *
sset_create (int cap) {
	int i;
	StringSet *p;

	/* create stringset */
	p=(StringSet*)emalloc(sizeof(StringSet));

	/* p other fields */
	p->cap = cap > 0 ? cap : SSET_DEFAULT_SIZE;
	p->size = 0;
	p->num_buckets = 0;
	p->buckets = (SSBucket*)emalloc(sizeof(SSBucket)*cap);

	/* init bucket stuff */
	for (i = 0; i < cap; i++) {
		p->buckets[i].num_elements = 0;
		p->buckets[i].elements = NULL;
	}
	return p;
}

StringSet * 
sset_clone (StringSet *ss) {
	int i;
	StringSet *p;

	/* create intset */
	p=sset_create(ss->cap);

	/* copy ints */	
	for (i = 0; i < ss->cap; i++) {
		SSBucket *b = &ss->buckets[i];
		SSBucketNode *curnode = b->elements;
		while(curnode != NULL) {
			sset_add(p,curnode->string);
			curnode = curnode->next;
		}
	}
	
	return p;
}

StringSet *
sset_createDefault() {
	return sset_create(SSET_DEFAULT_SIZE);
}

void 
sset_free(StringSet *p) {
	int i;

	for (i = 0; i < p->cap; i++) {
		SSBucket *b = &p->buckets[i];
		SSBucketNode *bn = b->elements;
		while (bn != NULL) {
			SSBucketNode* next = bn->next;
			free(bn);
			bn = next;
		}
	}

	free(p->buckets);

	free(p);
}

int 
sset_contains(StringSet *p, char *string) {
	unsigned int hash;
	int result = 0; 
	SSBucket *b;
	SSBucketNode *bn;

	hash = string_hash(string) % p->cap;

	b = &p->buckets[hash];
	bn=b->elements;

	for ( ;bn; bn = bn->next) {
		if (estrcmp(string, bn->string) == 0) {
			result = 1;
		}
	}

	return result;	  
}

char* 
sset_get(StringSet *p, char *string) {
	unsigned int hash;
	char* result = NULL;
	SSBucket *b;
	SSBucketNode *bn;

	hash = string_hash(string) % p->cap;

	b = &p->buckets[hash];
	bn=b->elements;

	for ( ;bn; bn = bn->next) {
		if (estrcmp(string, bn->string) == 0) {
			result = bn->string;
		}
	}

	return result;
}

void 
sset_add(StringSet *p, char *string) {
	unsigned int hash;
	SSBucket *b;
	SSBucketNode *bn;
	int already_here = 0;

	hash = string_hash(string) % p->cap;
	b = &p->buckets[hash];
	bn= b->elements;

	for ( ;bn; bn = bn->next) {
		if (estrcmp(string, bn->string) == 0) {
			already_here = 1;
			break;
		}
	}

	if (!already_here) {
		bn=(SSBucketNode*)emalloc(sizeof(SSBucketNode));
		bn->string = string;
		bn->next = b->elements;

		b->elements = bn;
		b->num_elements++;
		p->size++;

		if (!bn->next)
			p->num_buckets++;

		if (b->num_elements > SSET_MAX_DEPTH)
			sset_resize(p);
	}

}

void 
sset_addSet(StringSet *this, StringSet *p){
	int i;
	for (i = 0; i < p->cap; i++) {
		SSBucket *b = &p->buckets[i];
		SSBucketNode *curnode = b->elements;
		while(curnode != NULL) {
			sset_add(this,curnode->string);
			curnode = curnode->next;
		}
	}
}

int 
sset_remove(StringSet *p, char *string){
	unsigned int hash;
	int result = 0; 
	SSBucket *b;
	SSBucketNode *bn;
	SSBucketNode *pbn;
	SSBucketNode *freebn;

	hash = string_hash(string) % p->cap;
	b = &p->buckets[hash];

	pbn = NULL;
	bn=b->elements;
	freebn = NULL;

	while (bn) {
		if (estrcmp(string,bn->string) == 0) {

			result = 1;

			if (!pbn) {		// First node in bucket
				freebn = b->elements;
				b->elements = bn->next;
			}
			else {
				freebn = pbn->next;
				pbn->next = bn->next;
			}

			/*free bn */
			free(freebn);

			b->num_elements--;
			if (b->num_elements == 0)
				p->num_buckets--;

			p->size--;
			break;
		}
		pbn = bn;
		bn = bn->next;
	}

	return result;
}

void 
sset_removeSet(StringSet *this, StringSet *p){
	int i;
	for (i = 0; i < p->cap; i++) {
		SSBucket *b = &p->buckets[i];
		SSBucketNode *curnode = b->elements;
		while(curnode != NULL) {
			sset_remove(this,curnode->string);
			curnode = curnode->next;
		}
	}
}

void 
sset_clear(StringSet *p) {
	int i;

	for (i = 0; i < p->cap; i++) {
		SSBucket *b = &p->buckets[i];
		SSBucketNode *bn = b->elements;
		b->num_elements = 0;
		b->elements = 0;
		while (bn != NULL) {
			SSBucketNode* next = bn->next;
			free(bn);
			bn = next;
		}
	}
	p->size = 0;

}

int 
sset_size(StringSet *p) {
	int result;
	result = p->size;
	return result;
}

UnArray* 
sset_toArray(StringSet* p) {
	int i=0,length = p->size;
	UnArray* arr = arr_create(1,0,0,&length,ARR_REF_TYPE);
	Enumeration* e = sset_elements(p);
	while(enum_hasNext(e)){
		*rsub(arr,i) = enum_next(e);
		i++;
	}
	enum_free(e);
	return arr;
}

Enumeration *
sset_elements(StringSet *p){
	StringSetIterator *hi;
	Enumeration *e;

	hi = createIterator(p);
	e = enum_create(
						(void *)hi,
						(void *(*)(void *))next_key_fn,
						(char(*)(void *))has_more_keys_fn
						);

	return e;
}

int 
sset_density(StringSet *p){
	int result;
	result = (p->num_buckets *100) / p->cap;
	return result;
}

char *
sset_toString(StringSet *p) {
	char *result = NULL;
	StringBuffer *buf = buf_createDefault();	 
	Enumeration *e;  
	int i = 0;

	for (e = sset_elements(p); enum_hasNext(e); ) {
		void *curkey = (void *)enum_next(e);
		if (i > 0) {
			buf_puts(buf, ", ");
		}
		buf_putc(buf, '{');
		buf_puts(buf, curkey);
		buf_putc(buf, '}');
		i++;
	}
	result = buf_toString(buf);
	free(e);

	return result;
}

static void 
sset_resize(StringSet *p){
	int i, newcap = p->cap *2;
	int newnum_buckets = 0;
	SSBucket *newbuckets = (SSBucket*)emalloc(sizeof(SSBucket)*newcap);

	for (i = 0; i < newcap; i++) {
		newbuckets[i].num_elements = 0;
		newbuckets[i].elements = NULL;
	}

	// Free the old buckets for now (in the future we should consider reuse)

	for (i = 0; i < p->cap; i++) {
		SSBucket *b = &p->buckets[i];
		SSBucketNode *curnode = b->elements;
		while (curnode != NULL) {
			SSBucketNode* nextnode = curnode->next;

			// discover the new bucket this node belongs to 

			unsigned int hash = string_hash(curnode->string) % newcap; 

			// relink this node for that bucket

			SSBucket *b = &newbuckets[hash];
			curnode->next = b->elements;
			b->elements = curnode;
			b->num_elements++;

			if (!curnode->next)
				newnum_buckets++;

			// continue iterating over the nodes of the old stringset

			curnode = nextnode;
		}
	}

	free(p->buckets);

	p->num_buckets = newnum_buckets;
	p->cap = newcap;
	p->buckets = newbuckets;
}

/*=============================================================================
// Enumeration methods
==============================================================================*/

static StringSetIterator *
createIterator(StringSet *p) {
	StringSetIterator *hi = emalloc(sizeof(StringSetIterator));

	hi->p = p;
	hi->cur_bucket = -1;
	hi->cur_bucket_node = (SSBucketNode*)0;

	return hi;
}

static void *
next_key_fn(StringSetIterator *hi){
	void *data;

	if (!hi->cur_bucket_node) {
		while ( ++(hi->cur_bucket) < hi->p->cap &&
				  !(hi->cur_bucket_node = hi->p->buckets[hi->cur_bucket].elements));
		if (hi->cur_bucket == hi->p->cap)
			return(void *)0;
	}

	data = (void *)hi->cur_bucket_node->string;
	hi->cur_bucket_node = hi->cur_bucket_node->next;
	return data;
}

static char 
has_more_keys_fn(StringSetIterator *hi){
	int cur_bucket = hi->cur_bucket;

	if (hi->cur_bucket_node) return(char)1;

	while (++cur_bucket < hi->p->cap && !hi->p->buckets[cur_bucket].elements);
	if (cur_bucket != hi->p->cap)
		return(char)1 ;

	return(char)0;
}

/*=============================================================================
// Overloaded Operations
==============================================================================*/

StringSet * 
sset_union(StringSet * i1, StringSet * i2) {

	StringSet *p;

	p = sset_clone(i1);
	sset_addSet(p, i2);

	return p;
		
}

StringSet * 
sset_intersection(StringSet * is1, StringSet * is2) {
	int i;
	StringSet *p;

	p = sset_createDefault();
	
	for (i = 0; i < is1->cap; i++) {
		SSBucket *b = &is1->buckets[i];
		SSBucketNode *curnode = b->elements;
		while(curnode != NULL) {
			if (sset_contains(is2,curnode->string)) 
				sset_add(p,curnode->string);
			curnode = curnode->next;
		}
	}

	return p;
		
}

StringSet * 
sset_difference(StringSet * i1, StringSet * i2) {

	StringSet *p;

	p = sset_clone(i1);
	sset_removeSet(p, i2);

	return p;
		
}

int 
sset_equals(StringSet* p1, StringSet* p2){
	StringSet* is1 = (StringSet*)p1;
	StringSet* is2 = (StringSet*)p2;
	int i;

	if (p2 == NULL) 
		return 0;
	
	if (sset_size(is1) != sset_size(is2)) 
		return 0;

	for (i = 0; i < is1->cap; i++) {
		SSBucket *b = &is1->buckets[i];
		SSBucketNode *curnode = b->elements;
		while(curnode != NULL) {
			if (!sset_contains(is2,curnode->string)) 
				return 0;
			curnode = curnode->next;
		}
	}
	
	return 1;
}

int 
sset_notequals(StringSet* p1, StringSet* p2){
	return !sset_equals(p1, p2);
}


/*=============================================================================
// end of file: $RCSfile: stringset.c,v $
==============================================================================*/

