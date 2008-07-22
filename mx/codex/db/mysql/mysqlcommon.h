#ifndef __MYSQLCOMMON_H
#define __MYSQLCOMMON_H

#include <mysql.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "exception.h"
#include "excpfn.h"
#include "mxarr.h"
#include "memsw.h"
#include "tabulardata.h"
#include "vector.h"
#include "cdate.h"
#include "stringbuffer.h"


typedef MYSQL*       SQLsock;
typedef MYSQL_RES*   SQLresult;
typedef MYSQL_ROW    SQLrow;

typedef struct {
   SQLsock stream;
   SQLsock (*init_fn)(void*);
   void (*close_fn)(SQLsock);
   char* (*error_fn)(SQLsock);
} MySQLConnection, Connection;


typedef struct {
   MySQLConnection *conn;
   int closed;    // Flag to easily tell if result set was closed
   int columns;   // Number of columns in result set
   int row_num;   // Current row cursor is at
   int row_count; // Store row count
   char *col_req; // Stores a column name. Used for correctly reporting errors.
   char **column_names;
   int *column_types;
   SQLresult res;
   SQLrow  row;
   unsigned long *lengths;
} MySQLResultSet, ResultSet;


#define DISCONNECT(connection)   \
   connection->close_fn(connection->stream); \
   free(connection); \


#define CHECK_ROW_BOUNDS(a) \
   if (rset->row[column] == NULL) \
      return a; \


#define CHECK_CLOSED(rset) \
   if (rset->closed == 1) \
      THROW("SQLException","ResultSet has been closed");


#define GET_SQL_ERROR(connection)   \
   connection->error_fn(connection->stream)


#endif



