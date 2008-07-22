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

   $RCSfile: motopp.l,v $   
   $Revision: 1.12 $
   $Author: dhakim $
   $Date: 2003/02/27 19:47:00 $
 
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
#include "enumeration.h"
#include "log.h"
#include "fileio.h"
#include "stack.h"

#include "motocodes.h"
#include "motoerr.h"
#include "motopp.tab.h"
#include "pp.h"
#include "pputil.h"
#include "motopplex.h"
#include "motohook.h"

#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_ALL
#endif

#define YY_DECL int yylex(YYSTYPE *lvalp, MotoPP *ppenv)

static int pdepth = 0;
static char c1;
static char c2;

int pperror(char *s);
int ppwrap();

%}
%option yylineno
%option stack

%s M L D	A R F ME

I		[0-9]
A		[a-zA-Z]
K		[_]
X		[0-9a-zA-Z!"%&'*+\055-\133\135-\176]
                
%%

\004 {
	if (YYSTATE != INITIAL) {
		pperror("");
	}
	return EOB;
}

<INITIAL>"#if" {
	istack_push(ppenv->dirstack, IF);
	BEGIN L;
	return IF;
}

<INITIAL>"#else" {
	int cur;
	if (istack_size(ppenv->dirstack) <= 0) {
		motopp_elseWithoutIf(ppenv, pplineno);
	} 
	cur = istack_peek(ppenv->dirstack);
	if (cur != IF && cur != IFDEF && cur != IFNDEF) {
		motopp_elseWithoutIf(ppenv, pplineno);
	}
	return ELSE;
}

<INITIAL>"#elseif" {
	int cur;
	if (istack_size(ppenv->dirstack) <= 0) {
		motopp_elseifWithoutIf(ppenv, pplineno);
	} 
	cur = istack_peek(ppenv->dirstack);
	if (cur != IF && cur != IFDEF && cur != IFNDEF) {
		motopp_elseifWithoutIf(ppenv, pplineno);
	}
	BEGIN L;
	return ELSEIF;
}

<INITIAL>"#endif"	{
	int cur;
	if (istack_size(ppenv->dirstack) <= 0) {
		motopp_endifWithoutIf(ppenv, pplineno);
	} 
	cur = istack_pop(ppenv->dirstack);
	if (cur != IF && cur != IFDEF && cur != IFNDEF) {
		motopp_endifWithoutIf(ppenv, pplineno);
	}
	return ENDIF;
}

<INITIAL>"#undef" {
	istack_push(ppenv->dirstack, UNDEF);
	BEGIN L;
	return UNDEF; 
}

<INITIAL>"#ifdef" {
	istack_push(ppenv->dirstack, IFDEF);
	BEGIN L;
	return IFDEF; 
}

<INITIAL>"#ifndef" {
	istack_push(ppenv->dirstack, IFNDEF);
	BEGIN L;
	return IFNDEF; 
}

<INITIAL>"#include" {
	istack_push(ppenv->dirstack, INCLUDE);
	BEGIN F;
	return INCLUDE; 
}

<INITIAL>"#readdefs" {
	istack_push(ppenv->dirstack, READDEFS);
	BEGIN F;
	return READDEFS; 
}

<INITIAL>"#define" {
	istack_push(ppenv->dirstack, DEFINE);
	BEGIN D;
	return DEFINE; 
}

<F>\"(\\.|[^\\"])*\" {
	lvalp->sval = motopp_strdup(ppenv, yytext);
	istack_pop(ppenv->dirstack);
	return QSTRING; 
}

<F>"(" {
	return '(';
}

<F>")" {
	BEGIN INITIAL;
	return ')';
}

<F>[ \t] {
}

<F>. {
	int directive = istack_pop(ppenv->dirstack);
	switch (directive) {
		case READDEFS:
			motopp_badReaddefsDirective(ppenv, pplineno);
			break;
		case INCLUDE:
			motopp_badIncludeDirective(ppenv, pplineno);
			break;
		default:
			pperror("");
			break;
	}	
}

<M>(.|[\r\n]) {
	int escape = 0;
	c1 = c2;
	c2 = yytext[0];
	if (c1 == '\\') {
		escape = 1;	
	}
	switch (c2) {
		case '(':
			if (escape == 1) {
				buf_putc(ppenv->argbuf, c2);
			}
			else {
				pdepth++;
				if (pdepth == 1) {
					return '(';
				}
				buf_putc(ppenv->argbuf, c2);
			}
			break;
		case ')':
			if (escape == 1) {
				buf_putc(ppenv->argbuf, c2);
			}
			else {
				pdepth--;
				if (pdepth == 0) {
					lvalp->sval = buf_toString(ppenv->argbuf);
					hset_add(ppenv->ptrs, lvalp->sval);
					buf_clear(ppenv->argbuf);
					unput(')');
					yy_pop_state();
					yy_push_state(ME);
					BEGIN ME;
					return ARG;
				}
				buf_putc(ppenv->argbuf, c2);
			}
			break;
		case '\\':
			if (escape == 1) {
				buf_putc(ppenv->argbuf, c2);
				c2 = 0;
			}
			break;
		case ',':
			if (escape == 1) {
				buf_putc(ppenv->argbuf, c2);
			}
			else {
				lvalp->sval = buf_toString(ppenv->argbuf);
				hset_add(ppenv->ptrs, lvalp->sval);
				buf_clear(ppenv->argbuf);
				return ARG;
				c1 = c2 = '\0';
			}
			break;
		default:
			if (escape == 1) {
				buf_putc(ppenv->argbuf, '\\');
			}
			buf_putc(ppenv->argbuf, c2);
			break;
	}
}

<ME>")" {
	yy_pop_state();
	return ')';
}

<INITIAL>"$"(break|case|continue|declare|do|else|elseif|endfor)("(")? {
	lvalp->sval = motopp_strdup(ppenv, yytext); 
	return ANYCHARS;
}

<INITIAL>"$"(endif|endswitch|endwhile|for|if|switch|use|while)("(")? {
	lvalp->sval = motopp_strdup(ppenv, yytext); 
	return ANYCHARS;
}

<INITIAL>"$"({A}|{K})({A}|{I}|{K})* { 
	lvalp->sval = emalloc(strlen(yytext));
	strcpy(lvalp->sval, yytext + 1);
	hset_add(ppenv->ptrs, lvalp->sval);
	return NOARGMACRO; 
}

<INITIAL>"$"({A}|{K})({A}|{I}|{K})*"(" { 
	int len = strlen(yytext);
	lvalp->sval = emalloc(len - 1);
	strncpy(lvalp->sval, yytext + 1, len - 1);
	lvalp->sval[len - 2] = '\0';
	hset_add(ppenv->ptrs, lvalp->sval);
	buf_clear(ppenv->argbuf);
	yy_push_state(M);
	unput('(');
	c1 = c2 = '\0';
	pdepth = 0;
	return ARGMACRO; 
}

<D>({A}|{K})({A}|{I}|{K})* { 
	lvalp->sval = motopp_strdup(ppenv, yytext); 
	BEGIN R;
	return IDENTIFIER; 
}

<D>({A}|{K})({A}|{I}|{K})*"(" { 
	int len = strlen(yytext);
	lvalp->sval = emalloc(len + 1);
	strcpy(lvalp->sval, yytext);
	hset_add(ppenv->ptrs, lvalp->sval);
	lvalp->sval[len - 1] = '\0';
	unput('(');
	BEGIN A;
	return IDENTIFIER; 
}

<A>"(" {
	return '(';
}

<A>"," {
	return ',';
}

<A>({A}|{K})({A}|{I}|{K})* { 
	lvalp->sval = motopp_strdup(ppenv, yytext); 
	return IDENTIFIER; 
}

<A>")" {
	BEGIN R;
	return ')';
}

<A>. {
	motopp_badDefineParamList(ppenv, pplineno);
}

<R>(.)* {
	lvalp->sval = motopp_strdup(ppenv, yytext); 
	return RTEXT; 
}

<R>"\n" {
	unput('\n');
	BEGIN INITIAL;
}

<L>"$"({A}|{K})({A}|{I}|{K})* { 
	lvalp->sval = emalloc(strlen(yytext));
	strcpy(lvalp->sval, yytext + 1);
	hset_add(ppenv->ptrs, lvalp->sval);
	return NOARGMACRO; 
}

<L>"$"({A}|{K})({A}|{I}|{K})*"(" { 
	int len = strlen(yytext);
	lvalp->sval = emalloc(len - 1);
	strncpy(lvalp->sval, yytext + 1, len - 1);
	lvalp->sval[len - 2] = '\0';
	hset_add(ppenv->ptrs, lvalp->sval);
	buf_clear(ppenv->argbuf);
	yy_push_state(M);
	unput('(');
	c1 = c2 = '\0';
	pdepth = 0;
	return ARGMACRO; 
}

<L>"defined" {
	return DEFINED;
}

<L>({A}|{K})({A}|{I}|{K})*("("[0-9]+")")? { 
	lvalp->sval = motopp_strdup(ppenv, yytext); 
	return MACROSIG; 
}

<L>"==" {
	return REL_EQ;
}

<L>"!=" {
	return REL_NE;
}

<L>"!" {
	return NOT;
}

<L>"&&" {
	return AND;
}

<L>"||" {
	return OR;
}

<L>"(" {
	return '(';
}

<L>")" {
	return ')';
}

<L>\"(\\.|[^\\"])*\" {
	lvalp->sval = motopp_strdup(ppenv, yytext);
	return QSTRING; 
}

<L>[ \t] {
}

<L>"\n" { 
	unput('\n');
	BEGIN INITIAL;
}

<L>. {
	int directive = istack_pop(ppenv->dirstack);
	switch (directive) {
		case IF:
			motopp_badIfDirective(ppenv, pplineno);
			break;
		case ELSEIF:
			motopp_badElseIfDirective(ppenv, pplineno);
			break;
		case DEFINE:
			motopp_badDefineDirective(ppenv, pplineno);
			break;
		case UNDEF:
			motopp_badUndefDirective(ppenv, pplineno);
			break;
		case IFDEF:
			motopp_badIfdefDirective(ppenv, pplineno);
			break;
		case IFNDEF:
			motopp_badIfndefDirective(ppenv, pplineno);
			break;
		case READDEFS:
			motopp_badReaddefsDirective(ppenv, pplineno);
			break;
		case INCLUDE:
			motopp_badIncludeDirective(ppenv, pplineno);
			break;
		default:
			pperror("");
			break;
	}	
}

<INITIAL>([ \t\n]*{X}*[ \t\n]*)+ {
	lvalp->sval = motopp_strdup(ppenv, yytext); 
	return ANYCHARS;
}

<INITIAL>{X}+ {
	lvalp->sval = motopp_strdup(ppenv, yytext); 
	return ANYCHARS;
}

<INITIAL>. {
	lvalp->sval = motopp_strdup(ppenv, yytext); 
	return ANYCHARS;
}

<INITIAL>("\n")* {
	lvalp->sval = motopp_strdup(ppenv, yytext); 
	return ANYCHARS;
}

<<EOF>> {
	if (YYSTATE != INITIAL) {
		pperror("");
	}
	log_debug(__FILE__, ">>> <<EOF>>\n");
	motopp_freeFrame(ppenv);
	yyterminate();	
}

. {}

%%

int pperror(char *s) {
	MotoPP *ppenv = motopp_getEnv();
	motopp_syntaxError(ppenv, pplineno);
	return -1;
}

int ppwrap() {
	return 1;
}

void motopp_beginInitial() {
	yy_pop_state();
}

void motopp_freeYYBuffer(struct yy_buffer_state *yybuf) {
	pp_delete_buffer(yybuf);
}

void motopp_freeYYStack() {
	if (yy_start_stack) {
		free(yy_start_stack);
	}
}

void motopp_prepareFrameForParenthood(MotoPP *ppenv) {
	ppenv->frame->yybuf = YY_CURRENT_BUFFER;
	ppenv->frame->lineno = pplineno;
}

int motopp_prepareMainBuffer(MotoPP *ppenv, char *filename) {
	int len;
	char *buf;	
	if (!fio_exists(filename)) {
	   return -1;
        } 	
	len = fio_get(filename, &buf);
	len += 2;
	buf = realloc(buf, len);
	excp_assert(buf != NULL, THROW_D("AllocationFailureException"));	
	buf[len - 2] = YY_END_OF_BUFFER_CHAR;
	buf[len - 1] = YY_END_OF_BUFFER_CHAR;
	pp_switch_to_buffer(yy_scan_buffer(buf, (yy_size_t)len));
	ppenv->frame->yybuf = YY_CURRENT_BUFFER;
	ppenv->frame->yybufmem = buf;
        pplineno = 1;
	BEGIN INITIAL;
	return 0;
}

void motopp_prepareFrameBuffer(MotoPP *ppenv, char *text) {
	int len;
	char *buf;
	len = strlen(text) + 3;
	buf = emalloc(len);
	strcpy(buf, text);
	buf[len - 3] = '\004';
	buf[len - 2] = YY_END_OF_BUFFER_CHAR;
	buf[len - 1] = YY_END_OF_BUFFER_CHAR;
	pp_switch_to_buffer(yy_scan_buffer(buf, (yy_size_t)len));
	ppenv->frame->yybuf = YY_CURRENT_BUFFER;
	ppenv->frame->yybufmem = buf;
	BEGIN INITIAL;
}

int motopp_prepareFileFrameBuffer(MotoPP *ppenv, char *filename) {
   int len;
   char *buf;	
   if (!fio_exists(filename)) {
      return -1;
   } 	
   len = fio_get(filename, &buf);
   len += 3;
   buf = erealloc(buf, len);
   buf[len - 3] = '\004';
   buf[len - 2] = YY_END_OF_BUFFER_CHAR;
   buf[len - 1] = YY_END_OF_BUFFER_CHAR;
   pp_switch_to_buffer(yy_scan_buffer(buf, (yy_size_t)len));
   ppenv->frame->yybuf = YY_CURRENT_BUFFER;
   ppenv->frame->yybufmem = buf;
   BEGIN INITIAL;
   return 0;
}

void motopp_prepareMacroFrameForExec(MotoPP *ppenv, MotoMacro *m) {
   ppenv->frame->macro = m;
   ppenv->frame->lineno = pplineno;
   pplineno = m->startline;
}

void motopp_prepareFrameForExec(MotoPP *ppenv, char *filename)	{
   ppenv->frame->lineno = pplineno;
   pplineno = 1;
}

/*=============================================================================
// end of file: $RCSfile: motopp.l,v $
==============================================================================*/

