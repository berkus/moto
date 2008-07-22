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

   $RCSfile: dfa.c,v $   
   $Revision: 1.14 $
   $Author: dhakim $
   $Date: 2003/05/29 02:14:08 $
 
==============================================================================*/
#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memsw.h"

#include "dfa.h"
#include "vector.h"
#include "inthashtable.h"
#include "intset.h"
#include "intstack.h"
#include "stack.h"
#include "hashtable.h"
#include "hashset.h"
#include "excpfn.h"

/**
 * NFA -> DFA construction
 **/

extern FATrans * t_create(char c, int dest);

typedef struct sdfatrans{
   char c;
   IntSet* to;
   struct sdfatrans* next;
} SDFATrans;

typedef struct sdfa {
   IntSet* start;
   HashSet* accepting;
   Hashtable* states;
} SDFA;

static IntSet* move(IntHashtable* states,IntSet* T,char a){
   IntEnumeration *e;
   IntSet* mt;

   mt = iset_createDefault();

   e=iset_elements(T);
   while(ienum_hasNext(e)){
      int t = ienum_next(e);
      FATrans* fat = (FATrans*)ihtab_get(states,t);

      while(fat) {
         if( fat->c == a) 
            iset_add(mt,fat->to);
         fat = fat->next;
      } 
   }
   ienum_free(e);

   return mt;
}

IntSet* eClosure(IntHashtable* states, IntSet* T){
   IntSet* ect;
   IntStack* istk;
   IntEnumeration* e;

   // Push all states in T onto the stack istk

   istk = istack_createDefault();
   e = iset_elements(T);

   while(ienum_hasNext(e)){
      istack_push(istk,ienum_next(e));
   }
   ienum_free(e);

   // Initialize e-closure(T) to T

   ect = iset_createDefault();
   iset_addSet(ect,T);

   // While the stack is not empty

   while(istack_size(istk) != 0) {

      int t = istack_pop(istk);
      FATrans* fat = (FATrans*)ihtab_get(states,t);

      while(fat) {
         if( fat->c == '\0' && !iset_contains(ect,fat->to)) {
            iset_add(ect,fat->to);
            istack_push(istk,fat->to);   
         }
         fat = fat->next;
      }
   }

   istack_free(istk);

   return ect;
}  

void printiset(IntSet* U){
   IntEnumeration* idenum;

   printf("{");
   for(idenum = iset_elements(U);ienum_hasNext(idenum);)
      printf(" %d ",ienum_next(idenum));
   printf("} hash: %u\n",iset_hash((void*)U));
}

SDFA* sdfa_createDefault(){
   SDFA* sdfa;

   sdfa = (SDFA*)emalloc(sizeof(SDFA));

   sdfa->states = htab_create(
      10,
      (unsigned int(*)(void*))iset_hash,
      (int(*)(const void*, const void*))iset_comp,
      0
   );

   sdfa->accepting = hset_create(
      10,
      (unsigned int(*)(void*))iset_hash,
      (int(*)(const void*,const void*))iset_comp,
      0
   );

   sdfa->start = iset_createDefault();

   return sdfa;
}

void sdfa_free(SDFA* sdfa){
   Enumeration* e;
   IntSet* iset;

   // free the state transitions;

   for(e=htab_getKeys(sdfa->states);enum_hasNext(e);){
      SDFATrans* sdfat,*next;
      iset=(IntSet*)enum_next(e);

      for(sdfat=htab_get(sdfa->states,iset);sdfat!=NULL;sdfat=next){
         next = sdfat->next;
         free(sdfat);
      }
   }

   enum_free(e);

   // free the states themselves

   for(e=htab_getKeys(sdfa->states);enum_hasNext(e);){
      iset=(IntSet*)enum_next(e);
      iset_free(iset);
   }

   enum_free(e);

   // free the state table

   htab_free(sdfa->states);

   // free the accepting set

   hset_free(sdfa->accepting);

   free(sdfa);
}

SDFA* sdfa_create(NFA* nfa) {
   SDFA* sdfa;
   Enumeration* e; 
   Stack* umstates;
   IntSet* ecss; 
   HashSet* hset; // Canonical IntSet set

   hset = hset_create(
      10,
      (unsigned int(*)(void*))iset_hash,
      (int(*)(const void*,const void*))iset_comp,
      0
   );

   sdfa = sdfa_createDefault();
   umstates = stack_createDefault();

   // initially eClosure(nfa->start) is the only DFA state and it is unmarked

   iset_add(sdfa->start,nfa->start);
   ecss = eClosure(nfa->states,sdfa->start);
   iset_free(sdfa->start);
   sdfa->start = ecss;

   // printf("### Adding initial dfa state ");
   // printiset(ecss);

   htab_put(sdfa->states,ecss,NULL);
   hset_add(hset,ecss);
   stack_push(umstates,ecss);

   // while there is an unmarked DFA state

   while( stack_size(umstates) != 0) {
      IntSet* T;
      char a;

      // choose an unmarked DFA state and mark it

      T = (IntSet*)stack_pop(umstates);

      // for each possible input symbol

      for(a=1;a<127;a++){
         IntSet* U, *mset;
         SDFATrans* trans;

         mset = move(nfa->states,T,a);
         U  = eClosure(nfa->states,mset);
         
         iset_free(mset);

         if (iset_size(U) == 0) {
            iset_free(U);
            continue;
         }

         if (!htab_contains(sdfa->states,U)) {

            // printf("### Adding new dfa state ");
            // printiset(U);

            htab_put(sdfa->states,U,NULL);
 
            // printf("\n### Adding new set : ");
            // printiset(U);
            hset_add(hset,U);
            stack_push(umstates,U);
         } else {
            IntSet* tU = U;
            U = hset_get(hset,tU);
            iset_free(tU);
         }
  
         trans = (SDFATrans*)emalloc(sizeof(SDFATrans));
         trans->c = a;
         trans->to = U;
         trans->next = htab_get(sdfa->states,T);
         
         // printf("### Adding transition on character '%c' to dfa state ",a);
         // printiset(T);

         htab_put(sdfa->states,T,trans);
      }
   }

   stack_free(umstates);
   hset_free(hset);

   // Fill in accepting states

   for(e=htab_getKeys(sdfa->states);enum_hasNext(e);){
      IntSet* state;

      state = (IntSet*)enum_next(e);
      if(iset_contains(state,nfa->finish)) {
         hset_add(sdfa->accepting,state);
      }
   }
   enum_free(e);

   return sdfa;
}

DFA* dfa_createDefault(){
   DFA* dfa;

   dfa = (DFA*)emalloc(sizeof(DFA));
   dfa->states = ihtab_createDefault();
   dfa->accepting = iset_createDefault();
   dfa->start = -1;

   return dfa;
}

void dfa_free(DFA* dfa){
   IntEnumeration* e;
   int i;

   for(e=ihtab_getKeys(dfa->states);ienum_hasNext(e);){
      FATrans* fat,*next;
      i=ienum_next(e);

      for(fat=ihtab_get(dfa->states,i);fat!=NULL;fat=next){
         next = fat->next;
         free(fat);
      }
   }

   ienum_free(e);
   ihtab_free(dfa->states);
   iset_free(dfa->accepting);
   free(dfa);
}

DFA* dfa_createFromSubsetDFA(SDFA* sdfa) {
   Hashtable* mapping;
   Enumeration* e;
   DFA* dfa;
   int count;

   dfa = dfa_createDefault();

   mapping = htab_create(
      htab_size(sdfa->states),
      (unsigned int(*)(void*))iset_hash,
      (int(*)(const void*, const void*))iset_comp,
      0
   );

   count = 0;

   // build mapping, migrate states, and set accepting states

   for (e=htab_getKeys(sdfa->states);enum_hasNext(e);){
      int* i;
      IntSet* iset;
 
      i = (int*)emalloc(sizeof(int));
      *i = count++ ;

      iset = enum_next(e);

      htab_put(mapping,iset,i);
      ihtab_put(dfa->states,*i,NULL); 

      if(hset_contains(sdfa->accepting,iset))
         iset_add(dfa->accepting,*i);
   }
   enum_free(e);
  
   // set the start state

   dfa->start = *((int*)htab_get(mapping,sdfa->start));

   // migrate transitions

   for (e=htab_getKeys(sdfa->states);enum_hasNext(e);){
      SDFATrans* dfat;
      IntSet* iset;
      int* i;

      iset = (IntSet*)enum_next(e);
      i = htab_get(mapping,iset);
      dfat = htab_get(sdfa->states,iset);

      while(dfat != NULL) {
         int* j;
         FATrans* fat;

         j = (int*)htab_get(mapping,dfat->to);

         fat = (FATrans*)emalloc(sizeof(FATrans));
         fat->c = dfat->c;
         fat->to = *j;
         fat->next = ihtab_get(dfa->states,*i);

         ihtab_put(dfa->states,*i,fat);

         dfat = dfat->next;
      }
   }
   enum_free(e); 

   for(e=htab_getKeys(mapping);enum_hasNext(e);){
      int* i = htab_get(mapping,enum_next(e));
      free(i);
   }
   enum_free(e);
   htab_free(mapping);
   return dfa;
}

DFA* dfa_create(NFA* nfa){

   SDFA* sdfa;
   DFA* dfa;

   sdfa = sdfa_create(nfa);
   dfa = dfa_createFromSubsetDFA(sdfa); 

   sdfa_free(sdfa);

   return dfa;
}

int dfa_matches(DFA* dfa, char* string){
   return dfa_starts( dfa, string) == strlen(string);
}

/* 
 * Returns -1 if the prefix of string is not matched by the dfa 
 * Otherwise returns the length of the match
 */

int dfa_starts(DFA* dfa, char* string){
   int i=0;
   int s=dfa->start;
   int matchlength = -1;

   /* If the start state is an accepting state than we've already 
      made a match */

   if(iset_contains(dfa->accepting,s))
      matchlength = 0;

   while(i<strlen(string)){
      FATrans* fat = (FATrans*)ihtab_get(dfa->states,s);
      while(fat){
         if(fat->c == string[i])
            break;
         fat=fat->next;
      }
      if (fat==NULL)
         return matchlength;
      else
         s=fat->to;

      if(iset_contains(dfa->accepting,s))
         matchlength = i+1;

      i++;
   }

   return matchlength;
}

void printPartition(HashSet* partition)
{
   Enumeration* groupe;
   IntSet* members;
   IntEnumeration* membere;

   printf("\n\n### Partition \n");

   for(groupe=hset_elements(partition);enum_hasNext(groupe);){
      members = (IntSet*)enum_next(groupe);
      printf("\n{");
      for(membere =iset_elements(members);ienum_hasNext(membere);)
         printf(" %d ",ienum_next(membere));

      printf("} %u",(unsigned int)members);
   }
   printf("\n");
}

DFA* dfa_createMinimized(DFA* dfa) {
   HashSet* partition;
   IntHashtable* partmap;
   DFA* newdfa;
   IntSet* F, *SMF;
   IntEnumeration* e;

   // Step 1: Construct the initial partition into accepting states F and
   // non-accepting states SMF

   partition = hset_createDefault();
   partmap = ihtab_createDefault();

   F= iset_createDefault();
   SMF = iset_createDefault();

   for(e=ihtab_getKeys(dfa->states);ienum_hasNext(e);){
      int state = ienum_next(e);
      if(iset_contains(dfa->accepting,state)){
         iset_add(F,state);
         ihtab_put(partmap,state,F);
      } else {
         iset_add(SMF,state);
         ihtab_put(partmap,state,SMF);
      }
   }
   ienum_free(e);

   hset_add(partition,F);
   if(iset_size(SMF) > 0)
      hset_add(partition,SMF);
   else
      iset_free(SMF);

   // printPartition(partition);

   while(1){
      Enumeration* groupe;
      HashSet* newpartition;
      char c;

      newpartition = hset_createDefault();

      for(groupe=hset_elements(partition);enum_hasNext(groupe);){
         IntSet* group = enum_next(groupe);
         hset_add(newpartition,group);
      }
      enum_free(groupe);

      for(c=1;c<127;c++){
         IntEnumeration* allse;
         IntHashtable* newpartmap;

         newpartmap = ihtab_createDefault();

         for(groupe=hset_elements(newpartition);enum_hasNext(groupe);){
            IntEnumeration* membere;
            
            IntSet* group = enum_next(groupe);
            Hashtable* whereto;
            FATrans* fat;
            int member;

            whereto = htab_createDefault(); 

            // maps the group that a given node has a transition on c to, to
            // the group that node should be moved to

            membere = iset_elements(group);

            member=ienum_next(membere);

            for(fat=(FATrans*)ihtab_get(dfa->states,member);fat!=NULL;fat=fat->next)
               if(fat->c == c)
                  break;

            if (fat==NULL) 
               htab_put(whereto,NULL,group);
            else
               htab_put(whereto,ihtab_get(partmap,fat->to),group);
            ihtab_put(newpartmap,member,group);

            while(ienum_hasNext(membere)) {
               IntSet* newgroup;

               member=ienum_next(membere);

               for(fat=(FATrans*)ihtab_get(dfa->states,member);fat!=NULL;fat=fat->next)
                  if(fat->c == c) 
                     break;
               
               if (fat == NULL)
                  newgroup= (IntSet*)htab_get(whereto,NULL);
               else
                  newgroup= (IntSet*)htab_get(whereto,ihtab_get(partmap,fat->to));

               if (newgroup == NULL) {
                  newgroup = iset_createDefault();
                  if (fat == NULL)
                     htab_put(whereto, NULL, newgroup);
                  else
                     htab_put(whereto, ihtab_get(partmap,fat->to), newgroup);
               }
               if(group != newgroup) {
                  iset_remove(group,member);
                  iset_add(newgroup,member);
               }
               ihtab_put(newpartmap,member,newgroup);
            }
            ienum_free(membere);
            htab_free(whereto);
         }
         enum_free(groupe);

         for(allse=ihtab_getKeys(newpartmap);ienum_hasNext(allse);)
            hset_add(newpartition,ihtab_get(newpartmap,ienum_next(allse)));
         ienum_free(allse);

         ihtab_free(partmap);
         partmap = newpartmap;
      }

      if( hset_size(partition) == hset_size(newpartition)) {
         hset_free(newpartition);
         break;
      }

      hset_free(partition);   
      partition = newpartition;
   }   

   // Step 4 turn the PDFA back into a dfa

   newdfa = dfa_createDefault();

   {
      Enumeration* groupe;
      Hashtable* mapping;
      int count=0;

      mapping = htab_create(
         hset_size(partition),
         (unsigned int(*)(void*))iset_hash,
         (int(*)(const void*,const void*))iset_comp,
         0
      );

      count = 0;

      // build mapping, migrate states, and set accepting states

      for(groupe=hset_elements(partition);enum_hasNext(groupe);){
         IntSet* members;
         int* i;

         i = (int*)emalloc(sizeof(int));
         *i = count++ ;

         members = (IntSet*)enum_next(groupe);

         htab_put(mapping,members,i);
         ihtab_put(newdfa->states,*i,NULL);

         // printf("\n### Mapping %d to " ,*i);
         // printiset(members);
      }
      enum_free(groupe);

      for(groupe=hset_elements(partition);enum_hasNext(groupe);){
         IntSet * members;
         int newstate;
         IntEnumeration* ie;
         IntHashtable* transitions;

         transitions = ihtab_createDefault();
         members = (IntSet*)enum_next(groupe);

         newstate = *((int*)htab_get(mapping,members));

         for(ie =iset_elements(members);ienum_hasNext(ie);){
            FATrans* fat;
            int member;

            member = ienum_next(ie);
            fat = (FATrans*)ihtab_get(dfa->states,member);

            while(fat){
               int* i;

               i = (int*)htab_get(mapping,ihtab_get(partmap,fat->to));
               ihtab_put(transitions,(char)fat->c,i);
                  
               fat=fat->next;
            }

            if(iset_contains(dfa->accepting,member))
               iset_add(newdfa->accepting,newstate);
            if(dfa->start == member)
               newdfa->start = newstate;
         }
         ienum_free(ie);

         for(ie =ihtab_getKeys(transitions);ienum_hasNext(ie);){
            char c = (char)ienum_next(ie);
            int* to = ihtab_get(transitions,(int)c);

            FATrans* fat = t_create(c,*to);
            fat->next = ihtab_get(newdfa->states,newstate);
            ihtab_put(newdfa->states,newstate,fat);
         }

         ienum_free(ie);
         ihtab_free(transitions);
      }
      enum_free(groupe);

      for(groupe=hset_elements(partition);enum_hasNext(groupe);){
         IntSet* members = (IntSet*)enum_next(groupe);
         int* i = htab_get(mapping,members);
         free(i);
         iset_free(members);
      }
      enum_free(groupe);
      hset_free(partition);
      ihtab_free(partmap);
      htab_free(mapping);
   }

   return newdfa;
}

/*=============================================================================
// end of file: $RCSfile: dfa.c,v $
==============================================================================*/

