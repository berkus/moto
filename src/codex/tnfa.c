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

   $RCSfile: tnfa.c,v $   
   $Revision: 1.8 $
   $Author: dhakim $
   $Date: 2003/01/07 23:21:18 $
 
==============================================================================*/
#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "memsw.h"

#include "tnfa.h"
#include "vector.h"
#include "inthashtable.h"
#include "itoihashtable.h"
#include "intset.h"
#include "intstack.h"
#include "intboundedqueue.h"
#include "stack.h"
#include "hashtable.h"
#include "hashset.h"
#include "excpfn.h"
#include "exception.h"
#include "objectpool.h"

char LEFT_PRIORITY = 'L';
char RIGHT_PRIORITY = 'R';
char CYCLE_PRIORITY = 'C';
char NO_PRIORITY = 'O';

char* TESDFAState_toString(IntSet* U,TNFA* tnfa);

typedef struct priorityList {
	int length;
	char priority[1];
} PriorityList;

PriorityList* plist_create(int max){
	PriorityList* result=emalloc(sizeof(PriorityList) + (max-1)*sizeof(char));
	result->length=0;
	return result;
}

inline PriorityList* plist_copyInto(PriorityList* dest,PriorityList* src){
	memcpy(dest,src, sizeof(PriorityList) + (src->length -1) * sizeof(char));
	return dest;
}

inline PriorityList* plist_copyIntoAndInsert(PriorityList* dest,PriorityList* src,char priority){
	memcpy(&dest->priority[1],&src->priority[0],src->length * sizeof(char));
	dest->length=src->length + 1;
	dest->priority[0] = priority;
	return dest;
}

PriorityList defaultPList = {0,{'\0'}};

char* plist_toString(PriorityList* p){
	StringBuffer* sb=buf_createDefault();
	char* result;
	int i;
	if (p==NULL) p=&defaultPList;
	for(i=0;i<p->length;i++){ 
		buf_putc(sb,p->priority[i]); if(i<p->length-1) buf_puts(sb,"<-");
	}
	result = buf_toString(sb);
	buf_free(sb);
	return result;
}

int plist_priority(PriorityList* a, PriorityList* b){

	int i,aCycles=0, bCycles=0;

   if(a==NULL) a=&defaultPList;
   if(b==NULL) b=&defaultPList;
   	
	for(i=0;i<a->length;i++)
		if (a->priority[i]==CYCLE_PRIORITY) {aCycles++; break;}
	
	for(i=0;i<b->length;i++)
		if (b->priority[i]==CYCLE_PRIORITY) {bCycles++; break;}
	
	// If B cycles and A doesn't then A beats B
	if(aCycles < bCycles) return 1;
		
	// If A cycles and B doesn't then B beats A
	if(aCycles > bCycles) return -1;
	
	if(aCycles == 0 && bCycles == 0){
		for(i=0;i<a->length&&i<b->length;i++){
			if(a->priority[i] == b->priority[i]) continue;
			if(a->priority[i] == 'L') return 1; else return -1;
		}
		//printf("Priority Is Equal!\n");
		return 0;
	} else {
		/* Both paths cycled hence we should choose the shorter cycle*/

		if (a->length > b->length) return -1;
		if (b->length > a->length) return 1;
		//printf("Cycles are of equal length priority Is Equal!\n");
		return 0;
	}
}

int 
tnfa_addState(TNFA* nfa){
   int state = nfa->laststate++;
   ihtab_put(nfa->states,state,NULL);
   itoi_put(nfa->inputOrder,state,0);
   return state;
}

inline TNFA* 
tnfa_createDefault(){
   TNFA* nfa = (TNFA*)emalloc(sizeof(TNFA));

   nfa->tagcount = 0;
   nfa->laststate = 0;
   nfa->minimized = iset_createDefault();
   nfa->states = ihtab_createDefault();
	nfa->inputOrder = itoi_createDefault();
   return nfa;
}

void 
tnfa_free(TNFA* nfa){
   Enumeration* e;

   for(e=ihtab_getValues(nfa->states);enum_hasNext(e);){
      TFATrans* fat,*next;
   
      for(fat=enum_next(e);fat!=NULL;fat=next){
         next = fat->next;
         free(fat);
      }
   }
   enum_free(e);
   
   ihtab_free(nfa->states);
	itoi_free(nfa->inputOrder);
	iset_free(nfa->minimized);
	
	free(nfa->cStates);
	free(nfa->cInputOrder);
   free(nfa);
}

inline void 
tnfa_addTransition(TNFA* nfa, int from, int to, char c, int tag,char priority){
   TFATrans* fat;
	TFATrans* t ;
	
   fat = (TFATrans*)ihtab_get(nfa->states,from);

   t = (TFATrans*)emalloc(sizeof(TFATrans));
   t->c = c;
   t->to = to;
   t->tag = tag;
   t->priority = priority;
   t->next = fat;

   ihtab_put(nfa->states,from,t);
   itoi_put(nfa->inputOrder,to,itoi_get(nfa->inputOrder,to)+1);
}

TNFASubExpression 
tnfa_char (TNFA* tnfa, char c){
   TNFASubExpression result = {tnfa_addState(tnfa),tnfa_addState(tnfa)};
   tnfa_addTransition(tnfa,result.start,result.finish,c,-1,NO_PRIORITY);
   return result;
} 

TNFASubExpression
tnfa_anyChar (TNFA* tnfa){
   char c;

   TNFASubExpression result = {tnfa_addState(tnfa),tnfa_addState(tnfa)};
   
   for(c=1;c<127;c++){
      tnfa_addTransition(tnfa,result.start,result.finish,c,-1,NO_PRIORITY);
   }

   return result;
}

TNFASubExpression 
tnfa_characterClass (TNFA* tnfa, char* regex,int sidx,int eidx){
   int i,j;
   char tarr[128];
   char invert = '\0';

	TNFASubExpression result = {tnfa_addState(tnfa),tnfa_addState(tnfa)};
   
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
            case 's' : tarr[(int)' '] = !invert ;
								tarr[(int)'\f'] = !invert;
								tarr[(int)'\n'] = !invert;
								tarr[(int)'\t'] = !invert;
								tarr[(int)'\r'] = !invert;
								break;
				case 'S' : tarr[(int)' '] = invert ;
								tarr[(int)'\f'] = invert;
								tarr[(int)'\n'] = invert;
								tarr[(int)'\t'] = invert;
								tarr[(int)'\r'] = invert;
								break;	
            case 'd' : for(j='0';j<'9';j++) tarr[j] = !invert; break;
            case 'D' : for(j='0';j<'9';j++) tarr[j] = invert; break;
            case 'w' : for(j='0';j<'9';j++) tarr[j] = !invert;
            				for(j='a';j<'z';j++) tarr[j] = !invert;
            				for(j='A';j<'Z';j++) tarr[j] = !invert;
            				break;
				case 'W' : for(j='0';j<'9';j++) tarr[j] = invert;
            				for(j='a';j<'z';j++) tarr[j] = invert;
            				for(j='A';j<'Z';j++) tarr[j] = invert;
            				break;
               				
            default  : tarr[(int)regex[i+1]] = !invert;
         }
         i++;
      } else { 
         tarr[(int)regex[i]] = !invert ;
      }
   }

   for(i=1;i<128;i++) if (tarr[i]) {
      tnfa_addTransition(tnfa,result.start,result.finish,(char)i,-1,NO_PRIORITY);
   }

   return result;
}

TNFASubExpression 
tnfa_or(TNFA* tnfa, TNFASubExpression left, TNFASubExpression right){

   TNFASubExpression result = {tnfa_addState(tnfa),tnfa_addState(tnfa)};

   tnfa_addTransition(tnfa,result.start,left.start,'\0',-1,NO_PRIORITY);
   tnfa_addTransition(tnfa,result.start,right.start,'\0',-1,NO_PRIORITY);
   tnfa_addTransition(tnfa,left.finish,result.finish,'\0',-1,LEFT_PRIORITY);
   tnfa_addTransition(tnfa,right.finish,result.finish,'\0',-1,RIGHT_PRIORITY);

   return result; 
}

TNFASubExpression 
tnfa_follows(TNFA* tnfa, TNFASubExpression left,TNFASubExpression right){

	TNFASubExpression result={left.start,right.finish};

	if(itoi_get(tnfa->inputOrder,right.start) == 0) {
		TFATrans* t = ihtab_get(tnfa->states,right.start);
		ihtab_put(tnfa->states,left.finish,t);
		ihtab_remove(tnfa->states,right.start);
		itoi_remove(tnfa->inputOrder,right.start);
	} else {
	
		/* Like hell am I going to traverse the rightmost NFA looking for transitions
			back to the start state */
			
		/* Add an epsilon transition from the left finish to the right start */
		tnfa_addTransition(tnfa,left.finish,right.start,'\0',-1,NO_PRIORITY);
	}
	
   return result;
}

TNFASubExpression
tnfa_tag(TNFA* tnfa,TNFASubExpression subex, int left_tag, int right_tag){
  
   TNFASubExpression result={ tnfa_addState(tnfa),tnfa_addState(tnfa)};
   
   tnfa_addTransition(tnfa,result.start,subex.start,'\0',left_tag,NO_PRIORITY);
   tnfa_addTransition(tnfa,subex.finish,result.finish,'\0',right_tag,NO_PRIORITY);

	iset_add(tnfa->minimized,left_tag);
	
   return result;
}

TNFASubExpression 
tnfa_addModifier(TNFA* tnfa,char c, TNFASubExpression subex ){
   TNFASubExpression result={ tnfa_addState(tnfa),tnfa_addState(tnfa)};
   
   tnfa_addTransition(tnfa,result.start,subex.start,'\0',-1,NO_PRIORITY);   
   tnfa_addTransition(tnfa,subex.finish,result.finish,'\0',-1,RIGHT_PRIORITY);
   
   switch(c) {
      case '*' : 
   		tnfa_addTransition(tnfa,result.start,result.finish,'\0',-1,LEFT_PRIORITY);
   		tnfa_addTransition(tnfa,subex.finish,subex.start,'\0',-1,CYCLE_PRIORITY);
   		break;
      case '+' : 
			tnfa_addTransition(tnfa,subex.finish,subex.start,'\0',-1,CYCLE_PRIORITY);
     		break;
      case '?' : 
			tnfa_addTransition(tnfa,result.start,result.finish,'\0',-1,LEFT_PRIORITY);
			break;
   }
	
   return result;
}

TNFASubExpression 
tnfa_parse (TNFA* tnfa, char* regex, int start, int end){
   int i = start;
   
   TNFASubExpression last_component_nfa={-1,-1};
   
   while (i< end){
   	TNFASubExpression component_nfa;
   	      
   	if(isspace(regex[i])) {i++; continue; }

      switch(regex[i]) {

         case '(' : {
            int j=i+1,opens = 0;
            int left_tag = tnfa->tagcount++;
            int right_tag = tnfa->tagcount++;
            
            while(regex[j] != ')' || opens != 0){
               if(regex[j] == '\\') j++;
               else if(regex[j] == '(') opens ++;
               else if(regex[j] == ')') opens --;
               j++;

               if (j>=end){ THROW("RegexParseException","Unmatched '('"); }
                  
            }
            
				component_nfa = tnfa_parse(tnfa,regex,i+1,j);
				component_nfa = tnfa_tag(tnfa,component_nfa,left_tag,right_tag);
            
            i=j+1;

         } break;

         case '.' : {
            component_nfa = tnfa_anyChar(tnfa);
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

            component_nfa = tnfa_characterClass(tnfa,regex,i+1,j);
           
            i=j+1;

         } break;

         case '|' : {

            component_nfa = tnfa_parse(tnfa,regex,i+1,end);

            if (last_component_nfa.start == -1)
               return component_nfa;
            
            else /* if (last_component_nfa != -1) */
               return tnfa_or(tnfa,last_component_nfa,component_nfa);
         }
	         case '\\' : {
            /* Character literal follows */

            if(i+1 >= end){ THROW("RegexParseException","Escape '\\' not followed"); } 

            switch(regex[i+1]){
               case 'f' : component_nfa = tnfa_char(tnfa,'\f') ; break;
               case 'n' : component_nfa = tnfa_char(tnfa,'\n') ; break;
               case 't' : component_nfa = tnfa_char(tnfa,'\t') ; break;
               case 'r' : component_nfa = tnfa_char(tnfa,'\r') ; break;
               case 's' : component_nfa = tnfa_characterClass(
               				tnfa," \\n\\r\\t\\f",0,strlen(" \\n\\r\\t\\f")); break;
               case 'S' : component_nfa = tnfa_characterClass(
               				tnfa,"^ \\n\\r\\t\\f",0,strlen("^ \\n\\r\\t\\f")); break;
               case 'd' : component_nfa = tnfa_characterClass(
               				tnfa,"0-9",0,strlen("0-9")); break;
               case 'D' : component_nfa = tnfa_characterClass(
               				tnfa,"^0-9",0,strlen("^0-9")); break;
               case 'w' : component_nfa = tnfa_characterClass(
               				tnfa,"0-9a-zA-Z",0,strlen("0-9a-zA-Z")); break;
               case 'W' : component_nfa = tnfa_characterClass(
               				tnfa,"^0-9a-zA-Z",0,strlen("^0-9a-zA-Z")); break;
               default  : component_nfa = tnfa_char(tnfa,regex[i+1]);
            }

            i+=2;   
         } break;
         default : {
            component_nfa = tnfa_char(tnfa,regex[i]); 
            i++;
         }
        
      } 

		while (isspace(regex[i]) && i<end) i++;
		
      while (regex[i] == '*' || regex[i] == '+' || regex[i] == '?'){
         component_nfa = tnfa_addModifier(tnfa,regex[i], component_nfa);
         i++;
      }

      if (last_component_nfa.start == -1)
         last_component_nfa = component_nfa;
      else /* if (component_nfa != NULL) */
         last_component_nfa = tnfa_follows(tnfa,last_component_nfa, component_nfa);
      
   }

   return last_component_nfa;
}

TNFA* tnfa_create(char* regex){
   TNFA* tnfa = tnfa_createDefault();
   TNFASubExpression tse;
	int i;

	int left_tag=tnfa->tagcount++;
   int right_tag=tnfa->tagcount++;
   /*tse = tnfa_parse(tnfa,regex,0,strlen(regex));
	
	*/   
	
   tse = tnfa_tag(tnfa,  tnfa_parse(tnfa,regex,0,strlen(regex)) ,left_tag,right_tag);
  	tnfa->start = tse.start;
	tnfa->finish = tse.finish;
	
	tnfa->cStates = emalloc((tnfa->laststate +1)*sizeof(TFATrans*));
	tnfa->cInputOrder = emalloc((tnfa->laststate +1)*sizeof(int));
	tnfa->maxpath = 0;
	
	for(i=0;i<=tnfa->laststate;i++){
		tnfa->cStates[i] = ihtab_get(tnfa->states,i);
	   tnfa->cInputOrder[i] = itoi_get(tnfa->inputOrder,i);
	   if (tnfa->cInputOrder[i] > 0)
	   	tnfa->maxpath++;
	}

	return tnfa;
}

/*
	Def 3.1 pg 21 
	
	returns 1 if a beats b
	returns 0 if a equals b
	returns -1 if b beats a
	
*/

inline TagValueFunction* tvf_initialize(TagValueFunction* tvf,int tags){
	int i;
	for(i=0;i<tags;i++)
	   tvf[i] = -1;
	
	return tvf;
}

TagValueFunction* tvf_create(int tags){
	TagValueFunction* tvf = emalloc(sizeof(int)* tags);
	tvf_initialize(tvf,tags);
	return tvf;
}

inline int tvf_getTagValue(TagValueFunction* tvf,int tag){return tvf[tag];}

inline void tvf_setTagValue(TagValueFunction* tvf, int tag, int pos,int tags){
	int i;
	tvf[tag] = pos;
	/* Clear out higher numbered tags */
	if(tag%2==0)
		for(i=tag+1;i<tags;i++) 
			tvf[i] = -1;
}

inline TagValueFunction* tvf_copyInto(TagValueFunction* dest, TagValueFunction* src, int tags){
	memcpy(dest,src,tags* sizeof(int));
	return dest;
}

inline char* tvf_toString(TagValueFunction* tvf,TNFA* tnfa){
   StringBuffer* sb = buf_createDefault();
   char* result;
   int i;
   
   buf_puts(sb,"<");
   for(i=0;i<tnfa->tagcount;i++){
   	buf_printf(sb,"%d%s",tvf[i],i==tnfa->tagcount-1?"":",");
   } 
   buf_puts(sb,">");
   
   result = buf_toString(sb);
   buf_free(sb);
   return result;
}

int tnfa_priority(TNFA* tnfa, TagValueFunction* tvf_a, TagValueFunction* tvf_b){
	int x,tva_x=-1,tvb_x=-1;
	char minimize_tx;
	
   for(x=0;x<tnfa->tagcount;x++) {
   	tva_x = tvf_getTagValue(tvf_a,x);
   	tvb_x = tvf_getTagValue(tvf_b,x);
   	if(tva_x != tvb_x) break;
   }
  
   if(x == tnfa->tagcount)
   	return 0; /* TagValueFunction a is equivalent to b */
   	
   minimize_tx = iset_contains(tnfa->minimized,x);
      
	if ((minimize_tx && tva_x < tvb_x ) || (!minimize_tx && tva_x > tvb_x )){
		return 1; /* TagValueFunction a beats b */
	} else {
		return -1; /* TagValueFunction b beats a */
	}
   
}

typedef struct tnfaMatcher{
	TNFA* tnfa;
	int pos;
	int* counts;
	IntBoundedQueue* queue;
	PriorityList** priorities;
	TagValueFunction* tmp;
	ObjectPool* tvfpool;
	ObjectPool* plistpool;
} TNFAMatcher;

Vector* tnfa_close(TNFAMatcher m, Vector* states, TagValueFunction** tvfs){
	int i; 
	void** vd;
	
	/* Initialize the Priority lists */
	memset(m.priorities,'\0',(m.tnfa->laststate+1) * sizeof(PriorityList*));
	
	/* For each pair <q,v> in the input add q to the queue */

	ibqueue_clear(m.queue);	
	for(i=vec_size(states)-1,vd=vec_data(states);i>=0;i--)
		ibqueue_enqueue(m.queue,(int)vd[i]);
	
	/* For each state s initialize count(s) to the input order of s */
	memcpy(m.counts,m.tnfa->cInputOrder,(m.tnfa->laststate+1)*sizeof(int));
	
	/* While queue is not empty */
	while(!ibqueue_isEmpty(m.queue)){
		int s1 = ibqueue_dequeue(m.queue);
		TFATrans* t = m.tnfa->cStates[s1];
		TagValueFunction* v1 = tvfs[s1],*curr_v2,*v2=m.tmp;
		PriorityList* p1 = m.priorities[s1],*curr_p2,*p2;
		
		/* For each transition out of s1 */
		while(t){
			
		   /* If this is an epsilon transition */
		   if(t->c == '\0') {
		      int s2 = t->to;
				curr_v2 = tvfs[s2];
				v2 = tvf_copyInto(v2,v1,m.tnfa->tagcount);
				curr_p2 = m.priorities[s2];
				
				/* v_2(x) = {pos if x==t->tag and t->tag != -1 , v_1(x) otherwise } */
				
				if(t->tag != -1) tvf_setTagValue(v2,t->tag,m.pos,m.tnfa->tagcount);
				
				if(t->priority == NO_PRIORITY)
					if(p1 == NULL)
						p2 = NULL;
					else 
						p2 = plist_copyInto((PriorityList*)opool_grab(m.plistpool),p1); 
				else 
					if(p1==NULL){
						p2 = (PriorityList*)opool_grab(m.plistpool);
						p2->length = 1; 
						p2->priority[0] = t->priority;
					} else {
						// Never cycle twice!
						if(t->c == CYCLE_PRIORITY) {
							int twoCycles = 0;
							for(i=0;i<p1->length;i++)
								if (p1->priority[i]==CYCLE_PRIORITY){
									twoCycles = 1;
									break;
								}
							if(twoCycles)
								continue;		
						}
						p2 = plist_copyIntoAndInsert((PriorityList*)opool_grab(m.plistpool),p1,t->priority);
					}
				/* if no result for s2 is defined OR v2 is of a greater priority
					than the current result for s2 */
					 
			//	printf("Comparing priority of s%d:%s from s%d:%s with s%d:%s = %d\n",
			//		s2,plist_toString(p2),s1,plist_toString(p1),s2,plist_toString(curr_p2),plist_priority(p2,curr_p2)); 
								
				if(curr_v2 == NULL || curr_v2[0] > v2[0] || (curr_v2[0] == v2[0] && plist_priority(p2,curr_p2) >= 0 ) ){
					int curCount = m.counts[s2];
					
					/* set the current result for s2 */
					if (curr_v2 != NULL) tvfs[s2] = tvf_copyInto(tvfs[s2],v2,m.tnfa->tagcount);
					else { 
						tvfs[s2] = (TagValueFunction*)opool_grab(m.tvfpool);
						tvf_copyInto(tvfs[s2],v2,m.tnfa->tagcount);
					}
					
					m.priorities[s2]=p2;
					

					if (curr_v2 == NULL) vec_add(states,(void*)s2);
					if (curr_p2 != NULL) opool_release(m.plistpool, curr_p2);

			/*		{int i; 
							printf("{");
		for(i=0;i<vec_size(states);i++)
			printf("%d,",vec_get(states,i));
		printf("}");
		printf("{");
		for(i=0;i<m.tnfa->laststate+1;i++){
			int j;int found=0;
			if(tvfs[i] != NULL){
				for(j=0;j<vec_size(states);j++)
					if(vec_get(states,j) == i) found =1;
				if(found) printf("*,"); else printf("?");
			} else { printf("_,"); }
		}
		printf("}\n");
		}			*/		
					/* decrease count(s2) by 1 */
					--curCount;
					
					/* if count(s2) ==0 */
					if(curCount == 0) {
						/* put s2 at the front of the queue and reset its 
							count to its input order */
						ibqueue_prequeue(m.queue,s2);
						m.counts[s2]=m.tnfa->cInputOrder[s2];
					} else {
						/* put s2 at the back of the queue and set its count 
							to its old count -1 */
						ibqueue_enqueue(m.queue,s2);
						m.counts[s2]=curCount;
					}
				} else {
					if (p2 != NULL) opool_release(m.plistpool,p2);
				} 
		   }
		   t=t->next;
		}
		
	}
	
	/* Free the plists */
	
	for(i=vec_size(states)-1,vd=vec_data(states);i>=0;i--){
		int q= (int)vd[i];
		if(m.priorities[q] != NULL)
	   	opool_release(m.plistpool,m.priorities[q]); 		
	}
	
	return states;
}

TagValueFunction* tnfa_match(TNFA* tnfa, char* input, int anchorStart, int anchorEnd){
	Vector* reach, *reachNext, *tmp;
	TagValueFunction** reachTVFs, **reachNextTVFs, **tmpTVFs;
	int i;
	int leftmostMatchFound = 0;
	TagValueFunction* finalTVF = NULL;
	void** vd;
	TNFAMatcher m;
	
	/* Construct the matcher Object */
	m.tnfa = tnfa;
	m.pos = 0;
	m.counts = ecalloc(tnfa->laststate+1,sizeof(int));
	m.queue = ibqueue_create(tnfa->laststate+1);
	m.priorities = ecalloc(tnfa->laststate+1,sizeof(PriorityList*));
	m.tmp = tvf_create(tnfa->tagcount);
	m.tvfpool = opool_createWithExt((void*(*)(void*))tvf_create,shared_free,NULL,NULL,(void*)tnfa->tagcount,10);
	m.plistpool = opool_createWithExt((void*(*)(void*))plist_create,shared_free,NULL,NULL,(void*)tnfa->maxpath,10);
	
	reach = vec_createDefault();
	reachTVFs = ecalloc(tnfa->laststate+1,sizeof(TagValueFunction*));

	reachNext = vec_createDefault();
	reachNextTVFs = ecalloc(tnfa->laststate+1,sizeof(TagValueFunction*));

	/* Initialize reach to consistentClosure({<s,v0>}); */		
	vec_add(reach,(void*)tnfa->start);
	reachTVFs[tnfa->start] = (TagValueFunction*)opool_grab(m.tvfpool);

	reach = tnfa_close(m,reach,reachTVFs);
	
	/* If this regex matches the empty string then we may already have a possible match :P */
	if(reachTVFs[tnfa->finish]!=NULL && (!anchorEnd || (anchorEnd && !*input)) ){
		leftmostMatchFound = 1;
		finalTVF = (TagValueFunction*)opool_grab(m.tvfpool);
		tvf_copyInto(finalTVF,reachTVFs[tnfa->finish],m.tnfa->tagcount);
	}
		
	/*{int i; 
							printf("{");
		for(i=0;i<vec_size(reach);i++)
			printf("%d,",vec_get(reach,i));
		printf("}");
		printf("{");
		for(i=0;i<m.tnfa->laststate+1;i++){
			int j;int found=0;
			if(reachTVFs[i] != NULL){
				for(j=0;j<vec_size(reach);j++)
					if(vec_get(reach,j) == i) found =1;
				if(found) printf("*,"); else printf("?");
			} else { printf("_,"); }
		}
		printf("}\n");
		}	*/
		
	//printf("\nclosure(s%d) = %s",tnfa->start,match_toString(reach,tnfa));
	/* while pos < |input| */
	while(*input && vec_size(reach) > 0){
		
		/* fetch the next input symbol */
		char c = *input;
		input ++;
		m.pos++;
		
		vec_clear(reachNext);
		memset(reachNextTVFs,'\0',(tnfa->laststate+1) * sizeof(TagValueFunction*));
		

		/* For each item <q,v> in reach */
		
		for(i=vec_size(reach)-1,vd=vec_data(reach);i>=0;i--){
			TFATrans* t;
			TagValueFunction* v;
			int q = (int)vd[i];
			
			v = reachTVFs[q];
			
			/* For each transition on c out of q do */
			t = tnfa->cStates[q];
			while(t != NULL){
				/* add <t->to,v> to reachNext */
				if(t->c == c){
					
					/* WARNING: ... dangerous : I believe that with NFAs constructed
					with the thompson construction there is a 1to1 mapping between
					'from' states and 'to' states on a non-epsilon transition. If that
					were not the case then there might be two different ways to get to
					the same state from reach on c and I would need to add those seperately
					to reachNext prohibiting the use of a hash */
					 
					reachNextTVFs[t->to] = (TagValueFunction*)opool_grab(m.tvfpool);
					tvf_copyInto(reachNextTVFs[t->to],v,m.tnfa->tagcount);
					vec_add(reachNext,(void*)t->to);
				}
				t = t->next;
			}
		}
		
		/* Free reach */
		
		for(i=vec_size(reach)-1,vd=vec_data(reach);i>=0;i--)
			opool_release(m.tvfpool,reachTVFs[(int)vd[i]]);
		
		if(!anchorStart && (!leftmostMatchFound || anchorEnd)){
			if(reachNextTVFs[tnfa->start] == NULL){
				reachNextTVFs[tnfa->start] = (TagValueFunction*)opool_grab(m.tvfpool);
				tvf_initialize(reachNextTVFs[tnfa->start],tnfa->tagcount);
				vec_add(reachNext,(void*)tnfa->start);
			}
		}
		
		reachNext = tnfa_close(m,reachNext,reachNextTVFs);
		
		
		if(reachNextTVFs[tnfa->finish] != NULL){
			leftmostMatchFound = 1;
			if( (!anchorEnd && (finalTVF == NULL || reachNextTVFs[tnfa->finish][0] <= finalTVF[0])) ||
				(anchorEnd && !*input) ){
				if(finalTVF!=NULL) opool_release(m.tvfpool,finalTVF);
				finalTVF = (TagValueFunction*)opool_grab(m.tvfpool);
				tvf_copyInto(finalTVF,reachNextTVFs[tnfa->finish],m.tnfa->tagcount);
				// printf("\tFound potential match: %s\n",match_toString(reachNext,tnfa));
			}
		}
			
		//printf("\nclosure = %s",match_toString(reachNext,tnfa));
		
		// Swap reach and reachNext
		tmp = reach;
		tmpTVFs = reachTVFs;
		
		reach = reachNext;
		reachTVFs = reachNextTVFs;
		
		reachNext = tmp;
		reachNextTVFs = tmpTVFs;
	}
	
	/* Free reach */
	
	for(i=vec_size(reach)-1,vd=vec_data(reach);i>=0;i--)
		opool_release(m.tvfpool,reachTVFs[(int)vd[i]]);
			
	vec_free(reach);
	free(reachTVFs);
		
	vec_free(reachNext);
	free(reachNextTVFs);
	
	free(m.tmp);
	free(m.counts);
	free(m.priorities);
	ibqueue_free(m.queue);
	
	if(finalTVF){
		TagValueFunction* tmp = finalTVF;
		finalTVF = tvf_copyInto(tvf_create(tnfa->tagcount),finalTVF,tnfa->tagcount);
		opool_release(m.tvfpool,tmp);
	} else {
		finalTVF = tvf_create(tnfa->tagcount);
	}
	
	if(anchorEnd && *input) {
		TagValueFunction* tmp = finalTVF;
		finalTVF = tvf_create(tnfa->tagcount);
		free(tmp);
	}
	
	opool_free(m.tvfpool);
	opool_free(m.plistpool);
	
	return finalTVF;
}

char* tnfa_toString(TNFA* tnfa){
	StringBuffer* sb = buf_createDefault();
	char* result;
	int i;
	IntEnumeration* ie;
	
	buf_printf(sb,"\nStart state s%d\nFinish state s%d",tnfa->start,tnfa->finish);
	for(i=0;i<tnfa->laststate;i++){
		if(ihtab_contains(tnfa->states,i)){
			TFATrans* t;
			buf_printf(sb,"\ns%d in: %d out: ",i,itoi_get(tnfa->inputOrder,i));
			t= ihtab_get(tnfa->states, i);
			while(t!=NULL){
				if (t->c==0)
					buf_printf(sb,"<s%d,'\\0',%d,%c> ",t->to,t->tag,t->priority);
				else
					buf_printf(sb,"<s%d,'%c',%d,%c> ",t->to,t->c,t->tag,t->priority);
				t=t->next;
			}
		}
	}

	buf_printf(sb,"\n#Tags = %d #Minimized = %d Minimized Tags ={",tnfa->tagcount,iset_size(tnfa->minimized));
	ie = iset_elements(tnfa->minimized);
	while(ienum_hasNext(ie)){
	   buf_printf(sb,"%d",ienum_next(ie));
	   if(ienum_hasNext(ie)) buf_putc(sb,',');
	}
	buf_putc(sb,'}');
	result= buf_toString(sb);
	buf_free(sb);
	return(result);
}

/*

int main(int argc, char** argv){
   TNFA* foo;
   char* s,*t;
   char* bar;
      TagValueFunction* r;
   shared_init(32*1024*1024);


		{
			int i=0;
	char* tests[] = {
		"(foo)|(bar)",								"bar",
		"(foo)|(bar)",								"foo", 
		"(foo)|(foo)|(foo)",						"foo",
		"(bar)|(foo)|(foo)",						"foo",
		"(bar|bar)|(foo|foo)",					"foo",
		"(foo)?",									"foo",
		"(foo)?(foo)?",							"foo",
		"((foo)?)?",								"foo",
		"foo|(foo)?",								"foo",
		"bar|(foo)?|foo",							"foo",
		"(a*)",										"aaaa",
		"(a*)*",										"aaaa",
		"((a*)*)*",									"aaaa",
		"(a*)(a*)",									"aaa",
		"(a|b|aab|ab|bab)*",						"aaabab",
		"(x|a*)*",									"aaaa",
		"(x|a*)*",									"aaax",
		"(x|a*)*",									"aaxx",
		"(x|a*)*",									"axxaa",
		"(x|a*)*",									"xaaaa",
		"(x|a*)*",									"axxaaxxa",
		"(foo)|(fo*)",								"f",
		"(foo)|(fo*)",								"fo",
		"(foo)|(fo*)",								"foo",
		"(foo)|(fo*)",								"fooo",
		"(foo)?(foo)*",							"foo",
		"(foo)?(foo)*",							"foofoo",
		"(foo)?(foo)?(foo)*" ,					"foo" ,
		"(foo)?(foo)?(foo)*" ,					"foofoo" ,
		"(foo)?(foo)?(foo)*" ,					"foofoofoo" ,
		"(((bb)|(bbab)|(aabb))*)(a|b)*" , 	"bbabaabbbb" , 
		"(((bb)|(bbab)|(aabb))*)(a|b)*" , 	"bbabaabbbbbbabbbbbaabbbbabbbbbabbbaabbaabbaabbaabb" 
		"bbabbbabbbabbbabbbabbbabbbabbbabbbabbbabaabbaabbaabbaabbaabbaabbaabbaabbaabbaabbaabbaa"
		"bbaabbaabbaabbaabbbbbbbbbbbbbbbbbbabbbabbbabbbabbbabbbabbbabbbabbbabbbabbbabbbabbbabbb"
		"bbaabbaabbaabbaabbaabbaabbaabbaabbaabbaabbaabbaabbaabbaabbaabbaabbaabbaabbaabbaabbaabb"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa"
		"bbbbbbbbaabbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaabbbbbbaabbaabbaa",
													NULL,											NULL
	};
	
	while(1){
		char* rx = tests[i*2], * ts = tests[i*2+1], *r;
		TNFA* tnfa; 
		TagValueFunction* tvf; 
		
		if(!rx) break;
		tnfa = tnfa_create(rx);
		if(argc>1) printf("\nTNFA for /%s/ =%s\n",rx,tnfa_toString(tnfa));
		tvf = tnfa_match(tnfa,ts,0,0);
		r =  tvf_toString(tvf,tnfa);
		
		printf("Match of /%s/ against \"%s\" =%s\n", rx,ts,r);
		free(r); free(tvf); tnfa_free(tnfa);
		
		i++;
	}
	}
	

}
*/

/*=============================================================================
// end of file: $RCSfile: tnfa.c,v $
==============================================================================*/

