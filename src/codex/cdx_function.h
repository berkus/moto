/*=============================================================================

   Copyright (C) 2002-2003 David Hakim.
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

   $RCSfile: cdx_function.h,v $   
   $Revision: 1.7 $
   $Author: dhakim $
   $Date: 2003/03/13 15:59:43 $
 
==============================================================================*/
#ifndef __CDX_FUNCTION_H
#define __CDX_FUNCTION_H

#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "integer.h"

#define F_INTERPRETED	0x01

typedef enum {
   F_BOOLEAN_TYPE = 1,
   F_BYTE_TYPE    = 2,
   F_CHAR_TYPE    = 3,
   F_INT32_TYPE   = 4,
   F_INT64_TYPE   = 5,
   F_FLOAT_TYPE   = 6,
   F_DOUBLE_TYPE  = 7,
   F_VOID_TYPE    = 8,
   F_REF_TYPE     = 9,
} FuncTypeKind;

typedef struct argument{
	int index;
	int offset;
} Argument;

typedef struct function{	
	int flags;
	char* fname;
	void* fn;
	struct function *parent;
	void* self;
	int pargc; 				/* Partially filled argument count */
	void *pargv;			/* Partially filled argument values */ 
	Argument pindex[0];		/* Partially filled argument index */
} Function;


extern Function* CURRENT_FUNCTION;

extern unsigned char (*ifunc_bcall)(Function*, char*, ...);
extern char (*ifunc_ccall)(Function*, char*, ...);
extern double (*ifunc_dcall)(Function*, char*, ...);
extern float (*ifunc_fcall)(Function*, char*, ...);
extern int32_t (*ifunc_icall)(Function*, char*, ...);
extern int64_t (*ifunc_lcall)(Function*, char*, ...);
extern void* (*ifunc_rcall)(Function*, char*, ...);
extern signed char (*ifunc_ycall)(Function*, char*, ...);
extern void (*ifunc_vcall)(Function*, char*, ...);

Function* func_create(
	int flags,  
	char* fname,
	void *fn,
	void *self
);

Function* func_createPartial(
	int flags, 
	char* fname,
	void *fn,
	void *self,
	int pargc,				/* The number of arguments with declared values */
	int* pargi,				/* The indexes of those arguments in the original function */
	FuncTypeKind* pargt		/* The types of those arguments in the original function */
);

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
);

void func_setArg(Function* f,int i, FuncTypeKind argt, void* src);

void * func_getArg(Function* f,int i);

inline int32_t iarg(Function* f,int i);
inline int64_t larg(Function* f,int i);
inline float farg(Function* f,int i);
inline double darg(Function* f,int i);
inline unsigned char barg(Function* f,int i);
inline char carg(Function* f,int i);
inline signed char yarg(Function* f,int i);
inline void* rarg(Function* f,int i);

void func_free(Function* f);

#endif
