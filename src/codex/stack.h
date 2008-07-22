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

   $RCSfile: stack.h,v $   
   $Revision: 1.6 $
   $Author: dhakim $
   $Date: 2003/07/07 00:28:04 $
 
==============================================================================*/
#ifndef __STACK_H
#define __STACK_H

#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "vector.h"

/*-----------------------------------------------------------------------------
        typedefs
-----------------------------------------------------------------------------*/

/* the stack struct */
typedef Vector Stack;

/*-----------------------------------------------------------------------------
        allocation functions
-----------------------------------------------------------------------------*/

/**
 * Allocates a new stack with the given parameters.
 * @param cap the initial capacity of the stack.
 * @param cfn the comparator function to use on data items.
 * @return a pointer to the newly allocated stack.
 */
Stack *stack_create(int cap, int (*cfn)(const void *, const void *));

/**
 * Allocates a new stack with default parameters.
 * @return a pointer to the newly allocated stack.
 */
Stack *stack_createDefault();

/**
 * Allocates a new stack with the same attributes as the stack passed in.
 * @return a pointer to the newly allocated stack.
 */
Stack* stack_clone(Stack* p) ;

/**
 * Frees the memory associated with a stack.
 * @param table a pointer to a stack.
 */
void stack_free(Stack *s);

/*-----------------------------------------------------------------------------
        data structure functions
-----------------------------------------------------------------------------*/

/**
 * Clears a stack so it contains no elements.
 * @param s a pointer to a stack.
 * @return 0 on success, -1 on failure. 
 */
void stack_clear(Stack *s);

/**
 * Returns whether the stack contains the given data item.
 * @param s a pointer to a stack.
 * @param data the data to look for.
 * @return non-zero if the stack contains the data item, 0 otherwise. 
 */
int stack_contains(Stack *s, void *data);

/**
 * Returns an enumeration of the elements in a stack.
 * @param s a pointer to a stack.
 * @return an enumeration of the elements in the given stack. 
 */
Enumeration *stack_elements(Stack *v);

/**
 * Checks two stacks for equality.
 * @param s1 a pointer to a stack.
 * @param s2 a pointer to another stack.
 * @return 0 if the stacks are equal, 1 otherwise. 
 */
int stack_equals(Stack *s1, Stack *s2);

int stack_notequals(Stack *s1, Stack *s2);

/**
 * Returns the top item on a stack without removing it.
 * @param s a pointer to a stack.
 * @return the top item on the given stack. 
 */
void *stack_peek(Stack *s);

/**
 * Returns the nth item on a stack without removing it.
 * @param s a pointer to a stack.
 * @param nth the element to peek.
 * @return the nth item on the given stack. 
 */
void *stack_peekAt(Stack *s, int nth);

/**
 * Removes and returns the top item on a stack.
 * @param s a pointer to a stack.
 * @return the top item on the given stack. 
 */
void *stack_pop(Stack *s);

/**
 * Pushes an item onto a stack.
 * @param s a pointer to a stack.
 * @param data the data item to push onto the stack.
 */
void stack_push(Stack *s, void *data);

/**
 * Returns the size of a stack. The size equals the number of
 * elements contained in the stack.
 * @param s a pointer to a stack.
 * @return the size of the given stack. 
 */
int stack_size(Stack *s);

/**
 * Returns a string representation of a stack.
 * @param s a pointer to a stack.
 * @return a string representation of the given stack. 
 */
char *stack_toString(Stack *s);

#endif

/*=============================================================================
// end of file: $RCSfile: stack.h,v $
==============================================================================*/

