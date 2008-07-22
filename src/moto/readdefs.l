%{
/*=============================================================================

   Copyright (C) 2000 Kenneth Kocienda and David Hakim.
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

   $RCSfile: readdefs.l,v $   
   $Revision: 1.9 $
   $Author: dhakim $
   $Date: 2002/12/05 02:40:48 $
 
==============================================================================*/
#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "memsw.h"

#include "exception.h"
#include "excpfn.h"
#include "fileio.h"
#include "log.h"

#include "readdefs.tab.h"
#include "motoerr.h"
#include "pp.h"
#include "pputil.h"
#include "motohook.h"

#define YY_MALLOC_DECL /* stdlib.h and memsw.h are not playing happily */

%}
%option yylineno

%s M B P

I		[0-9]
A		[a-zA-Z]
K		[_]

%%

\004 {
	return EOB;
}

^#.*\n {
	/* this is a comment */
}

^({A}|{K})({A}|{I}|{K})* { 
	MotoPP *ppenv = motopp_getEnv();
	ryylval.sval = motopp_strdup(ppenv, ryytext);
	BEGIN P; 
	return NAME; 
}

<P>"(" {
	MotoPP *ppenv = motopp_getEnv();
	unput('(');
	ppenv->mb->startline = ryylineno;
	BEGIN M;
}

<P>. {
	MotoPP *ppenv = motopp_getEnv();
	unput(ryytext[0]);
	ppenv->mb->startline = ryylineno;
	BEGIN B;
}

<P>[ \t] {
}

<M>({A}|{K})({A}|{I}|{K})* { 
	MotoPP *ppenv = motopp_getEnv();
	ryylval.sval = motopp_strdup(ppenv, ryytext); 
	return ARG; 
}

<M>"(" {
	return '(';
}

<M>")" {
	BEGIN B;
	return ')';
}

<M>"," {
	return ',';
}

<M>[ \t] {
}

<M>"\n" {
	MotoPP *ppenv = motopp_getEnv();
	motopp_badDefineParamList(ppenv, ryylineno - 1);	
}

<B>"\n" {
	MotoPP *ppenv = motopp_getEnv();
	ryylval.sval = motopp_strdup(ppenv, ryytext); 
	return ANYCHARS; 	
}

<B>^" \t"+ {}

<B>. {
	MotoPP *ppenv = motopp_getEnv();
	ryylval.sval = motopp_strdup(ppenv, ryytext); 
	return ANYCHARS; 	
}

<<EOF>> {
	ryy_delete_buffer(YY_CURRENT_BUFFER);
	yyterminate();	
}

. {
	return DEF;
}

[ \t\n] {
}

%%

int ryyerror (char *s) {
	MotoPP *ppenv = motopp_getEnv();
	motopp_syntaxError(ppenv, ryylineno);
	return -1;
}

int ryywrap () {
	return 1;
}

void motopp_freeRYYBuffer() {
	ryy_delete_buffer(YY_CURRENT_BUFFER);
}

int motopp_prepareReaddefsFrameBuffer(MotoPP *ppenv, char *filename) {
	int len;
	char *buf;	
	if (!fio_exists(filename)) {
	   return -1;
	} 	
	len = fio_get(filename, &buf);
	len += 3;
	buf = realloc(buf, len);
	excp_assert(buf != NULL, THROW_D("AllocationFailureException"));	
	buf[len - 3] = '\004';
	buf[len - 2] = YY_END_OF_BUFFER_CHAR;
	buf[len - 1] = YY_END_OF_BUFFER_CHAR;
	ryy_switch_to_buffer(yy_scan_buffer(buf, (yy_size_t)len));
	ppenv->frame->yybuf = NULL /* YY_CURRENT_BUFFER */;
	ppenv->frame->yybufmem = buf;
	BEGIN INITIAL;
	return 0;
}

/*=============================================================================
// end of file: $RCSfile: readdefs.l,v $
==============================================================================*/

