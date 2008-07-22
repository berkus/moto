/*=============================================================================

   Copyright (C) 2001 David Hakim.
   This file is part of the Moto Programming Language.

   Moto Programming Language is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   Moto Programming Language is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the Codex C Library; see the file COPYING.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   $RCSfile: cookie.h,v $
   $Revision: 1.1 $
   $Author: dhakim $
   $Date: 2002/02/09 06:05:15 $

==============================================================================*/

#ifndef __COOKIE_H
#define __COOKIE_H

typedef struct cookie{
   char* name;
   char* value;
   /* Fields required for set cookie*/
   char* comment;
   char* domain;
   char* maxage;
   char* path;
   char secure;
   char* version;
} Cookie;

Cookie* cook_createDefault();

Cookie* cook_create(
   char* name,
   char* value,
   char* comment,
   char* domain,
   char* maxage,
   char* path,
   char secure,
   char* version
);

void cook_free(Cookie* c);

char* cook_toString(Cookie* c);

Cookie* cook_parse(char* s,char isSetCookie);

char* cook_getName(Cookie* c);
char* cook_getValue(Cookie* c);
char* cook_getComment(Cookie* c);
char* cook_getDomain(Cookie* c);
char* cook_getMaxAge(Cookie* c);
char* cook_getPath(Cookie* c);
char* cook_getVersion(Cookie* c);
char cook_isSecure(Cookie* c);

void cook_setName(Cookie* c,char* name);
void cook_setValue(Cookie* c,char* value);
void cook_setComment(Cookie* c,char* comment);
void cook_setDomain(Cookie* c,char* domain);
void cook_setMaxAge(Cookie* c,char* maxage);
void cook_setPath(Cookie* c,char* path);
void cook_setVersion(Cookie* c,char* version);
void cook_setSecure(Cookie* c,char secure);

#endif
