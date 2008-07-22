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

   $RCSfile: mman.h,v $   
   $Revision: 1.14 $
   $Author: dhakim $
   $Date: 2003/05/29 02:14:08 $
 
==============================================================================*/
#ifndef __MMAN_H
#define __MMAN_H

#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>

#include "sharedmem.h"
#include "vector.h"
#include "hashset.h"

typedef struct mman {
   HashSet *pools;
   struct mPool *pool;
} MMan;

/* #undef USE_VECTOR_FOR_POOL */

typedef struct mPool {
#ifdef USE_VECTOR_FOR_POOL
   Vector *list;
#else
   HashSet *list;
#endif
} MPool;

void mman_init();
int mman_initLocking(char* globalLockDir);
void mman_cleanup();

MPool *mpool_create(int size);

void mpool_free(MPool *pool);
void mpool_clear(MPool *pool);

void *mman_malloc(MPool *p, size_t size);
void *mman_mallocf(MPool *p, size_t size, void(*free_fn)(void *));

MPool *mman_getPool();

void *mman_track(MPool *p, void *ptr);
void *mman_trackf(MPool *pool, void *ptr, void(*free_fn)(void *));
void *mman_untrack(void *ptr);

void mman_free(void *ptr);

#endif

/*=============================================================================
// end of file: $RCSfile: mman.h,v $
==============================================================================*/

