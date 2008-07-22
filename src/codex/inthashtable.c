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

   $RCSfile: inthashtable.c,v $   
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

#include "inthashtable.h"
#include "stringbuffer.h"
#include "util.h"
#include "exception.h"
#include "excpfn.h"
#include "log.h"

#define IHTAB_MAX_DEPTH 4
#define IHTAB_DEFAULT_SIZE 101

static IntHashtableIterator *createIterator(IntHashtable *inthashtable);
static int next_key_fn(IntHashtableIterator *ihi);
static char has_more_keys_fn(IntHashtableIterator *ihi);
static void* next_value_fn(IntHashtableIterator *ihi);
static char has_more_values_fn(IntHashtableIterator *ihi);

static void ihtab_resize(IntHashtable *p);

IntHashtable *ihtab_create(int cap, short flags){
   int i;
   IntHashtable *p;

   /* create inthashtable */
   p=(IntHashtable*)emalloc(sizeof(IntHashtable));
	
   /* set other fields */
   p->cap = cap > 0 ? cap : IHTAB_DEFAULT_SIZE;
   p->size = 0;
   p->num_buckets = 0;
   p->buckets = (IBucket*)emalloc(sizeof(IBucket)*cap);
   p->flags = flags;
   
   /* init bucket stuff */
   for(i = 0; i < cap; i++){
      p->buckets[i].num_elements = 0;
      p->buckets[i].elements =0;
   }
   return p;
}

IntHashtable *ihtab_createDefault() {
   return ihtab_create(IHTAB_DEFAULT_SIZE, 0);
}

void ihtab_free(IntHashtable *p) {
   int i;
	
   for (i = 0; i < p->cap; i++) {
      IBucket *b = &p->buckets[i];
      IBucketNode *bn = b->elements;
      while(bn != NULL) {
         IBucketNode* next = bn->next;
         free(bn);
         bn = next;
      }
   }

   free(p->buckets);
   free(p);
}

void *ihtab_get(IntHashtable *p, int key){
   unsigned int hash;
   void *result = NULL; 
   IBucket *b;
   IBucketNode *bn;

   hash = key % p->cap;

   b = &p->buckets[hash];
   bn=b->elements;

   for( ;bn; bn = bn->next) {
      if(key == bn->key) {
         result = bn->value;
         break;
      }
   }
   
   return result;   
}

int ihtab_contains(IntHashtable *p, int key){
   unsigned int hash;
   int result = 0; 
   IBucket *b;
   IBucketNode *bn;

   hash = key % p->cap;

   b = &p->buckets[hash];
   bn=b->elements;

   for( ;bn; bn = bn->next) {
      if(key == bn->key) {
         result = 1;
         break;
      }
   }
   
   return result;   
}

int 
ihtab_equals(IntHashtable* p1, IntHashtable* p2){
	IntHashtable* is1 = (IntHashtable*)p1;
	IntHashtable* is2 = (IntHashtable*)p2;
	int i;

	if (p2 == NULL) 
		return 0;
	
	if (ihtab_size(is1) != ihtab_size(is2)) 
		return 0;

	for (i = 0; i < is1->cap; i++) {
		IBucket *b = &is1->buckets[i];
		IBucketNode *curnode = b->elements;
		while(curnode != NULL) {
			if (ihtab_get(is2,curnode->key) != curnode->value) 
				return 0;
			curnode = curnode->next;
		}
	}
	
	return 1;
}

int 
ihtab_notequals(IntHashtable* p1, IntHashtable* p2){
	return !ihtab_equals(p1, p2);
}

void *ihtab_put(IntHashtable *p, int key, void *value){
   unsigned int hash;
   void *result = NULL; 
   IBucket *b;
   IBucketNode *bn;
   
   hash = key % p->cap;
   b = &p->buckets[hash];
   bn= b->elements;

   for(;bn;bn=bn->next) {
      if(key == bn->key) {
         bn->value=value;
         result = bn->value;
         break;
      }
   }
   
   if (bn == NULL) {
      bn=(IBucketNode*)emalloc(sizeof(IBucketNode));
      bn->key = key;
      bn->value = value;
      result=bn->value;
      bn->next = b->elements;
      
      b->elements = bn;
      b->num_elements++;
      p->size++;
   
      if(!bn->next)
         p->num_buckets++;
   
      if(b->num_elements > IHTAB_MAX_DEPTH)
         ihtab_resize(p);
   }

   return result;
}

void *ihtab_remove(IntHashtable *p, int key){
   unsigned int hash;
   void *result = NULL; 
   IBucket *b;
   IBucketNode *bn;
   IBucketNode *pbn;
   IBucketNode *freebn;
      
   hash = key % p->cap;
   b = &p->buckets[hash];

   pbn = NULL;
   bn=b->elements;
	freebn = NULL;
	
   while(bn) {
      if(key == bn->key) {

         result = bn->value;

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
         if(b->num_elements == 0)
            p->num_buckets--;

   		p->size--;
         break;
      }
      pbn = bn;
      bn = bn->next;
   }
   
   return result;
}

void ihtab_clear(IntHashtable *p) {
   int i;
   
   for (i = 0; i < p->cap; i++) {
      IBucket *b = &p->buckets[i];
      IBucketNode *bn = b->elements;
      b->num_elements = 0;
      b->elements = 0;
      while(bn != NULL) {
         IBucketNode* next = bn->next;
         free(bn);
         bn = next;
      }
   }
   p->size = 0;

}

int ihtab_size(IntHashtable *p) {
   int result;
	result = p->size;
   return result;
}

IntEnumeration *ihtab_getKeys(IntHashtable *p){
   IntHashtableIterator *ihi;
   IntEnumeration *e;

   ihi = createIterator(p);
   e = ienum_create(
      (void *)ihi,
      (int (*)(void *))next_key_fn,
      (char(*)(void *))has_more_keys_fn
   );

   return e;
}

Enumeration *ihtab_getValues(IntHashtable *p){
   IntHashtableIterator *ihi;
   Enumeration *e;

   ihi = createIterator(p);
   e = enum_create(
      (void *)ihi,
      (void *(*)(void *))next_value_fn,
      (char(*)(void *))has_more_values_fn
   );

   return e;
}

int ihtab_density(IntHashtable *p){
   int result;
   result = (p->num_buckets *100) / p->cap;
   return result;
}

char *ihtab_toString(IntHashtable *p) {
	char *result = NULL;
	StringBuffer *buf = buf_createDefault();   
   IntEnumeration *e;  
   int i = 0;
 
   buf_putc(buf, '{');  
   for(e = ihtab_getKeys(p); ienum_hasNext(e); ) { 
      int curkey = ienum_next(e);
      void *curval = ihtab_get(p, curkey);
      
      if (i > 0) {
      	buf_puts(buf, ", ");
		}

      buf_printf(buf, "%d", curkey);
      buf_putc(buf, '=');
      buf_putp(buf, curval);
      
   	i++;
   }
   buf_putc(buf, '}');
   result = buf_toString(buf);
	free(e);
   
   return result;
}

static void ihtab_resize(IntHashtable *p){
   int i, newcap = p->cap *2;
   int newnum_buckets = 0;
   IBucket *newbuckets = (IBucket*)emalloc(sizeof(IBucket)*newcap);
   
   /* init bucket stuff */
   for(i = 0; i < newcap; i++){
      newbuckets[i].num_elements = 0;
      newbuckets[i].elements =0;
   }

   // Free the old buckets for now (in the future we should consider reuse)

   for (i = 0; i < p->cap; i++) {
      IBucket *b = &p->buckets[i];
      IBucketNode *curnode = b->elements;
      while(curnode != NULL) {
         IBucketNode* nextnode = curnode->next;

         // discover the new bucket this node belongs to 

         unsigned int hash = curnode->key % newcap; 

         // relink this node for that bucket
        
         IBucket *b = &newbuckets[hash];
         curnode->next = b->elements;
         b->elements = curnode;
         b->num_elements++;

         if(!curnode->next)
            newnum_buckets++;
    
         // continue iterating over the nodes of the old inthashtable

         curnode = nextnode;
      }
   }

   free(p->buckets);

   p->num_buckets = newnum_buckets;
   p->cap = newcap;
   p->buckets = newbuckets;
}

static IntHashtableIterator *createIterator(IntHashtable *p) {
   IntHashtableIterator*hi = emalloc(sizeof(IntHashtableIterator));

   hi->p = p;
   hi->cur_bucket = -1;
   hi->cur_bucket_node = (IBucketNode*)0;

   return hi;
}

static int next_key_fn(IntHashtableIterator *ihi) {
   int data;

   if(!ihi->cur_bucket_node) {
      while( ++(ihi->cur_bucket) < ihi->p->cap &&
            !(ihi->cur_bucket_node = ihi->p->buckets[ihi->cur_bucket].elements ) );
      if (ihi->cur_bucket == ihi->p->cap)
         return 0;
   }

   data = ihi->cur_bucket_node->key;
   ihi->cur_bucket_node = ihi->cur_bucket_node->next;
   return data;
}

static char has_more_keys_fn(IntHashtableIterator *ihi) {
   int cur_bucket = ihi->cur_bucket;

   if(ihi->cur_bucket_node) return (char)1;

   while(++cur_bucket < ihi->p->cap &&
         !ihi->p->buckets[cur_bucket].elements );
   if (cur_bucket != ihi->p->cap)
      return (char)1 ;

   return (char)0;
}

static void* next_value_fn(IntHashtableIterator *ihi) {
   void* data;

   if(!ihi->cur_bucket_node) {
      while( ++(ihi->cur_bucket) < ihi->p->cap &&
            !(ihi->cur_bucket_node = ihi->p->buckets[ihi->cur_bucket].elements ) );
      if (ihi->cur_bucket == ihi->p->cap)
         return 0;
   }

   data = ihi->cur_bucket_node->value;
   ihi->cur_bucket_node = ihi->cur_bucket_node->next;
   return data;
}

static char has_more_values_fn(IntHashtableIterator *ihi) {
   int cur_bucket = ihi->cur_bucket;

   if(ihi->cur_bucket_node) return (char)1;

   while(++cur_bucket < ihi->p->cap &&
         !ihi->p->buckets[cur_bucket].elements );
   if (cur_bucket != ihi->p->cap)
      return (char)1 ;

   return (char)0;
}
/*=============================================================================
// end of file: $RCSfile: inthashtable.c,v $
==============================================================================*/

