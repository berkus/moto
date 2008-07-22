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

   $RCSfile: intset.h,v $   
   $Revision: 1.6 $
   $Author: dhakim $
   $Date: 2003/05/29 02:14:08 $
 
==============================================================================*/
#ifndef __INTSET_H
#define __INTSET_H

#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "intenumeration.h"

/*-----------------------------------------------------------------------------
        typedefs
-----------------------------------------------------------------------------*/

/* the bucket node struct - stores a chained list of node "collisions" */
typedef struct isBucketNode {
   int key;
   struct isBucketNode *next;
} ISBucketNode;

/* the bucket struct - stores the head of the chained list of bucket nodes */
typedef struct isBucket {
   int num_elements;	/* The number of nodes in this bucket */
   struct isBucketNode *elements;
} ISBucket;

/* the intset struct */
typedef struct intset {
   int num_buckets;	/* The number of buckets in use */
   int cap;				/* The total number of buckets */
   int size;			/* The number of n-v pairs in the intset */
   struct isBucket *buckets;
   short flags;
} IntSet;

/* the intset iterator struct */
typedef struct intsetIterator {
   IntSet *p;
   int cur_bucket;
   struct isBucketNode *cur_bucket_node;
} IntSetIterator;

/*-----------------------------------------------------------------------------
        allocation functions
-----------------------------------------------------------------------------*/

/**
 * Allocates a new intset with the given parameters.
 * @param cap the initial capacity of the intset.
 * @param hfn the hash function to use on names.
 * @param cfn the comparator function to use on names.
 * @return a pointer to the newly allocated intset.
 */
IntSet *iset_create (int cap, short flags);

/**
 * Returns a newly allocated intset copying all the values
 * from an existing one
 */
IntSet * iset_clone (IntSet *is);

/**
 * Allocates a new intset with default parameters.
 * @return a pointer to the newly allocated intset.
 */
IntSet *iset_createDefault();

/**
 * Frees the memory associated with a intset.
 * @param set a pointer to a intset.
 */
void iset_free(IntSet *set);

/*-----------------------------------------------------------------------------
        data structure functions
-----------------------------------------------------------------------------*/

/**
 * Hash and comparison functions
 */

unsigned int iset_hash(void* p1);
int iset_comp(void* p1, void* p2);

/**
 * Puts a name-value mapping into the intset. Any previous value
 * that was mapped to the given name is overwritten.
 * @param set a pointer to a intset.
 * @param key the int to add to the intset.
 */
void iset_add(IntSet *set, int key);

void iset_addSet(IntSet *this, IntSet *set);

/**
 * Clears the intset so it contains no name-value mappings.
 * @param set a pointer to a intset.
 */
void iset_clear(IntSet *set);

/**
 * Returns whether the intset contains the given name.
 * @param set a pointer to a intset.
 * @param key the int to look for.
 * @return non-zero if the intset contains the int, 0 if it does not. 
 */
int iset_contains(IntSet *set, int key);

/**
 * Returns the density of a intset. The density equals the number of
 * intset buckets that are in use divided by 100.
 * @param set a pointer to a intset.
 * @return the density of the given intset. 
 */
int iset_density(IntSet *set);

/**
 * Returns an enumeration of the elements in a intset.
 * @param set a pointer to a intset.
 * @return an enumeration of the elements in the given intset. 
 */
IntEnumeration *iset_elements(IntSet *set);

/**
 * Removes a name-value mapping from the intset.
 * @param set a pointer to a intset.
 * @param key the int to remove.
 * @return 0 if the intset contained the name, 1 if it did not. 
 */
int iset_remove(IntSet *set, int key);

void iset_removeSet(IntSet *this, IntSet *p);

/**
 * Returns the size of a intset. The size equals the number of
 * name-value mappings contained in the intset.
 * @param set a pointer to a intset.
 * @return the size of the given intset. 
 */
int iset_size(IntSet *set);

/**
 * Returns a string representation of a intset.
 * @param set a pointer to a intset.
 * @return a string representation of the given intset. 
 */
char *iset_toString(IntSet *set);

#endif

/*=============================================================================
// end of file: $RCSfile: intset.h,v $
==============================================================================*/

