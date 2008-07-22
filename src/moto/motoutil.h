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

   $RCSfile: motoutil.h,v $   
   $Revision: 1.7 $
   $Author: dhakim $
   $Date: 2002/10/01 22:33:50 $
 
==============================================================================*/
#ifndef __MOTOUTIL_H
#define __MOTOUTIL_H

#include "integer.h"

#include "env.h"
#include "cell.h"
#include "type.h"

inline char *
uc_str(const UnionCell *p, int index);
inline char *
uc_code(const UnionCell *p, int index);
inline int 
uc_opcode(const UnionCell *p);
inline int 
uc_opcount(const UnionCell *p);
inline UnionCell *
uc_operand(const UnionCell *p, int index);
inline MetaInfo *
uc_meta(const UnionCell *p);

inline MotoVal *
opstack_pop(MotoEnv *env);
inline MotoVal *
opstack_peek(MotoEnv *env);
inline MotoVal *
opstack_peekAt(MotoEnv *env, int index);
inline void 
opstack_push(MotoEnv *env, MotoVal *val);
inline int 
opstack_size(MotoEnv *env);

inline char 
bv(MotoVal *val);
inline char 
cv(MotoVal *val);
inline signed char
yv(MotoVal *val);
inline int32_t 
iv(MotoVal *val);
inline int64_t 
lv(MotoVal *val);
inline float 
fv(MotoVal *val);
inline double 
dv(MotoVal *val);
inline char *
sv(MotoVal *val);

inline int 
isType(char *type, MotoVal *val);
inline int 
isVarType(char *type, MotoVar *var);
inline int 
isObject(MotoVal *val);
inline int 
isString(MotoVal *val);
inline int 
isRegex(MotoVal *val);
inline int
isArray(MotoVal *val);

char *
moto_strdup(MotoEnv *env, char *s);
void *
moto_malloc(MotoEnv *env, size_t size);
char *
moto_nameForOp(int opcode);
char *
moto_symbolForOp(int opcode);

char* 
escapeStrConst(char* s);
char* 
unescapeStrConst(char* s);

#endif

/*=============================================================================
// end of file: $RCSfile: motoutil.h,v $
==============================================================================*/

