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

   $RCSfile: mxarr.h,v $
   $Revision: 1.13 $
   $Author: dhakim $
   $Date: 2003/03/03 03:40:15 $

==============================================================================*/
#ifndef __MXARR_H
#define __MXARR_H

#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "integer.h"
#include "exception.h"

/* moto type variants */
typedef enum {
   ARR_BOOLEAN_TYPE = 1,
   ARR_BYTE_TYPE 	= 2,
   ARR_CHAR_TYPE    = 3,
   ARR_INT32_TYPE   = 4,
   ARR_INT64_TYPE   = 5,
   ARR_FLOAT_TYPE   = 6,
   ARR_DOUBLE_TYPE  = 7,
   ARR_VOID_TYPE    = 8,
   ARR_REF_TYPE     = 9,
} ArrTypeKind;

typedef struct arrMeta {
   int length;
   int dim;
   void* subtype;
} ArrMeta;

typedef struct booleanArr {
   struct arrMeta meta;
   unsigned char data[1];
} BooleanArray;

typedef struct byteArr {
   struct arrMeta meta;
   unsigned char data[1];
} ByteArray;

typedef struct charArr {
   struct arrMeta meta;
   char data[1];
} CharArray;

typedef struct intArr {
   struct arrMeta meta;
   int32_t data[1];
} IntArray;

typedef struct longArr {
   struct arrMeta meta;
   int64_t data[1];
} LongArray;

typedef struct floatArr {
   struct arrMeta meta;
   float data[1];
} FloatArray;

typedef struct doubleArr {
   struct arrMeta meta;
   double data[1];
} DoubleArray;

typedef struct refArr {
   struct arrMeta meta;
   void* data[1];
} RefArray;

typedef struct arrArr {
   struct arrMeta meta;
   union unArr* data[1];
} ArrArray;

typedef union unArr {
   struct arrMeta meta;
   struct booleanArr ba;
   struct byteArr ya;
   struct charArr ca;
   struct intArr ia;
   struct longArr la;
   struct floatArr fa;
   struct doubleArr da;
   struct refArr ra;
   struct arrArr aa;
} UnArray;

UnArray* arr_create(int dim,char unspec,int l,int* dimarr,int subtype);
UnArray* arr_create_and_init(
	int dim,char unspec,int l,int* dimarr,int subtype,int num_init, ...);

inline int arr_length(UnArray* ua);

inline int32_t* isub(UnArray* ua,int index);
inline int64_t* lsub(UnArray* ua,int index);
inline float* fsub(UnArray* ua,int index);
inline double* dsub(UnArray* ua,int index);
inline unsigned char* bsub(UnArray* ua,int index);
inline char* csub(UnArray* ua,int index);
inline signed char* ysub(UnArray* ua,int index);
inline void** rsub(UnArray* ua,int index);

inline int32_t* inline_isub(UnArray* ua,int index);
inline int64_t* inline_lsub(UnArray* ua,int index);
inline float* inline_fsub(UnArray* ua,int index);
inline double* inline_dsub(UnArray* ua,int index);
inline unsigned char* inline_bsub(UnArray* ua,int index);
inline char* inline_csub(UnArray* ua,int index);
inline signed char* inline_ysub(UnArray* ua,int index);
inline void** inline_rsub(UnArray* ua,int index);

inline int32_t inline_checkForIntZero(int32_t p);
inline int64_t inline_checkForLongZero(int64_t p);
inline float inline_checkForFloatZero(float p);
inline double inline_checkForDoubleZero(double p);
inline char inline_checkForCharZero(char p);
inline signed char inline_checkForByteZero(signed char p);   
   
inline void* inline_checkForNullDereference(void* p);
inline void* inline_checkForNullMethCall(void* p);
inline void* inline_checkForNullCallee(void* p);

#define CHK_NULL_M(SELF_PTR) (excp_file=__FILE__,excp_line=__LINE__,inline_checkForNullMethCall(SELF_PTR))
#define CHK_NULL_D(SELF_PTR) (excp_file=__FILE__,excp_line=__LINE__,inline_checkForNullDereference(SELF_PTR))
#define CHK_NULL_C(CALLEE_PTR) (excp_file=__FILE__,excp_line=__LINE__,inline_checkForNullCallee(CALLEE_PTR))

#endif
