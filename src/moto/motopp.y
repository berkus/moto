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

   $RCSfile: motopp.y,v $   
   $Revision: 1.10 $
   $Author: dhakim $
   $Date: 2003/06/09 18:35:49 $
 
==============================================================================*/
#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memsw.h"

#include "log.h"
#include "number.h"

#include "pp.h"
#include "ppcell.h"
#include "motoppi.h"
#include "motohook.h"
#include "excpfn.h"

#define YYPARSE_PARAM param
#define YYLEX_PARAM param

extern struct yy_buffer_state *pp_switch_to_buffer(struct yy_buffer_state *buf);
extern int pperror(char *s);
extern int pplex();
extern int pplineno;
extern FILE *ppin;

static PPUnionCell *ls(char *value);
static PPUnionCell *s(char *value);
static PPUnionCell *op(int opcode, int opcount, ...);
static void freeCell(PPUnionCell *p);

%}

%pure_parser

%union {
	char *sval;
	union ppUnionCell *cp;
}

%token <sval> ANYCHARS 
%token <sval> ARG
%token <sval> ARGMACRO
%token <sval> IDENTIFIER
%token <sval> MACROSIG 
%token <sval> NOARGMACRO
%token <sval> RTEXT 
%token <sval> QSTRING 

%token AND
%token DEFARG
%token DEFNOARG
%token DEFINE
%token DEFINED
%token INCLUDE
%token ELSE
%token ELSEIF
%token ELSEIFLIST
%token ENDIF
%token EOB
%token IF
%token IFDEF
%token IFNDEF
%token LIST
%token MACROCALL
%token NOT
%token OR
%token PRINT
%token READDEFS
%token REL_EQ
%token REL_NE
%token UNDEF

%type <cp> conditional_directive
%type <cp> define_directive
%type <cp> defined_expression
%type <cp> elseif_conditional
%type <cp> elseif_conditional_block
%type <cp> equality_expression
%type <cp> expression
%type <cp> include_directive
%type <cp> logical_and_expression
%type <cp> logical_or_expression
%type <cp> macro_argument
%type <cp> macro_argument_list
%type <cp> macrodef_argument
%type <cp> macrodef_argument_list
%type <cp> macro_expression
%type <cp> macro_expression_argument
%type <cp> macro_statement
%type <cp> primary_expression
%type <cp> readdefs_directive
%type <cp> statement
%type <cp> statement_list
%type <cp> unary_expression
%type <cp> undef_directive

%%
program
	: translation_unit { } 
;

translation_unit
	: translation_unit statement  
		{ 
			motoppi($2); 
			freeCell($2); 
		}
	|
;

primary_expression
	: ARG  { $$ = s($1); }
	| QSTRING  { $$ = ls($1); }
	| '(' expression ')' { $$ = $2; }
;

macro_expression
	: NOARGMACRO
		{
			$$ = op(MACROCALL, 1, s($1));
		}
	| ARGMACRO '(' macro_argument_list ')'
		{
			$$ = op(MACROCALL, 2, s($1), $3);
		}
	| primary_expression { $$ = $1; }
;

defined_expression
	: DEFINED MACROSIG 
		{
			$$ = op(DEFINED, 1, s($2));
		}
	| macro_expression { $$ = $1; }
;

unary_expression
	: NOT defined_expression
		{
			$$ = op(NOT, 1, $2);
		}
	| defined_expression { $$ = $1; }
;

equality_expression
	: equality_expression REL_EQ defined_expression
		{
			$$ = op(REL_EQ, 2, $1, $3); 
		}
	| equality_expression REL_NE defined_expression
		{
			$$ = op(REL_NE, 2, $1, $3); 
		}
	| unary_expression 
;

logical_and_expression
	: logical_and_expression AND equality_expression
		{
			$$ = op(AND, 2, $1, $3); 
		}
	| equality_expression  
;

logical_or_expression
	: logical_or_expression OR logical_and_expression
		{
			$$ = op(OR, 2, $1, $3); 
		}
	| logical_and_expression  
;

expression
	: logical_or_expression { $$ = $1; } 
;

undef_directive
	: UNDEF MACROSIG
		{
			$$ = op(UNDEF, 1, s($2));
		}	
;

macrodef_argument_list
	: macrodef_argument { $$ = $1; }
	| macrodef_argument_list ',' macrodef_argument { $$ = op(LIST, 2, $1, $3); }
;

macrodef_argument
	: IDENTIFIER { $$ = s($1); }
;

define_directive
	: DEFINE IDENTIFIER
		{
			$$ = op(DEFNOARG, 1, s($2));
		}	
	| DEFINE IDENTIFIER RTEXT
		{
			$$ = op(DEFNOARG, 2, s($2), s($3));

		}	
	| DEFINE IDENTIFIER '(' macrodef_argument_list ')'
		{
			$$ = op(DEFARG, 2, s($2), $4);
		}	
	| DEFINE IDENTIFIER '(' macrodef_argument_list ')' RTEXT
		{
			$$ = op(DEFARG, 3, s($2), $4, s($6));
		}	
;

macro_argument_list
	: macro_argument { $$ = $1; }
	| macro_argument_list macro_argument { $$ = op(LIST, 2, $1, $2); }
;

macro_argument
	: ARG {  $$ = s($1); }	
	| macro_expression_argument { $$ = $1; }	
;

macro_expression_argument
	: NOARGMACRO
		{
			$$ = op(MACROCALL, 1, s($1));
		}
	| ARGMACRO '(' macro_argument_list ')'
		{
			$$ = op(MACROCALL, 2, s($1), $3);
		}
;

macro_statement
	: NOARGMACRO
		{
			$$ = op(MACROCALL, 1, s($1));
		}
	| ARGMACRO '(' macro_argument_list ')'
		{
			$$ = op(MACROCALL, 2, s($1), $3);
		}
;

include_directive
	: INCLUDE '(' QSTRING ')'
		{
			$$ = op(INCLUDE, 1, ls($3));
		}
;

readdefs_directive
	: READDEFS '(' QSTRING ')'
		{
			$$ = op(READDEFS, 1, ls($3));
		}
;

conditional_directive
	: IF expression statement_list ENDIF 
		{
			$$ = op(IF, 2, $2, $3);  
		}
 	| IF expression statement_list ELSE statement_list ENDIF
		{
			$$ = op(IF, 3, $2, $3, $5);  
		}
 	| IF expression statement_list elseif_conditional_block ENDIF
		{
			$$ = op(IF, 3, $2, $3, $4);  
		}
 	| IF expression statement_list elseif_conditional_block ELSE statement_list ENDIF
		{
			$$ = op(IF, 4, $2, $3, $4, $6);  
		}
	| IFDEF MACROSIG statement_list ENDIF 
		{
			$$ = op(IFDEF, 2, s($2), $3);  
		}
 	| IFDEF MACROSIG statement_list ELSE statement_list ENDIF
		{
			$$ = op(IFDEF, 3, s($2), $3, $5);  
		}
 	| IFDEF MACROSIG statement_list elseif_conditional_block ENDIF
		{
			$$ = op(IFDEF, 3, s($2), $3, $4);  
		}
 	| IFDEF MACROSIG statement_list elseif_conditional_block ELSE statement_list ENDIF
		{
			$$ = op(IFDEF, 4, s($2), $3, $4, $6);  
		}
	| IFNDEF MACROSIG statement_list ENDIF 
		{
			$$ = op(IFNDEF, 2, s($2), $3);  
		}
 	| IFNDEF MACROSIG statement_list ELSE statement_list ENDIF
		{
			$$ = op(IFNDEF, 3, s($2), $3, $5);  
		}
 	| IFNDEF MACROSIG statement_list elseif_conditional_block ENDIF
		{
			$$ = op(IFNDEF, 3, s($2), $3, $4);  
		}
 	| IFNDEF MACROSIG statement_list elseif_conditional_block ELSE statement_list ENDIF
		{
			$$ = op(IFNDEF, 4, s($2), $3, $4, $6);  
		}
;

elseif_conditional_block
	:	elseif_conditional { $$ = $1; }
	|	elseif_conditional_block elseif_conditional { $$ = op(ELSEIFLIST, 2, $1, $2); } 
;

elseif_conditional
	: ELSEIF expression statement_list 
		{			
			$$ = op(ELSEIF, 2, $2, $3);
		}
;

statement_list
	: statement { $$ = $1; }
	| statement_list statement { $$ = op(LIST, 2, $1, $2); }
;

statement
	: define_directive { $$ = $1; }
	| undef_directive { $$ = $1; }
	| include_directive { $$ = $1; }
	| readdefs_directive { $$ = $1; }
	| conditional_directive { $$ = $1; }
	| macro_statement { $$ = op(PRINT, 1, $1); }
	| ANYCHARS { $$ = op(PRINT, 1, s($1)); }
	| EOB { YYACCEPT; }
;

%%

static union ppUnionCell *s(char *value) {
	MotoPP *ppenv = motopp_getEnv();
	PPUnionCell *p;

	p = (PPUnionCell *)mman_malloc(ppenv->mpool, sizeof(PPValueCell));
	
	p->type = VALUE_TYPE;					   
	p->valuecell.kind = STRING_VALUE;					   
	p->valuecell.lineno = pplineno;	

	p->valuecell.value = value;
	value[strlen(value)] = '\0';
	
	return p;
}

static union ppUnionCell *ls(char *value) {
	MotoPP *ppenv = motopp_getEnv();
	PPUnionCell *p;

	p = (PPUnionCell *)mman_malloc(ppenv->mpool, sizeof(PPValueCell));
	
	p->type = VALUE_TYPE;					   
	p->valuecell.kind = STRING_VALUE;					   
	p->valuecell.lineno = pplineno;	
	
	p->valuecell.value = emalloc(strlen(value)-1);
        strncpy(p->valuecell.value,value+1,strlen(value)-2);
	((char*)p->valuecell.value)[strlen(value) - 2] = '\0';
        hset_add(ppenv->ptrs,p->valuecell.value);

	/* fprintf(stderr,"%s\n",p->valuecell.value); */
	return p;
}

static union ppUnionCell *op(int opcode, int opcount, ...) {
	MotoPP *ppenv = motopp_getEnv();
	PPUnionCell *p;
	va_list ap;
	size_t size;
	int i;
	
	size = sizeof(PPOpCell) + ((opcount - 1) * sizeof(PPUnionCell *));
	p = (PPUnionCell *)mman_malloc(ppenv->mpool, size);

	p->type = OP_TYPE;
	p->opcell.opcode = opcode;
	p->opcell.opcount = opcount;
	va_start(ap, opcount);
	for (i = 0; i < opcount; i++) {
		p->opcell.operands[i] = va_arg(ap, union ppUnionCell *);
	}
	va_end(ap);
	p->opcell.lineno = pplineno;	

	return p;
}

static void freeCell(PPUnionCell *p) {
	int i;
	
	if (!p) {
		return;	
	}		
	if (p->type == OP_TYPE) {
		int opcount = p->opcell.opcount;
		for (i = 0; i < opcount; i++) {
			freeCell(p->opcell.operands[i]);
		}
	}
}

/*=============================================================================
// end of file: $RCSfile: motopp.y,v $
==============================================================================*/

