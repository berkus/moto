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

   $RCSfile: myrset.h,v $
   $Revision: 1.5 $
   $Author: shayh $
   $Date: 2003/01/27 16:03:44 $

==============================================================================*/
#ifndef __MYRSET_H
#define __MYRSET_H

#include "mysqlcommon.h"

/* Creates a default result set with all values set to default */
ResultSet* myrset_createDefault(Connection *conn, MYSQL_RES *result);

/* Checks if the result set has been closed */
void myrset_checkClosed(ResultSet *rset);

/* Checks for any database errors on connection */
void myrset_checkError(ResultSet *rset);

/* Checks if the column index is within the result set bounds */
void myrset_checkColumn(ResultSet *rset, int *column);

/* Checks if the result set has a current row */
void myrset_checkRow(ResultSet *rset);

/* Sets cursor to a give row if it exists */
int myrset_moveToRow(ResultSet *rset, int row);

/* Moves the cursor offset rows from its current position */
int myrset_moveRow(ResultSet *rset, int offset);

/* Moves the cursor to the next row */
int myrset_next(ResultSet *rset);

/* Moves the cursor to the previous row */
int myrset_prev(ResultSet *rset);

/* Returns the number of rows in the result set */
int myrset_rows(ResultSet *rset);

/* Returns the row that the cursor is currently at */
int myrset_row(ResultSet *rset);

/* Returns the number of columns present in the result set */
int myrset_columns(ResultSet *rset);

/* Gets an entire column as a Vector */
Vector* myrset_getArray(ResultSet *rset, int column);
Vector* myrset_getArrayByName(ResultSet *rset, char *column);

/* Retrieves a value as a Date object */
Date* myrset_getDateByColumnName(ResultSet *rset, char *colname);
Date* myrset_getDate(ResultSet *rset, int column);

/* Gets a byte value from the current result */
UnArray* myrset_getBytesByColumnName(ResultSet *rset, char *colname);
UnArray* myrset_getBytes(ResultSet *rset, int column);

/* Gets a string value from the current result */
char* myrset_getStringByColumnName(ResultSet *rset, char *colname);
char* myrset_getString(ResultSet *rset, int column);

/* gets a floating point value from the current result */
float myrset_getFloatByColumnName(ResultSet *rset, char *colname);
float myrset_getFloat(ResultSet *rset, int column);

/* gets a char value from the current result */
char myrset_getCharByColumnName(ResultSet *rset, char *colname);
char myrset_getChar(ResultSet *rset, int column);

/* Gets an integer value from the current result */
int myrset_getIntByColumnName(ResultSet *rset, char *colname);
int myrset_getInt(ResultSet *rset, int column);

/* Gets a double value from the current result */
double myrset_getDoubleByColumnName(ResultSet *rset, char *colname);
double myrset_getDouble(ResultSet *rset, int column);

/* Gets a long value from the current result */
int64_t myrset_getLongByColumnName(ResultSet *rset, char *colname);
int64_t myrset_getLong(ResultSet *rset, int column);

/* Converts the result set to a tabular data object */
TabularData* myrset_store(ResultSet *rset);

/* Gets the column number for a given column name */
char* myrset_getColumnName(ResultSet *rset, int col);

/* Returns the column number for a given column name */
int myrset_getColumnIndex(ResultSet *rset, char *colname);

/* Gets a given column's type */
char* myrset_getColumnType(ResultSet *rset, int col);

/* Closes the current result set. Any further use will result in an exception */
void myrset_close(ResultSet *rset);

/* Frees the stored result set */
void myrset_free(ResultSet *rset);

#endif

