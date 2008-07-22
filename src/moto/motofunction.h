/*=============================================================================

   Copyright (C) 2002 David Hakim.
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

   $RCSfile: motofunction.h,v $   
   $Revision: 1.7 $
   $Author: dhakim $
   $Date: 2003/03/09 07:15:30 $
 
==============================================================================*/
#ifndef __MOTOFUNCTION_H
#define __MOTOFUNCTION_H

#include "cell.h"
#include "mototype.h"

typedef struct motoFunction {
	char *name;					/* The base name of this function or method */
   char *motoname;
   
   char *classn;				/* The class this method belongs to or NULL if this is a function */
   char *rtype;         	/* The return type (canonical string) */
   char *deftype;				/* Argh ... this is awful FIXME */ 

   int argc;
   char **argtypes;     	/* Contains an array of the types (canonical strings) for the arguments */
   char **argnames;  		/* Contains an array of the arguments names. Only for MDFs */
		 
   char *csymbol;				/* The symbol name for this function used when generating C code 
   									For MDFs this field is filled in lazily during code 
   									generation and should only be accessed through mfn_csymbol */
   									
   char tracked;				/* 0 if the symbols rvalue is not tracked 1 if it is */

   const UnionCell* opcell;	/* Pointer to the DEFINE UnionCell for an MDF. NULL for EDFs */
   char *libname;
   void *handle;
   void (*fptr)();			/* Function pointer to the dynamically loaded symbol NULL for MDFs */
} MotoFunction;

/* Creates a MotoFunction from an extension symbol */
MotoFunction* mfn_createFromSymbol(char* sym, char* libname, void* handle);

/* Creates a MotoFunction */
MotoFunction* mfn_create(
   char* motoname,
   
   char* rtype,
   int argc,
   char** argtypes,
   char** argnames,
   
   char* csymbol,
   char tracked,
   
   const UnionCell* opcell,
   
   char* libname,
   void* handle,
   void (*fptr)()
);

/* Creates a MotoFunction from the symbol descriptor in extensions */
MotoFunction* mfn_createFromSymbol(char* sym, char* libname, void* handle);

/* Delete a MotoFunction */
void mfn_free(MotoFunction* mfn);

/* Get the functional type for this function */
MotoType* mfn_type(MotoFunction* mfn);

/* Return the C Symbol for this function */
char* mfn_csymbol(MotoFunction* mfn);

/* Return the C Prototype for this function */
char* mfn_cprototype(MotoFunction* f);

/* Returns true if this MotoFunction is a method. Returns false otherwise */
int mfn_isMethod(MotoFunction* mfn);

/* Returns true if this MotoFunction is a constructor. Returns false otherwise */
int mfn_isConstructor(MotoFunction* mfn);

#endif

/*=============================================================================
// end of file: $RCSfile: motofunction.h,v $
==============================================================================*/

