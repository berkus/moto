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

   $RCSfile: tnfa.h,v $   
   $Revision: 1.3 $
   $Author: dhakim $
   $Date: 2003/01/07 23:21:18 $
 
==============================================================================*/
#ifndef __TNFA_H
#define __TNFA_H

#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "itoihashtable.h"
#include "hashtable.h"
#include "intset.h"
#include "inthashtable.h"
#include "stringbuffer.h"
#include "vector.h"

typedef struct tfatrans{
   char c;
   int to;
   int tag;
   char priority;
   struct tfatrans* next;
} TFATrans;

/*
 * Adjacency list representation of an TNFA
 */

typedef struct tnfa {
   int tagcount;
   int laststate;
   int start;
   int finish;
   IntHashtable* states;
   IntSet* minimized;
   ItoIHashtable* inputOrder;
   TFATrans** cStates;
   int* cInputOrder;
   int maxpath;
} TNFA;

typedef struct tnfaSubExpression{
	int start;
	int finish;
}	TNFASubExpression;

typedef int TagValueFunction;

void tnfa_free(TNFA* nfa);

TNFA* tnfa_create(char* regex);
TagValueFunction* tnfa_match(TNFA* tnfa, char* input, int anchorStart, int anchorEnd);
char* tnfa_toString(TNFA* tnfa);
#endif

/*=============================================================================
// end of file: $RCSfile: tnfa.h,v $
==============================================================================*/
