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

   $RCSfile: blockpool.h,v $
   $Revision: 1.2 $
   $Author: dhakim $
   $Date: 2003/05/29 02:14:08 $

==============================================================================*/
#ifndef __BLOCKPOOL_H
#define __BLOCKPOOL_H

#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/* define POOLDEBUG */

#ifdef POOLDEBUG
#include "hashset.h"
#endif

typedef struct block{
   void* arr;
   struct block* next;
} Block;

typedef struct blockpool{
   void* (*newfn)(int);	        /* function to create new object pool objects */
   void* (*getfn)(void*,int);   /* function to create new object pool objects */
   void (*freefn)(void *);      /* function to free object pool objects */
   void (*grabfn)(void *);      /* function to call on objects prior to grab */
   void (*relfn)(void *);       /* function to call on objects after release */
   void** pool;
   int size;
   int max;
   char flags;
#ifdef POOLDEBUG
   HashSet* ptrs;
#endif
   Block* block;
} BlockPool;

/**
 * Creates an object pool of the specified size. Fills it with objects created
 * be the specified constructor function (newfn)
 */

BlockPool* bpool_create(
   void* (*newfn)(int),
   void* (*getfn)(void*,int),
   void (*freefn)(void *),
   void (*grabfn)(void *),
   void (*relfn)(void *),
   int size
);

/**
 * Frees the objects in the object pool by the specified free function. then
 * frees the pool itself
 */

void bpool_free(BlockPool* op);

/**
 * Grabs an object from the object pool
 */

void* bpool_grab(BlockPool* op);

/**
 * Returns an object to the object pool
 */

void bpool_release(BlockPool* op, void* obj);

#endif

/*=============================================================================
// end of file: $RCSfile: blockpool.h,v $
==============================================================================*/
