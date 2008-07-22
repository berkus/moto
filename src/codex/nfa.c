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

   $RCSfile: nfa.c,v $   
   $Revision: 1.13 $
   $Author: dhakim $
   $Date: 2002/12/05 02:40:48 $
 
==============================================================================*/
#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>

#include "memsw.h"

#include "nfa.h"
#include "vector.h"
#include "inthashtable.h"
#include "intset.h"
#include "intstack.h"
#include "stack.h"
#include "hashtable.h"
#include "hashset.h"
#include "excpfn.h"
#include "exception.h"

int last_state;

void 
nfa_init(){
   last_state = 0;
}

int 
nfa_addState(NFA* nfa){
   int state = last_state++;
   ihtab_put(nfa->states,state,NULL);
   return state;
}

void 
nfa_subsumeNFA(NFA* nfa,NFA* lunch){
   IntEnumeration* e;
   int i;
   
   for(e=ihtab_getKeys(lunch->states);ienum_hasNext(e);){
      FATrans* fat;
      i=ienum_next(e);
      
      fat=ihtab_get(lunch->states,i);
      ihtab_put(nfa->states,i,fat);
   }  
   
   ienum_free(e);
   ihtab_free(lunch->states);
   free(lunch);
}

FATrans * 
t_create(char c, int dest){
   FATrans* t = (FATrans*)emalloc(sizeof(FATrans));
   t->c = c;
   t->to = dest;
   
   return t;
}

inline NFA* 
nfa_createDefault(){
   NFA* nfa = (NFA*)emalloc(sizeof(NFA));

   nfa->states = ihtab_createDefault();
   nfa->start = nfa_addState(nfa);
   nfa->finish = nfa_addState(nfa);

   return nfa;
}

void 
nfa_free(NFA* nfa){
   IntEnumeration* e;
   int i;

   for(e=ihtab_getKeys(nfa->states);ienum_hasNext(e);){
      FATrans* fat,*next;
      i=ienum_next(e);
   
      for(fat=ihtab_get(nfa->states,i);fat!=NULL;fat=next){
         next = fat->next;
         free(fat);
      }
   }

   ienum_free(e);
   ihtab_free(nfa->states);

   free(nfa);
}

inline void 
nfa_addTransition(NFA* nfa, int from, FATrans* t){
   FATrans* fat;

   fat = (FATrans*)ihtab_get(nfa->states,from);
   t->next = fat;
   ihtab_put(nfa->states,from,t);
}

NFA* 
charNFA (char c){
   NFA* nfa;

   nfa = nfa_createDefault();
   nfa_addTransition(nfa,nfa->start,t_create(c,nfa->finish));

   return nfa;
} 

NFA* 
anyCharNFA (){
   char c;
   FATrans *trans;
   NFA* nfa;

   nfa = nfa_createDefault();

   for(c=1;c<127;c++){
      trans = t_create(c,nfa->finish);
      nfa_addTransition(nfa,nfa->start,trans);
   }

   return nfa;
}

NFA* 
characterClassNFA (char* regex,int sidx,int eidx){
   int i;
   FATrans *trans;
   NFA* nfa;
   char tarr[128];
   char invert = '\0';
   nfa = nfa_createDefault();

   /* Check if we need to invert the character class */

   if(regex[sidx] == '^'){ invert = '\1'; sidx++; }

   for(i=0;i<128;i++) tarr[i] = invert;

   for(i=sidx;i<eidx;i++){
      if(i+2 < eidx && regex[i+1] == '-'){
         char s=regex[i], f=regex[i+2];
         for(;s<=f;s++) tarr[(int)s] = !invert ;
         i+=2;
      } else if(regex[i]=='\\'){
         switch(regex[i+1]){
            case 'f' : tarr[(int)'\f'] = !invert ; break;
            case 'n' : tarr[(int)'\n'] = !invert ; break;
            case 't' : tarr[(int)'\t'] = !invert ; break;
            case 'r' : tarr[(int)'\r'] = !invert ; break;
            default  : tarr[(int)regex[i+1]] = !invert;
         }
         i++;
      } else { 
         tarr[(int)regex[i]] = !invert ;
      }
   }

   for(i=0;i<128;i++) if (tarr[i]) {
      trans = t_create((char)i,nfa->finish);
      nfa_addTransition(nfa,nfa->start,trans);
      /* printf("Added transition on '%c'\n",(char)i); */
   }

   return nfa;
}

NFA* 
orNFA(NFA* left, NFA* right){
   NFA* nfa;
   
   nfa = nfa_createDefault();

   nfa_addTransition(nfa,nfa->start,t_create('\0',left->start));
   nfa_addTransition(nfa,nfa->start,t_create('\0',right->start));
   nfa_addTransition(left,left->finish,t_create('\0',nfa->finish));
   nfa_addTransition(right,right->finish,t_create('\0',nfa->finish));

   nfa_subsumeNFA(nfa,left);
   nfa_subsumeNFA(nfa,right);
      
   return nfa; 
}

NFA* 
followsNFA(NFA* left,NFA* right){
   FATrans* fat;

   NFA* nfa = nfa_createDefault();
   
   nfa_addTransition(nfa,nfa->start,t_create('\0',left->start));

   fat = ihtab_get(right->states,right->start);
   ihtab_put(left->states,left->finish,fat);
   ihtab_remove(right->states,right->start);

   nfa_addTransition(right,right->finish,t_create('\0',nfa->finish));

   nfa_subsumeNFA(nfa,left);
   nfa_subsumeNFA(nfa,right);

   return nfa;
}

NFA* 
zeroOrMoreNFA(NFA* nfa){
   NFA* result;
   
   result = nfa_createDefault();

   nfa_addTransition(result,result->start,t_create('\0',nfa->start));
   nfa_addTransition(nfa,nfa->finish,t_create('\0',result->finish));
   nfa_addTransition(result,result->start,t_create('\0',result->finish));
   nfa_addTransition(nfa,nfa->finish,t_create('\0',nfa->start));

   nfa_subsumeNFA(result,nfa);

   return result;
}

NFA* 
oneOrMoreNFA(NFA* nfa){
   NFA* result;
  
   result = nfa_createDefault();

   nfa_addTransition(result,result->start,t_create('\0',nfa->start));
   nfa_addTransition(nfa,nfa->finish,t_create('\0',result->finish));
   nfa_addTransition(nfa,nfa->finish,t_create('\0',nfa->start));

   nfa_subsumeNFA(result,nfa);

   return result;
}

NFA* 
zeroOrOneNFA(NFA* nfa){
   NFA* result;
  
   result = nfa_createDefault();

   nfa_addTransition(result,result->start,t_create('\0',nfa->start));
   nfa_addTransition(nfa,nfa->finish,t_create('\0',result->finish));
   nfa_addTransition(result,result->start,t_create('\0',result->finish));

   nfa_subsumeNFA(result,nfa);

   return result;
}

NFA* 
addModifier(char c, NFA* nfa ){
   switch(c) {
      case '*' : nfa = zeroOrMoreNFA(nfa); break;
      case '+' : nfa = oneOrMoreNFA(nfa);  break;
      case '?' : nfa = zeroOrOneNFA(nfa);  break;
   }
   return nfa;
}

NFA* 
parse (char* regex, int start, int end){
   NFA* nfa = NULL;
   int i = start;

   while (i< end){

      NFA* component_nfa = NULL;

      switch(regex[i]) {
         case '\\' : {
            /* Character literal follows */

            if(i+1 >= end){ THROW("RegexParseException","Escape '\\' not followed"); } 

            switch(regex[i+1]){
               case 'f' : component_nfa = charNFA('\f') ; break;
               case 'n' : component_nfa = charNFA('\n') ; break;
               case 't' : component_nfa = charNFA('\t') ; break;
               case 'r' : component_nfa = charNFA('\r') ; break;
               default  : component_nfa = charNFA(regex[i+1]);
            }

            i+=2;   
         } break;
         case '(' : {
            int j=i+1,opens = 0;
   
            while(regex[j] != ')' || opens != 0){
               if(regex[j] == '\\') j++;
               else if(regex[j] == '(') opens ++;
               else if(regex[j] == ')') opens --;
               j++;

               if (j>=end){ THROW("RegexParseException","Unmatched '('"); }
                  
            }
            component_nfa = parse(regex,i+1,j);
           
            i=j+1;

         } break;

         case '.' : {
            component_nfa = anyCharNFA();
            i++;
         } break;

         case '[' : {

            int j=i+1,opens = 0;

            while(regex[j] != ']' || opens != 0){
               if(regex[j] == '\\') j++;
               else if(regex[j] == '[') opens ++;
               else if(regex[j] == ']') opens --;
               j++;

               if (j>=end){ THROW("RegexParseException","Unmatched '['"); }
            }

            component_nfa = characterClassNFA(regex,i+1,j);
           
            i=j+1;

         } break;

         case '|' : {

            component_nfa = parse(regex,i+1,end);

            if (nfa == NULL)
               nfa = component_nfa;
            else if (component_nfa != NULL)
               nfa = orNFA(nfa,component_nfa);

            return nfa;
         }

         default : {
            component_nfa = charNFA(regex[i]); 
            i++;
         }
        
      } 

      if (regex[i] == '*' || regex[i] == '+' || regex[i] == '?'){
         component_nfa = addModifier(regex[i], component_nfa);
         i++;
      }

      if (nfa == NULL)
         nfa = component_nfa;
      else if (component_nfa != NULL)
         nfa = followsNFA(nfa,component_nfa);
      
   }

   return nfa;
}

/*=============================================================================
// end of file: $RCSfile: nfa.c,v $
==============================================================================*/

