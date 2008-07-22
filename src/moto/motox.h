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

   $RCSfile: motox.h,v $   
   $Revision: 1.4 $
   $Author: dhakim $
   $Date: 2003/05/29 02:14:08 $
 
==============================================================================*/
#ifndef __MOTOX_H
#define __MOTOX_H

#include "motofunction.h"
#include "type.h"
#include "cell.h"

/* Recursively extracts a MotoType from and TYPE, ARRAY_TYPE, or DEF cell */
MotoType* motox_extractType(const UnionCell *p);

/**
 * Try to locate a moto function or method with the specified base name, arguments, and cononical 
 * argument types. 
 *
 * First, try finding a method using the type of 'self' as the classname if self is != null.
 *
 * If no method is found and the opcode allows for the possiblity that we are interested in 
 * functions as well than try looking for one of those. In the end return 'code' and set f
 * if we find anything.
 **/
 
int motox_lookupMethodOrFn(
	MotoFunction **f, MotoVal* self,char* fbasename, int argc, char** types, int opcode);

/**
 * Returns true if the callee specified is a dynamically defined function (identified 
 * function or partially applied function or method) 
 * 
 * A callee is considered dynamically defined iff:
 * 1) The opcode of the callee is FN but the function name being called is a variable
 * 2) The opcode of the callee is FN but we are inside a class definition and 
 * 	the function name being called is a class member variable 
 * 3) The opcode of the callee is METHOD but the method name being called is a
 * 	class member variable
 * 4) The opcode of the callee is FCALL
 **/
 
int motox_calleeIsDDF(const UnionCell* p, MotoVal* self);

/* Fill the argv and types arrays with the appropriate values from the args array */
void 
motox_fillArgs(int argc,MotoVal** args, void** argv,char** types);

/** 
 * Try to locate a suitable operator method for the given operator.
 * If it finds it:
 * 	- in verifier mode it creates a MotoVal of the respective type
 * 	- in interpreter mode it executes the function and creates the MotoVal
 **/ 
int 
motox_try_overloaded_method_op(int op,MotoVal* v1, MotoVal* v2, MotoVal *v3);

/** 
 * Try to locate a suitable operator function for the given operator.
 * If it finds it:
 * 	- in verifier mode it creates a MotoVal of the respective type
 * 	- in interpreter mode it executes the function and creates the MotoVal
 **/ 
int 
motox_try_overloaded_function_op(int op,MotoVal* v1, MotoVal* v2);

#endif

/*=============================================================================
// end of file: $RCSfile: motox.h,v $
==============================================================================*/
