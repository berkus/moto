#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "memsw.h"
#include "excpfn.h"
#include "cdx_function.h"

Function* CURRENT_FUNCTION = NULL;

unsigned char (*ifunc_bcall)(Function*, char*, ...) = NULL;
char (*ifunc_ccall)(Function*, char*, ...) = NULL;
double (*ifunc_dcall)(Function*, char*, ...) = NULL;
float (*ifunc_fcall)(Function*, char*, ...) = NULL;
int32_t (*ifunc_icall)(Function*, char*, ...) = NULL;
int64_t (*ifunc_lcall)(Function*, char*, ...) = NULL;
void* (*ifunc_rcall)(Function*, char*, ...) = NULL;
signed char (*ifunc_ycall)(Function*, char*, ...) = NULL;
void (*ifunc_vcall)(Function*, char*, ...) = NULL;

Function* func_create(
	int flags, 
	char* fname,
	void *fn,
	void *self
){
	Function* f = (Function*)emalloc(sizeof(Function));
	f->flags = flags;
	f->fname = fname;
	f->fn = fn;
	f->self = self;
	f->pargc = 0;
	return f;
}


/** 
 * Takes: the number of types specified by argc in the array argt and an unfilled offsets array
 *	(also of length argc)
 *	
 *	Actions: Fills in the offsets array with the offset location of a value of the specified type
 *	in a structure that would hold arguments of those types in that order.
 *   
 * Returns: The size of the structure needed to store the values of the specified types 
**/
   
int func_fillOffsets(int argc, FuncTypeKind* argt, int* offsets){
   int i,curVarSize = 0,curOffset = 0;
   
   for(i=0;i<argc;i++) {
      switch(argt[i]) {
			case F_BOOLEAN_TYPE: curVarSize = sizeof(char); break;
			case F_BYTE_TYPE: curVarSize = sizeof(signed char); break;
			case F_CHAR_TYPE: curVarSize = sizeof(char); break;
			case F_INT32_TYPE: curVarSize = sizeof(int32_t); break;
			case F_INT64_TYPE: curVarSize = sizeof(int64_t); break;
			case F_FLOAT_TYPE: curVarSize = sizeof(float); break;
			case F_DOUBLE_TYPE: curVarSize = sizeof(double); break;
			case F_VOID_TYPE: curVarSize = 0; break;
			case F_REF_TYPE: curVarSize = sizeof(void*); break;
		}	
		if(curOffset % curVarSize != 0)
		   curOffset += curVarSize - curOffset % curVarSize;
		
		offsets[i]=curOffset;
		curOffset += curVarSize;
   }
   
   return curOffset;
}

Function* 
func_createPartial(
	int flags, 
	char* fname,
	void *fn,
	void *self,
	int pargc,				/* The number of arguments with declared values */
	int* pargi,				/* The indexes of those arguments in the original function */
	FuncTypeKind* pargt	/* The types of those arguments in the original function */
){
	Function* f;
	int* offsets = emalloc(sizeof(int) * pargc);
	int argvSize = func_fillOffsets(pargc,pargt,offsets);
	int argiSize = pargc * sizeof(Argument);
	int i;
	
	f = (Function*)emalloc(
		sizeof(Function) + argiSize + argvSize + 
		(sizeof(Function) + argiSize) % sizeof(double) /* Padding if args has doubles */
	);
	f->flags = flags;
	f->fname = fname;
	f->fn = fn;
	f->self = self;
	
	f->pargc = pargc;
	
	for(i=0;i<pargc;i++){
		f->pindex[i].index = pargi[i];
		f->pindex[i].offset = offsets[i];
	}
	
	/* Start the arg values at the end of the arg indexes */
	f->pargv = f->pindex + pargc;
	f->pargv += (sizeof(Function) + argiSize) % sizeof(double);
	
	free(offsets); 
	return f;
}

Function* 
func_createPartialWithValues(
	int flags, 
	char* fname,
	void *fn,
	Function *parent,	
	void *self,
	int pargc,				/* The number of arguments with declared values */
	int* pargi,				/* The indexes of those arguments in the original function */
	FuncTypeKind* pargt,	/* The types of those arguments in the original function */
	...
){
	Function* f;
	int* offsets = emalloc(sizeof(int) * pargc);
	int argvSize = func_fillOffsets(pargc,pargt,offsets);
	int argiSize = pargc * sizeof(Argument);
	va_list args; 
	int i;
	
	f = (Function*)emalloc(sizeof(Function) + argiSize + argvSize);
	f->flags = flags;
	f->fname = fname;
	f->fn = fn;
	f->parent = parent;
	f->self = self;
	
	f->pargc = pargc;

	/* Start the arg values at the end of the arg indexes */
	f->pargv = f->pindex + pargc;
		
	for(i=0;i<pargc;i++){
		f->pindex[i].index = pargi[i];
		f->pindex[i].offset = offsets[i];
	}
	
	va_start(args, pargt);
	for(i=0;i<pargc;i++){
	   unsigned char ab;
	   signed char ay;
	   char ac;
	   int32_t ai;
	   int64_t al;
	   float af;
	   double ad;
	   void* ar;

		switch(pargt[i]) {
			
			case F_BOOLEAN_TYPE: 
				ab =(unsigned char)va_arg(args, int); func_setArg(f,i,pargt[i],&ab); break;
			case F_BYTE_TYPE: 
				ay = (signed char)va_arg(args, int); func_setArg(f,i,pargt[i],&ay); break;
			case F_CHAR_TYPE:
				ac = (char)va_arg(args, int); func_setArg(f,i,pargt[i],&ac); break;
			case F_INT32_TYPE: 
				ai = (int32_t)va_arg(args, int32_t); func_setArg(f,i,pargt[i],&ai); break;
			case F_INT64_TYPE: 
				al = (int64_t)va_arg(args, int64_t); func_setArg(f,i,pargt[i],&al); break;
			case F_FLOAT_TYPE: 
				af = (float)va_arg(args, double); func_setArg(f,i,pargt[i],&af); break;
			case F_DOUBLE_TYPE: 
				ad = (double)va_arg(args, double); func_setArg(f,i,pargt[i],&ad); break;
			case F_VOID_TYPE:  break;
			case F_REF_TYPE: 
				ar = (void*)va_arg(args, void*); func_setArg(f,i,pargt[i],&ar); break;
		}	   
	}
	va_end(args);
	
	free(offsets); 
	return f;
}

void 
func_setArg(Function* f,int i, FuncTypeKind argt, void* src){
	void* dest = f->pargv + f->pindex[i].offset;
	switch(argt) {
		
		case F_BOOLEAN_TYPE: *(unsigned char*)dest = *(unsigned char*)src; break;
		case F_BYTE_TYPE: *(signed char*)dest = *(signed char*)src; break;
		case F_CHAR_TYPE: *(char*)dest = *(char*)src; break;
		case F_INT32_TYPE: *(int32_t*)dest = *(int32_t*)src; break;
		case F_INT64_TYPE: *(int64_t*)dest = *(int64_t*)src; break;
		case F_FLOAT_TYPE: *(float*)dest = *(float*)src; break;
		case F_DOUBLE_TYPE: *(double*)dest = *(double*)src; break;
		case F_VOID_TYPE:  break;
		case F_REF_TYPE: *(void**)dest = *(void**)src; break;
	}	
}

void * 
func_getArg(Function* f,int i){
	return f->pargv + f->pindex[i].offset;	
}

inline int32_t 
iarg(Function* f,int i){ return *(int32_t*)(f->pargv + f->pindex[i].offset); }

inline int64_t 
larg(Function* f,int i){ return *(int64_t*)(f->pargv + f->pindex[i].offset); }

inline float 
farg(Function* f,int i){ return *(float*)(f->pargv + f->pindex[i].offset); }

inline double 
darg(Function* f,int i){ return *(double*)(f->pargv + f->pindex[i].offset); }

inline unsigned char 
barg(Function* f,int i){ return *(unsigned char*)(f->pargv + f->pindex[i].offset); }

inline char
carg(Function* f,int i){ return *(char*)(f->pargv + f->pindex[i].offset); }

inline signed char
yarg(Function* f,int i){ return *(signed char*)(f->pargv + f->pindex[i].offset); }

inline void*
rarg(Function* f,int i){ return *(void**)(f->pargv + f->pindex[i].offset); }

void func_free(Function* f){
	free(f);
}
