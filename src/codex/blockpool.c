#include "memsw.h"
#include "blockpool.h"
#include "exception.h"
#include "excpfn.h"

/*
 */

BlockPool* bpool_create(
   void* (*newfn)(int), 
   void* (*getfn)(void *, int),
   void (*freefn)(void *),
   void (*grabfn)(void *),
   void (*relfn)(void *),
   int size
){
   int i;
   BlockPool* bpool;

   bpool = (BlockPool*)emalloc(sizeof(BlockPool));
   bpool->pool = (void**)emalloc(size * sizeof(void*));
   bpool->max = size;
   bpool->size = size;

   bpool->newfn=newfn;
   bpool->getfn=getfn;
   bpool->freefn=freefn;
   bpool->grabfn=grabfn;
   bpool->relfn=relfn;

#ifdef POOLDEBUG
   bpool->ptrs = hset_createDefault();
#endif

   bpool->block = (Block*)emalloc(sizeof(Block));
   bpool->block->arr = (*bpool->newfn)(size);
   bpool->block->next = NULL;

   for(i=0;i<size;i++){
      bpool->pool[i] = (*bpool->getfn)(bpool->block->arr,i);
#ifdef POOLDEBUG
      hset_add(bpool->ptrs,bpool->pool[i]); 
#endif
   }

   return bpool;
}

/*
 * Frees the objects in the object pool by the specified free function. then
 * frees the pool itself
 */

void bpool_free(BlockPool* bpool){
   Block *cb,*nb;

   cb=bpool->block;
   while(cb){
      nb = cb->next;
      (*bpool->freefn)(cb->arr);
      free(cb);
      cb = nb;
   }
   free(bpool->pool);
   free(bpool);
}

/* 
 * Grabs an object from the object pool
 */

void* bpool_grab(BlockPool* bpool){
   int i;
   void* r;

   if(bpool->size > 0){
      bpool->size -= 1;
      r = bpool->pool[bpool->size];
   } else {
      Block* cb;

      bpool->size = bpool->max;
      bpool->max *= 2;
      bpool->pool = (void**)erealloc(bpool->pool,bpool->max * sizeof(void*));

      cb = bpool->block;
      bpool->block = (Block*)emalloc(sizeof(Block));
      bpool->block->arr = (*bpool->newfn)(bpool->size);
      bpool->block->next = cb;

      for(i=0;i<bpool->size;i++){
         bpool->pool[i] = (*bpool->getfn)(bpool->block->arr,i);
#ifdef POOLDEBUG
         hset_add(bpool->ptrs,bpool->pool[i]);
#endif
      }
      bpool->size -= 1;
      r = bpool->pool[bpool->size];
   }
   
   if (bpool->grabfn != NULL){
      (*bpool->grabfn)(r);
   }
   
   return r;
}

/*
 * Returns an object to the object pool
 */

void bpool_release(BlockPool* bpool, void* obj){

   if(obj == NULL){
      THROW("IllegalArgumentException","Attempt to release a null pointer");
   }

#ifdef POOLDEBUG
   if(!hset_contains(bpool->ptrs,obj)){
      printf("!!! crap! where did this come from!?!");
      exit(-1);
   }   
#endif  

   if(bpool->size >= bpool->max) {
      THROW_D("ArrayBoundsException");
   }

   if (bpool->relfn != NULL){
      (*bpool->relfn)(obj);
   }

   bpool->pool[bpool->size]= obj;
   bpool->size += 1;
}
