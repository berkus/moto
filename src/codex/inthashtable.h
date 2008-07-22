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

   $RCSfile: inthashtable.h,v $   
   $Revision: 1.8 $
   $Author: dhakim $
   $Date: 2003/07/07 00:28:04 $
 
==============================================================================*/
#ifndef __INTHASHTABLE_H
#define __INTHASHTABLE_H

#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "intenumeration.h"
#include "enumeration.h"

/*-----------------------------------------------------------------------------
        typedefs
-----------------------------------------------------------------------------*/

/* the bucket node struct - stores a chained list of node "collisions" */
typedef struct iBucketNode {
   int key;
   void *value;
   struct iBucketNode *next;
} IBucketNode;

/* the bucket struct - stores the head of the chained list of bucket nodes */
typedef struct iBucket {
   int num_elements;	/* The number of nodes in this bucket */
   struct iBucketNode *elements;
} IBucket;

/* the int hashtable struct */
typedef struct IntHashtable {
   int num_buckets;	/* The number of buckets in use */
   int cap;				/* The total number of buckets */
   int size;			/* The number of k-v pairs in the int hashtable */
   struct iBucket *buckets;
   short flags;
} IntHashtable;

/* the int hashtable iterator struct */
typedef struct IntHashtableIterator {
   IntHashtable *p;
   int cur_bucket;
   struct iBucketNode *cur_bucket_node;
} IntHashtableIterator;

/*-----------------------------------------------------------------------------
        allocation functions
-----------------------------------------------------------------------------*/

/**
 * Allocates a new int hashtable with the given parameters.
 * @param cap the initial capacity of the int hashtable.
 * @param hfn the hash function to use on names.
 * @param cfn the comparator function to use on names.
 * @return a pointer to the newly allocated int hashtable.
 */
IntHashtable *ihtab_create(int cap, short flags);

/**
 * Allocates a new int hashtable with default parameters.
 * @return a pointer to the newly allocated int hashtable.
 */
IntHashtable *ihtab_createDefault();

/**
 * Frees the memory associated with a int hashtable.
 * @param table a pointer to a int hashtable.
 */
void ihtab_free(IntHashtable *table);

/*-----------------------------------------------------------------------------
        data structure functions
-----------------------------------------------------------------------------*/

/**
 * Clears the int hashtable so it contains no name-value mappings.
 * @param table a pointer to a int hashtable.
 */
void ihtab_clear(IntHashtable *table);

/**
 * Returns the density of a int hashtable. The density equals the number of
 * int hashtable buckets that are in use divided by 100.
 * @param table a pointer to a int hashtable.
 * @return the density of the given int hashtable. 
 */
int ihtab_density(IntHashtable *table);

/**
 * Returns a pointer to the value mapped to the given name.
 * @param table a pointer to a int hashtable.
 * @param name the name to whose value is to be retrieved.
 * @return a pointer to the value mapped to the given name, or NULL is the
 * name is not mapped to a value. 
 */
void *ihtab_get(IntHashtable *table, int key);


int ihtab_contains(IntHashtable *table, int key);

/** 
 * Returns true if this int hashtable contains the same elements as <i>S</i> 
 */
int ihtab_equals(IntHashtable* p1, IntHashtable* p2);

int ihtab_notequals(IntHashtable* p1, IntHashtable* p2);

/**
 * Returns an enumeration of the elements in a int hashtable.
 * @param table a pointer to a int hashtable.
 * @return an enumeration of the elements in the given int hashtable. 
 */
IntEnumeration *ihtab_getKeys(IntHashtable *table);

/**
 * Returns an enumeration of the values in a int hashtable.
 * @param table a pointer to a int hashtable.
 * @return an enumeration of the elements in the given int hashtable. 
 */
Enumeration *ihtab_getValues(IntHashtable *table);

/**
 * Puts a name-value mapping into the int hashtable. Any previous value
 * that was mapped to the given name is overwritten.
 * @param table a pointer to a int hashtable.
 * @param name the name to use to create the name-value mapping.
 * @param value the value to use to create the name-value mapping.
 */
void *ihtab_put(IntHashtable *table, int key, void *value);

/**
 * Removes a name-value mapping from the int hashtable.
 * @param table a pointer to a int hashtable.
 * @param name the name to whose value is to be removed.
 */
void *ihtab_remove(IntHashtable *table, int key);

/**
 * Returns the size of a int hashtable. The size equals the number of
 * name-value mappings contained in the int hashtable.
 * @param table a pointer to a int hashtable.
 * @return the size of the given int hashtable. 
 */
int ihtab_size(IntHashtable *table);

/**
 * Returns a string representation of a int hashtable.
 * @param table a pointer to a int hashtable.
 * @return a string representation of the given int hashtable. 
 */
char *ihtab_toString(IntHashtable *table);

#endif

/*=============================================================================
// end of file: $RCSfile: inthashtable.h,v $
==============================================================================*/

