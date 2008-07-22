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

   $RCSfile: tabulardata.h,v $
   $Revision: 1.3 $
   $Author: dhakim $
   $Date: 2003/01/09 05:04:12 $

==============================================================================*/

#ifndef __TABULAR_H
#define __TABULAR_H

#include <sys/types.h>
#include "exception.h"

typedef struct tabularDataDefinition {
   int columns;
   char** column_names;
   int* column_types;
   size_t* column_offsets;
   size_t row_size; /* in bytes */
} TabularDataDefinition;

typedef struct tabularData {
   TabularDataDefinition* tdd;
   int rows;
   void* data;
   char* strings;
   int strslen;
} TabularData;

enum TabularTypes {TABULAR_STRING, TABULAR_INT, TABULAR_CHAR, TABULAR_FLOAT, TABULAR_DOUBLE};

excp_extern(TabularDataException);

TabularDataDefinition* tdd_create(int columns, char* colnames, char* coltypes);
void tdd_free(TabularDataDefinition* tdd);
TabularData* tdata_create(int rows, int cols, char* colnames, char* coltypes);
void tdata_free(TabularData* tdata);
void tdata_optimize(TabularData* tdata);

void tdata_setInt(TabularData* tdata,int row, int col, int value);
void tdata_setFloat(TabularData* tdata,int row, int col, float value);
void tdata_setChar(TabularData* tdata,int row, int col, char value);
void tdata_setString(TabularData* tdata,int row, int col, char* value);
void tdata_setDouble(TabularData* tdata,int row, int col, double value);

int tdata_getIntByColumnName(TabularData* tdata, int row, char* col);
float tdata_getFloatByColumnName(TabularData* tdata, int row, char* col);
char tdata_getCharByColumnName(TabularData* tdata, int row, char* col);
char* tdata_getStringByColumnName(TabularData* tdata, int row, char* col);
double tdata_getDoubleByColumnName(TabularData* tdata, int row, char* col);

int tdata_getInt(TabularData* tdata, int row, int col);
float tdata_getFloat(TabularData* tdata, int row, int col);
char tdata_getChar(TabularData* tdata, int row, int col);
char* tdata_getString(TabularData* tdata, int row, int col);
double tdata_getDouble(TabularData* tdata, int row, int col);

int tdata_getRows(TabularData* tdata);
int tdata_getColumns(TabularData* tdata);
char* tdata_getColumnName(TabularData* tdata,int col);
char* tdata_getColumnType(TabularData* tdata,int col);
char* tdata_getColumnTypeByColumnName(TabularData* tdata,char* colname);

#endif
