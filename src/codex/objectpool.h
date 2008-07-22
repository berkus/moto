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

   $RCSfile: objectpool.h,v $
   $Revision: 1.5 $
   $Author: dhakim $
   $Date: 2003/05/29 02:14:08 $

==============================================================================*/
#ifndef __OBJECTPOOL_H
#define __OBJECTPOOL_H

#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/* define POOLDEBUG */

#include "hashset.h"

typedef struct objectpool{
   void*(*newdfn)(void);			/* function to create new object pool objects */
   void*(*newfn)(void*);			/* function to create new object pool objects 
   											with the help of external data */
   void (*freefn)(void *);			/* function to free object pool objects */
   void (*grabfn)(void *);			/* function to call on objects prior to grab */
   void (*relfn)(void *);			/* function to call on objects after release */
   void** pool;
   void* ext;							/* external data the functions might need */
   int size;
   int max;
   char flags;
#ifdef POOLDEBUG
   HashSet* ptrs;
#endif
} ObjectPool;

/**
 * Creates an object pool of the specified size. Fills it with objects created
 * be the specified constructor function (newfn)
 */

ObjectPool* opool_create(
   void*(*newfn)(void),
   void (*freefn)(void *),
   void (*grabfn)(void *),
   void (*relfn)(void *),
   int size
);

ObjectPool* opool_createWithExt(
   void*(*newfn)(void*),
   void (*freefn)(void*),
   void (*grabfn)(void *),
   void (*relfn)(void *),
   void *ext,
   int size
);

/**
 * Frees the objects in the object pool by the specified free function. then
 * frees the pool itself
 */

void opool_free(ObjectPool* op);

/**
 * Grabs an object from the object pool
 */

void* opool_grab(ObjectPool* op);

/**
 * Returns an object to the object pool
 */

void opool_release(ObjectPool* op, void* obj);

#endif

/*=============================================================================
// end of file: $RCSfile: objectpool.h,v $
==============================================================================*/
