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

   $RCSfile: intset.c,v $   
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

#include <stdlib.h>

#include "memsw.h"

#include "intset.h"
#include "stringbuffer.h"
#include "util.h"
#include "exception.h"
#include "excpfn.h"
#include "log.h"

#define ISET_MAX_DEPTH 4
#define ISET_DEFAULT_SIZE 101

static IntSetIterator *createIterator(IntSet *intset);
static int next_key_fn(IntSetIterator *hi);
static char has_more_keys_fn(IntSetIterator *hi);
static void iset_resize(IntSet *p);

IntSet *iset_create (int cap, short flags) {
   int i;
   IntSet *p;

   /* create intset */
   p=(IntSet*)emalloc(sizeof(IntSet));
	
   /* p other fields */
   p->cap = cap > 0 ? cap : ISET_DEFAULT_SIZE;
   p->size = 0;
   p->num_buckets = 0;
   p->buckets = (ISBucket*)emalloc(sizeof(ISBucket)*cap);
   p->flags = flags;

   /* init bucket stuff */
   for(i = 0; i < cap; i++){
      p->buckets[i].num_elements = 0;
      p->buckets[i].elements = NULL;
   }
   return p;
}

IntSet * iset_clone (IntSet *is) {
   int i;
   IntSet *p;

   /* create intset */
   p=iset_create(is->cap, is->flags);

   /* copy ints */   
   for (i = 0; i < is->cap; i++) {
      ISBucket *b = &is->buckets[i];
      ISBucketNode *curnode = b->elements;
      while(curnode != NULL) {
         iset_add(p,curnode->key);
         curnode = curnode->next;
      }
   }
   
   return p;
}

unsigned int iset_hash(void* p1){
   IntSet* is1 = (IntSet*)p1;
   unsigned int h = 0;
   int i;

   for (i = 0; i < is1->cap; i++) {
      ISBucket *b = &is1->buckets[i];
      ISBucketNode *curnode = b->elements;
      while(curnode != NULL) {
         h= h + curnode->key;
         curnode = curnode->next;
      }
   }
  
   return h*31;
}

int iset_comp(void* p1, void* p2){
   IntSet* is1 = (IntSet*)p1;
   IntSet* is2 = (IntSet*)p2;
   int i;

   if (iset_size(is1) != iset_size(is2)) 
      return -1;

   for (i = 0; i < is1->cap; i++) {
      ISBucket *b = &is1->buckets[i];
      ISBucketNode *curnode = b->elements;
      while(curnode != NULL) {
         if (!iset_contains(is2,curnode->key)) 
            return -1;
         curnode = curnode->next;
      }
   }
   
   return 0;
}

IntSet *iset_createDefault() {
   return iset_create(ISET_DEFAULT_SIZE, 0);
}

void iset_free(IntSet *p) {
   int i;
	
   for (i = 0; i < p->cap; i++) {
      ISBucket *b = &p->buckets[i];
      ISBucketNode *bn = b->elements;
      while(bn != NULL) {
         ISBucketNode* next = bn->next;
         free(bn);
         bn = next;
      }
   }

   free(p->buckets);
   free(p);
}

int iset_contains(IntSet *p, int key) {
   unsigned int hash;
   int result = 0; 
   ISBucket *b;
   ISBucketNode *bn;

   hash = key % p->cap;

   b = &p->buckets[hash];
   bn=b->elements;

   for( ;bn; bn = bn->next) {
      if(key == bn->key) {
         result = 1;
      }
   }
   
   return result;   
}

void iset_add(IntSet *p, int key) {
   unsigned int hash;
   ISBucket *b;
   ISBucketNode *bn;
   int already_here = 0;

   hash = key % p->cap;
   b = &p->buckets[hash];
   bn= b->elements;

   for( ;bn; bn = bn->next) {
      if(key == bn->key) {
         already_here = 1;
         break;
      }
   }
   
   if (!already_here) {
      bn=(ISBucketNode*)emalloc(sizeof(ISBucketNode));
      bn->key = key;
      bn->next = b->elements;
      
      b->elements = bn;
      b->num_elements++;
      p->size++;
   
      if(!bn->next)
         p->num_buckets++;
   
      if(b->num_elements > ISET_MAX_DEPTH)
         iset_resize(p);
   }

}

void iset_addSet(IntSet *this, IntSet *p){
   int i;
   for (i = 0; i < p->cap; i++) {
      ISBucket *b = &p->buckets[i];
      ISBucketNode *curnode = b->elements;
      while(curnode != NULL) {
         iset_add(this,curnode->key);
         curnode = curnode->next;
      }
   }
}

int iset_remove(IntSet *p, int key){
   unsigned int hash;
   int result = 0; 
   ISBucket *b;
   ISBucketNode *bn;
   ISBucketNode *pbn;
   ISBucketNode *freebn;
      
   hash = key % p->cap;
   b = &p->buckets[hash];

   pbn = NULL;
   bn=b->elements;
	freebn = NULL;
	
   while(bn) {
      if(key==bn->key) {

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

void iset_removeSet(IntSet *this, IntSet *p){
   int i;
   for (i = 0; i < p->cap; i++) {
      ISBucket *b = &p->buckets[i];
      ISBucketNode *curnode = b->elements;
      while(curnode != NULL) {
         iset_remove(this,curnode->key);
         curnode = curnode->next;
      }
   }
}

void iset_clear(IntSet *p) {
   int i;
   
   for (i = 0; i < p->cap; i++) {
      ISBucket *b = &p->buckets[i];
      ISBucketNode *bn = b->elements;
      b->num_elements = 0;
      b->elements = 0;
      while(bn != NULL) {
         ISBucketNode* next = bn->next;
         free(bn);
         bn = next;
      }
   }
   p->size = 0;

}

int iset_size(IntSet *p) {
   int result;
	result = p->size;
   return result;
}

IntEnumeration *iset_elements(IntSet *p){
   IntSetIterator *hi;
   IntEnumeration *e;

   hi = createIterator(p);
   e = ienum_create(
      (void *)hi,
      (int (*)(void *))next_key_fn,
      (char(*)(void *))has_more_keys_fn
   );

   return e;
}

int iset_density(IntSet *p){
   int result;
   result = (p->num_buckets *100) / p->cap;
   return result;
}

char *iset_toString(IntSet *p) {
	char *result = NULL;
	StringBuffer *buf = buf_createDefault();   
   IntEnumeration *e;  
   int i = 0;

   buf_putc(buf, '{');   
   for(e = iset_elements(p); ienum_hasNext(e); ) { 
      int curkey = ienum_next(e);
		if (i > 0) {
      	buf_puts(buf, ",");
		}

      buf_puti(buf, curkey);

   	i++;
   }
   buf_putc(buf, '}');
   result = buf_toString(buf);
	free(e);
   
   return result;
}

static void iset_resize(IntSet *p){
   int i, newcap = p->cap *2;
   int newnum_buckets = 0;
   ISBucket *newbuckets = emalloc(sizeof(ISBucket) * newcap);
   
   /* init bucket stuff */
   for(i = 0; i < newcap; i++){
      newbuckets[i].num_elements = 0;
      newbuckets[i].elements = NULL;
   }

   /* Free the old buckets for now 
      (in the future we should consider reuse) */

   for (i = 0; i < p->cap; i++) {
      ISBucket *b = &p->buckets[i];
      ISBucketNode *curnode = b->elements;
      while(curnode != NULL) {
         ISBucketNode* nextnode = curnode->next;

         // discover the new bucket this node belongs to 

         unsigned int hash = curnode->key % newcap; 

         // relink this node for that bucket
        
         ISBucket *b = &newbuckets[hash];
         curnode->next = b->elements;
         b->elements = curnode;
         b->num_elements++;

         if(!curnode->next)
            newnum_buckets++;
    
         // continue iterating over the nodes of the old intset

         curnode = nextnode;
      }
   }

   free(p->buckets);

   p->num_buckets = newnum_buckets;
   p->cap = newcap;
   p->buckets = newbuckets;
}

static IntSetIterator *createIterator(IntSet *p) {
   IntSetIterator *hi = emalloc(sizeof(IntSetIterator));

   hi->p = p;
   hi->cur_bucket = -1;
   hi->cur_bucket_node = (ISBucketNode*)0;

   return hi;
}

static int next_key_fn(IntSetIterator *hi){

   int data;

   if(!hi->cur_bucket_node) {
      while( ++(hi->cur_bucket) < hi->p->cap &&
            !(hi->cur_bucket_node = hi->p->buckets[hi->cur_bucket].elements));
      if (hi->cur_bucket == hi->p->cap)
         return 0;
   }

   data = hi->cur_bucket_node->key;
   hi->cur_bucket_node = hi->cur_bucket_node->next;
   return data;
}

static char has_more_keys_fn(IntSetIterator *hi){
   int cur_bucket = hi->cur_bucket;

   if(hi->cur_bucket_node) return (char)1;

   while(++cur_bucket < hi->p->cap && !hi->p->buckets[cur_bucket].elements);
   if (cur_bucket != hi->p->cap)
      return (char)1 ;

   return (char)0;
}

/*=============================================================================
// end of file: $RCSfile: intset.c,v $
==============================================================================*/

