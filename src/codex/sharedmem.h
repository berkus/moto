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

   $RCSfile: sharedmem.h,v $   
   $Revision: 1.13 $
   $Author: corz $
   $Date: 2002/10/30 23:39:32 $
 
==============================================================================*/
#ifndef __SHAREDMEM_H
#define __SHAREDMEM_H

#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>

/*
 * Memory Layout
 *
 *      +----------------------+
 *  0   | Malloc / Free in Use |
 *      +----------------------+
 * -4   | Free Range Root*     |--------+
 *      +----------------------+        |
 * -8   | Used Range Root*     |-----+  |
 *      +----------------------+     |  |
 * -16  | Low Memory*          |--+  |  |
 *      +----------------------+  |  |  |
 * -20  |                      |  |  |  |
 *      | Free Range Tree Node |  |  |  |
 *      |                      |  |  |  |
 *      |                      |  |  |  |
 *      |                      |  |  |  |
 *      +----------------------+  |  |  |
 * -40  |                      |  |  |  |
 *      | Free Range Tree Node |  |  |  |
 *      |                      |  |  |  |
 *      |                      |  |  |  |
 *      |                      |  |  |  |
 *      +----------------------+  |  |  |
 *      |                      |<-|--+  |
 *      | Used Range Tree Node |  |     |
 *      |                      |  |     |
 *      |                      |  |     |
 *      |                      |  |     |
 *      +----------------------+  |     |
 *      |                      |  |     |
 *      | Free Range Tree Node |  |     |
 *      |                      |  |     |
 *      |                      |  |     |
 *      |                      |  |     |
 *      +----------------------+  |     |
 *      |                      |  |     |
 *      | Used Range Tree Node |  |     |
 *      |                      |  |     |
 *      |                      |  |     |
 *      |                      |  |     |
 *      +----------------------+  |     |
 *      |                      |<-|-----+
 *      | Free Range Tree Node |  |
 *      |                      |  |
 *      |                      |  |
 *      |                      |  |
 *      +----------------------+  |
 *                              <-+
 */

int shared_initDefault();
int shared_init(size_t size);
int shared_initLocking(char* globalLockDir);
void shared_cleanup();
void* shared_alloc(size_t size);
void shared_free(void* address);
int shared_check(void* address);
void* shared_calloc(size_t nmemb,size_t size);
void* shared_realloc(void* address, size_t size);

int shared_getTotalMemory();
int shared_getFreeMemory();
int shared_getANodes();
int shared_getFNodes();

void shared_mark(void* rootAddress);
void shared_sweep();

unsigned int shared_nodesAllocated();
void debug();

/* Allow storage of pool and destructor in info record */

inline void shared_setPool(void* address, void* pool);
inline void shared_setDestructor(void* address, void(*freefn)(void *));
inline void* shared_getPool(void* address);
inline void* shared_getDestructor(void* address);
#endif

/*=============================================================================
// end of file: $RCSfile: sharedmem.h,v $
==============================================================================*/

