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

   $RCSfile: mc.c,v $   
   $Revision: 1.10 $
   $Author: dhakim $
   $Date: 2003/02/27 19:46:59 $
 
==============================================================================*/

#include <string.h>

#include "excpfn.h"
#include "memsw.h"

#include "mc.h"
#include "env.h"
#include "motohook.h"

/* Creates a new Moto Class Definition */
MotoClassDefinition* 
mcd_create(char* classn, const UnionCell* p) {
   MotoClassDefinition* mcd = (MotoClassDefinition*)malloc(sizeof(MotoClassDefinition));
   mcd->classn = classn;
   mcd->memberTypes = stab_createDefault();
   mcd->memberOffsets = stab_createDefault();
   mcd->memberVarNames = vec_createDefault();
   mcd->definition = p;
   mcd->size=0;
   return mcd;
}

/* Frees a Moto Class Definition */
void
mcd_free(MotoClassDefinition* mcd){
   stab_free(mcd->memberTypes);
   stab_free(mcd->memberOffsets);
   vec_free(mcd->memberVarNames);
   free(mcd);
}

/* Adds a new member variable and assocites it with the specified type */
void 
mcd_addMember(MotoClassDefinition* mcd,char* name, char* typen){
   vec_add(mcd->memberVarNames,name);
   stab_put(mcd->memberTypes,name,typen);
}

/* Computes relative offsets from start of a byte array needed for storing
	member data aligned to type size and in order of declaration */
	
void
mcd_computeOffsetsAndSize(MotoClassDefinition* mcd){
   int curOffset = 0;
   
   Enumeration* e = vec_elements(mcd->memberVarNames);
   while(enum_hasNext(e)) {
      char* varn = enum_next(e);
      char* typen = stab_get(mcd->memberTypes,varn);
      int curVarSize = 0;
      
      if(typen[strlen(typen)-1] == ')' || typen[strlen(typen)-1] == ']') /* FIXME: please hide this in a function */
         curVarSize = sizeof(void*);
      else if(!estrcmp("int",typen))
      	curVarSize = sizeof(int32_t);
		else if(!estrcmp("long",typen))
			curVarSize = sizeof(int64_t);
		else if(!estrcmp("double",typen))
			curVarSize = sizeof(double);
		else if(!estrcmp("float",typen))
			curVarSize = sizeof(float);
		else if(!estrcmp("boolean",typen))
			curVarSize = sizeof(char);
		else if(!estrcmp("char",typen))
			curVarSize = sizeof(char);
		else if(!estrcmp("byte",typen))
			curVarSize = sizeof(signed char);
		else
			curVarSize = sizeof(void*);
			
		if(curOffset % curVarSize != 0)
		   curOffset += curVarSize - curOffset % curVarSize;
		
		stab_put(mcd->memberOffsets,varn,(void*)curOffset);
		curOffset += curVarSize;
   }
   enum_free(e);
   
   mcd->size=curOffset;
}


/* Returns a String representation of a member variableÕs type */
char* 
mcd_getMemberType(MotoClassDefinition* mcd, char* name){
   return (char*)stab_get(mcd->memberTypes,name);
}

/* Returns a member variableÕs offset */
int 
mcd_getMemberOffset(MotoClassDefinition* mcd, char* name){
   return (int)stab_get(mcd->memberOffsets,name);
}

/* Creates a new Moto Class Instance from a Moto Class Definition */
MotoClassInstance*
mci_create(MotoClassDefinition* mcd){

   /* Allocate the storage for the packed structure */
   MotoClassInstance* mci = (MotoClassInstance*)ecalloc(mcd->size,1);

   return mci;
}

/*=============================================================================
// end of file: $RCSfile: mc.c,v $
==============================================================================*/

