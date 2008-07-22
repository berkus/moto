/*=============================================================================

   Copyright (C) 2002 Cory Wright
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

   $RCSfile: pgrset.h,v $
   $Revision: 1.2 $
   $Author: shayh $
   $Date: 2003/01/24 16:26:03 $

==============================================================================*/
#ifndef __PGRSET_H
#define __PGRSET_H

#include "pgsqlcommon.h"

/* Creates a default result set with all values set to default */
PGResultSet* pgrset_createDefault(PGConnection *conn, PGresult *result);

/* Checks if the result set has been closed */
void pgrset_checkClosed(PGResultSet *rset);

/* Checks for any database errors on connection */
void pgrset_checkError(PGResultSet *rset);

/* Checks if the column index is within the result set bounds */
void pgrset_checkColumn(PGResultSet *rset, int *column);

/* Checks if the result set has a current row */
void pgrset_checkRow(PGResultSet *rset);

/* Sets cursor to a give row if it exists */
int pgrset_moveToRow(PGResultSet *rset, int row);

/* Moves the cursor offset rows from its current position */
int pgrset_moveRow(PGResultSet *rset, int offset);

/* Moves the cursor to the next row */
int pgrset_next(PGResultSet* rset);

/* Moves the cursor to the previous row */
int pgrset_prev(PGResultSet *rset);

/* Returns the number of rows in the result set */
int pgrset_rows(PGResultSet *rset);

/* Returns the row that the cursor is currently at */
int pgrset_row(PGResultSet *rset);

/* Returns the number of columns present in the result set */
int pgrset_columns(PGResultSet *rset);

/* Gets an entire column as a Vector */
Vector* pgrset_getArray(PGResultSet *rset, int column);
Vector* pgrset_getArrayByName(PGResultSet *rset, char *column);

/* Retrieves a value as a Date object */
Date* pgrset_getDateByColumnName(PGResultSet *rset, char *colname);
Date* pgrset_getDate(PGResultSet *rset, int column);

/* Gets a byte value from the current result */
UnArray* pgrset_getBytesByColumnName(PGResultSet *rset, char *colname);
UnArray* pgrset_getBytes(PGResultSet *rset, int column);

/* Gets a string value from the current result */
char* pgrset_getStringByColumnName(PGResultSet* rset,char* colname);
char* pgrset_getString(PGResultSet* rset,int column);

/* gets a floating point value from the current result */
float pgrset_getFloatByColumnName(PGResultSet* rset,char* colname);
float pgrset_getFloat(PGResultSet* rset,int column);

/* gets a char value from the current result */
char pgrset_getCharByColumnName(PGResultSet* rset,char* colname);
char pgrset_getChar(PGResultSet* rset,int column);

/* Gets an integer value from the current result */
int pgrset_getIntByColumnName(PGResultSet* rset,char* colname);
int pgrset_getInt(PGResultSet* rset,int column);

/* Gets a double value from the current result */
double pgrset_getDoubleByColumnName(PGResultSet* rset,char* colname);
double pgrset_getDouble(PGResultSet* rset,int column);

/* Gets a long value from the current result */
int64_t pgrset_getLongByColumnName(PGResultSet *rset, char *colname);
int64_t pgrset_getLong(PGResultSet *rset, int column);

/* Converts the result set to a tabular data object */
TabularData* pgrset_store(PGResultSet* rset);

/* Returns the column number for a given column name */
char* pgrset_getColumnName(PGResultSet* rset,int col);

/* Returns the column number for a given column name */
int pgrset_getColumnIndex(PGResultSet *rset, char *colname);

/* Gets a given column's type */
char* pgrset_getColumnType(PGResultSet* rset,int col);

/* Closes the current result set. Any further use will result in an exception */
void pgrset_close(PGResultSet *rset);

/* Frees the stored result set */
void pgrset_free(PGResultSet* rset);

#endif

