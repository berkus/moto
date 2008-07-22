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

   $RCSfile: mx.l,v $   
   $Revision: 1.10 $
   $Author: dhakim $
   $Date: 2003/05/29 02:14:08 $
 
==============================================================================*/
#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "memsw.h"

#include "err.h"

#include "mxcg.h"
#include "mman.h"
#include "mx.tab.h"
#include "mxcerr.h"

extern MXC *c;

static void count();

static int colno = 0;

static char *mstrdup(char *s) {
   int i;
   int len = strlen(s);
	char *dup = (char *)malloc(strlen(s) + 1);
	err_assert(dup != NULL, err_f("AllocationFailure"));
	for (i = 0; i < len; i++) {
		*(dup + i) = *(s + i);
	}
   *(dup + len) = '\0';
	mman_track(c->mpool, dup);
   return dup;
}

%}

%option yylineno
%s C_ONLY

O								(eq|ne|\+|\*|\-|gt|lt|gte|lte)
OO								(\[\]|\+\=|\*\=|\-\=|\-\-|\+\+)
D                       [0-9]
L                       [a-zA-Z_]
I                       {L}({L}|{D})*

%%
^"#".*			{ /* comment */ }
"/**"([^*]|[*][^*]|[*][*][^/])*"**/" { count(); yylval.sval = mstrdup(yytext); return(COMMENT); }

"Extension:"            { count(); return(EXTENSION); }
"LibraryPath:"          { count(); return(LIBRARYPATH); }
"Library:"          	{ count(); return(LIBRARY); }
"Archive:"          	{ count(); return(ARCHIVE); }
"IncludePath:"          { count(); return(INCLUDEPATH); }
"File:"          		{ count(); return(XFILE); }
"Include:"          	{ count(); return(INCLUDE); }
"Interface:"          	{ count(); return(INTERFACE); }

<C_ONLY>"auto"          { count(); yylval.sval = mstrdup(yytext); return(AUTO); }
<C_ONLY>"const"         { count(); yylval.sval = mstrdup(yytext); return(CONST); }
<C_ONLY>"enum"          { count(); yylval.sval = mstrdup(yytext); return(ENUM); }
<C_ONLY>"extern"        { count(); yylval.sval = mstrdup(yytext); return(EXTERN); }
<C_ONLY>"register"      { count(); yylval.sval = mstrdup(yytext); return(REGISTER); }
<C_ONLY>"signed"        { count(); yylval.sval = mstrdup(yytext); return(SIGNED); }
<C_ONLY>"static"        { count(); yylval.sval = mstrdup(yytext); return(STATIC); }
<C_ONLY>"struct"        { count(); yylval.sval = mstrdup(yytext); return(STRUCT); }
<C_ONLY>"union"         { count(); yylval.sval = mstrdup(yytext); return(UNION); }
<C_ONLY>"unsigned"      { count(); yylval.sval = mstrdup(yytext); return(UNSIGNED); }
<C_ONLY>"volatile"      { count(); yylval.sval = mstrdup(yytext); return(VOLATILE); }

"tracked"				{ count(); yylval.sval = mstrdup(yytext); return(TRACKED); }

<INITIAL>{O}       		{ count(); yylval.sval = mstrdup(yytext); return(OPERATOR); }
{I}       				{ count(); yylval.sval = mstrdup(yytext); return(IDENTIFIER); }
{I}"::"("~")?{I} 		{ count(); yylval.sval = mstrdup(yytext); return(SCOPED_IDENTIFIER); }
{I}"::"{O} 				{ count(); yylval.sval = mstrdup(yytext); return(SCOPED_OPERATOR); }
{I}"::"{OO} 			{ count(); yylval.sval = mstrdup(yytext); return(SCOPED_OPERATOR); }

"char"                  { count(); yylval.sval = mstrdup(yytext); return(CHAR); }
"double"                { count(); yylval.sval = mstrdup(yytext); return(DOUBLE); }
"float"                 { count(); yylval.sval = mstrdup(yytext); return(FLOAT); }
"int"                   { count(); yylval.sval = mstrdup(yytext); return(INT); }
"long"                  { count(); yylval.sval = mstrdup(yytext); return(LONG); }
"short"                 { count(); yylval.sval = mstrdup(yytext); return(SHORT); }
"void"                  { count(); yylval.sval = mstrdup(yytext); return(VOID); }

L?\"(\\.|[^\\"])*\"     { count(); yylval.sval = mstrdup(yytext); return(STRING_LITERAL); }
L?<(\\.|[^<^=>])*>     	{ count(); yylval.sval = mstrdup(yytext); return(TAG_LITERAL); }

"-"{I}                  { count(); yylval.sval = mstrdup(yytext); return(LIBRARY_NAME); }
<C_ONLY>"*"             { count(); yylval.sval = mstrdup(yytext); return(STAR); }

"=>"                   	{ count(); BEGIN C_ONLY; return(MAP); }

"..."                   { count(); return(ELLIPSIS); }
";"                     { count(); BEGIN INITIAL; return(SEMICOLON); }
("{")              		{ count(); return(CURLY_L); }
("}")              		{ count(); return(CURLY_R); }
","                     { count(); return(COMMA); }
"("                     { count(); return(PAREN_L); }
")"                     { count(); return(PAREN_R); }
("[")              		{ count(); return(BRACKET_L); }
("]")              		{ count(); return(BRACKET_R); }

[ \t\v\n\f]             { count(); }
.                       {  }

%%

int yywrap() {
	return 1;
}

/*
static void comment() {
   char c, c1;
	
	loop:
		while ((c = input()) != '*' && c != 0) {
			putchar(c);
		}		
		if ((c1 = input()) != '/' && c != 0) {
			unput(c1);
			goto loop;
		}		
		if (c != 0) {
			putchar(c1);
		}
}
*/

static void count() {
	int i;
	
	for (i = 0; yytext[i] != '\0'; i++) {
		if (yytext[i] == '\n') {
			colno = 0;
		}
		else if (yytext[i] == '\t') {
			colno += 8 - (colno % 8);
		}
		else {
			colno++;
		}
	}
}

int yyerror(char *s) {
	mxc_syntaxError(yyin, c->filename, yylineno, colno);
	return -1;
}

/*=============================================================================
// end of file: $RCSfile: mx.l,v $
==============================================================================*/

