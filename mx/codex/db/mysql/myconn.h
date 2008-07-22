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

   $RCSfile: myconn.h,v $
   $Revision: 1.3 $
   $Author: shayh $
   $Date: 2003/01/27 16:03:44 $

==============================================================================*/

#ifndef __MYSQLCONN_H
#define __MYSQLCONN_H

#include "mysqlcommon.h"

Connection* myconn_create(const char *host, const char *db, const char *user, const char *password);

void myconn_free(Connection *conn);

int myconn_update(Connection *conn, const char *query);

ResultSet* myconn_query(Connection *conn, const char *query);

int64_t myconn_insertID(Connection *conn);

char* myconn_escape(Connection *conn, char *string);

char* myconn_escapeBytes(Connection *conn, UnArray *arr);

ResultSet* myconn_listTables(Connection *conn);

ResultSet* myconn_listTablesMatching(Connection *conn, char *pattern);

#endif

