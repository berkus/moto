/*=============================================================================

   Copyright (C) 2002 David Hakim.
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

   $RCSfile: tokenizer.h,v $   
   $Revision: 1.5 $
   $Author: dhakim $
   $Date: 2002/09/12 23:11:57 $
 
==============================================================================*/
#ifndef __TOKENIZER_H
#define __TOKENIZER_H

#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "kmpstring.h"
#include "cdx_regex.h"

typedef struct tokenizer{
   void* tokenizer;
   char*(*next_fn)(void*,char);
   void (*free_fn)(void*);
   char returnDelimiters;
} Tokenizer;

Tokenizer* tok_createCTok(char* string, char c);
Tokenizer* tok_createKMPTok(char* string, KMPString* token);
Tokenizer* tok_createRXTok(char* string, Regex* rx);
void tok_setReturnDelimiters(Tokenizer* tok,char b);
char* tok_next(Tokenizer* t);
void tok_free(Tokenizer* t);

#endif

/*=============================================================================
// end of file: $RCSfile: tokenizer.h,v $
==============================================================================*/

