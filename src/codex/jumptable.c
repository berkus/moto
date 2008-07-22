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

   $RCSfile: jumptable.c,v $   
   $Revision: 1.13 $
   $Author: dhakim $
   $Date: 2003/01/07 23:21:18 $
 
==============================================================================*/
#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>

#include "exception.h"
#include "excpfn.h"
#include "jumptable.h"
#include "stack.h"
#include "memsw.h"

int findDCol(IntSet* rows,size_t* lengths, char** argv){
   IntEnumeration* ienum;
   int maxDCol = -1,maxDColSize = 0,curlength,i,maxLength = 0;

   ienum = iset_elements(rows);

   while(ienum_hasNext(ienum))
      if((curlength = lengths[ienum_next(ienum)]) > maxLength)
         maxLength = curlength;
   ienum_free(ienum);

   for(i=0;i<maxLength;i++){
      IntSet* charSet = iset_createDefault();
      
      ienum = iset_elements(rows);
      while(ienum_hasNext(ienum)) {
         int cur = ienum_next(ienum);
         char* curString = argv[cur];
         iset_add (charSet, (int)( i < lengths[cur] ? curString[i] : '\0' ));
      }
      ienum_free(ienum);

      if (iset_size(charSet) > maxDColSize){
         maxDColSize = iset_size(charSet);
         maxDCol = i;
      }
      iset_free(charSet);
   }

   return maxDCol;
}

JumpColumn* jcol_create(IntSet* rows, size_t* lengths,char** argv){
   JumpColumn* jcol;
   IntHashtable* jumpsets;
   IntEnumeration* ienum;

   jcol = (JumpColumn*)emalloc(sizeof(JumpColumn));
   jcol->column = findDCol(rows,lengths,argv);
   jcol->transitions = ihtab_createDefault();

   jumpsets = ihtab_createDefault();

   ienum = iset_elements(rows);

   while(ienum_hasNext(ienum)){
      int row = ienum_next(ienum);
      IntSet* rowSet;
      char* curString = argv[row];
      char c = '\0'; 

      if (jcol->column < lengths[row])
         c = curString[jcol->column];

      rowSet = ihtab_get(jumpsets,(int)c);

      if(rowSet == NULL){
         rowSet = iset_createDefault();
         ihtab_put(jumpsets,(int)c,rowSet);
      }
      iset_add(rowSet,row);
   }    
   ienum_free(ienum);

   ienum = ihtab_getKeys(jumpsets);
   while (ienum_hasNext(ienum)){
      char c = (char)ienum_next(ienum);
      IntSet* rowSet= (IntSet*)ihtab_get(jumpsets,(int)c);
      JTrans* jtrans;

      jtrans = (JTrans*)emalloc(sizeof(JTrans));

      if(iset_size(rowSet) == 1){
         IntEnumeration* picker = iset_elements(rowSet);
         jtrans->match = ienum_next(picker);
         ienum_free(picker);
         iset_free(rowSet);
      } else {
         jtrans->rows = rowSet;
         jtrans->match = -1;
      }

      ihtab_put(jcol->transitions,(int)c,jtrans);
   }
   ienum_free(ienum);
   ihtab_free(jumpsets);

   return jcol;
}

void jtab_free(JumpTable* jtab){
   Enumeration* e;

   e=htab_getKeys(jtab->columns);
   while(enum_hasNext(e)) {
      IntSet* iset = (IntSet*)enum_next(e);
      JumpColumn* jcol= htab_get(jtab->columns,iset);
      IntEnumeration* ienum;

      ienum = ihtab_getKeys(jcol->transitions);
      while(ienum_hasNext(ienum))
         free(ihtab_get(jcol->transitions,ienum_next(ienum)));
      ienum_free(ienum);
      ihtab_free(jcol->transitions);
      free(jcol);

      iset_free(iset);
   }
   enum_free(e);

   htab_free(jtab->columns);
   if(jtab->lengths != NULL)
      free(jtab->lengths);
   
   free(jtab);
}
   
JumpTable* jtab_create(int argc, char** argv){
   size_t* lengths;
   int i;
   IntSet* initialRows;
   Stack* stack;
   JumpTable* jtab;
   JumpColumn* initialCol;

   lengths = argc > 0?(size_t*)emalloc(sizeof(size_t)*argc):NULL;
   initialRows = iset_createDefault();

   for(i=0;i<argc;i++){
      lengths[i] = strlen(argv[i]);
      iset_add(initialRows,i);
   }

   jtab = (JumpTable*)emalloc(sizeof(JumpTable));
   jtab->lengths = lengths;
   jtab->argc = argc;
   jtab->argv = argv;
   jtab->columns = htab_create(
      10,
      (unsigned int(*)(void*))iset_hash,
      (int(*)(const void*,const void*))iset_comp,
      0
   );
   jtab->start = initialRows;

   initialCol = jcol_create(initialRows,lengths,argv);
   htab_put(jtab->columns,initialRows,initialCol);
   stack = stack_createDefault();
   stack_push(stack,initialCol);
   
   while (stack_size(stack)>0){
      JumpColumn* jcol = (JumpColumn*)stack_pop(stack);
      IntEnumeration* ienum;

      ienum = ihtab_getKeys(jcol->transitions);
      while(ienum_hasNext(ienum)){
         char c = (char)ienum_next(ienum);
         JTrans* jtrans = ihtab_get(jcol->transitions,(int) c);
        
         if(jtrans->match == -1) {
            JumpColumn* new = jcol_create(jtrans->rows,lengths,argv);

            htab_put(jtab->columns,jtrans->rows,new);
            stack_push(stack,new);

            // printf("\nsize = %d ",iset_size(jtrans->rows));
            // printiset(jtrans->rows);
         }
      }
      ienum_free(ienum);
   }
   stack_free(stack);

   return jtab;
}

MJTable* mjtab_create(JumpTable* jtab){
   Hashtable* mapping;
   Enumeration* e;
   int j,count = 0;
   MJTable* mjt;

   mapping = htab_createDefault();

   e = htab_getKeys(jtab->columns);
   while(enum_hasNext(e)){
      int* j;
      IntSet* iset;
      
      iset = (IntSet*)enum_next(e);
      j = (int*)emalloc(sizeof(int));
      *j = count++;
      htab_put(mapping,iset,j);
   }
   enum_free(e);

   mjt = emalloc(sizeof(MJTable));
   mjt->transitions = (MJTrans*)emalloc((sizeof(MJTrans)*count)<<7);
   mjt->columns = (short*)emalloc(sizeof(short)*count);
   mjt->start = *((int*)htab_get(mapping,jtab->start));
   mjt->lengths = jtab->lengths;
   mjt->argv = jtab->argv;
   mjt->argc = jtab->argc;
   mjt->states = count;

   for (j=0;j<count;j++){
      char c;
      for(c=0;c<127;c++){
         MJTrans* mjtrans = &mjt->transitions[(j<<7)+ c];
         mjtrans->jump=-1;
         mjtrans->match=-1;
      } 
   }

   e = htab_getKeys(jtab->columns);
   while(enum_hasNext(e)){
      int mjcolumn;
      IntSet* iset;
      JumpColumn* jcol;
      IntEnumeration* ienum;

      iset = (IntSet*)enum_next(e);
      mjcolumn = *((int*)htab_get(mapping,iset));
      jcol = htab_get(jtab->columns,iset);
      
      ienum = ihtab_getKeys(jcol->transitions);
      while(ienum_hasNext(ienum)){
         char c = (char)ienum_next(ienum);
         JTrans* jtrans = ihtab_get(jcol->transitions,(int)c);
         MJTrans* mjtrans = &mjt->transitions[(mjcolumn<<7)+c];

         if(jtrans->match == -1){
            mjtrans->jump = *((int*)htab_get(mapping,jtrans->rows));
         } else {
            mjtrans->match = jtrans->match;
         } 
      }
      ienum_free(ienum);
   
      mjt->columns[mjcolumn] = (short)jcol->column;
   }  
   enum_free(e);

   e = htab_getKeys(mapping);
   while(enum_hasNext(e)){
      int* i = (int*)htab_get(mapping,enum_next(e));
      free(i);
   } 
   enum_free(e); 
   htab_free(mapping);

   return mjt;
}

/* 
 * returns -1 if the input string s does not match one of the strings in argv
 * returns the index of the matched string otherwise
 */

inline int mjtab_strmatch(MJTable* mjt,char* s){
   short state=mjt->start, length= strlen(s);

   while(1){
      MJTrans* mjtrans;
      short column;

      column = mjt->columns[state];
      mjtrans = &mjt->transitions[ (state<<7) + (column<length ? s[column] : '\0') ];

      if (mjtrans->match >= 0){
         return (estrcmp(s,mjt->argv[mjtrans->match]) == 0) ? mjtrans->match : -1;
      } else if (mjtrans->jump < 0) {
         return -1;
      } else {
         state = mjtrans->jump;
      }
   }
}

inline int mjtab_strnmatch(MJTable* mjt,char* s ,size_t length){
   short state=mjt->start;

   while(1){
      MJTrans* mjtrans;
      short column;

      column = mjt->columns[state];
      mjtrans = &mjt->transitions[ (state<<7) + (column<length ? s[column] : '\0') ];

      if (mjtrans->match >= 0){
         return (estrcmp(s,mjt->argv[mjtrans->match]) == 0) ? mjtrans->match : -1;
      } else if (mjtrans->jump < 0) {
         return -1;
      } else {
         state = mjtrans->jump;
      }
   }
}

void mjtab_generateCode(MJTable* mjt,StringBuffer* out,char* prefix){
   int i;

   buf_printf(out,"\nstatic char* %s_strings[%d] = {",prefix,mjt->argc);

   for(i=0;i<mjt->argc;i++){
      buf_printf(out,"\n   \"%s\"",mjt->argv[i]);
      if(i+1 != mjt->argc)
         buf_printf(out,",");
   }

   buf_printf(out,"\n};\n");

   buf_printf(out,"\nstatic size_t %s_lengths[%d] = {",prefix,mjt->argc);

   for(i=0;i<mjt->argc;i++){
      if (i%40 == 0) buf_printf (out,"\n   ");
      buf_printf(out,"%d",mjt->lengths[i]);
      if(i+1 != mjt->argc)
         buf_printf(out,",");
   }

   buf_printf(out,"\n};\n");

   buf_printf(out,"\nstatic short %s_columns[%d] = {",prefix,mjt->states);

   for(i=0;i<mjt->states;i++){
      if (i%40 == 0) buf_printf (out,"\n   ");
      buf_printf(out,"%hd",mjt->columns[i]);
      if(i+1 != mjt->states)
         buf_printf(out,",");
   }

   buf_printf(out,"\n};\n");

   buf_printf(out,"\nstatic MJTrans %s_transitions[%d] = {",prefix,(mjt->states<<7));

   for(i=0;i<(mjt->states<<7);i++){
      if (i%5 == 0) buf_printf (out,"\n   ");
      buf_printf(out,"{0x%04hX,0x%04hX}",mjt->transitions[i].jump,mjt->transitions[i].match);
      if(i+1 != (mjt->states<<7))
         buf_printf(out,",");
   }
   
   buf_printf(out,"\n};\n");

   buf_printf(
      out,
      "\nstatic MJTable %s = {%s_transitions,%s_columns,%hd,%s_lengths,%s_strings,%d,%d};\n",
      prefix,
      prefix,
      prefix,
      mjt->start,
      prefix,
      prefix,
      mjt->argc,
      mjt->states
   );

}

/*=============================================================================
// end of file: $RCSfile: jumptable.c,v $
==============================================================================*/

