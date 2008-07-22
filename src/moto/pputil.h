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

   $RCSfile: pputil.h,v $   
   $Revision: 1.3 $
   $Author: kocienda $
   $Date: 2000/04/30 23:10:03 $
 
==============================================================================*/
#ifndef __PPUTIL_H
#define __PPUTIL_H

#include "ppcell.h"

inline char *ppuc_str(const PPUnionCell *p, int index);
inline int ppuc_opcode(const PPUnionCell *p);
inline int ppuc_opcount(const PPUnionCell *p);
inline PPUnionCell *ppuc_operand(const PPUnionCell *p, int index);

inline MotoPPVal *ppopstack_pop();
inline MotoPPVal *ppopstack_peek();
inline MotoPPVal *ppopstack_peekAt(int index);
inline void ppopstack_push(MotoPPVal *val);
inline int ppopstack_size();

inline char *motopp_strdup(MotoPP *ppenv, const char *s);
int motopp_isInIncludeChain(MotoPP *ppenv, char *name);
char *motopp_uncomment(const char *s);

#endif

/*=============================================================================
// end of file: $RCSfile: pputil.h,v $
==============================================================================*/


