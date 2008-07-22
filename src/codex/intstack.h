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

   $RCSfile: intstack.h,v $   
   $Revision: 1.6 $
   $Author: dhakim $
   $Date: 2003/07/07 00:28:04 $
 
==============================================================================*/
#ifndef __INTSTACK_H
#define __INTSTACK_H

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

/* the intstack struct */
typedef struct intStack {
	int size;
	int cap;
	int *elements;
   short flags;
} IntStack;

/* the intstack iterator struct */
typedef struct intStackIterator {
   IntStack *p;
   int index;
   int size;
} IntStackIterator;

/*-----------------------------------------------------------------------------
        allocation functions
-----------------------------------------------------------------------------*/

/**
 * Allocates a new intstack with the given parameters.
 * @param cap the initial capacity of the intstack.
 * @param cfn the comparator function to use on data items.
 * @return a pointer to the newly allocated intstack.
 */
IntStack *istack_create(int cap, short flags);

/**
 * Allocates a new intstack with default parameters.
 * @return a pointer to the newly allocated intstack.
 */
IntStack *istack_createDefault();

/**
 * Frees the memory associated with a intstack.
 * @param table a pointer to a intstack.
 */
void istack_free(IntStack *s);

/*-----------------------------------------------------------------------------
        data structure functions
-----------------------------------------------------------------------------*/

/**
 * Clears a intstack so it contains no elements.
 * @param s a pointer to a intstack.
 * @return 0 on success, -1 on failure. 
 */
void istack_clear(IntStack *s);

/**
 * Returns whether the intstack contains the given data item.
 * @param s a pointer to a intstack.
 * @param data the data to look for.
 * @return non-zero if the intstack contains the data item, 0 otherwise. 
 */
int istack_contains(IntStack *s, int data);

/**
 * Returns an enumeration of the elements in a intstack.
 * @param s a pointer to a intstack.
 * @return an enumeration of the elements in the given intstack. 
 */
IntEnumeration *istack_elements(IntStack *v);

/**
 * Checks two stacks for equality.
 * @param s1 a pointer to a intstack.
 * @param s2 a pointer to another intstack.
 * @return 0 if the stacks are equal, 1 otherwise. 
 */
int istack_equals(IntStack *s1, IntStack *s2);

int istack_notequals(IntStack *s1, IntStack *s2) ;

/**
 * Returns the top item on a intstack without removing it.
 * @param s a pointer to a intstack.
 * @return the top item on the given intstack. 
 */
int istack_peek(IntStack *s);

/**
 * Returns the nth item on a intstack without removing it.
 * @param s a pointer to a intstack.
 * @param nth the element to peek.
 * @return the nth item on the given intstack. 
 */
int istack_peekAt(IntStack *s, int nth);

/**
 * Removes and returns the top item on a intstack.
 * @param s a pointer to a intstack.
 * @return the top item on the given intstack. 
 */
int istack_pop(IntStack *s);

/**
 * Pushes an item onto a intstack.
 * @param s a pointer to a intstack.
 * @param data the data item to push onto the intstack.
 */
void istack_push(IntStack *s, int data);

/**
 * Returns the size of a intstack. The size equals the number of
 * elements contained in the intstack.
 * @param s a pointer to a intstack.
 * @return the size of the given intstack. 
 */
int istack_size(IntStack *s);

/**
 * Returns a string representation of a intstack.
 * @param s a pointer to a intstack.
 * @return a string representation of the given intstack. 
 */
char *istack_toString(IntStack *s);

#endif

/*=============================================================================
// end of file: $RCSfile: intstack.h,v $
==============================================================================*/

