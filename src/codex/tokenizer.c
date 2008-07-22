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

   $RCSfile: tokenizer.c,v $   
   $Revision: 1.12 $
   $Author: dhakim $
   $Date: 2002/09/12 23:11:57 $
 
==============================================================================*/
#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include "memsw.h"
#include "exception.h"
#include "excpfn.h"
#include "tokenizer.h"
#include "cdx_regex.h"

typedef struct kmptokenizer{
   char* string;
   KMPString* kmp;
} KMPTokenizer;

typedef struct ctokenizer{
   char* string;
   char c;
} CTokenizer;

typedef struct rxtokenizer{
   char* string;
   Regex* rx;
} RXTokenizer;

KMPTokenizer* kmptok_create(char* string, KMPString* kmp){
   KMPTokenizer* t;

   t= emalloc(sizeof(KMPTokenizer));
   t->string = string;
   t->kmp = kmp;
 
   return t;
}

void kmptok_free(KMPTokenizer* tok){
   free(tok);
}

RXTokenizer* rxtok_create(char* string, Regex* rx){
   RXTokenizer* t;

   t= emalloc(sizeof(RXTokenizer));
   t->string = string;
   t->rx = rx;
 
   return t;
}

void rxtok_free(RXTokenizer* tok){
   free(tok);
}

/*
FTokenizer* ftok_create(char* string,int tlength, (char*)(*)(char*) f){
   FTokenizer* t;

   t= emalloc(sizeof(FTokenizer));
   t->string = string;
   t->tlength = tlength;
   t->f = f;

   return t;
}
*/

CTokenizer* ctok_create(char* string, char c){
   CTokenizer* t;

   t= emalloc(sizeof(CTokenizer));
   t->string = string;
   t->c = c;

   return t;
}

char* ctok_next(CTokenizer* t,char returnDelimiters){
   char* last;
   char* result;
   int rlength;

   if (!t->string)
      return 0;

   if (!(*t->string)){
      t->string = 0;
      return 0;
   }

   last = t->string;

   t->string = strchr(t->string,t->c);

   /* If the pattern was not found then the rest of our string is our token 
   	and we are done tokenizing */
   
   if (!t->string) {
      return estrdup(last);
   }
   	
	/* if we are returning delimiters and t->string didn't move then set the result to c
		and move forward 1 character */
		
	if(last == t->string && returnDelimiters) {
		t->string ++;
		result = emalloc(2);
		result[0]=t->c;
		result[1]='\0';
		return result;
	}
	
	/* If the pattern was found at the beginning of the string and we are not
   	returning tokens then we better keep eating matched patterns */
	while (last == t->string){
      last = t->string += 1;
   	/* If the remainder of the string was all patterns then we were done matching */
   
   	if(!*t->string)
   		return NULL;
      
      t->string = strchr(t->string,t->c);
   	
   	/* If the pattern was not found then the rest of our string is our token 
   		and we are done tokenizing */
   	
   	if (!t->string) {
			return estrdup(last);
  	 	}
   }
   
   result = (char*)emalloc((rlength = t->string - last)+1);
   result[rlength] = '\0';
   strncpy(result,last,rlength);

   return result;
}

char* kmptok_next(KMPTokenizer* t,char returnDelimiters){
   char* last;
   char* result;
   int rlength;

	/* If we've already reached the end of the string we better return NULL */
   if (!t->string)
      return 0;

   if (!(*t->string)){
      t->string = 0;
      return 0;
   }

	/* Set the last ptr to the current position of t->string */
   last = t->string;
   
   /* Attempt to find the pattern within t->string */
   t->string = kmp_strstr(t->kmp,t->string);

	/* If the pattern was not found then the rest of our string is our token 
   	and we are done tokenizing */
   	
   if (!t->string) {
      return estrdup(last);
   }
   
   /* If the pattern was found at the beginning of the string and we are
   	returning tokens then return the match */
   	
   if(last == t->string && returnDelimiters){
      t->string += t->kmp->length;
   	result = (char*)emalloc(t->kmp->length+1);
   	result[t->kmp->length] = '\0';
   	strncpy(result,last,t->kmp->length);
   	return result;
   }
   
   /* If the pattern was found at the beginning of the string and we are not
   	returning tokens then we better keep eating matched patterns */
   		
   while (last == t->string) {
      last = t->string += t->kmp->length;
      
      /* If the remainder of the string was all patterns then we were done matching */
   	if(!*t->string)
   		return NULL;
   		
      t->string = kmp_strstr(t->kmp,t->string);
      
      /* If the pattern was not found then the rest of our string is our token 
   		and we are done tokenizing */
 
      if(!t->string){
         return estrdup(last);
      }
   }

   result = (char*)emalloc((rlength = t->string - last)+1);
   result[rlength] = '\0';
   strncpy(result,last,rlength);

   return result;
}

char* rxtok_next(RXTokenizer* t,char returnDelimiters){
   char* last;
   char* result;
	Match* match;

	/* If we've already reached the end of the string we better return NULL */
   if (!(*t->string)){
      t->string = 0;
      return 0;
   }

	/* Set the last ptr to the current position of t->string */
   last = t->string;
   
   /* Attempt to find the pattern within t->string */
   match = regex_search(t->rx,t->string);
   
   /* If the pattern was not found then the rest of our string is our token 
   	and we are done tokenizing */
   		
   if(match_startIndex(match,0) == -1) {
   	result = estrdup(last);
   	t->string += strlen(t->string);
   	match_free(match);
   	return result;
   }
   
   /* If the pattern was found at the beginning of the string and we are
   	returning tokens then return the match */
   	
   if(match_startIndex(match,0) == 0 && returnDelimiters) {
   	result = match_subMatch(match,0);
   	t->string += match_endIndex(match,0);
   	match_free(match);
   	return result;
   }
   
   /* If the pattern was found at the beginning of the string and we are not
   	returning tokens then we better keep eating matched patterns */
   	
   while(match_startIndex(match,0) == 0){
   	last = t->string += match_endIndex(match,0);
   	match_free(match);
   	
   	/* If the remainder of the string was all patterns then we were done matching */
   	if(!*t->string)
   		return NULL;
   		
   	match = regex_search(t->rx,t->string);
   	
   	/* If the pattern was not found then the rest of our string is our token 
   		and we are done tokenizing */
   	
   	if(match_startIndex(match,0) == -1) {
   	   result = estrdup(last);
   		t->string += strlen(t->string);
   		match_free(match);
   		return result;		
   	}
   }
   

	t->string += match_startIndex(match,0);
		
	/* Set the result to the value of the pre-match */
   result = match_preMatch(match);

	match_free(match);
	
   return result;
}


Tokenizer* tok_createCTok(char* string, char c){
   CTokenizer* ctok;
   Tokenizer* tok;

   ctok = ctok_create(string,c);

   tok = emalloc(sizeof(Tokenizer));
   tok->tokenizer = ctok;
   tok->next_fn = (char*(*)(void*,char)) ctok_next;
   tok->free_fn = (void (*)(void*)) shared_free;
	tok->returnDelimiters = 0;
   return tok;
}

Tokenizer* tok_createKMPTok(char* string, KMPString* token){
   KMPTokenizer* kmptok;
   Tokenizer* tok;

   kmptok = kmptok_create(string,token);

   tok = emalloc(sizeof(Tokenizer));
   tok->tokenizer = kmptok;
   tok->next_fn = (char*(*)(void*,char)) kmptok_next;
   tok->free_fn = (void (*)(void*)) kmptok_free;
	tok->returnDelimiters = 0;
   return tok;
}

Tokenizer* tok_createRXTok(char* string, Regex* token){
   RXTokenizer* rxtok;
   Tokenizer* tok;

   rxtok = rxtok_create(string,token);

   tok = emalloc(sizeof(Tokenizer));
   tok->tokenizer = rxtok;
   tok->next_fn = (char*(*)(void*,char)) rxtok_next;
   tok->free_fn = (void (*)(void*)) rxtok_free;
	tok->returnDelimiters = 0;
   return tok;
}

void tok_setReturnDelimiters(Tokenizer* tok,char b){
	tok->returnDelimiters = b;
}

char* tok_next(Tokenizer* t){
   return (*t->next_fn)(t->tokenizer,t->returnDelimiters);
}

void tok_free(Tokenizer* t){
   if(t->free_fn != NULL)
      (*t->free_fn)(t->tokenizer);
   free(t);
}

/*=============================================================================
// end of file: $RCSfile: tokenizer.c,v $
==============================================================================*/

