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

   $RCSfile: itoihashtable.h,v $   
   $Revision: 1.8 $
   $Author: dhakim $
   $Date: 2003/07/07 00:28:04 $
 
==============================================================================*/
#ifndef __ITOIHASHTABLE_H
#define __ITOIHASHTABLE_H

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
typedef struct itoIBucketNode {
   int key;
   int value;
   struct itoIBucketNode *next;
} ItoIBucketNode;

/* the bucket struct - stores the head of the chained list of bucket nodes */
typedef struct itoIBucket {
   int num_elements;	/* The number of nodes in this bucket */
   struct itoIBucketNode *elements;
} ItoIBucket;

/* the int hashtable struct */
typedef struct ItoIHashtable {
   int num_buckets;	/* The number of buckets in use */
   int cap;				/* The total number of buckets */
   int size;			/* The number of k-v pairs in the int hashtable */
   struct itoIBucket *buckets;
   short flags;
} ItoIHashtable;

/* the int hashtable iterator struct */
typedef struct itoIHashtableIterator {
   ItoIHashtable *p;
   int cur_bucket;
   struct itoIBucketNode *cur_bucket_node;
} ItoIHashtableIterator;

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
ItoIHashtable *itoi_create (int cap);

/**
 * Allocates a new int hashtable with default parameters.
 * @return a pointer to the newly allocated int hashtable.
 */
ItoIHashtable *itoi_createDefault();

/**
 * Frees the memory associated with a int hashtable.
 * @param table a pointer to a int hashtable.
 */
void itoi_free(ItoIHashtable *table);

/*-----------------------------------------------------------------------------
        data structure functions
-----------------------------------------------------------------------------*/

/**
 * Clears the int hashtable so it contains no name-value mappings.
 * @param table a pointer to a int hashtable.
 */
void itoi_clear(ItoIHashtable *table);

/**
 * Returns the density of a int hashtable. The density equals the number of
 * int hashtable buckets that are in use divided by 100.
 * @param table a pointer to a int hashtable.
 * @return the density of the given int hashtable. 
 */
int itoi_density(ItoIHashtable *table);

/**
 * Returns a pointer to the value mapped to the given name.
 * @param table a pointer to a int hashtable.
 * @param name the name to whose value is to be retrieved.
 * @return a pointer to the value mapped to the given name, or NULL is the
 * name is not mapped to a value. 
 */
int itoi_get(ItoIHashtable *table, int key);

/**
 * Returns 1 if the key is present in the int hashtable
 * 0 otherwise.
 * @param table a pointer to a int hashtable.
 * @return the size of the given int hashtable.
 */
int itoi_contains(ItoIHashtable *table, int key);

/** 
 * Returns true if this int hashtable contains the same elements as <i>S</i> 
 */
int itoi_equals(ItoIHashtable* p1, ItoIHashtable* p2);

int itoi_notequals(ItoIHashtable* p1, ItoIHashtable* p2);

/**
 * Returns an enumeration of the elements in a int hashtable.
 * @param table a pointer to a int hashtable.
 * @return an enumeration of the elements in the given int hashtable. 
 */
IntEnumeration *itoi_getKeys(ItoIHashtable *table);

/**
 * Puts a name-value mapping into the int hashtable. Any previous value
 * that was mapped to the given name is overwritten.
 * @param table a pointer to a int hashtable.
 * @param name the name to use to create the name-value mapping.
 * @param value the value to use to create the name-value mapping.
 */
int itoi_put(ItoIHashtable *table, int key, int value);

/**
 * Removes a name-value mapping from the int hashtable.
 * @param table a pointer to a int hashtable.
 * @param name the name to whose value is to be removed.
 */
int itoi_remove(ItoIHashtable *table, int key);

/**
 * Returns the size of a int hashtable. The size equals the number of
 * name-value mappings contained in the int hashtable.
 * @param table a pointer to a int hashtable.
 * @return the size of the given int hashtable. 
 */
int itoi_size(ItoIHashtable *table);

/**
 * Returns a string representation of a int hashtable.
 * @param table a pointer to a int hashtable.
 * @return a string representation of the given int hashtable. 
 */
char *itoi_toString(ItoIHashtable *table);

#endif

/*=============================================================================
// end of file: $RCSfile: itoihashtable.h,v $
==============================================================================*/

