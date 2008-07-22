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

   $RCSfile: myconn.c,v $
   $Revision: 1.8 $
   $Author: shayh $
   $Date: 2003/03/07 20:01:48 $

==============================================================================*/

#include "myconn.h"
#include "myrset.h"


char* myconn_err(SQLsock sock){return (char*)mysql_error(sock);}

Connection *
myconn_create(const char *host, const char *db, const char *user, const char *password)
{
   Connection *conn;

   if (db == NULL)
      THROW("SQLException","Database may not be (null)");

   conn = (Connection*)malloc(sizeof(Connection));
   conn->init_fn  = (SQLsock(*)(void*))mysql_init;
   conn->close_fn = (void(*)(SQLsock))mysql_close;
   conn->error_fn = (char*(*)(SQLsock))myconn_err;

   conn->stream = conn->init_fn(NULL);

   if (!mysql_real_connect(conn->stream, host, user, password, db, 0, NULL, 0))
      THROW("SQLException","%s", GET_SQL_ERROR(conn));

   return conn;
}

void myconn_free(Connection *conn){
   DISCONNECT(conn);
}

/* function to call for inserts or updates, returns # of rows affected */
int myconn_update(Connection *conn, const char *query){
   if (mysql_query(conn->stream, query) != 0)
      THROW("SQLException","%s", GET_SQL_ERROR(conn));

   return mysql_affected_rows(conn->stream);
}

/* function to call for selects, returns result set */
ResultSet* myconn_query(Connection *conn, const char *query){
   int i;
   MYSQL_FIELD *field;
   ResultSet *rset;

   if (mysql_query(conn->stream, query) != 0)
      THROW("SQLException","%s", GET_SQL_ERROR(conn));

   rset = myrset_createDefault(conn, mysql_store_result(conn->stream));

   for(i = 0, field = mysql_fetch_field(rset->res); field != NULL; \
       i++, field = mysql_fetch_field(rset->res)){

      rset->column_names[i] = estrdup(field->name);
      rset->column_types[i] = field->type;
   }

   return rset;
}

ResultSet* myconn_listTables(Connection *conn) {
   return myconn_listTablesMatching(conn, "%");
}

ResultSet* myconn_listTablesMatching(Connection *conn, char *pattern){
   int i;
   MYSQL_FIELD *field;
   ResultSet *rset;

   rset = myrset_createDefault(conn, mysql_list_tables(conn->stream, pattern));

   for(i=0,field = mysql_fetch_field(rset->res);field != NULL;i++,field = mysql_fetch_field(rset->res)){
      rset->column_names[i] = estrdup(field->name);
      rset->column_types[i] = field->type;
   }

   return rset;
}

int64_t myconn_insertID(Connection* conn){
   return (int64_t)mysql_insert_id(conn->stream);
}

char* myconn_escape(Connection *conn, char *string){
   int length;
   char *newstring;

   if (string == NULL)
      THROW_D("NullPointerException");

   newstring = (char*)emalloc((length = strlen(string))*2 + 1);
   mysql_escape_string(newstring, string, length);

   return newstring;
}

char* myconn_escapeBytes(Connection *conn, UnArray *arr){
   int length;
   char *newstring;

   if (arr == NULL)
      THROW_D("NullPointerException");

   newstring = (char*)emalloc((length = arr->meta.length)*2 + 1);
   mysql_escape_string(newstring, &arr->ya.data[0], length);

   return newstring;
}

