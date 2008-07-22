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

   $RCSfile: vector.h,v $   
   $Revision: 1.12 $
   $Author: dhakim $
   $Date: 2003/07/07 00:28:04 $
 
==============================================================================*/
#ifndef __VECTOR_H
#define __VECTOR_H

#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "enumeration.h"
#include "mxarr.h"
#include "cdx_function.h"

/*-----------------------------------------------------------------------------
        typedefs
-----------------------------------------------------------------------------*/

/* the vector struct */
typedef struct vector {
	int size;
	int cap;
	void **elements;
	int (*comp_fn)(const void *, const void *);
} Vector;

/* the vector iterator struct */
typedef struct vectorIterator {
   Vector *p;
   int index;
   int size;
} VectorIterator;

/*-----------------------------------------------------------------------------
        allocation functions
-----------------------------------------------------------------------------*/

/**
 * Allocates a new vector with the given parameters.
 * @param cap the initial capacity of the vector.
 * @param cfn the comparator function to use on data items.
 * @return a pointer to the newly allocated vector.
 */
Vector *vec_create (int cap, int (*cfn)(const void *, const void *));

/** Constructs a new Vector from an Object[] */
Vector *vec_createFromArray(UnArray* x);

/**
 * Allocates a new vector with default parameters.
 * @return a pointer to the newly allocated vector.
 */
Vector *vec_createDefault();

/**
 * Frees the memory associated with a vector.
 * @param table a pointer to a vector.
 */
void vec_free(Vector *v);

/*-----------------------------------------------------------------------------
        data structure functions
-----------------------------------------------------------------------------*/

/**
 * Appends a data item to the end of the vector.
 * @param v a pointer to a vector.
 * @param data the data item to add.
 * @return 0 on success, -1 on failure. 
 */
int vec_add(Vector *v, void *data);

/**
 * Clears a vector so it contains no elements.
 * @param v a pointer to a vector.
 * @return 0 on success, -1 on failure. 
 */
int vec_clear(Vector *v);

Vector* vec_clone(Vector* v);

/**
 * Returns whether the vector contains the given data item.
 * @param v a pointer to a vector.
 * @param data the data to look for.
 * @return non-zero if the vector contains the data item, 0 otherwise. 
 */
int vec_contains(Vector *v, void *data);

/**
 * Returns an enumeration of the elements in a vector.
 * @param v a pointer to a vector.
 * @return an enumeration of the elements in the given vector. 
 */
Enumeration *vec_elements(Vector *v);

/**
 * Checks two vectors for equality.
 * @param v1 a pointer to a vector.
 * @param v2 a pointer to another vector.
 * @return 0 if the vectors are equal, 1 otherwise. 
 */
int vec_equals(Vector *v1, Vector *v2);

int vec_notequals(Vector *v1, Vector *v2);

/**
 * Retrieves a data item at a given index.
 * @param v a pointer to a vector.
 * @param index the index of the item to retrieve.
 * @return the data item at the given index or NULL if there is
 * no data item at the given index. 
 */
void *vec_get(Vector *v, int index);

/**
 * Inserts a data item into a vector at the given index. If there is
 * an item already at the given index, it, and all items after it, are
 * slid down one index location.
 * @param v a pointer to a vector.
 * @param data the data item to insert.
 * @param index the index at which the data item is inserted.
 * @return 0 on success, -1 on failure. 
 */
int vec_insert(Vector *v, void *data, int index);

/**
 * Removes a data item from a vector. The item is searched for in a linear
 * fashion.
 * @param v a pointer to a vector.
 * @param data the data item to remove.
 * @return 0 on success, -1 on failure. 
 */
int vec_remove(Vector *v, void *data);

/**
 * Removes a data item at the given index.
 * @param v a pointer to a vector.
 * @param index the index of the item to remove.
 * @return a pointer to the item removed, or NULL if there was no
 * data item at the index given. 
 */
void *vec_removeAt(Vector *v, int index);

/**
 * Sets a data item into a vector at the given index. If there is
 * an item already at the given index, it is overwritten.
 * @param v a pointer to a vector.
 * @param data the data item to insert.
 * @param index the index at which the data item is inserted.
 * @return a pointer to the previous item at the given index.
 */
void *vec_setAt(Vector *v, void *data, int index);

void* vec_setAtOverloaded(Vector *p, int index, void* data);

/**
 * Returns the size of a vector. The size equals the number of
 * elements contained in the vector.
 * @param v a pointer to a vector.
 * @return the size of the given vector. 
 */
int vec_size(Vector *v);

/**
 * Shuffles the contents of the vector with Knuth's shuffling algorithm
 */

void vec_shuffle(Vector *v);

/**
 * Sorts the vector in place using the given comparison function. 
 * The comparison function must return an integer less than, equal to, or 
 * greater than zero if the first argument is considered to be respectively 
 * less than, equal to, or greater than the second.
 * When this function returns, the vector is sorted in ascending order.
 * @param v a pointer to a vector.
 * @param cmp a comparison function.
 */
void vec_sort_cmp(Vector *v, Function* compfn);

/**
 * Returns a string representation of a vector.
 * @param v a pointer to a vector.
 * @return a string representation of the given vector. 
 */
char *vec_toString(Vector *v1);

UnArray* vec_toArray(Vector* p);

void** vec_data(Vector* v1);

void vec_each(Vector *p, Function* f);

int vec_resize(Vector *p, int newcap);

#endif

/*=============================================================================
// end of file: $RCSfile: vector.h,v $
==============================================================================*/

