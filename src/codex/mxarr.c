/*=============================================================================

   Copyright (C) 2000 Kenneth Kocienda and David Hakim.
   This file is part of the Moto Programming Language.

   Moto Programming Language is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   Moto Programming Language is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the Codex C Library; see the file COPYING.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   $RCSfile: mxarr.c,v $
   $Revision: 1.11 $
   $Author: dhakim $
   $Date: 2003/03/03 03:40:15 $

==============================================================================*/

#include "stdio.h"

#include "mxarr.h"
#include "runtime.h"
#include "memsw.h"


inline int arr_length(UnArray* ua){
	if (ua == NULL) {
		THROW_D("NullPointerException");
	}
	return ua->meta.length;
}

static void inline checkForNullAndBounds(UnArray* ua,int index){
   if (ua == NULL) {
      THROW_D("NullPointerException");
   } if (index < 0 || index >= ua->meta.length){
      THROW_D("ArrayBoundsException");
   } 
}

static void inline inline_checkForNullAndBounds(UnArray* ua,int index){
   if (ua == NULL) {
      excp_getFillAndThrow("NullPointerException","Attempt to subscript null");
   } if (index < 0 || index >= ua->meta.length){
      excp_getFillAndThrow("ArrayBoundsException","Attempt to subscript array outside of declared bounds");
   } 
}

inline void* inline_checkForNullDereference(void* p){
   if (p == NULL) excp_getFillAndThrow("NullPointerException","Attempt to dereference null");
   return p;
}

inline void* inline_checkForNullMethCall(void* p){
   if (p == NULL) excp_getFillAndThrow("NullPointerException","A method may not be called on null");
   return p;
}

inline void* inline_checkForNullCallee(void* p){
   if (p == NULL) excp_getFillAndThrow("NullPointerException","Cannot call null");
   return p;
}

inline int32_t inline_checkForIntZero(int32_t p){
   if (p == 0) excp_getFillAndThrow("MathException","Attempt to divide by zero"); return p; }
inline int64_t inline_checkForLongZero(int64_t p){
   if (p == 0) excp_getFillAndThrow("MathException","Attempt to divide by zero"); return p; }
inline float inline_checkForFloatZero(float p){
   if (p == 0) excp_getFillAndThrow("MathException","Attempt to divide by zero"); return p; }
inline double inline_checkForDoubleZero(double p){
   if (p == 0) excp_getFillAndThrow("MathException","Attempt to divide by zero"); return p; }
inline char inline_checkForCharZero(char p){
   if (p == 0) excp_getFillAndThrow("MathException","Attempt to divide by zero"); return p; }
inline signed char inline_checkForByteZero(signed char p){
   if (p == 0) excp_getFillAndThrow("MathException","Attempt to divide by zero"); return p; }
   
inline int32_t* isub(UnArray* ua,int index){
   checkForNullAndBounds(ua,index); return &ua->ia.data[index]; }
inline int64_t* lsub(UnArray* ua,int index){
   checkForNullAndBounds(ua,index); return &ua->la.data[index]; }
inline float* fsub(UnArray* ua,int index){
   checkForNullAndBounds(ua,index); return &ua->fa.data[index]; }
inline double* dsub(UnArray* ua,int index){
   checkForNullAndBounds(ua,index); return &ua->da.data[index]; }
inline unsigned char* bsub(UnArray* ua,int index){
   checkForNullAndBounds(ua,index); return &ua->ba.data[index]; }
inline char* csub(UnArray* ua,int index){
   checkForNullAndBounds(ua,index); return &ua->ca.data[index]; }
inline signed char* ysub(UnArray* ua,int index){
   checkForNullAndBounds(ua,index); return &ua->ya.data[index]; }
inline void** rsub(UnArray* ua,int index){
   checkForNullAndBounds(ua,index); return &ua->ra.data[index]; }

inline int32_t* inline_isub(UnArray* ua,int index){
   inline_checkForNullAndBounds(ua,index); return &ua->ia.data[index]; }
inline int64_t* inline_lsub(UnArray* ua,int index){
   inline_checkForNullAndBounds(ua,index); return &ua->la.data[index]; }
inline float* inline_fsub(UnArray* ua,int index){
   inline_checkForNullAndBounds(ua,index); return &ua->fa.data[index]; }
inline double* inline_dsub(UnArray* ua,int index){
   inline_checkForNullAndBounds(ua,index); return &ua->da.data[index]; }
inline unsigned char* inline_bsub(UnArray* ua,int index){
   inline_checkForNullAndBounds(ua,index); return &ua->ba.data[index]; }
inline char* inline_csub(UnArray* ua,int index){
   inline_checkForNullAndBounds(ua,index); return &ua->ca.data[index]; }
inline signed char* inline_ysub(UnArray* ua,int index){
   inline_checkForNullAndBounds(ua,index); return &ua->ya.data[index]; }
inline void** inline_rsub(UnArray* ua,int index){
   inline_checkForNullAndBounds(ua,index); return &ua->ra.data[index]; }
   
UnArray* arr_create(int dim,char unspec,int l,int* dimarr,int subtype){
   UnArray* arr = NULL;
   int i;

   if (dim < 1)
     THROW_D("ArrayBoundsException");

   if(l == dim - 1){
      if(unspec == 0 ){
         switch(subtype){
            case ARR_INT32_TYPE:
               arr = (UnArray*)calloc(sizeof(UnArray) + sizeof(int32_t)*(dimarr[l]-1),1); break;
            case ARR_INT64_TYPE:
               arr = (UnArray*)calloc(sizeof(UnArray) + sizeof(int64_t)*(dimarr[l]-1),1); break;
            case ARR_FLOAT_TYPE:
               arr = (UnArray*)calloc(sizeof(UnArray) + sizeof(float)*(dimarr[l]-1),1); break;
            case ARR_DOUBLE_TYPE:
               arr = (UnArray*)calloc(sizeof(UnArray) + sizeof(double)*(dimarr[l]-1),1); break;
            case ARR_BOOLEAN_TYPE:
               arr = (UnArray*)calloc(sizeof(UnArray) + sizeof(unsigned char)*(dimarr[l]-1),1); break;
            case ARR_CHAR_TYPE:
               arr = (UnArray*)calloc(sizeof(UnArray) + sizeof(char)*(dimarr[l]-1),1); break;
            case ARR_BYTE_TYPE:
               arr = (UnArray*)calloc(sizeof(UnArray) + sizeof(signed char)*(dimarr[l]-1),1); break;
            case ARR_REF_TYPE:
               arr = (UnArray*)calloc(sizeof(UnArray) + sizeof(void*)*(dimarr[l]-1),1); break;
         }
      } else 
         arr=(UnArray*)calloc(sizeof(UnArray) + sizeof(void*)*(dimarr[l]-1),1);
   } else {
      arr=(UnArray*)calloc(sizeof(UnArray) + sizeof(void*)*(dimarr[l]-1),1);
      for(i=0;i<dimarr[l];i++){
         arr->aa.data[i] = arr_create(dim,unspec,l+1,dimarr,subtype);
      }
   }

   arr->meta.length=dimarr[l];
   mman_track(rtime_getMPool(),arr);
   return arr;
}


UnArray* arr_create_and_init(int dim,char unspec,int l,int* dimarr,int subtype, 
					 int num_init, ...){

	va_list args;
	int count;
	UnArray * ua;

	ua = arr_create(dim, unspec, l, dimarr, subtype);

	va_start(args, num_init);
	for (count = 0; count < num_init; count++) {
		switch(subtype){
			case ARR_INT32_TYPE:
				*(&ua->ia.data[count]) = (int32_t) va_arg(args, int32_t); break;
			case ARR_INT64_TYPE:
				*(&ua->la.data[count]) = (int64_t) va_arg(args, int64_t); break;
			case ARR_FLOAT_TYPE:
				*(&ua->fa.data[count]) = (float) va_arg(args, double); break;
			case ARR_DOUBLE_TYPE:
				*(&ua->da.data[count]) = (double) va_arg(args, double); break;
			case ARR_BOOLEAN_TYPE:
				*(&ua->ba.data[count]) = (unsigned char) va_arg(args, int); break;
			case ARR_CHAR_TYPE:
				*(&ua->ca.data[count]) = (char) va_arg(args, int); break;
			case ARR_BYTE_TYPE:
				*(&ua->ya.data[count]) = (signed char) va_arg(args, int); break;
			case ARR_REF_TYPE:
				*(&ua->ra.data[count]) = va_arg(args, void *); break;
		}
	}
	va_end(args);	

	return ua;

}
