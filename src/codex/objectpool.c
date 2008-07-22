#include "memsw.h"
#include "objectpool.h"
#include "exception.h"
#include "excpfn.h"

/*
 * Creates an object pool of the specified size. Fills it with objects created
 * be the specified constructor function (newfn)
 */

ObjectPool* opool_create(
   void* (*newfn)(void), 
   void(*freefn)(void *),
   void(*grabfn)(void *),
   void(*relfn)(void *),
   int size
){
   int i;
   ObjectPool* opool;

   opool = (ObjectPool*)emalloc(sizeof(ObjectPool));
   opool->pool = (void**)emalloc(size * sizeof(void*));
   opool->max = size;
   opool->size = size;

   opool->newdfn=newfn;
   opool->freefn=freefn;
   opool->grabfn=grabfn;
   opool->relfn=relfn;
   
   opool->ext=NULL;
   
#ifdef POOLDEBUG
   opool->ptrs = hset_createDefault();
#endif

   for(i = 0;i<size;i++){
      opool->pool[i] = (*opool->newdfn)();
#ifdef POOLDEBUG
      hset_add(opool->ptrs,opool->pool[i]); 
#endif
   }

   return opool;
}

ObjectPool* opool_createWithExt(
   void*(*newfn)(void *), 
   void(*freefn)(void *),
   void(*grabfn)(void *),
   void(*relfn)(void *),
   void* ext,
   int size
){
   int i;
   ObjectPool* opool;

   opool = (ObjectPool*)emalloc(sizeof(ObjectPool));
   opool->pool = (void**)emalloc(size * sizeof(void*));
   opool->max = size;
   opool->size = size;

   opool->newfn=newfn;
   opool->freefn=freefn;
   opool->grabfn=grabfn;
   opool->relfn=relfn;
	opool->ext=ext;
	
#ifdef POOLDEBUG
   opool->ptrs = hset_createDefault();
#endif

   for(i = 0;i<size;i++){
      opool->pool[i] = (*opool->newfn)(opool->ext);
#ifdef POOLDEBUG
      hset_add(opool->ptrs,opool->pool[i]); 
#endif
   }

   return opool;
}

/*
 * Frees the objects in the object pool by the specified free function. then
 * frees the pool itself
 */

void opool_free(ObjectPool* op){
   int i;

   for(i = 0;i<op->size;i++)
      if(op->freefn != NULL)
      	(*op->freefn)(op->pool[i]);
   free(op->pool);
   free(op);
}

/* 
 * Grabs an object from the object pool
 */

void* opool_grab(ObjectPool* op){
   int i;
   void* r;

   if(op->size > 0){
      op->size -= 1;
      r = op->pool[op->size];
   } else {
      op->size = op->max;
      op->max *= 2;
      op->pool = (void**)erealloc(op->pool,op->max * sizeof(void*));
      for(i=0;i<op->size;i++){
      	if(op->ext == NULL)
         	op->pool[i] = (*op->newdfn)();
         else
         	op->pool[i] = (*op->newfn)(op->ext);
#ifdef POOLDEBUG
         hset_add(op->ptrs,op->pool[i]);
#endif
      }
      op->size -= 1;
      r = op->pool[op->size];
   }
   
   if (op->grabfn != NULL){
      (*op->grabfn)(r);
   }
   
   return r;
}

/*
 * Returns an object to the object pool
 */

void opool_release(ObjectPool* op, void* obj){

   if(obj == NULL){
      THROW("IllegalArgumentException","Attempt to release a null pointer");
   }

#ifdef POOLDEBUG
   if(!hset_contains(op->ptrs,obj)){
      printf("!!! crap! where did this come from!?!");
      exit(-1);
   }   
#endif  

   if(op->size >= op->max) {
      THROW_D("ArrayBoundsException");
   }

   if (op->relfn != NULL){
      (*op->relfn)(obj);
   }

   op->pool[op->size]= obj;
   op->size += 1;
}
