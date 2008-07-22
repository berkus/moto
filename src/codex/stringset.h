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

   $RCSfile: stringset.h,v $   
   $Revision: 1.7 $
   $Author: dhakim $
   $Date: 2003/07/07 00:28:04 $
 
==============================================================================*/
#ifndef __STRINGSET_H
#define __STRINGSET_H

#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "enumeration.h"
#include "mxarr.h"

/*-----------------------------------------------------------------------------
        typedefs
-----------------------------------------------------------------------------*/

/* the bucket node struct - stores a chained list of node "collisions" */
typedef struct ssBucketNode {
   char *string;
   struct ssBucketNode *next;
} SSBucketNode;

/* the bucket struct - stores the head of the chained list of bucket nodes */
typedef struct ssBucket {
   int num_elements;	/* The number of nodes in this bucket */
   struct ssBucketNode *elements;
} SSBucket;

/* the stringset struct */
typedef struct stringset {
   int num_buckets;	/* The number of buckets in use */
   int cap;				/* The total number of buckets */
   int size;			/* The number of n-v pairs in the stringset */
   struct ssBucket *buckets;
} StringSet;

/* the stringset iterator struct */
typedef struct stringsetIterator {
   StringSet *p;
   int cur_bucket;
   struct ssBucketNode *cur_bucket_node;
} StringSetIterator;

/*-----------------------------------------------------------------------------
        allocation functions
-----------------------------------------------------------------------------*/

/**
 * Allocates a new stringset with the given parameters.
 * @param cap the initial capacity of the stringset.
 * @param hfn the hash function to use on names.
 * @param cfn the comparator function to use on names.
 * @return a pointer to the newly allocated stringset.
 */
StringSet *sset_create (
   int cap /* The initial number of buckets */
);

/**
 * Allocates a new stringset with default parameters.
 * @return a pointer to the newly allocated stringset.
 */
StringSet *sset_createDefault();

/** 
 * Clones the contents of this StringSet 
 */
StringSet *sset_clone (StringSet *ss);

/**
 * Frees the memory associated with a stringset.
 * @param set a pointer to a stringset.
 */
void sset_free(StringSet *set);

/*-----------------------------------------------------------------------------
        data structure functions
-----------------------------------------------------------------------------*/

/**
 * Puts a name-value mapping into the stringset. Any previous value
 * that was mapped to the given name is overwritten.
 * @param set a pointer to a stringset.
 * @param string the string to add to the stringset.
 */
void sset_add(StringSet *set, char *string);

/** 
 * Add the contents of StringSet <i>s</i> to this StringSet. If a matching String
 * was already present it is replaces with <i>s</i> 
 */
void sset_addSet(StringSet *this, StringSet *p);

/**
 * Clears the stringset so it contains no name-value mappings.
 * @param set a pointer to a stringset.
 */
void sset_clear(StringSet *set);

/**
 * Returns whether the stringset contains the given name.
 * @param set a pointer to a stringset.
 * @param string the string to look for.
 * @return non-zero if the stringset contains the string, 0 if it does not. 
 */
int sset_contains(StringSet *set, char *string);

/**
 * Retrieves the interned string from the stringset.
 * @param set a pointer to a stringset.
 * @param string the string to retrieve the stringset.
 */
char* sset_get(StringSet *set, char *string);

/**
 * Returns the density of a stringset. The density equals the number of
 * stringset buckets that are in use divided by 100.
 * @param set a pointer to a stringset.
 * @return the density of the given stringset. 
 */
int sset_density(StringSet *set);

/**
 * Returns an enumeration of the elements in a stringset.
 * @param set a pointer to a stringset.
 * @return an enumeration of the elements in the given stringset. 
 */
Enumeration *sset_elements(StringSet *set);

/** 
 * Returns true if this StringSet contains the same elements as <i>S</i> 
 */
int sset_equals(StringSet* p1, StringSet* p2);

int sset_notequals(StringSet* p1, StringSet* p2);

/**
 * Removes a name-value mapping from the stringset.
 * @param set a pointer to a stringset.
 * @param string the string to remove.
 * @return 0 if the stringset contained the name, 1 if it did not. 
 */
int sset_remove(StringSet *set, char *string);

/** 
 * Removes the contents of StringSet s that are contained in this
 * StringSet 
 */
void sset_removeSet(StringSet *this, StringSet *p);

/**
 * Returns the size of a stringset. The size equals the number of
 * name-value mappings contained in the stringset.
 * @param set a pointer to a stringset.
 * @return the size of the given stringset. 
 */
int sset_size(StringSet *set);

/**
 * Returns a string representation of a stringset.
 * @param set a pointer to a stringset.
 * @return a string representation of the given stringset. 
 */
char *sset_toString(StringSet *set);

/** 
 * Returns the elements in this StringSet as a vector of Strings 
 */
UnArray* sset_toArray(StringSet* p);

/*-----------------------------------------------------------------------------
        Overloaded operations
-----------------------------------------------------------------------------*/

/** 
 * Returns the union of two StringSets - {x:x is in s1 or x is in s2 } 
 */
StringSet * sset_union(StringSet * i1, StringSet * i2);

/** 
 * Returns the intersection of two StringSets - {x:x is in s1 and x is in s2 } 
 */
StringSet * sset_intersection(StringSet * i1, StringSet * i2);

/**
 * Returns the difference StringSet i1 and StringSet i2 - {x:x is in s1, x is not in s2} 
 */
StringSet * sset_difference(StringSet * i1, StringSet * i2);

#endif

/*=============================================================================
// end of file: $RCSfile: stringset.h,v $
==============================================================================*/

