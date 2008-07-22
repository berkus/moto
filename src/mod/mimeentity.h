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

   $RCSfile: mimeentity.h,v $
   $Revision: 1.4 $
   $Author: dhakim $
   $Date: 2002/12/05 02:40:48 $

==============================================================================*/

#ifndef __MIMEENTITY_H
#define __MIMEENTITY_H

#include "symboltable.h"
#include "exception.h"
#include "mxarr.h"

excp_extern(MIMEParseException);

typedef struct mimeEntity{
   /* Content-dispostion fields */
   char* disposition;
   char* name;
   char* filename;
   /* Content-type fields */
   char* type;
   char* charset;
   /* Content-tansfer-encoding fields */
   char* encoding; 
   SymbolTable* headers;
   char*  body;
   size_t bodylen;
} MIMEEntity;

MIMEEntity* mime_create(char* msg,size_t msglen);
void mime_free(MIMEEntity* mm);

char* mime_getName(MIMEEntity* mm);

char* mime_getFileName(MIMEEntity* mm);

char* mime_getContentType(MIMEEntity* mm);

char* mime_getContentTransferEncoding(MIMEEntity* mm);

char* mime_getHeader(MIMEEntity* mm, char* hdr);

char* mime_getBody(MIMEEntity* mm);

UnArray* mime_getBody_yarr(MIMEEntity* mm);

char* mime_getBodyAsString(MIMEEntity* mm);

size_t mime_getBodyLength(MIMEEntity* mm);

#endif
