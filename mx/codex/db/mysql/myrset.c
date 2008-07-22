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

   $RCSfile: myrset.c,v $
   $Revision: 1.10 $
   $Author: dhakim $
   $Date: 2003/02/01 01:10:55 $

==============================================================================*/

#include "myrset.h"


ResultSet* myrset_createDefault(Connection *conn, MYSQL_RES *result){
   ResultSet *rset = (ResultSet*)emalloc(sizeof(ResultSet));

   rset->conn         = conn;
   rset->res          = result;
   rset->row          = NULL;
   rset->columns      = mysql_num_fields(result);
   rset->column_names = (char**)ecalloc(rset->columns, sizeof(char*));
   rset->column_types = (int*)ecalloc(rset->columns, sizeof(int));
   rset->closed       = 0;
   rset->row_num      = -1;
   rset->row_count    = mysql_num_rows(result);
   rset->col_req      = NULL; 

   myrset_checkError(rset);
   return rset;
}

void myrset_checkError(ResultSet *rset){
   if (mysql_errno(rset->conn->stream) != 0)
      THROW("SQLException","%s", GET_SQL_ERROR(rset->conn));
}

void myrset_checkColumn(ResultSet *rset, int *column){
   if((*column < 0) || (*column >= rset->columns)){
      if (rset->col_req == NULL)
         THROW("ArrayBoundsException", "Column %d does not exist in result set", *column);
      else
         THROW("ArrayBoundsException", "Column \"%s\" does not exist in result set", rset->col_req);
   }

   rset->col_req = NULL;
}

void myrset_checkRow(ResultSet *rset){
   if (rset->row == NULL)
      THROW("ArrayBoundsException", "Row (null) does not exist in result set");
}


int myrset_moveToRow(ResultSet *rset, int row){
   CHECK_CLOSED(rset)

	//Allow to set from 0 to n+1
   if ((row > myrset_rows(rset)+1) || (row < 0))
      THROW("ArrayBoundsException", "Row %d does not exist in result set", row);

   mysql_data_seek(rset->res, row);
   rset->row_num = row;
   myrset_checkError(rset);

   return 1;
}

int myrset_moveRow(ResultSet *rset, int offset){
   if (offset == 0)
      return 1;

   return myrset_moveToRow(rset, rset->row_num+offset);
}

int myrset_next(ResultSet *rset){
   CHECK_CLOSED(rset)

   rset->row = mysql_fetch_row(rset->res);
   rset->lengths = mysql_fetch_lengths(rset->res);
   rset->row_num++;

   return (rset->row) != NULL;
}

int myrset_prev(ResultSet *rset){
   if (rset->row_num == 0)
      return 0;

   myrset_moveRow(rset, -1);
   rset->row = mysql_fetch_row(rset->res);
   rset->lengths = mysql_fetch_lengths(rset->res);

   return (rset->row) != NULL;
}

int myrset_columns(ResultSet *rset){
   return rset->columns;
}

int myrset_rows(ResultSet *rset) {
   return rset->row_count;
}

int myrset_row(ResultSet *rset){
   return rset->row_num;
}

char* myrset_getColumnName(ResultSet *rset, int column){
   CHECK_CLOSED(rset)
   myrset_checkColumn(rset, &column);

   return rset->column_names[column]; 
}

int myrset_getColumnIndex(ResultSet *rset, char *colname){
   int col;

   CHECK_CLOSED(rset)
   rset->col_req = colname;

   for(col = 0; col < rset->columns; col++){
      if(estrcasecmp(colname, rset->column_names[col]) == 0)
         break;
   }

   myrset_checkColumn(rset, &col);
   return col;
}

char* myrset_getColumnType(ResultSet *rset, int column){
   myrset_checkColumn(rset, &column);

   switch(rset->column_types[column]){
      case FIELD_TYPE_BLOB:
         return estrdup("BLOB");
      case FIELD_TYPE_CHAR:
         return estrdup("CHAR");
      case FIELD_TYPE_DECIMAL:
         return estrdup("DECIMAL");
      case FIELD_TYPE_DATE:
         return estrdup("DATE");
      case FIELD_TYPE_DATETIME:
         return estrdup("DATETIME");
      case FIELD_TYPE_DOUBLE:
         return estrdup("DOUBLE");
      case FIELD_TYPE_FLOAT:
         return estrdup("FLOAT");
      case FIELD_TYPE_INT24:
         return estrdup("INT24");
      case FIELD_TYPE_LONGLONG:
         return estrdup("LONGLONG");
      case FIELD_TYPE_LONG_BLOB:
         return estrdup("LONG_BLOB");
      case FIELD_TYPE_LONG:
         return estrdup("LONG");
      case FIELD_TYPE_MEDIUM_BLOB:
         return estrdup("MEDIUM_BLOB");
      case FIELD_TYPE_NULL:
         return estrdup("NULL");
      case FIELD_TYPE_SHORT:
         return estrdup("SHORT");
      case FIELD_TYPE_STRING:
         return estrdup("STRING");
      case FIELD_TYPE_TIME:
         return estrdup("TIME");
      case FIELD_TYPE_TIMESTAMP:
         return estrdup("TIMESTAMP");
      case FIELD_TYPE_TINY_BLOB:
         return estrdup("TINY_BLOB");
      case FIELD_TYPE_VAR_STRING:
         return estrdup("VAR_STRING");
      default:
         return estrdup("???");
   }
}

char* myrset_getStringByColumnName(ResultSet *rset, char *colname){
   return myrset_getString(rset, myrset_getColumnIndex(rset, colname));
}

char* myrset_getString(ResultSet *rset, int column){
   myrset_checkRow(rset);
   myrset_checkColumn(rset, &column);

   CHECK_ROW_BOUNDS(NULL)

   return estrdup(rset->row[column]);
}

Date* myrset_getDateByColumnName(ResultSet *rset, char *colname){
   return myrset_getDate(rset, myrset_getColumnIndex(rset, colname));
}

Date* myrset_getDate(ResultSet *rset, int column){
   char format[18] = {'\0'};

   myrset_checkRow(rset);
   myrset_checkColumn(rset, &column);

   CHECK_ROW_BOUNDS(NULL)

   switch(rset->column_types[column]){
      case FIELD_TYPE_TIMESTAMP:
         if (rset->lengths[column] == 14)
            memcpy(format, "%Y%m%d%H%M%S", 12);
         else if (rset->lengths[column] == 12)
            memcpy(format, "%y%m%d%H%M%S", 12);
         else if (rset->lengths[column] == 8)
            memcpy(format, "%Y%m%d", 6);
         else if (rset->lengths[column] == 6)
            memcpy(format, "%y%m%d", 6);
         else
            THROW("SQLException","");

         break;

      case FIELD_TYPE_DATE:
         memcpy(format, "%Y-%m-%d", 8);
         break;

      case FIELD_TYPE_TIME:
         memcpy(format, "%H:%M:%S", 8);
         break;

      case FIELD_TYPE_DATETIME:
         memcpy(format, "%Y-%m-%d %H:%M:%S", 17);
         break;

      case FIELD_TYPE_YEAR:
      {
         Date *date = date_createDefault();

         if (rset->lengths[column] == 2)
            date->tm_year = ((int)strtol(rset->row[column], (char**)NULL, 10))+100;
         else
            date->tm_year = ((int)strtol(rset->row[column], (char**)NULL, 10))-1900;

         return date;
      }

      break;

      default:
         THROW("IllegalConversionException", "MySQL column type is not a DATE/TIME");
   }

   return date_create(format, rset->row[column]);
}

/* gets a byte[] value from the current result */
UnArray* myrset_getBytesByColumnName(ResultSet *rset, char *colname){
   return myrset_getBytes(rset, myrset_getColumnIndex(rset, colname));
}

UnArray* myrset_getBytes(ResultSet *rset, int column){
   UnArray* arr;

   myrset_checkRow(rset);
   myrset_checkColumn(rset, &column);

   CHECK_ROW_BOUNDS(NULL)

   arr = arr_create(1, 0, 0, (int*)&rset->lengths[column], ARR_BYTE_TYPE);
   memcpy(&arr->ya.data[0], rset->row[column], rset->lengths[column]);
   return arr;
}

/* gets an integer value from the current result */
int myrset_getIntByColumnName(ResultSet *rset, char *colname){
   return myrset_getInt(rset, myrset_getColumnIndex(rset, colname));
}

/* if the value is SQL NULL, the value returned is 0 */
int myrset_getInt(ResultSet *rset,int column){
   myrset_checkRow(rset);
   myrset_checkColumn(rset, &column);

   CHECK_ROW_BOUNDS(0)

   return (int)strtol(rset->row[column], (char**)NULL, 10);
}

/* gets a floating point value from the current result */
float myrset_getFloatByColumnName(ResultSet *rset, char *colname){
   return myrset_getFloat(rset, myrset_getColumnIndex(rset, colname));
}

float myrset_getFloat(ResultSet *rset, int column){
   myrset_checkRow(rset);
   myrset_checkColumn(rset, &column);

   CHECK_ROW_BOUNDS(0)

   return (float)strtod(rset->row[column], (char**)NULL);
}

/* gets a double floating point value from the current result */
double myrset_getDoubleByColumnName(ResultSet *rset, char *colname){
   return myrset_getDouble(rset, myrset_getColumnIndex(rset, colname));
}

double myrset_getDouble(ResultSet *rset, int column){
   myrset_checkRow(rset);
   myrset_checkColumn(rset, &column);

   CHECK_ROW_BOUNDS(0)

   return strtod(rset->row[column], (char**)NULL);
}

/* Gets a long value from the current result */
int64_t myrset_getLongByColumnName(ResultSet *rset, char *colname){
   return myrset_getLong(rset, myrset_getColumnIndex(rset, colname));
}

int64_t myrset_getLong(ResultSet *rset, int column){
   myrset_checkRow(rset);
   myrset_checkColumn(rset, &column);

   CHECK_ROW_BOUNDS(0)

   return (int64_t)strtoll(rset->row[column], (char**)NULL, 10);
}

/* gets a char value from the current result */
char myrset_getCharByColumnName(ResultSet *rset, char *colname){
   return myrset_getChar(rset, myrset_getColumnIndex(rset, colname));
}

char myrset_getChar(ResultSet *rset, int column){
   myrset_checkRow(rset);
   myrset_checkColumn(rset, &column);

   CHECK_ROW_BOUNDS('\0')

   return ((char*)(rset->row[column]))[0];
}

/* Gets a Vector for a given column */
Vector* myrset_getArray(ResultSet *rset, int column){
  MYSQL_ROW_OFFSET cursor;
  Vector *vec = vec_create(myrset_rows(rset), NULL);

  CHECK_CLOSED(rset)
  
  //Get current cursor position
  cursor = mysql_row_tell(rset->res);
  myrset_moveToRow(rset, 0);
  myrset_checkColumn(rset, &column);
  
  while(myrset_next(rset))
    vec_add(vec, rset->row[column]);

  //Place cursor back where it was
  mysql_row_seek(rset->res, cursor);

  return vec;
}

Vector* myrset_getArrayByName(ResultSet *rset, char *column){
   return myrset_getArray(rset, myrset_getColumnIndex(rset, column));
}


/* frees the stored result set */
void myrset_free(ResultSet *rset){
   int i;
   for(i = 0; i < rset->columns; i++)
      free(rset->column_names[i]);

   free(rset->column_names);
   free(rset->column_types);
   if (rset->closed != 1)
      if (rset->res != NULL)
         mysql_free_result(rset->res);
   
   free(rset);
}

void myrset_close(ResultSet *rset){
   if (rset->closed != 1){
  	   rset->closed = 1;
  	   mysql_free_result(rset->res);
      rset->res = NULL;
  	   myrset_checkError(rset);
	}
}


/* Gets a TabularData object for the result set */
TabularData* myrset_store(ResultSet *rset){
   int i, j, columns, rows;
   StringBuffer *colnames;
   StringBuffer *coltypes;
   TabularData  *tdata;
   
   columns = rset->columns;
   rows = myrset_rows(rset);
   
   colnames = buf_createDefault();
   coltypes = buf_createDefault();
   
   for (i = 0; i < columns; i++){
      switch(rset->column_types[i]){
         case FIELD_TYPE_DECIMAL:
         case FIELD_TYPE_TINY:
         case FIELD_TYPE_SHORT:
         case FIELD_TYPE_LONG:
         case FIELD_TYPE_LONGLONG:
         case FIELD_TYPE_INT24:
            buf_puts(coltypes, "int");
         break;
         case FIELD_TYPE_DOUBLE:
            buf_puts(coltypes, "double");
         break;
         case FIELD_TYPE_FLOAT:
            buf_puts(coltypes, "float");
         break;
         case FIELD_TYPE_STRING:
         case FIELD_TYPE_VAR_STRING:
         case FIELD_TYPE_TINY_BLOB:
         case FIELD_TYPE_MEDIUM_BLOB:
         case FIELD_TYPE_LONG_BLOB:
         case FIELD_TYPE_BLOB:
            buf_puts(coltypes, "String");
         break;
         default:
            buf_free(coltypes);
            buf_free(colnames);
            THROW("SQLException","Cannot store field '%s'", rset->column_names[i]);
         break;
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

   mysql_data_seek(rset->res,0);

   for (i = 0, myrset_next(rset); i < rows; i++, myrset_next(rset)){
      for (j = 0; j < columns; j++){
         switch(rset->column_types[j]){
            case FIELD_TYPE_DECIMAL:
            case FIELD_TYPE_TINY:
            case FIELD_TYPE_SHORT:
            case FIELD_TYPE_LONG:
            case FIELD_TYPE_LONGLONG:
            case FIELD_TYPE_INT24:
               tdata_setInt(tdata, i, j, myrset_getInt(rset, j));
            break;
            case FIELD_TYPE_FLOAT:
               tdata_setFloat(tdata, i, j, myrset_getFloat(rset, j));
            break;
            case FIELD_TYPE_DOUBLE:
               tdata_setDouble(tdata, i, j, myrset_getDouble(rset, j));
            break;
            case FIELD_TYPE_STRING:
            case FIELD_TYPE_VAR_STRING:
            case FIELD_TYPE_TINY_BLOB:
            case FIELD_TYPE_MEDIUM_BLOB:
            case FIELD_TYPE_LONG_BLOB:
            case FIELD_TYPE_BLOB:
               tdata_setString(tdata, i, j, myrset_getString(rset, j));
            break;
            default:
              THROW("SQLException","Cannot store data for column %d", j);
            break;
         }
      }
   }

   mysql_data_seek(rset->res,0);
   tdata_optimize(tdata);

   return tdata;
}

