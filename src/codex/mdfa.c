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

   $RCSfile: mdfa.c,v $   
   $Revision: 1.10 $
   $Author: dhakim $
   $Date: 2002/04/05 02:03:14 $
 
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
#include "excpfn.h"
#include "mdfa.h"

MDFA* mdfa_create(DFA* dfa){
   MDFA* mdfa = (MDFA*)emalloc(sizeof(MDFA));
   IntEnumeration * ie;
   int i;

   mdfa->numstates = (short)ihtab_size(dfa->states);
   mdfa->start = (short)dfa->start;
   mdfa->accepting = (char*)ecalloc(1,mdfa->numstates);
   mdfa->smatrix = (short*)emalloc(sizeof(short)*128*mdfa->numstates);
   
   for( i=0;i<((int)mdfa->numstates * 128);i++)
      mdfa->smatrix[i] = -1;
   
   for(ie= ihtab_getKeys(dfa->states);ienum_hasNext(ie);){
      int state = ienum_next(ie);
      FATrans* fat;

      if(iset_contains(dfa->accepting,state))
         mdfa->accepting[state] = '\1';

      for(fat= ihtab_get(dfa->states,state);fat!=NULL;fat=fat->next){
         mdfa->smatrix[(state*128) + fat->c] = (short)fat->to;
      }
   }
   ienum_free(ie);

   return mdfa;
}

void mdfa_free(MDFA* mdfa){
   free(mdfa->accepting);
   free(mdfa->smatrix);
   free(mdfa);
}

void mdfa_generateCode(MDFA* mdfa,StringBuffer* out,char* prefix){
   int i;

   buf_printf(out,"\nstatic char %s_accepting[%d] = {",prefix,mdfa->numstates);

   for(i=0;i<mdfa->numstates;i++){
      if (i%38 == 0) buf_printf (out,"\n   ");
      if(mdfa->accepting[i] == '\1')
         buf_printf(out,"1");
      else
         buf_printf(out,"0");
      if(i+1 != mdfa->numstates)
         buf_printf(out,",");
   }

   buf_printf(out,"\n};");

   buf_printf(out,"\nstatic short %s_transitions[%d] = {",prefix,(int)mdfa->numstates*128);

   for(i=0;i<(int)mdfa->numstates*128;i++){
      if (i%10 == 0) buf_printf (out,"\n   ");
      buf_printf(out,"0x%04hX",mdfa->smatrix[i]);
      if(i+1 != (int)mdfa->numstates * 128)
         buf_printf(out,",");
   }

   buf_printf(out,"\n};");
  
   buf_printf(out,"\nstatic MDFA %s = {%hd,%hd,%s_accepting,%s_transitions};\n\n",
      prefix,mdfa->numstates,mdfa->start,prefix,prefix);

}

int mdfa_matches(MDFA* mdfa,char* string){ 
   int i=0 , l=strlen(string);
   short s=mdfa->start ;

   for(i=0;i<l;i++){
      s = mdfa->smatrix[(int)string[i] + (s*128)];
      if(s==-1) return 0;
   }
   return (int)mdfa->accepting[s];
}

int mdfa_starts(MDFA* mdfa,char* string){
   int i=0 , l=strlen(string);
   int matchlength = -1;
   short s=mdfa->start ;

   if (mdfa->accepting[s]) matchlength = 0;

   for(i=0;i<l;i++){
      s = mdfa->smatrix[(int)string[i] + (s*128)];
      if(s==-1) return matchlength;
      if(mdfa->accepting[s]) matchlength = i+1;
   }
   return matchlength;
}

MDFA* mdfa_createFromRX(char *rx) {
   MDFA *mdfa;
   NFA *nfa;
   DFA *dfa;
   DFA *odfa;
   /* create nfa */
   nfa_init();
   nfa = parse(rx,0,strlen(rx));
   /* create dfa */
   dfa = dfa_create(nfa);
   nfa_free(nfa);
   /* create minimized dfa */
   odfa = dfa_createMinimized(dfa);
   dfa_free(dfa);
   dfa = odfa;
   mdfa = mdfa_create(dfa);
   dfa_free(dfa);
   return mdfa;
}

/*=============================================================================
// end of file: $RCSfile: mdfa.c,v $
==============================================================================*/

