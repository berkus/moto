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

   $RCSfile: enumeration.h,v $   
   $Revision: 1.4 $
   $Author: dhakim $
   $Date: 2003/02/27 19:46:59 $
 
==============================================================================*/
#ifndef __ENUMERATION_H
#define __ENUMERATION_H

#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "cdx_function.h"

/**
 * The Enumeration structure.
 * @element iterator the iterator for the Enumeration.
 * @element next_fn pointer to a function that returns the next element in the
 * Enumeration.
 * @element has_more_fn pointer to a function that determines if there is a 
 * next element in the Enumeration.
 */
typedef struct enumeration {
   void *iterator;
   void *(*next_fn)(void *);
   char (*has_more_fn)(void *);
} Enumeration;

/**
 * Allocates an Enumeration using the given parameters.
 * @param iterator the iterator for the Enumeration.
 * @param next_fn pointer to a function that returns the next element in the
 * Enumeration.
 * @param has_more_fn pointer to a function that determines if there is a next 
 * element in the Enumeration.
 * @return a pointer to the newly-allocated Enumeration.
 */
Enumeration *enum_create
(void *iterator, void *(*next_fn)(void *), char (*has_more_fn)(void *));

/**
 * Frees an Enumeration.
 * @param e a pointer to an Enumeration.
 */
void enum_free(Enumeration *e);

/**
 * Returns the next element in an Enumeration.
 * @param e a pointer to an Enumeration.
 * @return the next element in the given Enumeration.
 */
void *enum_next(Enumeration *e);

/**
 * Returns whether there are more elements in an Enumeration.
 * @param e a pointer to an Enumeration.
 * @return zero if there are no more elements in the Enumeration, non-zero
 * if there are more elements.
 */
int enum_hasNext(Enumeration *e);

/**
 * Executes the function f on each remaining element of the enumeration.
 */
void enum_each(Enumeration *e,Function *f);

#endif

/*=============================================================================
// end of file: $RCSfile: enumeration.h,v $
==============================================================================*/

