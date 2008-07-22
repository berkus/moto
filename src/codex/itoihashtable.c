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

   $RCSfile: itoihashtable.c,v $   
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

#include "itoihashtable.h"
#include "stringbuffer.h"
#include "util.h"
#include "exception.h"
#include "excpfn.h"
#include "log.h"

#define IHTAB_MAX_DEPTH 4
#define IHTAB_DEFAULT_SIZE 101

static ItoIHashtableIterator *createIterator(ItoIHashtable *inthashtable);

static int next_key_fn(ItoIHashtableIterator *ihi);
static char has_more_keys_fn(ItoIHashtableIterator *ihi);
static void itoi_resize(ItoIHashtable *p);

ItoIHashtable *itoi_create(int cap) {
   int i;
   ItoIHashtable *p;

   /* create inthashtable */
   p=(ItoIHashtable*)emalloc(sizeof(ItoIHashtable));
	
   /* set other fields */
   p->cap = cap > 0 ? cap : IHTAB_DEFAULT_SIZE;
   p->size = 0;
   p->num_buckets = 0;
   p->buckets = (ItoIBucket*)emalloc(sizeof(ItoIBucket)*cap);

   /* init bucket stuff */
   for(i = 0; i < cap; i++){
      p->buckets[i].num_elements = 0;
      p->buckets[i].elements =0;
   }
   return p;
}

ItoIHashtable *itoi_createDefault() {
   return itoi_create(IHTAB_DEFAULT_SIZE);
}

void itoi_free(ItoIHashtable *p) {
   int i;
	
   for (i = 0; i < p->cap; i++) {
      ItoIBucket *b = &p->buckets[i];
      ItoIBucketNode *bn = b->elements;
      while(bn != NULL) {
         ItoIBucketNode* next = bn->next;
         free(bn);
         bn = next;
      }
   }

   free(p->buckets);
   free(p);
}

int itoi_get(ItoIHashtable *p, int key){
   unsigned int hash;
   int result = 0; 
   ItoIBucket *b;
   ItoIBucketNode *bn;

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

int itoi_contains(ItoIHashtable *p, int key){
   unsigned int hash;
   int result = 0;
   ItoIBucket *b;
   ItoIBucketNode *bn;

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
itoi_equals(ItoIHashtable* p1, ItoIHashtable* p2){
	ItoIHashtable* is1 = (ItoIHashtable*)p1;
	ItoIHashtable* is2 = (ItoIHashtable*)p2;
	int i;

	if (p2 == NULL) 
		return 0;
	
	if (itoi_size(is1) != itoi_size(is2)) 
		return 0;

	for (i = 0; i < is1->cap; i++) {
		ItoIBucket *b = &is1->buckets[i];
		ItoIBucketNode *curnode = b->elements;
		while(curnode != NULL) {
			if (itoi_get(is2,curnode->key) != curnode->value) 
				return 0;
			curnode = curnode->next;
		}
	}
	
	return 1;
}

int 
itoi_notequals(ItoIHashtable* p1, ItoIHashtable* p2){
	return !itoi_equals(p1, p2);
}

int itoi_put(ItoIHashtable *p, int key, int value){
   unsigned int hash;
   int result = 0; 
   ItoIBucket *b;
   ItoIBucketNode *bn;
   
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
   
   if (!bn) {
      bn=(ItoIBucketNode*)emalloc(sizeof(ItoIBucketNode));
      bn->key = key;
      bn->value = value;
      bn->next = b->elements;
      
      b->elements = bn;
      b->num_elements++;
      p->size++;
   
      if(!bn->next)
         p->num_buckets++;
   
      if(b->num_elements > IHTAB_MAX_DEPTH)
         itoi_resize(p);
   }

   return result;
}

int itoi_remove(ItoIHashtable *p, int key){
   unsigned int hash;
   int result = 0; 
   ItoIBucket *b;
   ItoIBucketNode *bn;
   ItoIBucketNode *pbn;
   ItoIBucketNode *freebn;
      
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

void itoi_clear(ItoIHashtable *p) {
   int i;
   
   for (i = 0; i < p->cap; i++) {
      ItoIBucket *b = &p->buckets[i];
      ItoIBucketNode *bn = b->elements;
      b->num_elements = 0;
      b->elements = 0;
      while(bn != NULL) {
         ItoIBucketNode* next = bn->next;
         free(bn);
         bn = next;
      }
   }
   p->size = 0;

}

int itoi_size(ItoIHashtable *p) {
   int result;
	result = p->size;
   return result;
}

IntEnumeration *itoi_getKeys(ItoIHashtable *p){
   ItoIHashtableIterator *ihi;
   IntEnumeration *e;

   ihi = createIterator(p);
   e = ienum_create(
      (void *)ihi,
      (int (*)(void *))next_key_fn,
      (char(*)(void *))has_more_keys_fn
   );

   return e;
}

int itoi_density(ItoIHashtable *p){
   int result;
   result = (p->num_buckets *100) / p->cap;
   return result;
}

char *itoi_toString(ItoIHashtable *p) {
	char *result = NULL;
	StringBuffer *buf = buf_createDefault();   
   IntEnumeration *e;  
   int i = 0;
   
   for(e = itoi_getKeys(p); ienum_hasNext(e); ) { 
      int curkey = ienum_next(e);
      int curval = itoi_get(p, curkey);
      
      if (i > 0) {
      	buf_puts(buf, ", ");
		}
      buf_putc(buf, '{');
      buf_printf(buf, "%d", curkey);
      buf_putc(buf, '=');
      buf_puti(buf, curval);
      buf_putc(buf, '}');
   	i++;
   }
   result = buf_toString(buf);
	free(e);
   
   return result;
}

static void itoi_resize(ItoIHashtable *p){
   int i, newcap = p->cap *2;
   int newnum_buckets = 0;
   ItoIBucket *newbuckets = (ItoIBucket*)emalloc(sizeof(ItoIBucket)*newcap);
   
   // Free the old buckets for now (in the future we should consider reuse)

   for (i = 0; i < p->cap; i++) {
      ItoIBucket *b = &p->buckets[i];
      ItoIBucketNode *curnode = b->elements;
      while(curnode != NULL) {
         ItoIBucketNode* nextnode = curnode->next;

         // discover the new bucket this node belongs to 

         unsigned int hash = curnode->key % newcap; 

         // relink this node for that bucket
        
         ItoIBucket *b = &newbuckets[hash];
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

static ItoIHashtableIterator *createIterator(ItoIHashtable *p) {
   ItoIHashtableIterator *hi = emalloc(sizeof(ItoIHashtableIterator));

   hi->p = p;
   hi->cur_bucket = -1;
   hi->cur_bucket_node = (ItoIBucketNode*)0;

   return hi;
}

static int next_key_fn(ItoIHashtableIterator *ihi) {
   int data;

   if(!ihi->cur_bucket_node) {
      while( ++(ihi->cur_bucket) < ihi->p->cap &&
            !(ihi->cur_bucket_node = ihi->p->buckets[ihi->cur_bucket].elements));
      if (ihi->cur_bucket == ihi->p->cap)
         return 0;
   }

   data = ihi->cur_bucket_node->key;
   ihi->cur_bucket_node = ihi->cur_bucket_node->next;
   return data;
}

static char has_more_keys_fn(ItoIHashtableIterator *ihi) {
   int cur_bucket = ihi->cur_bucket;

   if(ihi->cur_bucket_node) return (char)1;

   while(++cur_bucket < ihi->p->cap &&
         !ihi->p->buckets[cur_bucket].elements );
   if (cur_bucket != ihi->p->cap)
      return (char)1 ;

   return (char)0;
}

/*=============================================================================
// end of file: $RCSfile: itoihashtable.c,v $
==============================================================================*/

