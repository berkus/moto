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

   $RCSfile: symboltable.c,v $   
   $Revision: 1.19 $
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
#include <string.h>

#include "memsw.h"

#include "exception.h"
#include "excpfn.h"
#include "log.h"
#include "stringbuffer.h"
#include "symboltable.h"
#include "util.h"

#define STAB_MAX_DEPTH 4
#define STAB_DEFAULT_SIZE 101

static SymbolTableIterator *createIterator(SymbolTable *p);
static void *next_hash_key_fn(SymbolTableIterator *hi);
static char has_more_hash_keys_fn(SymbolTableIterator *hi);
static void *next_hash_value_fn(SymbolTableIterator *hi);
static char has_more_hash_values_fn(SymbolTableIterator *hi);
static void stab_resize(SymbolTable *p);

SymbolTable *stab_create (int cap, unsigned int (*hash_fn)(void *), 
                          int (*comp_fn)(void *,void *)){
   int i;
   SymbolTable *p;

   /* create symbol table */
   p=(SymbolTable*)emalloc(sizeof(SymbolTable));
	
   /* set function pointers */
   if (hash_fn != NULL) {
   	p->hash_fn = hash_fn;
   }
   else {
    	p->hash_fn = string_hash;
   }
   if (comp_fn != NULL) {
   	p->comp_fn = comp_fn;
   }
   else {
    	p->comp_fn = (int(*)(void*,void*))strcmp;
   }
	
   /* set other fields */
   p->cap = cap > 0 ? cap : STAB_DEFAULT_SIZE;
   p->size = 0;
   p->num_buckets = 0;
   p->buckets = (SBucket*)emalloc(sizeof(SBucket)*cap);

   /* init bucket stuff */
   for(i = 0; i < cap; i++){
      p->buckets[i].num_elements = 0;
      p->buckets[i].elements = 0;
   }
   return p;
}

SymbolTable *stab_createDefault() {
   return stab_create(STAB_DEFAULT_SIZE, NULL, NULL);
}

void stab_free(SymbolTable *p) {
   int i;
   
   for (i = 0; i < p->cap; i++) {
      SBucket *b = &p->buckets[i];
      SBucketNode *bn = b->elements;
      while(bn != NULL) {
         SBucketNode* next = bn->next;
         free(bn);
         bn = next;
      }
   }   
   free(p->buckets);
   
   free(p);
}

void *stab_get(SymbolTable *p, char *name){
   unsigned int hash;
   void *result = NULL; 
   SBucket *b;
   SBucketNode *bn;

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

int stab_contains(SymbolTable *p, char *name){
   unsigned int hash;
   int result = 0; 
   SBucket *b;
   SBucketNode *bn;

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

void *stab_put(SymbolTable *p, char *name, void *value){
   unsigned int hash;
   void *result = NULL; 
   SBucket *b;
   SBucketNode *bn;
   
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
      bn=(SBucketNode*)emalloc(sizeof(SBucketNode));
      bn->name = name;
      bn->value = value;
      result = bn->value;
      bn->next = b->elements;
      
      b->elements = bn;
      b->num_elements++;
      p->size++;
   
      if(!bn->next)
         p->num_buckets++;
   
      if(b->num_elements > STAB_MAX_DEPTH)
         stab_resize(p);
   }

   return result;
}

void *stab_remove(SymbolTable *p, char *name) {
   unsigned int hash;
   void *result = NULL; 
   SBucket *b;
   SBucketNode *bn;
   SBucketNode *pbn;
   SBucketNode *freebn;
      
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

void stab_clear(SymbolTable *p) {
   int i;
   
   for (i = 0; i < p->cap; i++) {
      SBucket *b = &p->buckets[i];
      SBucketNode *bn = b->elements;
      b->num_elements = 0;
      b->elements = 0;
      while(bn != NULL) {
         SBucketNode* next = bn->next;
         free(bn);
         bn = next;
      }
   }
   p->size = 0;

}


int 
stab_equals(SymbolTable* p1, SymbolTable* p2){
	SymbolTable* is1 = (SymbolTable*)p1;
	SymbolTable* is2 = (SymbolTable*)p2;
	int i;

	if (p2 == NULL) 
		return 0;
	
	if (stab_size(is1) != stab_size(is2)) 
		return 0;

	for (i = 0; i < is1->cap; i++) {
		SBucket *b = &is1->buckets[i];
		SBucketNode *curnode = b->elements;
		while(curnode != NULL) {
			if (stab_get(is2,curnode->name) != curnode->value) 
				return 0;
			curnode = curnode->next;
		}
	}
	
	return 1;
}

int 
stab_notequals(SymbolTable* p1, SymbolTable* p2){
	return !stab_equals(p1, p2);
}


int stab_size(SymbolTable *p) {
   int result;
	result = p->size;
   return result;
}

Enumeration *stab_getKeys(SymbolTable *p){
   SymbolTableIterator *hi;
   Enumeration *e;

   hi = createIterator(p);
   e = enum_create(
      (void *)hi,
      (void *(*)(void *))next_hash_key_fn,
      (char(*)(void *))has_more_hash_keys_fn
   );

   return e;
}

Enumeration *stab_getValues(SymbolTable *p){
   SymbolTableIterator *hi;
   Enumeration *e;

   hi = createIterator(p);
   e = enum_create(
      (void *)hi,
      (void *(*)(void *))next_hash_value_fn,
      (char(*)(void *))has_more_hash_values_fn
   );

   return e;
}

int stab_density(SymbolTable *p){
   int result;
   result = (p->num_buckets *100) / p->cap;
   return result;
}

char *stab_toString(SymbolTable *p) {
	char *result = NULL;
	StringBuffer *buf = buf_createDefault();   
   Enumeration *e;  
   int i = 0;
   
   for(e = stab_getKeys(p); enum_hasNext(e); ) { 
      void *curkey = (void *)enum_next(e);
      void *curval = stab_get(p, curkey);
		if (i > 0) {
      	buf_puts(buf, ", ");
		}
      buf_putc(buf, '{');
      buf_puts(buf, curkey);
      buf_putc(buf, '=');
      buf_puts(buf, curval);
      buf_putc(buf, '}');
   	i++;
   }
   result = buf_toString(buf);
	free(e);
   
   return result;
}

static void stab_resize(SymbolTable *p){
   int i, newcap = p->cap *2;
   int newnum_buckets = 0;
   SBucket *newbuckets = (SBucket*)emalloc(sizeof(SBucket)*newcap);
   
   /* init bucket stuff */
   for(i = 0; i < newcap; i++){
      newbuckets[i].num_elements = 0;
      newbuckets[i].elements = 0;
   }

   // Free the old buckets for now (in the future we should consider reuse)

   for (i = 0; i < p->cap; i++) {
      SBucket *b = &p->buckets[i];
      SBucketNode *curnode = b->elements;
      while(curnode != NULL) {
         SBucketNode* nextnode = curnode->next;

         // discover the new bucket this node belongs to 

         unsigned int hash = (*p->hash_fn)(curnode->name) % newcap; 

         // relink this node for that bucket
        
         SBucket *b = &newbuckets[hash];
         curnode->next = b->elements;
         b->elements = curnode;
         b->num_elements++;

         if(!curnode->next)
            newnum_buckets++;
    
         // continue iterating over the nodes of the old symboltable

         curnode = nextnode;
      }
   }

   free(p->buckets);

   p->num_buckets = newnum_buckets;
   p->cap = newcap;
   p->buckets = newbuckets;
}

static SymbolTableIterator *createIterator(SymbolTable *p) {
   SymbolTableIterator*hi = emalloc(sizeof(SymbolTableIterator));

   hi->p = p;
   hi->cur_bucket = -1;
   hi->cur_bucket_node = (SBucketNode*)0;

   return hi;
}

static void *next_hash_key_fn(SymbolTableIterator *hi){
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

static void *next_hash_value_fn(SymbolTableIterator *hi){
   void *data;

   if(!hi->cur_bucket_node) {
      while( ++(hi->cur_bucket) < hi->p->cap &&
            !(hi->cur_bucket_node = hi->p->buckets[hi->cur_bucket].elements));
      if (hi->cur_bucket == hi->p->cap){
         return (void *)0;
      }
   }

   data = (void *)hi->cur_bucket_node->value;
   hi->cur_bucket_node = hi->cur_bucket_node->next;

   return data;
}

static char has_more_hash_keys_fn(SymbolTableIterator *hi){
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

static char has_more_hash_values_fn(SymbolTableIterator *hi){
   return has_more_hash_keys_fn(hi);
}

/*=============================================================================
// end of file: $RCSfile: symboltable.c,v $
==============================================================================*/

