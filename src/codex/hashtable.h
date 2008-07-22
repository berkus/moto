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

   $RCSfile: hashtable.h,v $   
   $Revision: 1.4 $
   $Author: kocienda $
   $Date: 2000/04/30 23:10:01 $
 
==============================================================================*/
#ifndef __HASHTABLE_H
#define __HASHTABLE_H

#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "enumeration.h"

/*-----------------------------------------------------------------------------
        typedefs
-----------------------------------------------------------------------------*/

/* the bucket node struct - stores a chained list of node "collisions" */
typedef struct bucketNode {
   void *name;
   void *value;
   struct bucketNode *next;
} BucketNode;

/* the bucket struct - stores the head of the chained list of bucket nodes */
typedef struct bucket {
   int num_elements;	/* The number of nodes in this bucket */
   struct bucketNode *elements;
} Bucket;

/* the hashtable struct */
typedef struct hashtable {
   int num_buckets;	/* The number of buckets in use */
   int cap;				/* The total number of buckets */
   int size;			/* The number of n-v pairs in the hashtable */
   struct bucket *buckets;
   unsigned int (*hash_fn)(void *); /* The hash function */
   int (*comp_fn)(const void *, const void *); /* The key comparator */
   short flags;
} Hashtable;

/* the hashtable iterator struct */
typedef struct hashtableIterator {
   Hashtable *p;
   int cur_bucket;
   struct bucketNode *cur_bucket_node;
} HashtableIterator;

/*-----------------------------------------------------------------------------
        allocation functions
-----------------------------------------------------------------------------*/

/**
 * Allocates a new hashtable with the given parameters.
 * @param cap the initial capacity of the hashtable.
 * @param hfn the hash function to use on names.
 * @param cfn the comparator function to use on names.
 * @return a pointer to the newly allocated hashtable.
 */
Hashtable *htab_create (
   int cap, 
   unsigned int (*hash_fn)(void *),
   int (*comp_fn)(const void *, const void *),
   short flags
);

/**
 * Allocates a new hashtable with default parameters.
 * @return a pointer to the newly allocated hashtable.
 */
Hashtable *htab_createDefault();

/**
 * Frees the memory associated with a hashtable.
 * @param table a pointer to a hashtable.
 */
void htab_free(Hashtable *table);

/*-----------------------------------------------------------------------------
        data structure functions
 -----------------------------------------------------------------------------*/

/**
 * Clears the hashtable so it contains no name-value mappings.
 * @param table a pointer to a hashtable.
 */
void htab_clear(Hashtable *table);

/**
 * Returns the density of a hashtable. The density equals the number of
 * hashtable buckets that are in use divided by 100.
 * @param table a pointer to a hashtable.
 * @return the density of the given hashtable. 
 */
int htab_density(Hashtable *table);


void htab_dump(Hashtable *table);

/**
 * Returns a pointer to the value mapped to the given name.
 * @param table a pointer to a hashtable.
 * @param name the name to whose value is to be retrieved.
 * @return a pointer to the value mapped to the given name, or NULL is the
 * name is not mapped to a value. 
 */
void *htab_get(Hashtable *table, void *name);

int htab_contains(Hashtable *table, void *name);

/**
 * Returns an enumeration of the keys in a hashtable.
 * @param table a pointer to a hashtable.
 * @return an enumeration of the keys in the given hashtable. 
 */
Enumeration *htab_getKeys(Hashtable *table);

/**
 * Puts a name-value mapping into the hashtable. Any previous value
 * that was mapped to the given name is overwritten.
 * @param table a pointer to a hashtable.
 * @param name the name to use to create the name-value mapping.
 * @param value the value to use to create the name-value mapping.
 * @return the old value mapped to the given name, or NULL if the name
 * was not previously mapped. 
 */
void *htab_put(Hashtable *table, void *name, void *value);

/**
 * Removes a name-value mapping from the hashtable.
 * @param table a pointer to a hashtable.
 * @param name the name to whose value is to be removed.
 */
void *htab_remove(Hashtable *table, void *name);

/**
 * Returns the size of a hashtable. The size equals the number of
 * name-value mappings contained in the hashtable.
 * @param table a pointer to a hashtable.
 * @return the size of the given hashtable. 
 */
int htab_size(Hashtable *table);

/**
 * Returns a string representation of a hashtable.
 * @param table a pointer to a hashtable.
 * @return a string representation of the given hashtable. 
 */
char *htab_toString(Hashtable *table);

#endif

/*=============================================================================
// end of file: $RCSfile: hashtable.h,v $
==============================================================================*/

