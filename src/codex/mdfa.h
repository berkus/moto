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

   $RCSfile: mdfa.h,v $   
   $Revision: 1.4 $
   $Author: dhakim $
   $Date: 2002/02/09 03:27:13 $
 
==============================================================================*/
#ifndef __MDFA_H
#define __MDFA_H

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
#include "dfa.h"

/*
 * Matrix representation of a DFA
 */

typedef struct mdfa {
   short numstates;
   short start;
   char* accepting;
   short* smatrix;
} MDFA;

MDFA* mdfa_create(DFA* dfa);
MDFA* mdfa_createFromRX(char *rx);

void mdfa_free(MDFA* mdfa);
void mdfa_generateCode(MDFA* mdfa, StringBuffer* out,char* prefix);
int mdfa_matches(MDFA* mdfa, char* string);
int mdfa_starts(MDFA* mdfa,char* string);

#endif

/*=============================================================================
// end of file: $RCSfile: mdfa.h,v $
==============================================================================*/

