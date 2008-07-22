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

   $RCSfile: mx.y,v $   
   $Revision: 1.11 $
   $Author: dhakim $
   $Date: 2003/05/29 02:14:08 $
 
==============================================================================*/
#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>

#include "memsw.h"

#include "sharedmem.h"
#include "symboltable.h"
#include "err.h"
#include "log.h"

#include "cell.h"
#include "mxcg.h"

extern char yytext[];
extern FILE *yyin;
extern int yylineno;

static UnionCell *s(char *value);
static UnionCell *op(int opcode, int opcount, ...);
static UnionCell *listAdd(UnionCell * list, UnionCell* addition);

static void freeCell(UnionCell *p);

extern MXC *c;

%}

%union {
	char *sval;
	union unionCell *cp;
}

%token <sval> ARRAY_TYPE
%token <sval> FUNC_TYPE
%token <sval> AUTO 
%token <sval> BRACKET_L 
%token <sval> BRACKET_R 
%token <sval> CHAR 
%token <sval> CONST 
%token <sval> COMMENT
%token <sval> DOUBLE 
%token <sval> ELLIPSIS
%token <sval> ENUM 
%token <sval> EXTERN 
%token <sval> XFILE 
%token <sval> FLOAT 
%token <sval> IDENTIFIER 
%token <sval> INTERFACE 
%token <sval> INT
%token <sval> LIBRARY_NAME 
%token <sval> LONG 
%token <sval> MAP
%token <sval> OPERATOR
%token <sval> REGISTER
%token <sval> SCOPED_IDENTIFIER 
%token <sval> SCOPED_OPERATOR
%token <sval> SHORT 
%token <sval> SIGNED 
%token <sval> STAR
%token <sval> STATIC 
%token <sval> STRING_LITERAL 
%token <sval> STRUCT 
%token <sval> TAG_LITERAL 
%token <sval> TRACKED
%token <sval> TYPE 
%token <sval> UNION 
%token <sval> UNSIGNED 
%token <sval> VOID
%token <sval> VOLATILE 

%type <cp> c_declaration_specifiers 
%type <cp> c_function_declaration
%type <cp> c_function_declarator 
%type <cp> c_function_direct_declarator
%type <cp> c_parameter_declaration
%type <cp> c_parameter_declarator 
%type <cp> c_parameter_direct_declarator
%type <cp> c_parameter_list 
%type <cp> c_parameter_type_list
%type <cp> c_pointer 
%type <cp> c_type_qualifier 
%type <cp> c_type_qualifier_list 
%type <cp> c_type_specifier
%type <cp> extension_declaration 
%type <cp> extension_file_qualifier 
%type <cp> extension_header_declaration
%type <cp> extension_header_qualifier 
%type <cp> extension_header_qualifier_list
%type <cp> extension_includepath_qualifier
%type <cp> extension_librarypath_qualifier 
%type <cp> extension_archive_qualifier 
%type <cp> extension_include_qualifier 
%type <cp> extension_library_qualifier
%type <cp> interface_declaration 
%type <cp> interface_mapping 
%type <cp> local_file_specification
%type <cp> m_function_declaration
%type <cp> m_function_declarator
%type <cp> m_function_name
%type <cp> m_operator_declaration
%type <cp> m_operator_declarator
%type <cp> m_operator_name
%type <cp> m_parameter_declaration
%type <cp> m_parameter_declarator
%type <cp> m_parameter_list 
%type <cp> m_parameter_type_list
%type <cp> m_type
%type <cp> m_type_list
%type <cp> standard_file_specification

%token ARCHIVE
%token CLASS 
%token COLON 
%token COMMA 
%token CURLY_L 
%token CURLY_R 
%token EXTENSION 
%token EXTDECL
%token INCLUDE 
%token INCLUDEPATH
%token LIBRARY
%token LIBRARYPATH
%token LIST
%token PAREN_L 
%token PAREN_R 
%token SEMICOLON 

%token C_FUNCTION_DECLARATION
%token C_FUNCTION_DIRECT_DECLARATOR
%token C_PARAMETER_DECLARATION
%token C_PARAMETER_LIST

%token M_FUNCTION_DECLARATION
%token M_OPERATOR_DECLARATION
%token M_FUNCTION_DECLARATOR
%token M_OPERATOR_DECLARATOR
%token M_FUNCTION_NAME
%token M_PARAMETER_DECLARATOR
%token M_PARAMETER_DECLARATION
%token M_PARAMETER_TYPE_LIST
%token M_PARAMETER_LIST
%token M_PARAMETER_DECLARATION
%token M_DECLARATION_SPECIFIERS
%token M_TYPE_SPECIFIER

%%
program
	: translation_unit { } 
;

translation_unit
	: translation_unit extension_declaration  { mxc_c(c, $2); freeCell($2); }
	|
;

extension_declaration 
	: extension_header_declaration INTERFACE interface_declaration
		{
			$$ = op(EXTENSION, 2, $1, $3);
		}
	;

extension_header_declaration
	: extension_declaration extension_header_qualifier_list
		{
			$$ = op(LIST, 2, $1, $2);
		}	 
	;

extension_declaration
	: EXTENSION IDENTIFIER
		{
			$$ = op(EXTDECL, 1, s($2));
		}	 
	;

extension_header_qualifier_list
	: extension_header_qualifier { $$ = $1; }
	| extension_header_qualifier_list extension_header_qualifier
		{
			$$ = op(LIST, 2, $1, $2);
		}
	;

extension_header_qualifier
	:	extension_file_qualifier { $$ = $1; }
	|	extension_includepath_qualifier { $$ = $1; }
	|	extension_librarypath_qualifier { $$ = $1; }
	|	extension_archive_qualifier { $$ = $1; }
	|	extension_include_qualifier { $$ = $1; }
	|	extension_library_qualifier { $$ = $1; }
	;

extension_file_qualifier
	: XFILE local_file_specification
		{
			$$ = op(XFILE, 1, $2);
		}
	;

extension_include_qualifier
	: INCLUDE standard_file_specification
		{ $$ = op(INCLUDE, 1, $2); }
	| INCLUDE local_file_specification
		{ $$ = op(INCLUDE, 1, $2); }
	;

extension_archive_qualifier
	: ARCHIVE local_file_specification
		{ $$ = op(ARCHIVE, 1, $2); }
	;
	
extension_includepath_qualifier
	: INCLUDEPATH local_file_specification
		{ $$ = op(INCLUDEPATH, 1, $2); }
	;

extension_librarypath_qualifier
	: LIBRARYPATH local_file_specification
		{ $$ = op(LIBRARYPATH, 1, $2); }
	;
	
extension_library_qualifier
	: LIBRARY LIBRARY_NAME
		{ $$ = op(LIBRARY, 1, s($2)); }
	;

standard_file_specification
	: TAG_LITERAL { $$ = s($1); }
	;

local_file_specification
	: STRING_LITERAL { $$ = s($1); }
	;

interface_declaration
	:	interface_mapping  { $$ = $1; }
	|	interface_declaration interface_mapping
		{
			$$ = op(LIST, 2, $1, $2);
		}
	;
	
interface_mapping
	: m_function_declaration MAP c_function_declaration SEMICOLON 
		{ 
			$$ = op(MAP, 2, $1, $3);
		}
	| m_operator_declaration MAP c_function_declaration SEMICOLON 
		{ 
			$$ = op(MAP, 2, $1, $3);
		}
	;
	
c_function_declaration
	: c_declaration_specifiers c_function_declarator
		{
			$$ = op(C_FUNCTION_DECLARATION, 2, $1, $2);
		}
	;

c_function_declarator
	: c_pointer c_function_direct_declarator
		{
			$$ = op(LIST, 2, $1, $2);
		}
	| c_function_direct_declarator
		{
			$$ = $1;
		}
	;

c_function_direct_declarator
	: IDENTIFIER PAREN_L c_parameter_type_list PAREN_R
		{
			$$ = op(C_FUNCTION_DIRECT_DECLARATOR, 2, s($1), $3);
		}
	| IDENTIFIER PAREN_L PAREN_R
		{
			$$ = op(C_FUNCTION_DIRECT_DECLARATOR, 1, s($1));
		}
	;

c_parameter_declarator
	: c_pointer c_parameter_direct_declarator
		{
			$$ = op(LIST, 2, $1, $2);
		}
	| c_parameter_direct_declarator
		{
			$$ = $1;
		}
	;

c_parameter_direct_declarator
	: IDENTIFIER { $$ = s($1); }
	;

c_parameter_declaration
	: c_declaration_specifiers c_parameter_declarator
		{
			$$ = op(C_PARAMETER_DECLARATION, 2, $1, $2);
		}
	;

c_parameter_type_list
	: c_parameter_list
		{
			$$ = $1;
		}
	| c_parameter_list COMMA ELLIPSIS
		{
			$$ = op(C_PARAMETER_LIST, 2, $1, s($3));
		}
	;

c_parameter_list
	: c_parameter_declaration
		{
			$$ = $1;
		}
	| c_parameter_list COMMA c_parameter_declaration
		{
			$$ = op(C_PARAMETER_LIST, 2, $1, $3);
		}
	;

c_pointer
	: STAR
		{
			$$ = s($1);
		}
	| STAR c_type_qualifier_list
		{
			$$ = op(LIST, 2, s($1), $2);
		}
	| STAR c_pointer
		{
			$$ = op(LIST, 2, s($1), $2);
		}
	| STAR c_type_qualifier_list c_pointer
		{
			$$ = op(LIST, 3, s($1), $2, $3);
		}
	;

c_declaration_specifiers
	: c_type_specifier { $$ = $1; }
	| c_type_qualifier c_declaration_specifiers
		{ $$ = op(LIST, 2, $1, $2); }
	;

c_type_qualifier_list
	: c_type_qualifier
		{
			$$ = $1;
		}
	| c_type_qualifier_list c_type_qualifier
		{
			$$ = op(LIST, 2, $1, $2);
		}
	;

c_type_qualifier
	: CONST { $$ = s($1); }
	| VOLATILE { $$ = s($1); }
	| SIGNED { $$ = s($1); }
	| UNSIGNED { $$ = s($1); }
	| STRUCT { $$ = s($1); }	
	| UNION { $$ = s($1); }	
	| ENUM { $$ = s($1); }			
	;

c_type_specifier
	: VOID		{ $$ = s($1); }
	| CHAR		{ $$ = s($1); }
	| SHORT		{ $$ = s($1); }
	| INT			{ $$ = s($1); }
	| LONG		{ $$ = s($1); }
	| FLOAT		{ $$ = s($1); }
	| DOUBLE		{ $$ = s($1); }
	| IDENTIFIER	{ $$ = s($1); }
	;

m_function_declaration
	: COMMENT m_type m_function_declarator
		{
			$$ = op(M_FUNCTION_DECLARATION, 4, $2, $3, s("0"),s($1));
		}
	| COMMENT TRACKED m_type m_function_declarator
		{
			$$ = op(M_FUNCTION_DECLARATION, 4, $3, $4, s("1"),s($1));
		}
	| m_type m_function_declarator
		{
			$$ = op(M_FUNCTION_DECLARATION, 4, $1, $2, s("0"),s("/****/"));
		}
   | TRACKED m_type m_function_declarator
		{
			$$ = op(M_FUNCTION_DECLARATION, 4, $2, $3, s("1"),s("/****/"));
		}
	;

m_operator_declaration
	: COMMENT m_type m_operator_declarator
		{
			$$ = op(M_OPERATOR_DECLARATION, 4, $2, $3, s("0"),s($1));
		}
	| COMMENT TRACKED m_type m_operator_declarator
		{
			$$ = op(M_OPERATOR_DECLARATION, 4, $3, $4, s("1"),s($1));
		}
	| m_type m_operator_declarator
		{
			$$ = op(M_OPERATOR_DECLARATION, 4, $1, $2, s("0"),s("/****/"));
		}
   | TRACKED m_type m_operator_declarator
		{
			$$ = op(M_OPERATOR_DECLARATION, 4, $2, $3, s("1"),s("/****/"));
		}
	;

m_function_declarator
	: m_function_name PAREN_L m_parameter_type_list PAREN_R
		{
			$$ = op(M_FUNCTION_DECLARATOR, 2, $1, $3);
		}
	| m_function_name PAREN_L PAREN_R
		{
			$$ = op(M_FUNCTION_DECLARATOR, 1, $1);
		}
	;

m_operator_declarator
	: m_operator_name PAREN_L m_parameter_type_list PAREN_R
		{
			$$ = op(M_OPERATOR_DECLARATOR, 2, $1, $3);
		}
	| m_operator_name PAREN_L PAREN_R
		{
			$$ = op(M_OPERATOR_DECLARATOR, 1, $1);
		}
	;

m_function_name
	:	IDENTIFIER 
		{ 
			$$ = s($1);
		}
	|	SCOPED_IDENTIFIER
		{
			$$ = s($1);
		}
	;

m_operator_name
	:	OPERATOR
		{ 
			$$ = s($1);
		}
	|	SCOPED_OPERATOR
		{
			$$ = s($1);
		}
	;

m_parameter_type_list
	: m_parameter_list
		{
			$$ = $1;
		}
 	| m_parameter_list COMMA ELLIPSIS
		{
 			$$ = op(M_PARAMETER_LIST, 2, $1, s($3));
		}
	;

m_parameter_list
	: m_parameter_declaration
		{
			$$ = $1;
		}
	| m_parameter_list COMMA m_parameter_declaration
		{
			$$ = op(M_PARAMETER_LIST, 2, $1, $3);
		}
	;

m_parameter_declaration
	: m_type m_parameter_declarator
		{
			$$ = op(M_PARAMETER_DECLARATION, 2, $1, $2);
		}
	;

m_parameter_declarator
	: IDENTIFIER 
		{ 
			$$ = s($1);
		}
	;

m_type
	: VOID { $$ = op(TYPE, 1, s($1)); }
	| CHAR { $$ = op(TYPE, 1, s($1)); }
	| SHORT	{ $$ = op(TYPE, 1, s($1)); }
	| INT { $$ = op(TYPE, 1, s($1)); }
	| LONG { $$ = op(TYPE, 1, s($1)); }
	| FLOAT { $$ = op(TYPE, 1, s($1)); }
	| DOUBLE { $$ = op(TYPE, 1, s($1)); }	
	| IDENTIFIER { $$ = op(TYPE, 1, s($1)); }
	| m_type BRACKET_L BRACKET_R
		{ $$ = op(ARRAY_TYPE, 1, $1); }		
	| m_type PAREN_L PAREN_R
		{ $$ = op(FUNC_TYPE, 2,$1,op(LIST, 0)); }
	| m_type PAREN_L m_type_list PAREN_R
		{ $$ = op(FUNC_TYPE, 2,$1,$3); }
;

m_type_list
	: m_type { $$ = op(LIST, 1,$1); }
	| m_type_list COMMA m_type { $$ = listAdd($1,$3); }
;

%%
UnionCell *s(char *value) {
	UnionCell *p;

	p = (UnionCell *)malloc(sizeof(StrCell));
	if (p == NULL) {
		yyerror("Memory allocation failure (ls)");
	}	
	
	p->celltype = VALUE_TYPE;					   
	
	p->strcell.value = value;
	
	return p;
}

UnionCell *op(int opcode, int opcount, ...) {
	UnionCell *p;
	va_list ap;
	size_t size;
	int i;
	
	size = sizeof(OpCell) + ((opcount - 1) * sizeof(UnionCell *));
	p = (UnionCell *)malloc(size);
	if (p == NULL) {
		yyerror("Memory allocation failure (op)");
	}	

	p->celltype = OP_TYPE;
	p->opcell.opcode = opcode;
	p->opcell.opcount = opcount;
	va_start(ap, opcount);
	for (i = 0; i < opcount; i++) {
		p->opcell.operands[i] = va_arg(ap, UnionCell *);
	}
	va_end(ap);

	return p;
}

UnionCell *
listAdd(UnionCell * list, UnionCell* addition) {
	UnionCell *p;
	size_t size;
	size = sizeof(OpCell) + ((list->opcell.opcount) * sizeof(UnionCell *));
	
	p = (UnionCell *)realloc(list,size);
	
	p->opcell.operands[p->opcell.opcount] = addition;
	p->opcell.opcount += 1;
	
	return p;
}

void freeCell(UnionCell *p) {
	int i;
	
	if (!p) {
		return;	
	}	
	
	if (p->celltype == OP_TYPE) {
		int opcount = p->opcell.opcount;
		for (i = 0; i < opcount; i++) {
			freeCell(p->opcell.operands[i]);
		}
	}
	free(p);
}

/*=============================================================================
// end of file: $RCSfile: mx.y,v $
==============================================================================*/

