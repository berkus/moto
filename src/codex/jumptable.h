/*=============================================================================

   Copyright (C) 2000 Kenneth Kocienda and David Hakim.
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

   $RCSfile: jumptable.h,v $   
   $Revision: 1.5 $
   $Author: dhakim $
   $Date: 2003/01/07 23:25:26 $
 
==============================================================================*/
#ifndef __JUMPTABLE_H
#define __JUMPTABLE_H

#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "inthashtable.h"
#include "intset.h"
#include "hashtable.h"
#include "stringbuffer.h"

typedef struct jumptable {
   size_t* lengths;
   char** argv;
   int argc;
   IntSet* start;
   Hashtable* columns;
} JumpTable;

typedef struct jtrans{
   IntSet* rows;
   int match;
} JTrans;

typedef struct jumpcolumn {
   int column;
   IntHashtable* transitions;
} JumpColumn;

typedef struct mjtrans {
   short jump;
   short match;
} MJTrans;

typedef struct mjtable {
   MJTrans* transitions;
   short* columns;
   short start;
   size_t* lengths;
   char** argv;
   int argc;
   int states;
} MJTable;

JumpTable* jtab_create(int argc, char** argv);
MJTable* mjtab_create(JumpTable* jtab);
int mjtab_strmatch(MJTable* mjt,char* s);
int mjtab_strnmatch(MJTable* mjt,char* si,size_t length);
void jtab_free(JumpTable* jtab);
void mjtab_generateCode(MJTable* mjt,StringBuffer* out,char* prefix);

#endif

/*=============================================================================
// end of file: $RCSfile: jumptable.h,v $
==============================================================================*/

