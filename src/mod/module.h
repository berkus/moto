/*=============================================================================

   Copyright (C) 2000-2002 David Hakim.
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

   $RCSfile: module.h,v $   
   $Revision: 1.21 $
   $Author: dhakim $
   $Date: 2003/01/25 02:53:12 $
 
==============================================================================*/
#ifndef __MODULE_H
#define __MODULE_H

#include "httpd.h"
#include "http_config.h"
#include "http_protocol.h"
#include "http_log.h"
#include "util_script.h"
#include "http_main.h"
#include "http_request.h"

#include "mdfa.h"
#include "state.h"
#include "exception.h"
#include "mimeentity.h"
#include "enumeration.h"
#include "cookie.h"
#include "vector.h"

/*
 *  Exceptions
 */

excp_extern(HTTPRedirectException);

/*
 *  State initialization
 */

State* addNVPairsFromQS(State* state, request_rec *r, const char* qs);
State* getParentStateFromQS(request_rec *r, const char* qs);

MDFA** lex_constructLexer(int rxc, char** rxv); /* FIXME: Should go elsewhere */

SymbolTable *parseCookies(request_rec *r);
Vector* parseMIMEPartsFromPost(request_rec *r);

void bindStateToSession(State* ps, State* s);
State* statefullInitFromQS(request_rec *r);
State* statefullInitFromPost(request_rec *r);
State* statefullInitFromMIMEParts(Vector* parts);

/*
 *  Access functions to page global variables
 */
 
extern request_rec *current_request;
extern State *current_state;
extern table* current_options;
extern Vector* current_parts;
extern SymbolTable* current_cookies;
extern char headers_already_sent;

typedef request_rec HTTPRequest;
typedef request_rec HTTPResponse;

char* getConfigOption(char* name);
char getBoolConfigOption(char* name);

HTTPRequest* getRequest();
HTTPResponse* getResponse();

void req_init(HTTPRequest* r,table* options);
void req_cleanup();

char* req_getFileName(HTTPRequest* this);
char* req_getURI(HTTPRequest* this);
char* req_getPathInfo(HTTPRequest* this);
char* req_getMethod(HTTPRequest* this);
char* req_getHostName(HTTPRequest* this);
char* req_getProtocol(HTTPRequest* this);
char* req_getHeader(HTTPRequest* r,char* header);
char* req_getHeaders(HTTPRequest* r);
char* req_getClientIP(HTTPRequest* r);

Enumeration* req_getMIMEParts(HTTPRequest* r);
MIMEEntity* req_getMIMEPart(HTTPRequest* r,char* name);

void cook_init(); /* FIXME: should probaly find a better home for this */
Cookie* req_getCookie(HTTPRequest* r,char* name);
Enumeration* req_getCookies(HTTPRequest* r);

void res_setHeader(HTTPResponse* r, char* name, char* value);
void res_setCookie(HTTPResponse* r, Cookie* c);
void res_sendHeaders(HTTPResponse* r);
void res_flushOutput(HTTPResponse* r);

State *getState();
char *getValue(char* name);
char *getValueWDefault(char* name, char* value);
char* urlEncode(unsigned char* s);
char* urlDecode(unsigned char* s);
void sendRedirect(char* redirect);

/*
 *  Bonus functions
 */

void logExecutionTime(request_rec *r,char* template,float exectime);
void installSignalHandlers();
void unInstallSignalHandlers();
void signalHandler(int signal);

#endif

/*=============================================================================
// end of file: $RCSfile: module.h,v $
==============================================================================*/

