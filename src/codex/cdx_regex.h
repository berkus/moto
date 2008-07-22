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

   $RCSfile: cdx_regex.h,v $   
   $Revision: 1.2 $
   $Author: dhakim $
   $Date: 2003/02/27 19:46:59 $
 
==============================================================================*/
#ifndef __CDX_REGEX_H
#define __CDX_REGEX_H

#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "tnfa.h"
#include "cdx_function.h"

typedef TNFA Regex;

typedef struct match{
	char* input;
	TNFA* tnfa;
	TagValueFunction* tvf;
} Match;

Regex* regex_create(char* rx);
void regex_free(Regex* rx);

Match* regex_match(Regex* rx, char* input);
Match* regex_search(Regex* rx, char* input);
Match* regex_matchPrefix(Regex* rx,char* input);
Match* regex_matchSuffix(Regex* rx,char* input);

char regex_matches(Regex* rx, char* input);
char regex_matchesPrefix(Regex* rx, char* input);
char regex_matchesSuffix(Regex* rx, char* input);
char regex_matchesSubstring(Regex* rx, char* input);
char* regex_toTNFAString(Regex* rx);

void match_free(Match* m);
char* match_subMatch(Match* m, int i);
char* match_preMatch(Match* m);
char* match_postMatch(Match* m);
int match_startIndex(Match* m,int i);
int match_endIndex(Match* m,int i);
char* match_toString(Match* m);

char* regex_sub(char* string, Regex* pattern, char* substitution);
char* regex_fsub(char* s, Regex* rx, Function* sub);
UnArray* regex_split(char* s, Regex* rx);
char* regex_join(UnArray* a, char* s);

/*
int mxstr_prefixMatches(char* s,Regex* rx);
int mxstr_suffixMatches(char* s,Regex* rx);
int mxstr_contains(char* s, Regex* rx);
int mxstr_matches(char* s, Regex* rx);

*/


#endif
