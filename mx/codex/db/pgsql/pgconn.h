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

   $RCSfile: pgconn.h,v $
   $Revision: 1.2 $
   $Author: shayh $
   $Date: 2003/01/24 16:26:03 $

==============================================================================*/

#ifndef __PGCONN_H
#define __PGCONN_H

#include "pgsqlcommon.h"

PGConnection* pgconn_createDefault();
PGConnection* pgconn_create(const char *host, const char *db, const char *user, const char *password);

void pgconn_free(PGConnection *conn);

/* Function to call for inserts or updates, returns # of rows affected */
int pgconn_update(PGConnection *conn, const char *query);

/* Function to call for selects, returns result set */
PGResultSet* pgconn_query(PGConnection *conn, const  char *query);

int pgconn_insertID(PGConnection *conn);

char* pgconn_escape(PGConnection *conn, char *string);
char* pgconn_escapeBytes(PGConnection *conn, UnArray *arr);

PGResultSet* pgconn_listTables(PGConnection *conn);

PGResultSet* pgconn_listTablesMatching(PGConnection *conn, const char *pattern);

PGResultSet* pgconn_listTablesNotMatching(PGConnection *conn, const char *pattern);

#endif
