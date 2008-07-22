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

   $RCSfile: mc.h,v $   
   $Revision: 1.7 $
   $Author: dhakim $
   $Date: 2003/02/27 19:46:59 $
 
==============================================================================*/
#ifndef __MC_H
#define __MC_H

#include "symboltable.h"
#include "vector.h"

#include "cell.h"
#include "type.h"

/* Structure for storing Moto Class Definitions */

typedef struct mcd {
   char* classn; 					/* The class name */
   Vector* memberVarNames;			/* Member variable names, in order of declaration */
   SymbolTable* memberTypes; 		/* Member Variable name -> type mapping */
   SymbolTable* memberOffsets;		/* Member Variable name -> offset mapping */
   int size;						/* Computed size needed to store structure in bytes */   
   const UnionCell* definition; 	/* OpCell associated with the class definition,
                        			   this gets interpreted as part of implicit construction */
   char* code;						/* Code generated for the implicit constructor by
   									   motoc_class */
} MotoClassDefinition;

/* Structure for storing Moto Class Instances (interpreter only) */

typedef unsigned char MotoClassInstance;

/* Creates a new Moto Class Definition */
MotoClassDefinition* mcd_create(char* classn, const UnionCell* p) ;

/* Frees a moto class definition */
void mcd_free(MotoClassDefinition* mcd);

/* Adds a new member variable and assocites it with the specified type */
void mcd_addMember(MotoClassDefinition* mcd,char* name, char* type);

/* Computes relative offsets from start of a byte array needed for storing
	member data aligned to type size and in order of declaration */
void mcd_computeOffsetsAndSize(MotoClassDefinition* mcd);

/* Returns a String representation of a member variable’s type */
char* mcd_getMemberType(MotoClassDefinition* mcd, char* name);

/* Returns a member variable’s offset */
int mcd_getMemberOffset(MotoClassDefinition* mcd, char* name);

/* Creates a new Moto Class Instance from a Moto Class Definition */
MotoClassInstance* mci_create(MotoClassDefinition* mcd) ;

#endif

/*=============================================================================
// end of file: $RCSfile: mc.h,v $
==============================================================================*/

