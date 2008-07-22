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

   $RCSfile: hashtable.c,v $   
   $Revision: 1.14 $
   $Author: dhakim $
   $Date: 2002/07/12 22:54:43 $
 
==============================================================================*/
#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>

#include "memsw.h"

#include "hashtable.h"
#include "stringbuffer.h"
#include "exception.h"
#include "excpfn.h"
#include "log.h"
#include "util.h"

#define HTAB_MAX_DEPTH 5
#define HTAB_DEFAULT_SIZE 101

static HashtableIterator *createIterator(Hashtable *hashtable);
static void *next_hash_key_fn(HashtableIterator *hi);
static char has_more_hash_keys_fn(HashtableIterator *hi);
static void htab_resize(Hashtable *p);

Hashtable *htab_create (
   int cap, 
   unsigned int (*hash_fn)(void *),
   int (*comp_fn)(const void *, const void *),
   short flags
){
   int i;
   Hashtable *p;
   p = (Hashtable*)emalloc(sizeof(Hashtable));
	
   /* set function pointers */
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
	
   /* set other fields */
   p->cap = cap > 0 ? cap : HTAB_DEFAULT_SIZE;
   p->size = 0;
   p->num_buckets = 0;
   p->buckets = (Bucket*)emalloc(sizeof(Bucket)*cap);
   p->flags = flags;
   
   /* init bucket stuff */
   for(i = 0; i < cap; i++){
      p->buckets[i].num_elements = 0;
      p->buckets[i].elements =0;
   }
   return p;
}

Hashtable *htab_createDefault() {
   return htab_create(HTAB_DEFAULT_SIZE, NULL, NULL, 0);
}

void htab_free(Hashtable *p) {
   int i;
	
   for (i = 0; i < p->cap; i++) {
      Bucket *b = &p->buckets[i];
      BucketNode *bn = b->elements;
      while(bn != NULL) {
         BucketNode* next = bn->next;
         free(bn);
         bn = next;
      }
   }

   free(p->buckets);
   free(p);
}

void *htab_get(Hashtable *p, void *name){
   unsigned int hash;
   void *result = NULL; 
   Bucket *b;
   BucketNode *bn;

   hash = (*p->hash_fn)(name) % p->cap;

   b = &p->buckets[hash];
   bn=b->elements;

   for( ;bn; bn = bn->next) {
      if((*p->comp_fn)(name,bn->name) == 0) {
         result = bn->value;
         break;
      }
   }
   
   return result;   
}

int htab_contains(Hashtable *p, void *name){
   unsigned int hash;
   int result = 0;
   Bucket *b;
   BucketNode *bn;

   hash = (*p->hash_fn)(name) % p->cap;

   b = &p->buckets[hash];
   bn=b->elements;

   for( ;bn; bn = bn->next) {
      if((*p->comp_fn)(name,bn->name) == 0) {
         result = 1;
         break;
      }
   }
  
   return result;
}

void *htab_put(Hashtable *p, void *name, void *value){
   unsigned int hash;
   void *result = NULL; 
   Bucket *b;
   BucketNode *bn;
   
   hash = (*p->hash_fn)(name) % p->cap;
   b = &p->buckets[hash];
   bn= b->elements;

   for(;bn;bn=bn->next) {
      if((*p->comp_fn)(name,bn->name) == 0) {
         bn->value=value;
         result = bn->value;
         break;
      }
   }
   
   if (bn == NULL) {
      bn=(BucketNode*)emalloc(sizeof(BucketNode));
      bn->name = name;
      bn->value = value;
      result = bn->value;
      bn->next = b->elements;
      
      b->elements = bn;
      b->num_elements++;
      p->size++;
   
      if(!bn->next)
         p->num_buckets++;
   
      if(b->num_elements > HTAB_MAX_DEPTH)
         htab_resize(p);
   }

   return result;
}

void *htab_remove(Hashtable *p, void *name){
   unsigned int hash;
   void *result = NULL; 
   Bucket *b;
   BucketNode *bn;
   BucketNode *pbn;
   BucketNode *freebn;
      
   hash = (*p->hash_fn)(name) % p->cap;
   b = &p->buckets[hash];

   pbn = NULL;
   bn=b->elements;
	freebn = NULL;
	
   while(bn) {
      if((*p->comp_fn)(name,bn->name) == 0) {

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

void htab_clear(Hashtable *p) {
   int i;
   
   for (i = 0; i < p->cap; i++) {
      Bucket *b = &p->buckets[i];
      BucketNode *bn = b->elements;
      b->num_elements = 0;
      b->elements = 0;
      while(bn != NULL) {
         BucketNode* next = bn->next;
         free(bn);
         bn = next;
      }
   }
   p->size = 0;

}

int htab_size(Hashtable *p) {
   int result;
	result = p->size;
   return result;
}

Enumeration *htab_getKeys(Hashtable *p){
   HashtableIterator *hi;
   Enumeration *e;

   hi = createIterator(p);
   e = enum_create(
      (void *)hi,
      (void *(*)(void *))next_hash_key_fn,
      (char(*)(void *))has_more_hash_keys_fn
   );

   return e;
}

int htab_density(Hashtable *p){
   int result;
   result = (p->num_buckets *100) / p->cap;
   return result;
}

char *htab_toString(Hashtable *p) {
	char *result = NULL;
	StringBuffer *buf = buf_createDefault();   
   Enumeration *e;  
   int i = 0;
   
   for(e = htab_getKeys(p); enum_hasNext(e); ) { 
      void *curkey = (void *)enum_next(e);
      void *curval = htab_get(p, curkey);
		if (i > 0) {
      	buf_puts(buf, ", ");
		}
      buf_putc(buf, '{');
      buf_printf(buf, "0x%p", curkey);
      buf_putc(buf, '=');
      buf_printf(buf, "0x%p", curval);
      buf_putc(buf, '}');
   	i++;
   }
   result = buf_toString(buf);
	free(e);
   
   return result;
}

static void htab_resize(Hashtable *p){
   int i, newcap = p->cap *2;
   int newnum_buckets = 0;
   Bucket *newbuckets = (Bucket*)emalloc(sizeof(Bucket) * newcap);
   
   for(i = 0; i < newcap; i++){
      newbuckets[i].num_elements = 0;
      newbuckets[i].elements = NULL;
   }


   // Free the old buckets for now (in the future we should consider reuse)

   for (i = 0; i < p->cap; i++) {
      Bucket *b = &p->buckets[i];
      BucketNode *curnode = b->elements;
      while(curnode != NULL) {
         BucketNode* nextnode = curnode->next;

         // discover the new bucket this node belongs to 

         unsigned int hash = (*p->hash_fn)(curnode->name) % newcap; 

         // relink this node for that bucket
        
         Bucket *b = &newbuckets[hash];
         curnode->next = b->elements;
         b->elements = curnode;
         b->num_elements++;

         if(!curnode->next)
            newnum_buckets++;
    
         // continue iterating over the nodes of the old hashtable

         curnode = nextnode;
      }
   }

   free(p->buckets);

   p->num_buckets = newnum_buckets;
   p->cap = newcap;
   p->buckets = newbuckets;
}

static HashtableIterator *createIterator(Hashtable *p) {
   HashtableIterator*hi = emalloc(sizeof(HashtableIterator));

   hi->p = p;
   hi->cur_bucket = -1;
   hi->cur_bucket_node = (BucketNode*)0;

   return hi;
}

static void *next_hash_key_fn(HashtableIterator *hi){

   void *data;
   
   if(!hi->cur_bucket_node) {
      while( ++(hi->cur_bucket) < hi->p->cap &&
            !(hi->cur_bucket_node = hi->p->buckets[hi->cur_bucket].elements ) );
      if (hi->cur_bucket == hi->p->cap) {
         return (void *)0;
      }
   }

   data = (void *)hi->cur_bucket_node->name;
   hi->cur_bucket_node = hi->cur_bucket_node->next;

   return data;
}

static char has_more_hash_keys_fn(HashtableIterator *hi){
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
// end of file: $RCSfile: hashtable.c,v $
==============================================================================*/

