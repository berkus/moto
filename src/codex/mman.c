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

   $RCSfile: mman.c,v $   
   $Revision: 1.33 $
   $Author: dhakim $
   $Date: 2002/10/31 18:52:03 $
 
==============================================================================*/
#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <sys/file.h>
#include <fcntl.h>
#include <string.h>

#include "memsw.h"
#include "sharedmem.h" /* FIXME: This will have to be wrapped when we go threaded */
#include "mman.h"
#include "exception.h"
#include "excpfn.h"
#include "hashtable.h"
#include "enumeration.h"
#include "err.h"
#include "log.h"

#ifdef SHARED_MALLOC
#	define STD_FREE_FN shared_free
#else
#	define STD_FREE_FN free
#endif

static MMan *mman = NULL;

void 
mman_init() {
   if (mman == NULL) {
      mman = emalloc(sizeof(MMan));
      mman->pools = hset_createDefault();
      mman->pool = mpool_create(10);
   } else {
      printf("\n### Hey, somebody already initialized the memory manager\n");
   }
}

int MMAN_LOCK = 0;

#define LOCK mman_lock()
#define UNLOCK mman_unlock()

#ifdef HAVE_FLOCK

inline 
void mman_lock(){
   if (MMAN_LOCK){
      flock(MMAN_LOCK,LOCK_EX);
   }
}

inline 
void mman_unlock(){
   if (MMAN_LOCK){
      flock(MMAN_LOCK,LOCK_UN);
   }
}

int 
mman_initLocking(char* globalLockDir){
   char lockPath[1024];
   char* lockDir = (globalLockDir != NULL) ? globalLockDir : "/tmp/";

   if(strlen(lockDir)+strlen("mman.lock")+1 < 1024) {
     sprintf(lockPath,"%s/mman.lock",lockDir);

     if (MMAN_LOCK == 0){
       if((MMAN_LOCK = open(lockPath,O_RDWR | O_CREAT,S_IRWXU|S_IRWXO|S_IRWXG))<0){
         fprintf(stderr,"### Could not initialize memory manager locking !!!\n");
         return -1; /* could not create lock */
       }
     }
     return 1;
   } else {
     fprintf(stderr,"### Could not initialize memory manager locking: ");
     fprintf(stderr,"Directory %s is too long.\n",lockDir);
     return -1;
   }

}

#else

static struct flock lock_it;
static struct flock unlock_it;

int 
mman_initLocking(char* globalLockDir){

    lock_it.l_whence = SEEK_SET;        /* from current point */
    lock_it.l_start = 0;                /* -"- */
    lock_it.l_len = 0;                  /* until end of file */
    lock_it.l_type = F_WRLCK;           /* set exclusive/write lock */
    lock_it.l_pid = 0;                  /* pid not actually interesting */
    unlock_it.l_whence = SEEK_SET;      /* from current point */
    unlock_it.l_start = 0;              /* -"- */
    unlock_it.l_len = 0;                /* until end of file */
    unlock_it.l_type = F_UNLCK;         /* set exclusive/write lock */
    unlock_it.l_pid = 0;                /* pid not actually interesting */

   char lockPath[1024];
   char* lockDir = (globalLockDir != NULL) ? globalLockDir : "/tmp/";

   if(strlen(lockDir)+strlen("mman.lock")+1 < 1024) {
     sprintf(lockPath,"%s/mman.lock",lockDir);

     if (MMAN_LOCK == 0){
       if((MMAN_LOCK = open(lockPath,O_RDWR | O_CREAT,S_IRWXU|S_IRWXO|S_IRWXG))<0) {
         fprintf(stderr,"### Could not initialize memory manager locking !!!\n");
         return -1; /* could not create lock */
       }
     }
     return 1;
   } else {
     fprintf(stderr,"### Could not initialize memory manager locking: ");
     fprintf(stderr,"Directory %s is too long.\n",lockDir);
     return -1;
   }

}

inline void 
mman_lock(){
   if (MMAN_LOCK){
      int ret;

      while ((ret = fcntl(MMAN_LOCK, F_SETLKW, &lock_it)) < 0 && errno == EINTR) {
         /* nop */
      }

      if (ret < 0) {fprintf(stderr,"Could not lock shared memory segment.\n"); }
   }
}

inline void 
mman_unlock(){
   if (MMAN_LOCK){
      int ret;

      while ((ret = fcntl(MMAN_LOCK, F_SETLKW, &unlock_it)) < 0 && errno == EINTR) {
         /* nop */
      }
      if (ret < 0) {fprintf(stderr,"Could not lock shared memory segment.\n"); }
   }
}

#endif

void 
mman_cleanup() {
   Enumeration *e = hset_elements(mman->pools);
      
   while (enum_hasNext(e)) {
      MPool *p = enum_next(e);
      mpool_free(p);
   }
   enum_free(e);
   hset_free(mman->pools);
   free(mman);
}

static unsigned int mpool_hash_fn(void* p){
   return (size_t)p/4;
}

MPool *
mpool_create(int size) {
   MPool *p;
   p = emalloc(sizeof(MPool));

#ifdef USE_VECTOR_FOR_POOL
   p->list = vec_create(size,NULL);
#else
   p->list = hset_create(size,mpool_hash_fn,NULL,HSET_POOLBNS);
#endif
   LOCK;
   hset_add(mman->pools, p);
   UNLOCK;
   return p;
}

void 
mpool_free(MPool *p) {
   Enumeration *e;

#ifdef USE_VECTOR_FOR_POOL
   e = vec_elements(p->list);
#else
   e = hset_elements(p->list);
#endif

   if (hset_get(mman->pools, p) == NULL) {
      fprintf(stderr,"### Pool not registered!\n");
   } else while (enum_hasNext(e)) {
      void *ptr = enum_next(e);
      void (*free_fn)(void *) = shared_getDestructor(ptr);

      if(shared_getPool(ptr) != p){
         fprintf(stderr,"### Pool doesn't match!\n");
      } else if(free_fn == NULL) {
         fprintf(stderr,"### Free function is NULL!!!\n");
      } else
      (*free_fn)(ptr);
   }

   enum_free(e);
#ifdef USE_VECTOR_FOR_POOL
   vec_free(p->list);
#else
   hset_free(p->list);
#endif
   
   LOCK;

   hset_remove(mman->pools, p);
   free(p);

   UNLOCK;
}

void 
mpool_clear(MPool *p) {
   Enumeration *e;

#ifdef USE_VECTOR_FOR_POOL
   e = vec_elements(p->list);
#else
   e = hset_elements(p->list);
#endif

   if (hset_get(mman->pools, p) == NULL) {
      fprintf(stderr,"### Pool not registered!\n");
   } else while (enum_hasNext(e)) {
      void *ptr = enum_next(e);
      void (*free_fn)(void *) = shared_getDestructor(ptr);

      if(shared_getPool(ptr) != p){
         fprintf(stderr,"### Pool doesn't match!\n");
      } else if(free_fn == NULL) {
         fprintf(stderr,"### Free function is NULL!!!\n");
      } else
      (*free_fn)(ptr);
   }

   enum_free(e);

#ifdef USE_VECTOR_FOR_POOL
   vec_clear(p->list);
#else
   hset_clear(p->list);
#endif
   
}

/*
 * Allocates and tracks a piece of memory
 */

void *
mman_malloc(MPool *p, size_t size) {
   return mman_mallocf(p,size,STD_FREE_FN);
}

void *
mman_mallocf(MPool *p, size_t size, void(*free_fn)(void *)) {
   void *ptr = emalloc(size);

   if(ptr != NULL){
      shared_setDestructor(ptr,free_fn);
      shared_setPool(ptr,p);
#ifdef USE_VECTOR_FOR_POOL
      vec_add(p->list, ptr );
#else
      hset_add(p->list, ptr );
#endif
   }

   return ptr;
}

MPool *
mman_getPool() {
   return mman->pool;
}

void *
mman_trackf(MPool *p, void *ptr, void(*free_fn)(void *)) {
   MPool *oldp = NULL;
   if (ptr == NULL) return ptr;

   /* Check if the node's current pool is the 
      one it's being tracked to */

   if((oldp = shared_getPool(ptr))!=p) {

      /* Check if the node's been tracked before */
     
#ifdef USE_VECTOR_FOR_POOL 
      if(oldp != NULL){ 

         /* It has, therefore this is a promotion */

         int i; void* swap;
         i=vec_size(oldp->list) -1;

         if(i==-1){
            fprintf(stderr,"### Node allocated to wrong pool!\n"); 
         } else {

            swap = vec_get(oldp->list,i);
            for(;i>=0;i--){
               if (vec_get(oldp->list,i) == ptr){
                  vec_setAt(oldp->list,swap,i);
                  vec_removeAt(oldp->list,vec_size(oldp->list) -1);
                  break;
               }
            }
            if(i==-1){
               fprintf(stderr,"### Node not found in pool!\n");
            }
         }
      } 
      vec_add(p->list,ptr);  /* Add the ptr to the new pool */
#else
      if(oldp != NULL) hset_remove(oldp->list, ptr);
      hset_add(p->list,ptr);
#endif
      
      shared_setPool(ptr,p); /* Set the pool for the ptr to the new pool */
   }

   /* Unless the free function is null (promotion) set the destructor to
      the free function specified */
   
   if(free_fn != NULL)   
      shared_setDestructor(ptr, free_fn);

   return ptr;
}

void *
mman_track(MPool *p, void *ptr) {
   return mman_trackf(p,ptr,STD_FREE_FN);
}

void *
mman_untrack(void *ptr) {
   MPool *oldp;
   
   oldp = shared_getPool(ptr);

   /* Check if the node's been tracked before */

   if(oldp != NULL){

#ifdef USE_VECTOR_FOR_POOL
      /* It has, therefore I must remove it */

      int i; void* swap;
      i=vec_size(oldp->list) -1;

      if(i==-1){
         fprintf(stderr,"### Node allocated to wrong pool!\n");
      } else {

         swap = vec_get(oldp->list,i);
         for(;i>=0;i--){
            if (vec_get(oldp->list,i) == ptr){
               vec_setAt(oldp->list,swap,i);
               vec_removeAt(oldp->list,vec_size(oldp->list) -1);
               break;
            }
         }
         if(i==-1){
            fprintf(stderr,"### Node not found in pool!\n");
         }
      }
#else
      hset_remove(oldp->list, ptr);
#endif

      shared_setPool(ptr,NULL);
      /* FIXME: should I unset the destructor as well ? */
   }

   return ptr;
}

void 
mman_free(void *ptr) {
   MPool *oldp;
   void(*free_fn)(void *);

   oldp = shared_getPool(ptr);
   free_fn = shared_getDestructor(ptr);
 
   if (oldp != NULL) {
      mman_untrack(ptr);
      (*free_fn)(ptr);
   }

}

/*=============================================================================
// end of file: $RCSfile: mman.c,v $
==============================================================================*/

