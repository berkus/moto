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

   $RCSfile: dfa.h,v $   
   $Revision: 1.6 $
   $Author: dhakim $
   $Date: 2003/01/07 23:21:18 $
 
==============================================================================*/
#ifndef __DFA_H
#define __DFA_H

#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "hashtable.h"
#include "intset.h"
#include "inthashtable.h"
#include "stringbuffer.h"
#include "nfa.h"

/*
 * Adjacency list representation of a DFA
 */

typedef struct dfa {
   int start;
   IntSet* accepting;
   IntHashtable* states;
} DFA;

DFA* dfa_create(NFA* nfa); 
void dfa_free(DFA* dfa);

DFA* dfa_createMinimized(DFA* dfa);

int dfa_matches(DFA* dfa, char* string);
int dfa_starts(DFA* dfa, char* string);

#endif

/*=============================================================================
// end of file: $RCSfile: dfa.h,v $
==============================================================================*/

