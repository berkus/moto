/*=============================================================================

   Copyright (C) 2002 Cory Wright and David Hakim.
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

   $RCSfile: pgrset.c,v $
   $Revision: 1.4 $
   $Author: shayh $
   $Date: 2003/01/24 16:26:03 $

==============================================================================*/

#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "memsw.h"
#include "exception.h"
#include "excpfn.h"
#include "pgrset.h"
#include "stringbuffer.h"

#define CHECK_ROW_BOUNDS(a)      \
    if (rset->row == -1) rset->row_num = rset->row = 0; \
    if (rset->row < 0 || rset->row > rset->row_count){ \
      return a; \
    }


PGResultSet*
pgrset_createDefault(PGConnection *conn, PGresult *result)
{
  PGResultSet *rset = (PGResultSet*)emalloc(sizeof(PGResultSet));

  rset->conn          = conn;
  rset->res           = result;
  rset->row           = -1;
  rset->columns       = PQnfields(result); 
  rset->column_names  = (char**)emalloc(sizeof(char*)*rset->columns);
  rset->column_types  = (int*)emalloc(sizeof(int)*rset->columns);
  rset->closed        = 0;
  rset->row_num       = -1;
  rset->row_count     = PQntuples(result);
  rset->col_req       = NULL;
  rset->lengths       = NULL; 

  pgrset_checkError(rset);
  return rset;
}

void
pgrset_checkClosed(PGResultSet *rset)
{
  if (rset->closed == 1)
    THROW("SQLException", "%s", "ResultSet has been closed");
}

void
pgrset_checkError(PGResultSet *rset)
{
  switch(PQresultStatus(rset->res)){
    case PGRES_EMPTY_QUERY:
    case PGRES_COMMAND_OK:
    case PGRES_TUPLES_OK:
    case PGRES_COPY_OUT:
    case PGRES_COPY_IN:
      break;

    case PGRES_BAD_RESPONSE:
    case PGRES_NONFATAL_ERROR:
    case PGRES_FATAL_ERROR:
      THROW("SQLException", "%s", PQresultErrorMessage(rset->res));
  }
}

void
pgrset_checkColumn(PGResultSet *rset, int *column)
{
  if((*column < 0) || (*column >= rset->columns)){
    if (rset->col_req == NULL)
      THROW("ArrayBoundsException", "Column %d does not exist in result set", *column);
    else
      THROW("ArrayBoundsException", "Column \"%s\" does not exist in result set", rset->col_req);
  }

  rset->col_req = NULL;
}

void
pgrset_checkRow(PGResultSet *rset)
{
  pgrset_checkClosed(rset);
  // Have to get next() and prev() to play nicely
  if (rset->row == -1) rset->row_num = rset->row = 0;

  if (rset->row < 0 || rset->row > rset->row_count)
    THROW("ArrayBoundsException", "Row (%d) does not exist in result set", rset->row);
}

int
pgrset_moveToRow(PGResultSet *rset, int row)
{
  pgrset_checkClosed(rset);

  //Allow to set from 0 to n+1
  if (row > rset->row+1 || row < 0)
    THROW("ArrayBoundsException", "Row %d does not exist in result set", row);

  rset->row_num = row;
  rset->row = row;

  return 1;
}

int
pgrset_moveRow(PGResultSet *rset, int offset)
{
  if (offset == 0)
    return 1;

  return pgrset_moveToRow(rset, rset->row_num+offset);
}

int
pgrset_next(PGResultSet* rset)
{
   return ( (rset->row_num = ++rset->row) < rset->row_count );
}

int
pgrset_prev(PGResultSet* rset)
{
   return ( (rset->row_num = --rset->row) > -1 );
}

int
pgrset_columns(PGResultSet* rset)
{
   return rset->columns;
}

int
pgrset_rows(PGResultSet *rset)
{
  return rset->row_count;
}

int
pgrset_row(PGResultSet *rset)
{
  return rset->row_num;
}

char *
pgrset_getColumnName(PGResultSet *rset, int column)
{
   pgrset_checkClosed(rset);
   pgrset_checkColumn(rset, &column);

   return rset->column_names[column];
}

int
pgrset_getColumnIndex(PGResultSet *rset, char *colname)
{
   int col;

   pgrset_checkClosed(rset);
   rset->col_req = colname;

   for(col = 0; col < rset->columns; col++){
      if(estrcasecmp(colname, rset->column_names[col]) == 0)
         break;
   }

   pgrset_checkColumn(rset, &col);
   return col;
}

char *
pgrset_getColumnType(PGResultSet *rset, int column)
{
   pgrset_checkColumn(rset, &column);

   return estrdup(ihtab_get(pgTypes, rset->column_types[column]));
}

char *
pgrset_getStringByColumnName(PGResultSet* rset, char* colname)
{
  return pgrset_getString(rset, pgrset_getColumnIndex(rset, colname));
}

char *
pgrset_getString(PGResultSet* rset, int column)
{
   char *val;
   pgrset_checkRow(rset);
   pgrset_checkColumn(rset, &column);

   CHECK_ROW_BOUNDS(NULL);

   if((val = PQgetvalue(rset->res, rset->row, column)) == NULL)
      return NULL;

   return estrdup(val); 
}

Date *
pgrset_getDateByColumnName(PGResultSet *rset, char *colname)
{
   return pgrset_getDate(rset, pgrset_getColumnIndex(rset, colname));}

Date *
pgrset_getDate(PGResultSet *rset, int column)
{
   char format[20] = {'\0'};
   char *type;

   pgrset_checkRow(rset);
   pgrset_checkColumn(rset, &column);

   CHECK_ROW_BOUNDS(NULL);

   type = ihtab_get(pgTypes, rset->column_types[column]);

   if (!estrcmp(type, "abstime")){
      memcpy(format, "%Y-%m-%d %H:%M:%S%Z", 19);
   }
   else if (!estrcmp(type, "date")){
      memcpy(format, "%Y-%m-%d", 8);
   }
   else if (!estrcmp(type, "time")){
      memcpy(format, "%H:%M:%S", 8);
   }
   else if (!estrcmp(type, "timestamp")){
      memcpy(format, "%Y-%m-%d %H:%M:%S", 17);
   }
   else if (!estrcmp(type, "timetz")){
      memcpy(format, "%H:%M:%S%Z", 10);
   }

   return date_create(format, PQgetvalue(rset->res, rset->row, column));
}

UnArray* pgrset_getBytesByColumnName(PGResultSet *rset, char *colname){
   return pgrset_getBytes(rset, pgrset_getColumnIndex(rset, colname));
}
   
UnArray* pgrset_getBytes(PGResultSet *rset, int column){
   UnArray* arr;
   int len;

   pgrset_checkRow(rset);
   pgrset_checkColumn(rset, &column);

   CHECK_ROW_BOUNDS(NULL);

   len = PQgetlength(rset->res, rset->row, column);
   arr = arr_create(1, 0, 0, &len, ARR_BYTE_TYPE);
   memcpy(&arr->ya.data[0], PQgetvalue(rset->res, rset->row, column), len);

   return arr;
}


int
 pgrset_getIntByColumnName(PGResultSet *rset, char *colname)
{
  return pgrset_getInt(rset, pgrset_getColumnIndex(rset, colname));
}

/* if the value is SQL NULL, the value returned is 0 */
int
pgrset_getInt(PGResultSet *rset, int column)
{
   char *val;
   pgrset_checkRow(rset);
   pgrset_checkColumn(rset, &column);

   CHECK_ROW_BOUNDS(0);

   if((val = PQgetvalue(rset->res, rset->row, column)) == NULL)
      return 0;

   return (int)strtol(val, (char**)NULL, 10);
}

float
pgrset_getFloatByColumnName(PGResultSet *rset, char *colname)
{
  return pgrset_getFloat(rset, pgrset_getColumnIndex(rset, colname));
}

float
pgrset_getFloat(PGResultSet* rset,int column)
{
   char *val;
   pgrset_checkRow(rset);
   pgrset_checkColumn(rset, &column);

   CHECK_ROW_BOUNDS(0);

   if((val = PQgetvalue(rset->res, rset->row, column)) == NULL)
      return 0;

   return (float)strtod(val, (char**)NULL);
}

double
pgrset_getDoubleByColumnName(PGResultSet *rset, char *colname)
{
  return pgrset_getDouble(rset, pgrset_getColumnIndex(rset, colname));
}

double
pgrset_getDouble(PGResultSet *rset, int column)
{
   char *val;
   pgrset_checkRow(rset);
   pgrset_checkColumn(rset, &column);

   CHECK_ROW_BOUNDS(0);

   if((val = PQgetvalue(rset->res, rset->row, column)) == NULL)
      return 0;

   return strtod(val, (char**)NULL);
}

/* Gets a long value from the current result */
int64_t
pgrset_getLongByColumnName(PGResultSet *rset, char *colname)
{
   return pgrset_getLong(rset, pgrset_getColumnIndex(rset, colname));
}

int64_t
pgrset_getLong(PGResultSet *rset, int column)
{
   char *val;
   pgrset_checkRow(rset);
   pgrset_checkColumn(rset, &column);

   CHECK_ROW_BOUNDS(0);

   if((val = PQgetvalue(rset->res, rset->row, column)) == NULL)
      return 0;

   return (int64_t)strtoll(val, (char**)NULL, 10);
}


char pgrset_getCharByColumnName(PGResultSet *rset, char *colname){
  return pgrset_getChar(rset, pgrset_getColumnIndex(rset, colname));
}

char
pgrset_getChar(PGResultSet *rset, int column)
{
   char *val;
   pgrset_checkRow(rset);
   pgrset_checkColumn(rset, &column);

   CHECK_ROW_BOUNDS('\0');

   if((val = PQgetvalue(rset->res, rset->row, column)) == NULL)
      return 0;

   return ((char*)val)[0];
}

//FIXME. NOT TESTED
Vector *
pgrset_getArray(PGResultSet *rset, int column)
{
  Vector *vec = vec_create(pgrset_rows(rset), NULL);
  int cursor;

  pgrset_checkClosed(rset);
  
  //Get current cursor position
  cursor = rset->row;
  rset->row = -1;
  pgrset_checkColumn(rset, &column);
  
  while(pgrset_next(rset))
    vec_add(vec, PQgetvalue(rset->res, rset->row, column));

  //Place cursor back where it was
  rset->row = cursor;

  return vec;
}

Vector *
pgrset_getArrayByName(PGResultSet *rset, char *column)
{
  return pgrset_getArray(rset, pgrset_getColumnIndex(rset, column));
}

void
pgrset_free(PGResultSet *rset)
{
   int i;
   for(i = 0; i < rset->columns; i++)
      free(rset->column_names[i]);

   free(rset->column_names);
   free(rset->column_types);

   if (rset->closed != 1)
    if (rset->res != NULL)
      PQclear(rset->res);

   free(rset);
}

void
pgrset_close(PGResultSet *rset)
{
  if (rset->closed != 1){
    rset->closed = 1;
    PQclear(rset->res);
    rset->res = NULL;
  }
}


TabularData *
pgrset_store(PGResultSet *rset)
{
   int i, j, columns, rows;
   StringBuffer *colnames; 
   StringBuffer *coltypes;
   TabularData *tdata;
   char *pgtype;

   columns  = rset->columns;
   rows     = rset->row_count;

   colnames = buf_createDefault();
   coltypes = buf_createDefault();
   
   for (i = 0; i < columns; i++){
    pgtype = ihtab_get(pgTypes, rset->column_types[i]);
      if(
        estrcmp(pgtype, "int2") == 0 ||
        estrcmp(pgtype, "int4") == 0 ||
        estrcmp(pgtype, "oid")  == 0
      )
         buf_puts(coltypes, "int");
      else if (
        estrcmp(pgtype, "double") == 0
      )
         buf_puts(coltypes, "double");
      else if (
        estrcmp(pgtype, "float4") == 0 ||
        estrcmp(pgtype, "float8") == 0
      )
         buf_puts(coltypes, "float");
      else if (
        estrcmp(pgtype, "name")    == 0 ||
        estrcmp(pgtype, "text")    == 0 ||
        estrcmp(pgtype, "varchar") == 0
      )
         buf_puts(coltypes, "String");
      else {
         buf_free(coltypes);
         buf_free(colnames);
         THROW("SQLException", "Cannot store field '%s'", rset->column_names[i]);
      }

      buf_puts(colnames, rset->column_names[i]);

      if(i < columns-1){
         buf_putc(colnames, ',');
         buf_putc(coltypes, ',');
      }
   }

   tdata = tdata_create(rows, columns, buf_data(colnames), buf_data(coltypes));

   buf_free(coltypes);
   buf_free(colnames);

   rset->row=-1;
   for (i = 0, pgrset_next(rset); i < rows; i++, pgrset_next(rset)){
      for (j = 0; j < columns; j++){
         pgtype = ihtab_get(pgTypes, rset->column_types[j]);

         if(
            estrcmp(pgtype, "int2") == 0 ||
            estrcmp(pgtype, "int4") == 0 ||
            estrcmp(pgtype, "oid")  == 0
         )
            tdata_setInt(tdata, i, j, pgrset_getInt(rset, j));
         else if (
            estrcmp(pgtype, "float4") == 0 ||
            estrcmp(pgtype, "float8") == 0
         )
            tdata_setFloat(tdata, i, j, pgrset_getFloat(rset, j));
         else if (
            estrcmp(pgtype, "double") == 0
         )
            tdata_setDouble(tdata, i, j, pgrset_getDouble(rset, j));
         else if (
            estrcmp(pgtype, "name") == 0 ||
            estrcmp(pgtype, "text") == 0 ||
            estrcmp(pgtype, "varchar") == 0
         )
            tdata_setString(tdata,i,j,pgrset_getString(rset,j));
         else {
            THROW_D("Exception");
         }
      }
   }

   rset->row = -1;
   tdata_optimize(tdata);

   return tdata;
}

