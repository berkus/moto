/*=============================================================================

   Copyright (C) 2002 Cory Wright and David Hakim
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

   $RCSfile: pgconn.c,v $
   $Revision: 1.4 $
   $Author: shayh $
   $Date: 2003/01/24 16:26:03 $

==============================================================================*/

#include <sys/time.h>
#include <stdlib.h>
#include <string.h>

#include "pgconn.h"
#include "pgrset.h"
#include "memsw.h"
#include "exception.h"
#include "excpfn.h"
#include "stringbuffer.h"

IntHashtable* pgTypes = NULL;
static Oid lastOid;

PGConnection *
pgconn_createDefault()
{
   return NULL;
}


PGConnection *
pgconn_create(const char *host, const char *db, const char *user, const char *password)
{
   PGConnection *conn;

   if (db == NULL)
      THROW("SQLException", "Database may not be (null)");

   conn = emalloc(sizeof(PGConnection));
   conn->stream = PQsetdbLogin(host,"5432",NULL,NULL,db,user,password);

   if (PQstatus(conn->stream) != CONNECTION_OK)
      THROW("SQLException", "%s", PQerrorMessage(conn->stream));

   if(pgTypes == NULL){
      PGResultSet *rset = pgconn_query(conn, "SELECT oid,typname FROM pg_type");
      pgTypes = ihtab_createDefault();

      while(pgrset_next(rset)){
        ihtab_put(pgTypes, pgrset_getInt(rset, 0), pgrset_getString(rset, 1));
      }
   }
   
   return conn;
}


void 
pgconn_free(PGConnection *conn)
{
   PQfinish(conn->stream);
   free(conn);
}

/* function to call for inserts or updates, returns # of rows affected */
int
pgconn_update(PGConnection *conn, const char *query)
{
   int rowsModified;
   PGresult *res;

   res = PQexec(conn->stream,query);

   if(res == NULL)
     THROW("SQLException", "%s", PQerrorMessage(conn->stream));

   if(PQresultStatus(res) != PGRES_COMMAND_OK)
     THROW("SQLException", "%s", PQresultErrorMessage(res));

   rowsModified = strtol(PQcmdTuples(res), NULL, 10);
   lastOid = PQoidValue(res);
   PQclear(res);

   return rowsModified;
}


PGResultSet *
pgconn_query(PGConnection *conn, const char *query)
{
   int i;
   PGresult *result;
   PGResultSet *rset;

   result = PQexec(conn->stream, query);

   if (result == NULL)
      THROW("SQLException", "%s", PQerrorMessage(conn->stream));

   if(PQresultStatus(result) != PGRES_TUPLES_OK)
      THROW("SQLException", "%s", PQresultErrorMessage(result));

   rset = pgrset_createDefault(conn, result);

   for(i = 0; i < rset->columns; i++){
      rset->column_names[i] = estrdup(PQfname(result, i));
      rset->column_types[i] = PQftype(result, i);
   }

   return rset;
}


PGResultSet *
pgconn_listTables(PGConnection* conn)
{
   return pgconn_listTablesNotMatching(conn, "pg_%");
}


PGResultSet *
pgconn_listTablesNotMatching(PGConnection *conn, const char *pattern)
{
	PGResultSet *rset;
	StringBuffer *sb;

	sb = buf_createDefault();
	buf_printf(sb, "SELECT tablename FROM pg_tables WHERE tablename not like '%s'",pattern);
	rset = pgconn_query(conn,buf_data(sb));
	buf_free(sb);

	return rset;
}

PGResultSet *
pgconn_listTablesMatching(PGConnection *conn, const char *pattern)
{
	PGResultSet *rset;
	StringBuffer *sb;

	sb = buf_createDefault();
	buf_printf(sb, "SELECT tablename FROM pg_tables WHERE tablename like '%s'",pattern);
	rset = pgconn_query(conn, buf_data(sb));
	buf_free(sb);

	return rset;
}


int
pgconn_insertID(PGConnection* conn)
{
   return (int)lastOid;
}


char *
pgconn_escape(PGConnection *conn, char *string)
{
   int length;
   char *newstring;

   if (string == NULL)
      THROW_D("NullPointerException");

   newstring = (char*)emalloc((length = strlen(string))*2 + 1);
   PQescapeString(newstring, string, length);

   return newstring;
}

char *
pgconn_escapeBytes(PGConnection *conn, UnArray *arr){
   int length;
   char *newstring;

   if (arr == NULL)
      THROW_D("NullPointerException");

   newstring = (char*)emalloc((length = arr->meta.length)*2 + 1);
   PQescapeString(newstring, &arr->ya.data[0], length);

   return newstring;
}

