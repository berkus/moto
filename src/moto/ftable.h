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

   $RCSfile: ftable.h,v $   
   $Revision: 1.11 $
   $Author: dhakim $
   $Date: 2003/03/05 21:49:22 $
 
==============================================================================*/
#ifndef __FTABLE_H
#define __FTABLE_H

#include "symboltable.h"

#include "motofunction.h"

enum {
   FTAB_OK = 0,
   FTAB_NONE = 1,
   FTAB_MULTIPLE = 2,
};

/*-----------------------------------------------------------------------------
        typedefs
-----------------------------------------------------------------------------*/

/* the ftable struct */
typedef struct ftable {
	SymbolTable *t;
} FTable;

/*-----------------------------------------------------------------------------
        allocation functions
-----------------------------------------------------------------------------*/

FTable *ftab_create();
void ftab_free(FTable *table);

/*-----------------------------------------------------------------------------
        data structure functions
 -----------------------------------------------------------------------------*/

void ftab_add(FTable *table, char *n, MotoFunction *f);
int ftab_getExactMatch(FTable *table, MotoFunction **f, char *n, int argc, char **types);
int ftab_get(FTable *table, MotoFunction **f, char *n, int argc, char **types);
int ftab_cacheGet(FTable *table,MotoFunction **f,char *name,int argc, char **types); 
int ftab_fnForName(FTable *table, MotoFunction **f,char *name);
#endif

/*=============================================================================
// end of file: $RCSfile: ftable.h,v $
==============================================================================*/

