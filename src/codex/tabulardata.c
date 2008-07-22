/*=============================================================================

   Copyright (C) 2000 David Hakim.
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

   $RCSfile: tabulardata.c,v $
   $Revision: 1.3 $
   $Author: dhakim $
   $Date: 2003/01/09 05:04:12 $

==============================================================================*/


#include <stdio.h>
#include <string.h>

#include "tabulardata.h"
#include "memsw.h"
#include "tokenizer.h"
#include "symboltable.h"
#include "exception.h"
#include "excpfn.h"

excp_declare(TabularDataException);

TabularDataDefinition* tdd_create(int columns, char* colnames, char* coltypes){
   Tokenizer* tok;
   char* cur_column,*cur_type;
   size_t offset;
   int i;

   TabularDataDefinition* tdd = 
      (TabularDataDefinition*)emalloc(sizeof(TabularDataDefinition));

   tdd->columns = columns;
   tdd->column_names = (char**)emalloc(sizeof(char*) * columns);
   tdd->column_types = (int*)emalloc(sizeof(int) * columns);
   tdd->column_offsets = (size_t*)emalloc(sizeof(int) * columns);

   tok = tok_createCTok(colnames,',');

   for (i=0, cur_column = tok_next(tok);cur_column != NULL; i++, cur_column = tok_next(tok)){
      tdd->column_names[i] = cur_column;
   }
   tok_free(tok);

   if (i != columns){
      THROW("TabularDataException","Not enough column names specified");
   }

   tok = tok_createCTok(coltypes,',');
   for (i=0,offset=0,cur_type = tok_next(tok);cur_type != NULL; i++, cur_type = tok_next(tok)){
      if(estrcmp("String",cur_type) == 0){
         tdd->column_types[i] = TABULAR_STRING;
         offset = (tdd->column_offsets[i] = offset) + sizeof(char*);
      } else if (estrcmp("float",cur_type) == 0){
         tdd->column_types[i] = TABULAR_FLOAT;
         offset = (tdd->column_offsets[i] = offset) + sizeof(float);
      } else if (estrcmp("int",cur_type) == 0) {
         tdd->column_types[i] = TABULAR_INT;
         offset = (tdd->column_offsets[i] = offset) + sizeof(int);
      } else if (estrcmp("char",cur_type) == 0) {
         tdd->column_types[i] = TABULAR_CHAR;
         offset = (tdd->column_offsets[i] = offset) + sizeof(char);
      } else if (estrcmp("double",cur_type) == 0) {
         tdd->column_types[i] = TABULAR_DOUBLE;
         offset = (tdd->column_offsets[i] = offset) + sizeof(double);
      } else {
         THROW("TabularDataException","Unknown type '%s'",cur_type);
      }

      free(cur_type);
   }
   tok_free(tok);

   if (i != columns){
      THROW("TabularDataException","Not enough column types specified");
   }

   tdd->row_size=offset;

   return tdd;
}

void tdd_free(TabularDataDefinition* tdd){
   int i;
   for(i=0;i<tdd->columns;i++){
      free(tdd->column_names[i]);
   }
   free(tdd->column_names);
   free(tdd->column_types);
   free(tdd->column_offsets);
   free(tdd);
}

TabularData* tdata_create(int rows, int cols, char* colnames, char* coltypes){
   TabularDataDefinition* tdd = tdd_create(cols,colnames,coltypes);
   TabularData* tdata = (TabularData*)emalloc(sizeof(TabularData));

   tdata->tdd = tdd;
   tdata->rows = rows;
   if(rows == 0)
      tdata->data = NULL;
   else
      tdata->data = ecalloc(rows,tdd->row_size);

   tdata->strings = NULL;
   tdata->strslen = 0;

   return tdata;
}

void tdata_free(TabularData* tdata){
   int i,j;

   for(i=0;i<tdata->rows;i++){
      for(j=0;j<tdata->tdd->columns;j++){
         if(tdata->tdd->column_types[j] == TABULAR_STRING ) {
            char* s = *(char**)((char*)tdata->data + i * tdata->tdd->row_size + tdata->tdd->column_offsets[j]);
            if(s != NULL && !(tdata->strings <= s && s < tdata->strings+tdata->strslen )){
               free(s);
            }
         }
      }
   }

   if (tdata->data != NULL)
      free(tdata->data);
   
   tdd_free(tdata->tdd);
   if(tdata->strings != NULL)
      free(tdata->strings);

   free(tdata);
}

void tdata_optimize(TabularData* tdata){
   char* strings, *strptr, *curstr;
   int strslen=0; /* The space needed to allocate all the strings referred to by this tdata */
   int row,col;

   /* Compute the combined length of all strings */

   for(row=0;row<tdata->rows;row++) {
      for(col=0;col<tdata->tdd->columns;col++) {
         if(tdata->tdd->column_types[col] == TABULAR_STRING){
            curstr = *(char**)((char*)tdata->data + row*tdata->tdd->row_size+ tdata->tdd->column_offsets[col]);
            if(curstr != NULL) {
               strslen += strlen(curstr) + 1;
            }
         }
      }
   }

   if (strslen > 0) strings=(char*)emalloc(strslen); else return;

   strptr = strings ;
   
   /* Copy all the strings into the newly allocated buffer and free the old strings */

   for(row=0;row<tdata->rows;row++) {
      for(col=0;col<tdata->tdd->columns;col++) {
         if(tdata->tdd->column_types[col] == TABULAR_STRING){
            curstr = *(char**)((char*)tdata->data + row*tdata->tdd->row_size+ tdata->tdd->column_offsets[col]);
            if(curstr != NULL) {
               int len = strlen(curstr);
               strncpy(strptr,curstr,len);
               strptr[len] = '\0';
               if(! (tdata->strings < curstr && curstr + len < tdata->strings+ tdata->strslen)) {
                  free(curstr);
               } 
               *(char**)((char*)tdata->data + row*tdata->tdd->row_size+ tdata->tdd->column_offsets[col]) = strptr;
               strptr += len + 1;
            }
         }
      }
   }

   /* If the data had been optimized previously than free the previous buffer */

   if (tdata->strings != NULL)
      free(tdata->strings);

   tdata->strings = strings;
   tdata->strslen = strslen;
   
}

int tdata_getRows(TabularData* tdata){
   return tdata->rows;
}

int tdata_getColumns(TabularData* tdata){
   return tdata->tdd->columns;
}

char* tdata_getColumnName(TabularData* tdata,int col){
   if(col >= tdata->tdd->columns || col < 0) {
      THROW_D("ArrayBoundsException");
   }
   return tdata->tdd->column_names[col];
}

char* tdata_getColumnType(TabularData* tdata,int col){
   if(col >= tdata->tdd->columns || col < 0) {
      THROW_D("ArrayBoundsException");
   }
   switch(tdata->tdd->column_types[col]){
      case TABULAR_INT: return "int";
      case TABULAR_STRING: return "String";
      case TABULAR_CHAR: return "char";
      case TABULAR_FLOAT: return "float";
      case TABULAR_DOUBLE: return "double";
      default: THROW_D("TabularDataException");
   }
   
   return NULL;
}
 
char* tdata_getColumnTypeByColumnName(TabularData* tdata,char* colname){
   int col;

   for(col=0;col<tdata->tdd->columns;col++){
      if(estrcmp(colname,tdata->tdd->column_names[col])==0)
         break;
   }
   if(col >= tdata->tdd->columns){
      THROW("TabularDataException","Column %s not present in tabular data",colname);
   }
   return tdata_getColumnType(tdata,col);
}

void tdata_setInt(TabularData* tdata,int row, int col, int value){
   if (row >= tdata->rows || row < 0) {
      THROW_D("ArrayBoundsException");
   } else if (col >= tdata->tdd->columns || col<0) {
      THROW_D("ArrayBoundsException");
   } else if (tdata->tdd->column_types[col] != TABULAR_INT) {
      THROW("TabularDataException","Column %d cannot hold type 'int'",col);
   }
   *(int*)((char*)tdata->data  + row*tdata->tdd->row_size + tdata->tdd->column_offsets[col]) = value;
}

void tdata_setFloat(TabularData* tdata,int row, int col, float value){
   if (row >= tdata->rows || row < 0) {
      THROW_D("ArrayBoundsException");
   } else if (col >= tdata->tdd->columns || col<0) {
      THROW_D("ArrayBoundsException");
   } else if (tdata->tdd->column_types[col] != TABULAR_FLOAT) {
      THROW("TabularDataException","Column %d cannot hold type 'float'",col);
   }
   *(float*)((char*)tdata->data +row*tdata->tdd->row_size + tdata->tdd->column_offsets[col]) = value;
}

void tdata_setDouble(TabularData* tdata,int row, int col, double value){
   if (row >= tdata->rows || row < 0) {
      THROW_D("ArrayBoundsException");
   } else if (col >= tdata->tdd->columns || col<0) {
      THROW_D("ArrayBoundsException");
   } else if (tdata->tdd->column_types[col] != TABULAR_DOUBLE) {
      THROW("TabularDataException","Column %d cannot hold type 'double'",col);
   }
   *(double*)((char*)tdata->data +row*tdata->tdd->row_size + tdata->tdd->column_offsets[col]) = value;
}

void tdata_setChar(TabularData* tdata,int row, int col, char value){
   if (row >= tdata->rows || row < 0) {
      THROW_D("ArrayBoundsException");
   } else if (col >= tdata->tdd->columns || col<0) {
      THROW_D("ArrayBoundsException");
   } else if (tdata->tdd->column_types[col] != TABULAR_CHAR) {
      THROW("TabularDataException","Column %d cannot hold type 'char'",col);
   }
   *(char*)((char*)tdata->data  + row*tdata->tdd->row_size + tdata->tdd->column_offsets[col]) = value;
}

void tdata_setString(TabularData* tdata,int row, int col, char* value){
   char* newValue;

   if (row >= tdata->rows || row < 0) {
      THROW_D("ArrayBoundsException");
   } else if (col >= tdata->tdd->columns || col<0) {
      THROW_D("ArrayBoundsException");
   } else if (tdata->tdd->column_types[col] != TABULAR_STRING) {
      THROW("TabularDataException","Column %d cannot hold type 'String'",col);
   } 

   newValue=estrdup(value);

   *(char**)((char*)tdata->data + row*tdata->tdd->row_size+ tdata->tdd->column_offsets[col])=newValue;
}

int tdata_getIntByColumnName(TabularData* tdata, int row, char* colname){
   int col;

   for(col=0;col<tdata->tdd->columns;col++){
      if(estrcmp(colname,tdata->tdd->column_names[col])==0)
         break;
   }
   if(col >= tdata->tdd->columns){
      THROW("TabularDataException","Column %s not present in tabular data",colname);
   }
   return tdata_getInt(tdata,row,col);
}

int tdata_getInt(TabularData* tdata, int row, int col){
   int rvalue;

   if (row >= tdata->rows || row < 0) {
      THROW_D("ArrayBoundsException");
   } else if (col >= tdata->tdd->columns || col<0) {
      THROW_D("ArrayBoundsException");
   } else if (tdata->tdd->column_types[col] != TABULAR_INT) {
      THROW("TabularDataException","Column %d cannot hold type 'int'",col);
   }

   rvalue = *(int*)((char*)tdata->data + row*tdata->tdd->row_size+ tdata->tdd->column_offsets[col]);
   return rvalue;
}

float tdata_getFloatByColumnName(TabularData* tdata, int row, char* colname){
   int col;

   for(col=0;col<tdata->tdd->columns;col++){
      if(estrcmp(colname,tdata->tdd->column_names[col])==0)
         break;
   }
   if(col >= tdata->tdd->columns){
      THROW("TabularDataException","Column %s not present in tabular data",colname);
   }
   return tdata_getFloat(tdata,row,col);
}

float tdata_getFloat(TabularData* tdata, int row, int col){
   float rvalue;

   if (row >= tdata->rows || row < 0) {
      THROW_D("ArrayBoundsException");
   } else if (col >= tdata->tdd->columns || col<0) {
      THROW_D("ArrayBoundsException");
   } else if (tdata->tdd->column_types[col] != TABULAR_FLOAT) {
      THROW("TabularDataException","Column %d cannot hold type 'float'",col);
   }

   rvalue = *(float*)((char*)tdata->data + row*tdata->tdd->row_size+ tdata->tdd->column_offsets[col]);
   return rvalue;
}

double tdata_getDoubleByColumnName(TabularData* tdata, int row, char* colname){
   int col;

   for(col=0;col<tdata->tdd->columns;col++){
      if(estrcmp(colname,tdata->tdd->column_names[col])==0)
         break;
   }
   if(col >= tdata->tdd->columns){
      THROW("TabularDataException","Column %s not present in tabular data",colname);
   }
   return tdata_getDouble(tdata,row,col);
}

double tdata_getDouble(TabularData* tdata, int row, int col){
   float rvalue;

   if (row >= tdata->rows || row < 0) {
      THROW_D("ArrayBoundsException");
   } else if (col >= tdata->tdd->columns || col<0) {
      THROW_D("ArrayBoundsException");
   } else if (tdata->tdd->column_types[col] != TABULAR_DOUBLE) {
      THROW("TabularDataException","Column %d cannot hold type 'double'",col);
   }

   rvalue = *(double*)((char*)tdata->data + row*tdata->tdd->row_size+ tdata->tdd->column_offsets[col]);
   return rvalue;
}

char tdata_getCharByColumnName(TabularData* tdata, int row, char* colname){
   int col;

   for(col=0;col<tdata->tdd->columns;col++){
      if(estrcmp(colname,tdata->tdd->column_names[col])==0)
         break;
   }
   if(col >= tdata->tdd->columns){
      THROW("TabularDataException","Column %s not present in tabular data",colname);
   }
   return tdata_getChar(tdata,row,col);
}

char tdata_getChar(TabularData* tdata, int row, int col){
   char rvalue ;

   if (row >= tdata->rows || row < 0) {
      THROW_D("ArrayBoundsException");
   } else if (col >= tdata->tdd->columns || col<0) {
      THROW_D("ArrayBoundsException");
   } else if (tdata->tdd->column_types[col] != TABULAR_CHAR) {
      THROW("TabularDataException","Column %d cannot hold type 'char'",col);
   }

   rvalue = *(char*)((char*)tdata->data + row*tdata->tdd->row_size+ tdata->tdd->column_offsets[col]);
   return rvalue;
}

char* tdata_getStringByColumnName(TabularData* tdata, int row, char* colname){
   int col;

   for(col=0;col<tdata->tdd->columns;col++){
      if(estrcmp(colname,tdata->tdd->column_names[col])==0)
         break;
   }
   if(col >= tdata->tdd->columns){
      THROW("TabularDataException","Column %s not present in tabular data",colname);
   }
   return tdata_getString(tdata,row,col);
}

char* tdata_getString(TabularData* tdata, int row, int col){
   char* rvalue ;

   if (row >= tdata->rows || row < 0) {
      THROW_D("ArrayBoundsException");
   } else if (col >= tdata->tdd->columns || col<0) {
      THROW_D("ArrayBoundsException");
   } else if (tdata->tdd->column_types[col] != TABULAR_STRING) {
      THROW("TabularDataException","Column %d cannot hold type 'String'",col);
   }

   rvalue = *(char**)((char*)tdata->data + row*tdata->tdd->row_size+ tdata->tdd->column_offsets[col]);
   return rvalue;
}
