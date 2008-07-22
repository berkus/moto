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

   $RCSfile: hashset.c,v $   
   $Revision: 1.15 $
   $Author: dhakim $
   $Date: 2003/05/29 02:14:08 $
 
==============================================================================*/
#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>

#include "memsw.h"

#include "hashset.h"
#include "stringbuffer.h"
#include "util.h"
#include "exception.h"
#include "excpfn.h"
#include "log.h"
#include "blockpool.h"

#define HSET_MAX_DEPTH 4
#define HSET_DEFAULT_SIZE 101

static HashSetIterator *createIterator(HashSet *hashset);
static void *next_hash_key_fn(HashSetIterator *hi);
static char has_more_hash_keys_fn(HashSetIterator *hi);
static void hset_resize(HashSet *p);

static HSBucketNode* allocate_bn_arr(int size){
   return (HSBucketNode*)emalloc(size * sizeof(HSBucketNode)); }
static HSBucketNode* get_bn_from_arr(HSBucketNode* arr,int i) { 
   return &arr[i]; }
static void free_bn_arr(HSBucketNode* arr) { free (arr); }
   

HashSet *hset_create (
   int cap, 
   unsigned int (*hash_fn)(void *),
   int (*comp_fn)(const void *, const void *),
   short flags
){
   int i;
   HashSet *p;

   /* create hashset */
   p=(HashSet*)emalloc(sizeof(HashSet));
	
   /* p function pointers */
   if (hash_fn != NULL) {
   	p->hash_fn = hash_fn;
   }
   else {
    	p->hash_fn = void_hash;
   }
   if (comp_fn != NULL) {
   	p->comp_fn = comp_fn;
   }
   else {
    	p->comp_fn = void_comp;
   }
	
   /* p other fields */
   p->cap = cap > 0 ? cap : HSET_DEFAULT_SIZE;
   p->size = 0;
   p->num_buckets = 0;
   p->buckets = (HSBucket*)emalloc(sizeof(HSBucket)*cap);
   p->flags = flags;

   if(p->flags & HSET_POOLBNS) 
      p->bpool = bpool_create(
         (void *(*)(int))allocate_bn_arr,
         (void *(*)(void *,int))get_bn_from_arr,
         (void (*)(void *))free_bn_arr,
         NULL,
         NULL,
         cap
      );
   else
      p->bpool = NULL; 

   /* init bucket stuff */
   for(i = 0; i < cap; i++){
      p->buckets[i].num_elements = 0;
      p->buckets[i].elements = NULL;
   }
   return p;
}

HashSet *hset_createDefault() {
   return hset_create(HSET_DEFAULT_SIZE, NULL, NULL, 0);
}

void hset_free(HashSet *p) {
   int i;

   if (p->bpool == NULL){	
      for (i = 0; i < p->cap; i++) {
         HSBucket *b = &p->buckets[i];
         HSBucketNode *bn = b->elements;
         while(bn != NULL) {
            HSBucketNode* next = bn->next;
            free(bn);
            bn = next;
         }
      }
   } else {
      bpool_free(p->bpool);
   }

   free(p->buckets);
   free(p);
}

int hset_contains(HashSet *p, void *name) {
   unsigned int hash;
   int result = 0; 
   HSBucket *b;
   HSBucketNode *bn;

   hash = (*p->hash_fn)(name) % p->cap;

   b = &p->buckets[hash];
   bn=b->elements;

   for( ;bn; bn = bn->next) {
      if(!(*p->comp_fn)(name,bn->name)) {
         result = 1;
      }
   }
   
   return result;   
}

void* hset_get(HashSet *p, void *name) {
   unsigned int hash;
   void* result = NULL;
   HSBucket *b;
   HSBucketNode *bn;

   hash = (*p->hash_fn)(name) % p->cap;

   b = &p->buckets[hash];
   bn=b->elements;

   for( ;bn; bn = bn->next) {
      if(!(*p->comp_fn)(name,bn->name)) {
         result = bn->name;
      }
   }
  
   return result;
}

inline HSBucketNode* hset_grabBN(HashSet *p){
   if (p->bpool == NULL) return (HSBucketNode*)emalloc(sizeof(HSBucketNode));
   else return (HSBucketNode*)bpool_grab(p->bpool);
}

inline void hset_releaseBN(HashSet *p, HSBucketNode* hsbn){
   if (p->bpool == NULL) free(hsbn);
   else bpool_release(p->bpool,hsbn);
}

void hset_add(HashSet *p, void *name) {
   unsigned int hash;
   HSBucket *b;
   HSBucketNode *bn;
   int already_here = 0;

   hash = (*p->hash_fn)(name) % p->cap;
   b = &p->buckets[hash];
   bn= b->elements;

   for(;bn;bn=bn->next) {
      if(!(*p->comp_fn)(name,bn->name)) {
         already_here = 1;
         break;
      }
   }
   
   if (!already_here) {
      bn=hset_grabBN(p);
      bn->name = name;
      bn->next = b->elements;
      
      b->elements = bn;
      b->num_elements++;
      p->size++;
   
      if(!bn->next)
         p->num_buckets++;
   
      if(b->num_elements > HSET_MAX_DEPTH)
         hset_resize(p);
   }
   
}

int hset_remove(HashSet *p, void *name){
   unsigned int hash;
   int result = 0; 
   HSBucket *b;
   HSBucketNode *bn;
   HSBucketNode *pbn;
   HSBucketNode *freebn;
      
   hash = (*p->hash_fn)(name) % p->cap;
   b = &p->buckets[hash];

   pbn = NULL;
   bn=b->elements;
	freebn = NULL;
	
   while(bn) {
      if(!(*p->comp_fn)(name,bn->name)) {

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
	 hset_releaseBN(p,freebn);
			
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

void hset_clear(HashSet *p) {
   int i;
   
   for (i = 0; i < p->cap; i++) {
      HSBucket *b = &p->buckets[i];
      HSBucketNode *bn = b->elements;
      b->num_elements = 0;
      b->elements = 0;
      while(bn != NULL) {
         HSBucketNode* next = bn->next;
         hset_releaseBN(p,bn);
         bn = next;
      }
   }
   p->size = 0;

}

int hset_size(HashSet *p) {
   int result;
	result = p->size;
   return result;
}

Enumeration *hset_elements(HashSet *p){
   HashSetIterator *hi;
   Enumeration *e;

   hi = createIterator(p);
   e = enum_create(
      (void *)hi,
      (void *(*)(void *))next_hash_key_fn,
      (char(*)(void *))has_more_hash_keys_fn
   );

   return e;
}

int hset_density(HashSet *p){
   int result;
   result = (p->num_buckets *100) / p->cap;
   return result;
}

char *hset_toString(HashSet *p) {
	char *result = NULL;
	StringBuffer *buf = buf_createDefault();   
   Enumeration *e;  
   int i = 0;
   
   for(e = hset_elements(p); enum_hasNext(e); ) { 
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

static void hset_resize(HashSet *p){
   int i, newcap = p->cap *2;
   int newnum_buckets = 0;
   HSBucket *newbuckets = (HSBucket*)emalloc(sizeof(HSBucket)*newcap);
   
   for(i = 0; i < newcap; i++){
      newbuckets[i].num_elements = 0;
      newbuckets[i].elements = NULL;
   }

   // Free the old buckets for now (in the future we should consider reuse)

   for (i = 0; i < p->cap; i++) {
      HSBucket *b = &p->buckets[i];
      HSBucketNode *curnode = b->elements;
      while(curnode != NULL) {
         HSBucketNode* nextnode = curnode->next;

         // discover the new bucket this node belongs to 

         unsigned int hash = (*p->hash_fn)(curnode->name) % newcap; 

         // relink this node for that bucket
        
         HSBucket *b = &newbuckets[hash];
         curnode->next = b->elements;
         b->elements = curnode;
         b->num_elements++;

         if(!curnode->next)
            newnum_buckets++;
    
         // continue iterating over the nodes of the old hashset

         curnode = nextnode;
      }
   }

   free(p->buckets);

   p->num_buckets = newnum_buckets;
   p->cap = newcap;
   p->buckets = newbuckets;
}

static HashSetIterator *createIterator(HashSet *p) {
   HashSetIterator*hi = emalloc(sizeof(HashSetIterator));

   hi->p = p;
   hi->cur_bucket = -1;
   hi->cur_bucket_node = (HSBucketNode*)0;

   return hi;
}

static void *next_hash_key_fn(HashSetIterator *hi) {

   void *data;

   if(!hi->cur_bucket_node) {
      while( ++(hi->cur_bucket) < hi->p->cap &&
            !(hi->cur_bucket_node = hi->p->buckets[hi->cur_bucket].elements));
      if (hi->cur_bucket == hi->p->cap){
         return (void *)0;
      }
   }

   data = (void *)hi->cur_bucket_node->name;
   hi->cur_bucket_node = hi->cur_bucket_node->next;

   return data;
}

static char has_more_hash_keys_fn(HashSetIterator *hi){
   int cur_bucket = hi->cur_bucket;

   if(hi->cur_bucket_node) {
      return (char)1;
   }

   while(++cur_bucket < hi->p->cap &&
         !hi->p->buckets[cur_bucket].elements );
   if (cur_bucket != hi->p->cap){
      return (char)1 ;
   }
   
   return (char)0;
}

/*=============================================================================
// end of file: $RCSfile: hashset.c,v $
==============================================================================*/

