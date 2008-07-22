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

   $RCSfile: nfa.h,v $   
   $Revision: 1.3 $
   $Author: kocienda $
   $Date: 2000/04/30 23:10:01 $
 
==============================================================================*/
#ifndef __NFA_H
#define __NFA_H

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

typedef struct fatrans{
   char c;
   int to;
   struct fatrans* next;
} FATrans;

/*
 * Adjacency list representation of an NFA
 */

typedef struct nfa {
   int start;
   int finish;
   IntHashtable* states;
} NFA;

void nfa_init();

void nfa_free(NFA* nfa);

NFA* parse (char* regex, int start, int end);

#endif

/*=============================================================================
// end of file: $RCSfile: nfa.h,v $
==============================================================================*/

