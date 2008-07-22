#ifndef __PGSQLCOMMON_H
#define __PGSQLCOMMON_H

#include <libpq-fe.h>
#include "tabulardata.h"
#include "inthashtable.h"
#include "mxarr.h"
#include "vector.h"
#include "cdate.h"

extern IntHashtable *pgTypes;

typedef struct {
  PGconn *stream;
} PGConnection;

typedef struct {
   PGConnection *conn;
   int closed;    // Flag to easily tell if result set was closed
   int columns;   // Number of columns in result set
   int row_num;   // Current row cursor is at
   int row_count; // Store row count
   char *col_req; // Stores a column name. Used for correctly reporting errors.
   char **column_names;
   int *column_types;
   PGresult *res;
   int row;
   unsigned long *lengths;
} PGResultSet;

#endif

