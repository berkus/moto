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

   $RCSfile: hashset.h,v $   
   $Revision: 1.6 $
   $Author: dhakim $
   $Date: 2003/05/29 02:14:08 $
 
==============================================================================*/
#ifndef __HASHSET_H
#define __HASHSET_H

#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "enumeration.h"
#include "blockpool.h"

#define HSET_POOLBNS 1

/*-----------------------------------------------------------------------------
        typedefs
-----------------------------------------------------------------------------*/

/* the bucket node struct - stores a chained list of node "collisions" */
typedef struct hsBucketNode {
   void *name;
   struct hsBucketNode *next;
} HSBucketNode;

/* the bucket struct - stores the head of the chained list of bucket nodes */
typedef struct hsBucket {
   int num_elements;	/* The number of nodes in this bucket */
   struct hsBucketNode *elements;
} HSBucket;

/* the hashset struct */
typedef struct hashset {
   int num_buckets;	/* The number of buckets in use */
   int cap;				/* The total number of buckets */
   int size;			/* The number of n-v pairs in the hashset */
   struct hsBucket *buckets;
   unsigned int (*hash_fn)(void*); /* The hash function */
   int (*comp_fn)(const void *, const void *); /* The key comparator */
   short flags;
   BlockPool* bpool;
} HashSet;

/* the hashset iterator struct */
typedef struct hashsetIterator {
   HashSet *p;
   int cur_bucket;
   struct hsBucketNode *cur_bucket_node;
} HashSetIterator;

/*-----------------------------------------------------------------------------
        allocation functions
-----------------------------------------------------------------------------*/

/**
 * Allocates a new hashset with the given parameters.
 * @param cap the initial capacity of the hashset.
 * @param hfn the hash function to use on names.
 * @param cfn the comparator function to use on names.
 * @return a pointer to the newly allocated hashset.
 */
HashSet *hset_create (
   int cap, /* The initial number of buckets */
   unsigned int (*hfn)(void*), /* The hash function */
   int (*cfn)(const void *, const void *), /* The key comparator */
   short flags      
);

/**
 * Allocates a new hashset with default parameters.
 * @return a pointer to the newly allocated hashset.
 */
HashSet *hset_createDefault();

/**
 * Frees the memory associated with a hashset.
 * @param table a pointer to a hashset.
 */
void hset_free(HashSet *table);

/*-----------------------------------------------------------------------------
        data structure functions
-----------------------------------------------------------------------------*/

/**
 * Puts a name-value mapping into the hashset. Any previous value
 * that was mapped to the given name is overwritten.
 * @param table a pointer to a hashset.
 * @param name the name to add to the hashset.
 */
void hset_add(HashSet *table, void *name);

/**
 * Clears the hashset so it contains no name-value mappings.
 * @param table a pointer to a hashset.
 */
void hset_clear(HashSet *table);

/**
 * Returns whether the hashset contains the given name.
 * @param table a pointer to a hashset.
 * @param name the name to look for.
 * @return non-zero if the hashset contains the name, 0 if it does not. 
 */
int hset_contains(HashSet *table, void *name);

/**
 * Returns interned element the hashset contains that compares equal to
 * the given name.
 * @param table a pointer to a hashset.
 * @param name the name to look for.
 * @return non-null if the hashset contains the name, null if it does not.
 */
void* hset_get(HashSet *table, void *name);

/**
 * Returns the density of a hashset. The density equals the number of
 * hashset buckets that are in use divided by 100.
 * @param table a pointer to a hashset.
 * @return the density of the given hashset. 
 */
int hset_density(HashSet *table);

/**
 * Returns an enumeration of the elements in a hashset.
 * @param table a pointer to a hashset.
 * @return an enumeration of the elements in the given hashset. 
 */
Enumeration *hset_elements(HashSet *table);

/**
 * Removes a name-value mapping from the hashset.
 * @param table a pointer to a hashset.
 * @param name the name to remove.
 * @return 0 if the hashset contained the name, 1 if it did not. 
 */
int hset_remove(HashSet *table, void *name);

/**
 * Returns the size of a hashset. The size equals the number of
 * name-value mappings contained in the hashset.
 * @param table a pointer to a hashset.
 * @return the size of the given hashset. 
 */
int hset_size(HashSet *table);

/**
 * Returns a string representation of a hashset.
 * @param table a pointer to a hashset.
 * @return a string representation of the given hashset. 
 */
char *hset_toString(HashSet *table);

#endif

/*=============================================================================
// end of file: $RCSfile: hashset.h,v $
==============================================================================*/

