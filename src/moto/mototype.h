/*=============================================================================

	Copyright (C) 2003 David Hakim.
	This file is part of the Moto Programming Language.

	Moto Programming Language is free software; you can redistribute it and/or
	modify it under the terms of the GNU Library General Public License as
	published by the Free Software Foundation; either version 2 of the
	License, or (at your option) any later version.

	Moto Programming Language is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the GNU
	Library General Public License for more details.

	You should have received a copy of the GNU Library General Public
	License along with the Codex C Library; see the file COPYING.	If not,
	write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
	Boston, MA 02111-1307, USA.

	$RCSfile: mototype.h,v $	 
	$Revision: 1.4 $
	$Author: dhakim $
	$Date: 2003/03/09 07:15:30 $
 
==============================================================================*/
#ifndef __MOTOTYPE_H
#define __MOTOTYPE_H

#include "symboltable.h"

/* MotoType 'kinds' */
typedef enum { 
   BOOLEAN_TYPE = 1, 
   BYTE_TYPE	 = 2,
   CHAR_TYPE    = 3, 
   INT32_TYPE   = 4, 
   INT64_TYPE   = 5, 
   FLOAT_TYPE   = 6, 
   DOUBLE_TYPE  = 7, 
   VOID_TYPE    = 8,
   REF_TYPE     = 9, 
} TypeKind;


/* A MotoType is a canonical entity stored in the env->types symboltable. Every moto type has
	a name which uniquely identifies it */
	
typedef struct motoType {
   TypeKind kind;
   char *name; 				/* The canonical, human readable type name e.g. "int", "int[]", "int()" */
   struct motoType *atype;	/* The ancestor type used for derived types such as Arrays or HOF types */
   								/* 	If this is an HOF type than the atype is the return type */
   								/* 	If this is an Array type than the atype is the type of Object the array stores */
   int dim;						/* The dimension (used for Array types) */
   int argc;					/* The number of type arguments for this type (used for HOFs) */
   struct motoType **argt;	/* The argument types (used for HOFs) */
   char isExternallyDefined;
} MotoType;

typedef SymbolTable MotoTypeTable;

/* Functions for adding and retrieving moto types from a MotoTypeTable*/

MotoType *mttab_add(MotoTypeTable *table, char *name, char isExternallyDefined);
MotoType *mttab_get(MotoTypeTable *table, char *name, int dim);
MotoType *mttab_getHOFType(MotoTypeTable* table, MotoType* rtype, int argc, MotoType** argt);
MotoType *mttab_typeForName(MotoTypeTable *table, char *typen);


/* Takes the functional type orig and removes pargc arguments from it. The
 * specific arguments to be removed are list in the index array pargi */
MotoType* mttab_apply(MotoTypeTable *table, MotoType* orig,int pargc, int* pargi);

/* Compiler related methods */

/* Returns the type that should be used in generated C code corresponding to
	this moto type */
char* mtype_toCType(MotoType* type);

/* Verifier related functions */

/* Returns true if a variable / value of type stype can be explicitly cast to a
	variable or value of type dtype */	
int mtype_checkCast(MotoType *dtype, MotoType *stype) ;

/* Returns true if a variable / value of type dtype can be assigned a
	variable or value of type stype */
int mtype_checkAssign(MotoType *dtype, MotoType *stype) ;

/* Returns true if a variable / value of to can be passed to a
	function argument of type 'from' */
int moto_checkImplicitCast(MotoType *from, MotoType *to);

#endif

/*=============================================================================
// end of file: $RCSfile: mototype.h,v $
==============================================================================*/
