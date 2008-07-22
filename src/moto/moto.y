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

   $RCSfile: moto.y,v $   
   $Revision: 1.76 $
   $Author: dhakim $
   $Date: 2003/05/29 02:14:08 $
 
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

#include "excpfn.h"
#include "mman.h"
#include "intstack.h"
#include "runtime.h"
#include "stack.h"
#include "symboltable.h"
#include "log.h"

#include "cell.h"
#include "number.h"
#include "motoerr.h"
#include "motolex.h"
#include "motohook.h"
#include "mototree.h"
#include "motoutil.h"

#define YYPARSE_PARAM param
#define YYLEX_PARAM param

#define ENV (MotoEnv *)param

#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_ALL
#endif
	
void pushScope(MotoEnv *env, int type);
int popScope(MotoEnv *env, int verify);
int anyScope(MotoEnv* env);
int definitionScope(MotoEnv *env);
int loopScope(MotoEnv *env);
int tryScope(MotoEnv *env);
int classDefinitionScope(MotoEnv *env);

extern char* motoyytext;
extern int yyerror(char *s);

static inline UnionCell *nullcell(MotoEnv *env, SVS *value);
static inline UnionCell *b(MotoEnv *env, SVS *value);
static inline UnionCell *c(MotoEnv *env, SVS *value);
static inline UnionCell *integer(MotoEnv *env, SVS *value);
static inline UnionCell *fp(MotoEnv *env, SVS *value);
static inline UnionCell *s(MotoEnv *env, SVS *value);
static inline UnionCell *as(MotoEnv *env, SVS *value);
static inline UnionCell *ls(MotoEnv *env, SVS *value);
static inline UnionCell *id(MotoEnv *env, SVS *name);
static inline UnionCell *rx(MotoEnv *env, SVS *name);
static inline UnionCell *op(MotoEnv *env, int opcode, MetaInfo* meta,int opcount, ...);
static inline UnionCell *listAdd(MotoEnv *env, union unionCell* list, union unionCell* addition);

static inline MetaInfo* IVSMetaInfo(IVS* ivs) ;
static inline MetaInfo* SVSMetaInfo(SVS* ivs) ;
static inline MetaInfo* UCMetaInfo(UnionCell* uc);

static int hexCharToInt(char c);

%}

%pure_parser

%union {
        union unionCell *cp;
	SVS* svs;
        IVS* ivs;
}

%{
int do_recover(YYSTYPE* lvalp, char embedded ); /* FIXME: There's got to be a better way */
%}

%token <svs> ANYCHARS 
%token <svs> BOOLEAN
%token <svs> THIS 
%token <svs> CAST 
%token <svs> CHAR 
%token <svs> FPNUM 
%token <svs> IDENTIFIER 
%token <svs> INTEGER 
%token <svs> MOTONULL 
%token <svs> NAME 
%token <svs> REGEX 
%token <svs> STRING 
%token <svs> WS 

%token <ivs> ADD_ASSIGN 
%token <ivs> AND 
%token <ivs> ASSIGN 
%token <ivs> ARRAY_DECLARATION
%token <ivs> ARRAY_NEW
%token <ivs> ARRAY_LVAL
%token <ivs> ARRAY_RVAL
%token <ivs> ARRAY_TYPE
%token <ivs> BITWISE_AND
%token <ivs> BITWISE_OR
%token <ivs> BITWISE_XOR
%token <ivs> BITWISE_NOT
%token <ivs> BREAK
%token <ivs> CASE
%token <ivs> CLASS
%token <ivs> CLASSDEF
%token <ivs> CLOSEBRACKET
%token <ivs> CLOSECURLY
%token <ivs> CLOSEDOLLAR
%token <ivs> CLOSEPAREN
%token <ivs> COLON
%token <ivs> COMMA
%token <ivs> CONTINUE
%token <ivs> DEC
%token <ivs> DECLARE
%token <ivs> DECLARATION
%token <ivs> DECLARATOR
%token <ivs> DEF
%token <ivs> HOF_NEW
%token <ivs> DEFAULT
%token <ivs> DEFINE
%token <ivs> DELETE
%token <ivs> DEREFERENCE_LVAL
%token <ivs> DEREFERENCE_RVAL
%token <ivs> DIV_ASSIGN
%token <ivs> DO
%token <ivs> DOT
%token <ivs> EXCP_CATCH
%token <ivs> EXCP_ENDTRY
%token <ivs> EXCP_FINALLY
%token <ivs> EXCP_THROW
%token <ivs> EXCP_TRY
%token <ivs> EXCP_TRY_HANDLERS
%token <ivs> EXCP_ECATCH
%token <ivs> EXCP_EFINALLY
%token <ivs> EXCP_ETHROW
%token <ivs> EXCP_ETRY
%token <ivs> EBREAK
%token <ivs> ECLASS
%token <ivs> ECONTINUE
%token <ivs> EDEFINE
%token <ivs> EIF 
%token <ivs> EELSE
%token <ivs> ESWITCH
%token <ivs> ECASE
%token <ivs> ECLASSDEF 
%token <ivs> EDEFAULT
%token <ivs> EWHILE 
%token <ivs> EFOR 
%token <ivs> EPRINT
%token <ivs> ERETURN
%token <ivs> EUSE 
%token <ivs> DOLLAROPEN
%token <ivs> ELSE 
%token <ivs> ELSEIF 
%token <ivs> ELSEIFLIST
%token <ivs> ENDCLASS
%token <ivs> ENDFOR
%token <ivs> ENDDEF
%token <ivs> ENDIF 
%token <ivs> ENDSWITCH 
%token <ivs> ENDWHILE 
%token <ivs> FCALL
%token <ivs> FN 
%token <ivs> FOR 
%token <ivs> FUNC_TYPE
%token <ivs> GLOBAL
%token <ivs> IDENTIFY
%token <ivs> IF 
%token <ivs> INC 
%token <ivs> LIST 
%token <ivs> MATH_ADD
%token <ivs> MATH_DIV
%token <ivs> MATH_MOD
%token <ivs> MATH_MULT
%token <ivs> MATH_SUB
%token <ivs> METHOD
%token <ivs> MINUS
%token <ivs> MOD_ASSIGN
%token <ivs> MULT_ASSIGN 
%token <ivs> NEW	
%token <ivs> NOOP
%token <ivs> NOT
%token <ivs> OR 
%token <ivs> OPENCURLY
%token <ivs> OPENPAREN
%token <ivs> OPENBRACKET
%token <ivs> OPENCLOSEBRACKET
%token <ivs> PARAMLIST 
%token <ivs> POSTFIX_DEC
%token <ivs> POSTFIX_INC
%token <ivs> PLUS 
%token <ivs> PREFIX_DEC
%token <ivs> PREFIX_INC
%token <ivs> PRINT 
%token <ivs> QUESTION
%token <ivs> REL_EQ_M
%token <ivs> REL_EQ_S
%token <ivs> REL_GT_M
%token <ivs> REL_GT_S
%token <ivs> REL_GTE_M
%token <ivs> REL_GTE_S
%token <ivs> REL_LT_M
%token <ivs> REL_LT_S
%token <ivs> REL_LTE_M
%token <ivs> REL_LTE_S
%token <ivs> REL_NE_M
%token <ivs> REL_NE_S
%token <ivs> RETURN
%token <ivs> RX_MATCH
%token <ivs> RX_NOT_MATCH
%token <ivs> SC 
%token <ivs> SCOPE
%token <ivs> SHIFT_LEFT
%token <ivs> SHIFT_RIGHT
%token <ivs> SUB_ASSIGN 
%token <ivs> SWITCH 
%token <ivs> TERNARY
%token <ivs> TYPE  
%token <ivs> TYPED_UNKNOWN  
%token <ivs> UNTYPED_UNKNOWN  
%token <ivs> USE 
%token <ivs> WHILE 
%token <ivs> VRETURN
%token <ivs> DECLARE_INLINE
%token <ivs> ARRAY_INIT

%left MATH_ADD 
%left MATH_DIV 
%left MATH_MOD
%left MATH_MULT 
%left MATH_SUB 
%left OPENBRACKET

%nonassoc UMINUS
%nonassoc LOWER_THAN_EELSE
%nonassoc EELSE

%nonassoc ENDIF
%nonassoc ELSE

%type <cp> try_block
%type <cp> try_handlers
%type <cp> catch_block_list
%type <cp> catch_block
%type <cp> finally_block
%type <cp> throw_statement
%type <cp> embedded_try_block
%type <cp> embedded_try_handlers
%type <cp> embedded_catch_block_list
%type <cp> embedded_catch_block
%type <cp> embedded_finally_block
%type <cp> embedded_throw_statement
%type <cp> additive_expression
%type <cp> allocation_expression
%type <cp> array_declaration_list
%type <cp> array_declaration
%type <cp> array_index_list
%type <cp> array_index
%type <cp> array_subscript_expression
%type <cp> bitwise_expression
%type <cp> function_declaration
%type <cp> declaration
%type <cp> declaration_list
%type <cp> assignment_expression
%type <cp> assignment_operator
%type <cp> embedded_case_statement
%type <cp> embedded_case_statement_list
%type <cp> embedded_class_statement
%type <cp> embedded_class_statement_list
%type <cp> class_definition_statement
%type <cp> embedded_class_definition_statement
%type <cp> definition_statement
%type <cp> dereference_expression
%type <cp> function_definition_statement
%type <cp> functional_expression
%type <cp> embedded_definition_statement
%type <cp> embedded_function_definition_statement
%type <cp> return_statement
%type <cp> embedded_return_statement
%type <cp> case_statement
%type <cp> case_statement_list
%type <cp> cast_expression 
%type <cp> conditional_expression 
%type <cp> embedded_conditional_statement
%type <cp> array_initializer
%type <cp> array_initializer_value_list
%type <cp> inline_declaration
%type <cp> embedded_declare_statement
%type <cp> embedded_do_statement
%type <cp> if_statement 
%type <cp> lvalue_expression 
%type <cp> switch_statement
%type <cp> conditional_statement
%type <cp> declare_statement
%type <cp> do_statement
%type <cp> elseif_conditional 
%type <cp> elseif_conditional_block 
%type <cp> equality_expression 
%type <cp> expression 
%type <cp> expression_list
%type <cp> free_expression
%type <cp> embedded_iterative_statement
%type <cp> for_statement
%type <cp> while_statement
%type <cp> iterative_statement 
%type <cp> embedded_jump_statement
%type <cp> jump_statement 
%type <cp> logical_and_expression
%type <cp> logical_or_expression 
%type <cp> multiplicative_expression 
%type <cp> postfix_expression 
%type <cp> primary_expression 
%type <cp> embedded_print_statement
%type <cp> print_statement
%type <cp> relational_expression
%type <cp> shift_expression
%type <cp> embedded_statement
%type <cp> embedded_statement_list
%type <cp> embedded_block
%type <cp> statement 
%type <cp> statement_list
%type <cp> type
%type <cp> unified_type 
%type <cp> type_qualifier
%type <cp> unary_expression
%type <cp> variable_declarator_list
%type <cp> variable_declarator 
%type <cp> embedded_use_statement
%type <cp> use_statement
%type <cp> embedded_constructor_definition_statement
%type <cp> functional_type
%type <cp> type_list
%%

program: statement_list { MotoEnv *env = param; vec_add(env->tree->cells, $1); }
	;

primary_expression
	: INTEGER  { $$ = integer(ENV, $1); }
	| FPNUM { $$ = fp(ENV, $1); }
	| STRING { $$ = ls(ENV, $1); }
	| CHAR { $$ = c(ENV, $1); }
	| BOOLEAN { $$ = b(ENV, $1); }
	| MOTONULL { $$ = nullcell(ENV, $1); }
	| THIS { if (!(classDefinitionScope(ENV) && definitionScope(ENV))) 
				moto_illegalThis(SVSMetaInfo($1)); 
			 $$ = id(ENV, $1); }
	| NAME { $$ = id(ENV, $1); }
	| REGEX { $$ = rx(ENV, $1); }
	| OPENPAREN expression CLOSEPAREN { $$ = $2; }
;

postfix_expression
	: postfix_expression INC
		{ 
			if ($1->type == OP_TYPE && $1->opcell.opcode == DEREFERENCE_RVAL) {
				$1->opcell.opcode = DEREFERENCE_LVAL; 
				$$ = op(ENV, POSTFIX_INC,IVSMetaInfo($2),  1, $1);
		  	} else if ( $1->type == OP_TYPE && $1->opcell.opcode == ARRAY_RVAL ) {
		  		$1->opcell.opcode = ARRAY_LVAL;
		  		$$ = op(ENV, POSTFIX_INC,IVSMetaInfo($2),  1, $1);
		  	} else if ( $1->type == VALUE_TYPE && $1->valuecell.kind == ID_VALUE) {
		  		$$ = op(ENV, POSTFIX_INC,IVSMetaInfo($2),  1, $1);
		 	} else {
				$$ = op(ENV, POSTFIX_INC,IVSMetaInfo($2),  1, $1);
			}
		}
	| postfix_expression DEC
		{ 
			if ($1->type == OP_TYPE && $1->opcell.opcode == DEREFERENCE_RVAL) {
				$1->opcell.opcode = DEREFERENCE_LVAL; 
				$$ = op(ENV, POSTFIX_DEC,IVSMetaInfo($2),  1, $1);
		  	} else if ( $1->type == OP_TYPE && $1->opcell.opcode == ARRAY_RVAL ) {
		  		$1->opcell.opcode = ARRAY_LVAL;
		  		$$ = op(ENV, POSTFIX_DEC,IVSMetaInfo($2),  1, $1);
		  	} else if ( $1->type == VALUE_TYPE && $1->valuecell.kind == ID_VALUE) {
		  		$$ = op(ENV, POSTFIX_DEC,IVSMetaInfo($2),  1, $1);
		 	} else {
				$$ = op(ENV, POSTFIX_DEC,IVSMetaInfo($2),  1, $1);
			}
		}
	| functional_expression {$$ = $1;}
	| dereference_expression {$$ = $1;}
	| array_subscript_expression {$$ = $1;}
	| primary_expression {$$=$1;}
;

functional_expression
	: postfix_expression OPENPAREN CLOSEPAREN
		{ 
			if($1->type == OP_TYPE && uc_opcode($1) == DEREFERENCE_RVAL)
				$$ = op(ENV, METHOD, UCMetaInfo($1),3,uc_operand($1,0),uc_operand($1,1),
					op(ENV, LIST, UCMetaInfo($1) ,0) );
			else if($1->type == VALUE_TYPE && $1->valuecell.kind == ID_VALUE)
				$$ = op(ENV, FN, UCMetaInfo($1), 2, $1,op(ENV, LIST, UCMetaInfo($1) ,0));
			else
				$$ = op(ENV, FCALL, UCMetaInfo($1), 2, $1,op(ENV, LIST, UCMetaInfo($1) ,0));				
		}
	| postfix_expression OPENPAREN expression_list CLOSEPAREN
		{
			int i=0;
			
			if($1->type == OP_TYPE && uc_opcode($1) == DEREFERENCE_RVAL) 
				$$ = op(ENV, METHOD, UCMetaInfo($1),3,uc_operand($1,0),uc_operand($1,1),$3);
			else if($1->type == VALUE_TYPE && $1->valuecell.kind == ID_VALUE) 
				$$ = op(ENV, FN, UCMetaInfo($1), 2, $1,$3);
			else
				$$ = op(ENV, FCALL, UCMetaInfo($1), 2, $1,$3);
			
			/* Does this call contain unknowns ? If so its an identify operation */
			for(i=0;i<$3->opcell.opcount;i++)
				if(uc_opcode(uc_operand($3,i)) == TYPED_UNKNOWN || 
					uc_opcode(uc_operand($3,i)) == UNTYPED_UNKNOWN) {
						$$ = op(ENV, IDENTIFY, UCMetaInfo($1), 1, $$); break;
					}
				
		}
;
		
allocation_expression
	: NEW NAME OPENPAREN CLOSEPAREN
		{ $$ = op(ENV, NEW, IVSMetaInfo($1), 2, s(ENV, $2),op(ENV, LIST, SVSMetaInfo($2) ,0));}
	| NEW NAME OPENPAREN expression_list CLOSEPAREN
		{ $$ = op(ENV, NEW,IVSMetaInfo($1),  2, s(ENV, $2),$4); }
	| NEW NAME array_index_list
		{ $$ = 
			op(ENV, ARRAY_NEW,IVSMetaInfo($1),3, 
				op(ENV, TYPE, SVSMetaInfo($2),2, s(ENV, $2), 
					op(ENV,LIST,SVSMetaInfo($2),0)
				),
				$3,
				op(ENV,LIST,SVSMetaInfo($2),0)
			); 
		}
	| NEW NAME array_index_list array_declaration_list
		{ $$ = 
			op(ENV, ARRAY_NEW,IVSMetaInfo($1),3, 
				op(ENV, TYPE, SVSMetaInfo($2),2, s(ENV, $2), 
					op(ENV,LIST,SVSMetaInfo($2),0)
				),
				$3,
				$4
			); 
		}
	| HOF_NEW functional_type array_index_list
		{ $$ = op(ENV, ARRAY_NEW,IVSMetaInfo($1),3,$2,$3,op(ENV,LIST,IVSMetaInfo($1),0)); }
	| HOF_NEW functional_type array_index_list array_declaration_list
		{ $$ = op(ENV, ARRAY_NEW,IVSMetaInfo($1),3,$2,$3,$4); }
;

array_initializer
	: OPENCURLY array_initializer_value_list CLOSECURLY
		{$$ = op(ENV, ARRAY_INIT,IVSMetaInfo($1),1, $2);}
	| OPENCURLY  CLOSECURLY
		{$$ = op(ENV, ARRAY_INIT,IVSMetaInfo($1),1, op(ENV, LIST,  IVSMetaInfo($1) ,0));}
;

array_initializer_value_list
	: assignment_expression { $$ = op(ENV, LIST,  UCMetaInfo($1) ,1,$1); }
	| array_initializer { $$ = op(ENV, LIST,  UCMetaInfo($1) ,1,$1); }
	| array_initializer_value_list COMMA assignment_expression { $$ = listAdd(ENV,$1,$3) ; }
	| array_initializer_value_list COMMA array_initializer { $$ = listAdd(ENV,$1,$3) ; }
;

array_index_list
	: array_index { $$ = op(ENV, LIST,  UCMetaInfo($1) ,1,$1); }
	| array_index_list array_index { $$ = listAdd(ENV,$1,$2) ; }
;

array_index
	: OPENBRACKET expression CLOSEBRACKET
		{ $$ = $2; }
;

free_expression
	: DELETE postfix_expression
		{ $$ = op(ENV, DELETE, IVSMetaInfo($1), 1, $2); }
;
		
expression_list
	: assignment_expression { $$ = op(ENV, LIST,  UCMetaInfo($1) ,1, $1); }
	| expression_list COMMA assignment_expression { $$ = listAdd(ENV,$1,$3) ; }
;

unary_expression
	: MINUS postfix_expression 
		{ $$ = op(ENV, UMINUS,IVSMetaInfo($1),  1, $2); } 
	| INC postfix_expression 
		{ 
			if ($2->type == OP_TYPE && $2->opcell.opcode == DEREFERENCE_RVAL) {
				$2->opcell.opcode = DEREFERENCE_LVAL; 
				$$ = op(ENV, PREFIX_INC,IVSMetaInfo($1),  1, $2);
		  	} else if ( $2->type == OP_TYPE && $2->opcell.opcode == ARRAY_RVAL ) {
		  		$2->opcell.opcode = ARRAY_LVAL;
		  		$$ = op(ENV, PREFIX_INC,IVSMetaInfo($1),  1, $2);
		  	} else if ( $2->type == VALUE_TYPE && $2->valuecell.kind == ID_VALUE) {
		  		$$ = op(ENV, PREFIX_INC,IVSMetaInfo($1),  1, $2);
		 	} else {
				$$ = op(ENV, PREFIX_INC,IVSMetaInfo($1),  1, $2);
			}
		}

	| DEC postfix_expression 
		{ 
			if ($2->type == OP_TYPE && $2->opcell.opcode == DEREFERENCE_RVAL) {
				$2->opcell.opcode = DEREFERENCE_LVAL; 
				$$ = op(ENV, PREFIX_DEC,IVSMetaInfo($1),  1, $2);
		  	} else if ( $2->type == OP_TYPE && $2->opcell.opcode == ARRAY_RVAL ) {
		  		$2->opcell.opcode = ARRAY_LVAL;
		  		$$ = op(ENV, PREFIX_DEC,IVSMetaInfo($1),  1, $2);
		  	} else if ( $2->type == VALUE_TYPE && $2->valuecell.kind == ID_VALUE) {
		  		$$ = op(ENV, PREFIX_DEC,IVSMetaInfo($1),  1, $2);
		 	} else {
				$$ = op(ENV, PREFIX_DEC,IVSMetaInfo($1),  1, $2);
			}
		} 
	| NOT postfix_expression 
		{ $$ = op(ENV, NOT,IVSMetaInfo($1),  1, $2); } 
	| BITWISE_NOT postfix_expression
		{ $$ = op(ENV, BITWISE_NOT,IVSMetaInfo($1),  1, $2); } 
	| BITWISE_AND functional_expression
		{ if(uc_opcode($2) == IDENTIFY) $$ = $2;
		  else $$ = op(ENV, IDENTIFY, IVSMetaInfo($1),  1, $2); }
	| allocation_expression 
		{ $$ = $1; }
	| free_expression
		{ $$ = $1; }
	| postfix_expression
;

cast_expression
	: REL_LT_M unified_type REL_GT_M cast_expression
		{ $$ = op(ENV, CAST,UCMetaInfo($2),  2, $2, $4); }
	| REL_LT_M unified_type REL_GT_M array_initializer
		{ $$ = op(ENV, CAST,UCMetaInfo($2),  2, $2 ,$4); }	
	| REL_LT_M unified_type REL_GT_M QUESTION 
		{ $$ = op(ENV, TYPED_UNKNOWN ,UCMetaInfo($2) , 1,$2); }	
	| unary_expression
;

multiplicative_expression
	: multiplicative_expression MATH_MULT cast_expression  
		{ $$ = op(ENV, MATH_MULT,IVSMetaInfo($2) ,  2, $1, $3); }
	| multiplicative_expression MATH_DIV cast_expression  
		{ $$ = op(ENV, MATH_DIV,IVSMetaInfo($2) ,  2, $1, $3); }
	| multiplicative_expression MATH_MOD cast_expression  
		{ $$ = op(ENV, MATH_MOD,IVSMetaInfo($2) ,  2, $1, $3); }
	| QUESTION
		{ $$ = op(ENV, UNTYPED_UNKNOWN ,IVSMetaInfo($1) , 0); }			
	| cast_expression  
;
 
additive_expression
	: additive_expression PLUS multiplicative_expression  
		{ $$ = op(ENV, MATH_ADD,IVSMetaInfo($2) ,  2, $1, $3); }
	| additive_expression MINUS multiplicative_expression  
		{ $$ = op(ENV, MATH_SUB,IVSMetaInfo($2) ,  2, $1, $3); }
	| multiplicative_expression  
;

shift_expression
	: shift_expression SHIFT_LEFT additive_expression
		{ $$ = op(ENV, SHIFT_LEFT,IVSMetaInfo($2) ,  2, $1, $3); }
	| shift_expression SHIFT_RIGHT additive_expression
		{ $$ = op(ENV, SHIFT_RIGHT,IVSMetaInfo($2) ,  2, $1, $3); }
	| additive_expression
;

relational_expression
	: relational_expression REL_GT_M shift_expression
		{ $$ = op(ENV, REL_GT_M, IVSMetaInfo($2) , 2, $1, $3); }	
	| relational_expression REL_GT_S shift_expression
		{ $$ = op(ENV, REL_GT_S, IVSMetaInfo($2) , 2, $1, $3); }	
	| relational_expression REL_GTE_M shift_expression
		{ $$ = op(ENV, REL_GTE_M,IVSMetaInfo($2) ,  2, $1, $3); }	
	| relational_expression REL_GTE_S shift_expression
		{ $$ = op(ENV, REL_GTE_S,IVSMetaInfo($2) ,  2, $1, $3); }	
	| relational_expression REL_LT_M shift_expression
		{ $$ = op(ENV, REL_LT_M, IVSMetaInfo($2) , 2, $1, $3); }	
	| relational_expression REL_LT_S shift_expression
		{ $$ = op(ENV, REL_LT_S, IVSMetaInfo($2) , 2, $1, $3); }	
	| relational_expression REL_LTE_M shift_expression
		{ $$ = op(ENV, REL_LTE_M, IVSMetaInfo($2) , 2, $1, $3); }	
	| relational_expression REL_LTE_S shift_expression
		{ $$ = op(ENV, REL_LTE_S, IVSMetaInfo($2) , 2, $1, $3); }	
	| shift_expression
;

equality_expression
	: equality_expression REL_EQ_M relational_expression
		{ $$ = op(ENV, REL_EQ_M, IVSMetaInfo($2) , 2, $1, $3); }
	| equality_expression REL_NE_M relational_expression
		{ $$ = op(ENV, REL_NE_M, IVSMetaInfo($2) , 2, $1, $3); }
	| equality_expression REL_EQ_S relational_expression
		{ $$ = op(ENV, REL_EQ_S, IVSMetaInfo($2) , 2, $1, $3); }
	| equality_expression REL_NE_S relational_expression
		{ $$ = op(ENV, REL_NE_S, IVSMetaInfo($2) , 2, $1, $3); }
	| equality_expression RX_MATCH relational_expression
		{ $$ = op(ENV, RX_MATCH, IVSMetaInfo($2) , 2, $1, $3); }
	| equality_expression RX_NOT_MATCH relational_expression
		{ $$ = op(ENV, RX_NOT_MATCH, IVSMetaInfo($2) , 2, $1, $3); }
	| relational_expression 
;

bitwise_expression
	: bitwise_expression BITWISE_AND equality_expression
		{ $$ = op(ENV, BITWISE_AND, IVSMetaInfo($2) , 2, $1, $3); }
	| bitwise_expression BITWISE_OR equality_expression
		{ $$ = op(ENV, BITWISE_OR, IVSMetaInfo($2) , 2, $1, $3); }
	| bitwise_expression BITWISE_XOR equality_expression
		{ $$ = op(ENV, BITWISE_XOR, IVSMetaInfo($2) , 2, $1, $3); }
	| equality_expression
;

logical_and_expression
	: logical_and_expression AND bitwise_expression
		{ $$ = op(ENV, AND, IVSMetaInfo($2) , 2, $1, $3); }
	| bitwise_expression
;

logical_or_expression
	: logical_or_expression OR logical_and_expression
		{ $$ = op(ENV, OR, IVSMetaInfo($2) , 2, $1, $3); }
	| logical_and_expression  
;

conditional_expression
	: logical_or_expression
	| logical_or_expression QUESTION expression COLON conditional_expression
		{ $$ = op(ENV, TERNARY, IVSMetaInfo($2) , 3, $1, $3, $5);  }
;

lvalue_expression
	: array_subscript_expression {$1->opcell.opcode = ARRAY_LVAL; $$ = $1;}
	| dereference_expression {
		if ($1->opcell.opcode == DEREFERENCE_RVAL) {
			$1->opcell.opcode = DEREFERENCE_LVAL; $$=$1;
		}
	  }
;

assignment_expression
	: NAME assignment_operator assignment_expression
		{ $$ = op(ENV, ASSIGN, UCMetaInfo($2), 3, id(ENV, $1), $2, $3);  }
	| lvalue_expression assignment_operator assignment_expression 
		{ $$ = op(ENV, ASSIGN, UCMetaInfo($2), 3, $1, $2, $3); }
	| conditional_expression
;

dereference_expression
	: postfix_expression DOT NAME
		{ $$ = op(ENV, DEREFERENCE_RVAL, UCMetaInfo($1) ,2, $1, id(ENV, $3)); }
;
		
array_subscript_expression
	: postfix_expression array_index {$$=op(ENV, ARRAY_RVAL,UCMetaInfo($1),2,$1,$2);}
;

assignment_operator
	: ASSIGN { $$ = op(ENV, ASSIGN, IVSMetaInfo($1), 0);  }
	| ADD_ASSIGN { $$ = op(ENV, MATH_ADD, IVSMetaInfo($1), 0);  }
	| SUB_ASSIGN { $$ = op(ENV, MATH_SUB, IVSMetaInfo($1), 0);  }
	| MULT_ASSIGN { $$ = op(ENV, MATH_MULT, IVSMetaInfo($1), 0);  }
	| DIV_ASSIGN { $$ = op(ENV, MATH_DIV, IVSMetaInfo($1), 0);  }
	| MOD_ASSIGN { $$ = op(ENV, MATH_MOD, IVSMetaInfo($1), 0);  }
;

expression
	: assignment_expression
	| expression COMMA assignment_expression
		{ $$ = op(ENV, COMMA, IVSMetaInfo($2) , 2, $1, $3); }
;

conditional_statement
	: { pushScope(ENV,IF); } if_statement { popScope(ENV,IF); $$=$2;} 
	| { pushScope(ENV,SWITCH); } switch_statement { popScope(ENV,SWITCH); $$=$2;}
;

if_statement
	: IF expression CLOSEPAREN statement_list ENDIF 
		{ $$ = op(ENV, IF, IVSMetaInfo($1), 4, $2, $4, op(ENV, NOOP,IVSMetaInfo($1),  0), op(ENV, NOOP, IVSMetaInfo($1), 0));  }
	| IF expression CLOSEPAREN ENDIF 
		{ $$ = op(ENV, IF, IVSMetaInfo($1), 4, $2, op(ENV, NOOP, IVSMetaInfo($1), 0), op(ENV, NOOP, IVSMetaInfo($1), 0), op(ENV, NOOP, IVSMetaInfo($1), 0));  }
 	| IF expression CLOSEPAREN statement_list ELSE statement_list ENDIF 
		{ $$ = op(ENV, IF, IVSMetaInfo($1), 4, $2, $4, op(ENV, NOOP, IVSMetaInfo($1), 0), $6);  }
 	| IF expression CLOSEPAREN ELSE statement_list ENDIF 
		{ $$ = op(ENV, IF, IVSMetaInfo($1), 4, $2, op(ENV, NOOP, IVSMetaInfo($1), 0), op(ENV, NOOP, IVSMetaInfo($1), 0), $5);  }
 	| IF expression CLOSEPAREN ELSE ENDIF 
		{ $$ = op(ENV, IF, IVSMetaInfo($1), 4, $2, op(ENV, NOOP, IVSMetaInfo($1), 0), op(ENV, NOOP, IVSMetaInfo($1), 0), op(ENV, NOOP, IVSMetaInfo($1), 0));  }
 	| IF expression CLOSEPAREN statement_list ELSE ENDIF 
		{ $$ = op(ENV, IF, IVSMetaInfo($1), 4, $2, $4, op(ENV, NOOP, IVSMetaInfo($1), 0), op(ENV, NOOP, IVSMetaInfo($1), 0));  }
 	| IF expression CLOSEPAREN statement_list elseif_conditional_block ENDIF
		{ $$ = op(ENV, IF, IVSMetaInfo($1), 4, $2, $4, $5, op(ENV, NOOP, IVSMetaInfo($1), 0));  }
 	| IF expression CLOSEPAREN elseif_conditional_block ENDIF
		{ $$ = op(ENV, IF, IVSMetaInfo($1), 4, $2, op(ENV, NOOP, IVSMetaInfo($1), 0), $4, op(ENV, NOOP, IVSMetaInfo($1), 0));  }
 	| IF expression CLOSEPAREN statement_list elseif_conditional_block ELSE statement_list ENDIF 
		{ $$ = op(ENV, IF, IVSMetaInfo($1), 4, $2, $4, $5, $7);  }
 	| IF expression CLOSEPAREN elseif_conditional_block ELSE statement_list ENDIF 
		{ $$ = op(ENV, IF, IVSMetaInfo($1), 4, $2, op(ENV, NOOP, IVSMetaInfo($1), 0), $4, $6);  }
 	| IF expression CLOSEPAREN statement_list elseif_conditional_block ELSE ENDIF 
		{ $$ = op(ENV, IF, IVSMetaInfo($1), 4, $2, $4, $5, op(ENV, NOOP, IVSMetaInfo($1), 0));  }
 	| IF expression CLOSEPAREN elseif_conditional_block ELSE ENDIF 
		{ $$ = op(ENV, IF, IVSMetaInfo($1), 4, $2, op(ENV, NOOP, IVSMetaInfo($1), 0), $4, op(ENV, NOOP, IVSMetaInfo($1), 0));  }

	| IF error CLOSEPAREN
                { yyerrok; moto_malformedIf(IVSMetaInfo($1),'\0'); $$ = op(ENV, NOOP,IVSMetaInfo($1),  0); }
;

switch_statement
 	: SWITCH expression CLOSEPAREN case_statement_list ENDSWITCH
		{ $$ = op(ENV, SWITCH, IVSMetaInfo($1), 3, $2, op(ENV, NOOP, IVSMetaInfo($1), 0), $4);  }
 	| SWITCH expression CLOSEPAREN statement_list ENDSWITCH
		{ $$ = op(ENV, SWITCH, IVSMetaInfo($1), 3, $2, $4, op(ENV, NOOP, IVSMetaInfo($1), 0));  }
 	| SWITCH expression CLOSEPAREN statement_list case_statement_list ENDSWITCH
		{ $$ = op(ENV, SWITCH, IVSMetaInfo($1), 3, $2, $4, $5);  }
 	| SWITCH expression CLOSEPAREN ENDSWITCH
		{ $$ = op(ENV, SWITCH, IVSMetaInfo($1), 3, $2, op(ENV, NOOP, IVSMetaInfo($1), 0));  }
	| SWITCH error CLOSEPAREN
		{ yyerrok; moto_malformedSwitch(IVSMetaInfo($1),'\0'); $$ = op(ENV, NOOP, IVSMetaInfo($1), 0); }
;

class_definition_statement
	: CLASS NAME CLOSEPAREN statement_list ENDCLASS
		{ $$ = op(ENV, CLASSDEF, IVSMetaInfo($1),2,s(ENV,$2),$4); }
	| CLASS NAME CLOSEPAREN ENDCLASS
		{ $$ = op(ENV, CLASSDEF, IVSMetaInfo($1),2,s(ENV,$2),op(ENV, NOOP, IVSMetaInfo($1), 0)); }
	| CLASS error CLOSEPAREN
		{ yyerrok; moto_malformedClassDefinition(IVSMetaInfo($1),'\0'); $$ = op(ENV, NOOP, IVSMetaInfo($1), 0); }
;

embedded_class_definition_statement
    : ECLASS NAME OPENCURLY
		{ if(anyScope(ENV)) moto_illegalClassDefinition(IVSMetaInfo($1),'\1'); pushScope(ENV,ECLASSDEF); } 
	  embedded_class_statement_list 
        { popScope(ENV,ECLASSDEF); }
      CLOSECURLY
        { $$ = op(ENV, CLASSDEF, IVSMetaInfo($1),2,s(ENV,$2),$5); }
;

definition_statement 
	: { if(anyScope(ENV)  && !(classDefinitionScope(ENV) && stack_size(((MotoEnv*)ENV)->scope) == 1)) 
    	   moto_illegalDefine(&((MotoEnv*)ENV)->meta,'\0'); pushScope(ENV,DEFINE); } 
          function_definition_statement { popScope(ENV,DEFINE); $$=$2;}
    | { if(anyScope(ENV)) moto_illegalClassDefinition(&((MotoEnv*)ENV)->meta,'\0'); pushScope(ENV,CLASSDEF); } 
          class_definition_statement { popScope(ENV,CLASSDEF); $$=$2;}
;
	
function_definition_statement
	: DEFINE function_declaration OPENPAREN declaration_list CLOSEPAREN CLOSEPAREN statement_list ENDDEF 
		{ $$ = op(ENV, DEFINE,  IVSMetaInfo($1),3,$2,$4,$7); }
 	| DEFINE function_declaration OPENPAREN CLOSEPAREN CLOSEPAREN statement_list ENDDEF
		{ $$ = op(ENV, DEFINE,  IVSMetaInfo($1),3,$2,op(ENV, NOOP,  IVSMetaInfo($1),0),$6); }
	| DEFINE function_declaration OPENPAREN declaration_list CLOSEPAREN CLOSEPAREN ENDDEF
		{ $$ = op(ENV, DEFINE,  IVSMetaInfo($1),3,$2,$4,op(ENV, NOOP,IVSMetaInfo($1),0)); }
	| DEFINE function_declaration OPENPAREN CLOSEPAREN CLOSEPAREN CLOSEPAREN ENDDEF
		{ $$ = op(ENV, DEFINE,  IVSMetaInfo($1),3,$2,op(ENV,NOOP,IVSMetaInfo($1),0),op(ENV,NOOP,IVSMetaInfo($1),0));}
	| DEFINE NAME OPENPAREN declaration_list CLOSEPAREN CLOSEPAREN statement_list ENDDEF 
		{ $$ = op(ENV, DEFINE,  IVSMetaInfo($1),3,id(ENV,$2),$4,$7); }
 	| DEFINE NAME OPENPAREN CLOSEPAREN CLOSEPAREN statement_list ENDDEF
		{ $$ = op(ENV, DEFINE,  IVSMetaInfo($1),3,id(ENV,$2),op(ENV, NOOP,  IVSMetaInfo($1),0),$6); }
	| DEFINE NAME OPENPAREN declaration_list CLOSEPAREN CLOSEPAREN ENDDEF
		{ $$ = op(ENV, DEFINE,  IVSMetaInfo($1),3,id(ENV,$2),$4,op(ENV, NOOP,IVSMetaInfo($1),0)); }
	| DEFINE NAME OPENPAREN CLOSEPAREN CLOSEPAREN CLOSEPAREN ENDDEF
		{ $$ = op(ENV, DEFINE,  IVSMetaInfo($1),3,id(ENV,$2),op(ENV,NOOP,IVSMetaInfo($1),0),op(ENV,NOOP,IVSMetaInfo($1),0));}
	| DEFINE error CLOSEPAREN
		{ yyerrok; moto_malformedDefine(IVSMetaInfo($1),'\0'); $$ = op(ENV, NOOP, IVSMetaInfo($1), 0); }
;

declaration_list
	: declaration { $$ = op(ENV, LIST,  UCMetaInfo($1) ,1,$1); }
	| declaration_list COMMA declaration { $$ = listAdd(ENV,$1,$3); }
;

declaration
	: type NAME 
		{ $$ = op(ENV,DECLARATION, UCMetaInfo($1),3,$1,id(ENV,$2),op(ENV, LIST, UCMetaInfo($1), 0)); }
	| type NAME array_declaration_list
		{ $$ = op(ENV,DECLARATION, UCMetaInfo($1),3,$1,id(ENV,$2),$3); }
;

function_declaration
	: type NAME 
		{ $$ = op(ENV,DECLARATION, UCMetaInfo($1),3,op(ENV, NOOP, UCMetaInfo($1), 0), $1,id(ENV,$2)); }
	| type_qualifier type NAME
		{ $$ = op(ENV,DECLARATION, UCMetaInfo($1),3,$1,$2,id(ENV,$3)); }
;

array_declaration_list 
	: array_declaration { $$ = op(ENV, LIST,  UCMetaInfo($1) ,1,$1); }
	| array_declaration_list array_declaration { $$ = listAdd(ENV,$1,$2) ; }
;

array_declaration 
	: OPENCLOSEBRACKET { $$ = op(ENV,NOOP,IVSMetaInfo($1),0); }
;

return_statement
	: RETURN expression CLOSEPAREN
		{ if(!definitionScope(ENV)) moto_illegalReturn(IVSMetaInfo($1),'\0'); $$ = op(ENV,RETURN,IVSMetaInfo($1),1,$2); }
	| VRETURN 
		{ if(!definitionScope(ENV)) moto_illegalReturn(IVSMetaInfo($1),'\0'); $$ = op(ENV,RETURN,IVSMetaInfo($1),0); }
	| RETURN error CLOSEPAREN
		{ yyerrok; moto_malformedReturn(IVSMetaInfo($1),'\0'); $$ = op(ENV, NOOP, IVSMetaInfo($1), 0); }

;

embedded_return_statement
	: ERETURN expression 
		{ if(!definitionScope(ENV)) moto_illegalReturn(IVSMetaInfo($1),'\1'); $$ = op(ENV,RETURN,IVSMetaInfo($1),1,$2); }
	| ERETURN
		{ if(!definitionScope(ENV)) moto_illegalReturn(IVSMetaInfo($1),'\1'); $$ = op(ENV,RETURN,IVSMetaInfo($1),0); }
	| ERETURN error 
		{ yyerrok; moto_malformedReturn(IVSMetaInfo($1),'\0'); $$ = op(ENV, NOOP, IVSMetaInfo($1), 0); }
;

elseif_conditional_block
	: elseif_conditional { $$ = $1; }
	| elseif_conditional_block elseif_conditional { $$ = op(ENV, ELSEIFLIST, UCMetaInfo($1) , 2, $1, $2); } 
;

elseif_conditional
	: ELSEIF expression CLOSEPAREN statement_list 
		{ $$ = op(ENV, ELSEIF, IVSMetaInfo($1), 2, $2, $4); }
	| ELSEIF expression CLOSEPAREN 
		{ $$ = op(ENV, ELSEIF, IVSMetaInfo($1), 2, $2, op(ENV, NOOP, IVSMetaInfo($1), 0)); }
	| ELSEIF error  CLOSEPAREN
		{yyerrok;  moto_malformedElseIf(IVSMetaInfo($1),'\0'); $$ = op(ENV, NOOP, IVSMetaInfo($1), 0); }
;

embedded_conditional_statement
	: EIF OPENPAREN expression CLOSEPAREN embedded_statement %prec LOWER_THAN_EELSE
		{ $$ = op(ENV, IF, IVSMetaInfo($1), 4, $3, $5, op(ENV, NOOP, IVSMetaInfo($1), 0), op(ENV, NOOP, IVSMetaInfo($1), 0)); }
	| EIF OPENPAREN expression CLOSEPAREN embedded_statement EELSE embedded_statement 
		{ $$ = op(ENV, IF, IVSMetaInfo($1), 4, $3, $5, op(ENV, NOOP, IVSMetaInfo($1), 0), $7); }
	| EIF OPENPAREN error CLOSEPAREN embedded_statement
		{ yyerrok; moto_malformedIf(IVSMetaInfo($1),'\1'); $$ = op(ENV, NOOP, IVSMetaInfo($1), 0); }
	| ESWITCH OPENPAREN expression CLOSEPAREN OPENCURLY embedded_case_statement_list EDEFAULT COLON embedded_statement_list CLOSECURLY
		{ $$ = op(ENV, SWITCH, IVSMetaInfo($1), 3, $3, $9, $6); }
	| ESWITCH OPENPAREN expression CLOSEPAREN OPENCURLY EDEFAULT COLON embedded_statement_list CLOSECURLY
		{ $$ = op(ENV, SWITCH, IVSMetaInfo($1), 3, $3, $8, op(ENV, NOOP, IVSMetaInfo($1), 0)); }
	| ESWITCH OPENPAREN expression CLOSEPAREN OPENCURLY embedded_case_statement_list CLOSECURLY 
		{ $$ = op(ENV, SWITCH, IVSMetaInfo($1), 3, $3, op(ENV, NOOP, IVSMetaInfo($1), 0), $6); }
	| ESWITCH OPENPAREN expression CLOSEPAREN OPENCURLY CLOSECURLY
		{ $$ = op(ENV, SWITCH, IVSMetaInfo($1), 3, $3, op(ENV, NOOP, IVSMetaInfo($1), 0), op(ENV, NOOP, IVSMetaInfo($1), 0)); }
	| ESWITCH OPENPAREN error CLOSEPAREN
		{ yyerrok; moto_malformedSwitch(IVSMetaInfo($1),'\1'); $$ = op(ENV, NOOP, IVSMetaInfo($1), 0); }
;

embedded_definition_statement
	: embedded_function_definition_statement
;

embedded_function_definition_statement
	: function_declaration OPENPAREN declaration_list CLOSEPAREN
	  { if(anyScope(ENV) && !(classDefinitionScope(ENV) && stack_size(((MotoEnv*)ENV)->scope) == 1)) moto_illegalDefine(UCMetaInfo($1),'\1'); pushScope(ENV,EDEFINE);} 
	    embedded_block 
	  { popScope(ENV,EDEFINE);  $$ = op(ENV, DEFINE, UCMetaInfo($1),3,$1,$3,$6); }
	| function_declaration OPENPAREN CLOSEPAREN
	  { if(anyScope(ENV) && !(classDefinitionScope(ENV) && stack_size(((MotoEnv*)ENV)->scope) == 1)) moto_illegalDefine(UCMetaInfo($1),'\1'); pushScope(ENV,EDEFINE); } 
	    embedded_block 
	  { popScope(ENV,EDEFINE); $$ = op(ENV, DEFINE, UCMetaInfo($1),3,$1,op(ENV, NOOP, UCMetaInfo($1),0),$5); }
;

embedded_constructor_definition_statement
	: NAME OPENPAREN declaration_list CLOSEPAREN 
	  { if(anyScope(ENV) && !(classDefinitionScope(ENV) && stack_size(((MotoEnv*)ENV)->scope) == 1)) moto_illegalDefine(SVSMetaInfo($1),'\1'); pushScope(ENV,EDEFINE);} 
	   embedded_block
	  { popScope(ENV,EDEFINE);  $$ = op(ENV, DEFINE, SVSMetaInfo($1),3,id(ENV,$1),$3,$6); }
	| NAME OPENPAREN CLOSEPAREN 
	  { if(anyScope(ENV) && !(classDefinitionScope(ENV) && stack_size(((MotoEnv*)ENV)->scope) == 1)) moto_illegalDefine(SVSMetaInfo($1),'\1'); pushScope(ENV,EDEFINE); } 
	   embedded_block
	  { popScope(ENV,EDEFINE); $$ = op(ENV, DEFINE, SVSMetaInfo($1),3,id(ENV,$1),op(ENV, NOOP, SVSMetaInfo($1),0),$5); }
;
	
case_statement_list
	: case_statement { $$ = op(ENV, LIST,  UCMetaInfo($1) ,1, $1); }
	| case_statement_list case_statement { $$ = listAdd(ENV,$1,$2) ; }
;

case_statement
	: CASE expression CLOSEPAREN statement_list
		{ $$ = op(ENV, CASE, IVSMetaInfo($1), 2, $2, $4); }
	| CASE expression CLOSEPAREN
		{ $$ = op(ENV, CASE, IVSMetaInfo($1), 2, $2, op(ENV, NOOP, IVSMetaInfo($1), 0)); }
	| CASE error  CLOSEPAREN
		{ yyerrok; moto_malformedCase(IVSMetaInfo($1),'\0'); $$ = op(ENV, NOOP, IVSMetaInfo($1), 0); }
;

embedded_case_statement_list
	: embedded_case_statement { $$ = op(ENV, LIST,  UCMetaInfo($1) ,1, $1); }
	| embedded_case_statement_list embedded_case_statement { $$ = listAdd(ENV,$1,$2) ; }
;

embedded_case_statement
	: ECASE expression COLON embedded_statement_list
		{ $$ = op(ENV, CASE, IVSMetaInfo($1), 2, $2, $4); }
	| ECASE expression COLON
		{ $$ = op(ENV, CASE, IVSMetaInfo($1), 2, $2, op(ENV, NOOP, IVSMetaInfo($1), 0)); }
	| ECASE error COLON 
		{ yyerrok; moto_malformedCase(IVSMetaInfo($1),'\1'); $$ = op(ENV, NOOP, IVSMetaInfo($1), 0); }
;

iterative_statement
	: { pushScope(ENV,WHILE); } while_statement { popScope(ENV,WHILE); $$=$2;}
        | { pushScope(ENV,FOR); } for_statement { popScope(ENV,FOR); $$=$2;}
;

while_statement
	: WHILE expression CLOSEPAREN statement_list ENDWHILE 
		{ $$ = op(ENV, WHILE, IVSMetaInfo($1), 2, $2, $4);  }
	| WHILE expression CLOSEPAREN ENDWHILE 
		{ $$ = op(ENV, WHILE, IVSMetaInfo($1), 2, $2, op(ENV, NOOP, IVSMetaInfo($1), 0));  }
	| WHILE error  CLOSEPAREN
        { yyerrok; moto_malformedWhile(IVSMetaInfo($1),'\0'); $$ = op(ENV, NOOP, IVSMetaInfo($1), 0); }
;

for_statement
	: FOR SC expression SC CLOSEPAREN statement_list ENDFOR
		{ $$ = op(ENV, FOR, IVSMetaInfo($1),4, op(ENV, NOOP, IVSMetaInfo($1), 0), $3, op(ENV, NOOP, IVSMetaInfo($1), 0), $6);  }
	| FOR assignment_expression SC expression SC CLOSEPAREN statement_list ENDFOR
		{ $$ = op(ENV, FOR, IVSMetaInfo($1),4, $2, $4, op(ENV, NOOP, IVSMetaInfo($1), 0), $7);  }
	| FOR inline_declaration SC expression SC CLOSEPAREN statement_list ENDFOR
		{ $$ = op(ENV, FOR, IVSMetaInfo($1),4, $2, $4, op(ENV, NOOP, IVSMetaInfo($1), 0), $7);  }
	| FOR SC expression SC assignment_expression CLOSEPAREN statement_list ENDFOR
		{ $$ = op(ENV, FOR, IVSMetaInfo($1),4, op(ENV, NOOP, IVSMetaInfo($1), 0), $3, $5, $7);  }
	| FOR assignment_expression SC expression SC assignment_expression CLOSEPAREN statement_list ENDFOR
		{ $$ = op(ENV, FOR, IVSMetaInfo($1),4, $2, $4, $6, $8);  }
	| FOR inline_declaration SC expression SC assignment_expression CLOSEPAREN statement_list ENDFOR
		{ $$ = op(ENV, FOR, IVSMetaInfo($1),4, $2, $4, $6, $8);  }
	| FOR SC expression SC CLOSEPAREN ENDFOR
		{ $$ = op(ENV, FOR, IVSMetaInfo($1),4, op(ENV, NOOP, IVSMetaInfo($1), 0), $3, op(ENV, NOOP, IVSMetaInfo($1), 0), op(ENV, NOOP, IVSMetaInfo($1), 0));  }
	| FOR assignment_expression SC expression SC CLOSEPAREN ENDFOR
		{ $$ = op(ENV, FOR, IVSMetaInfo($1),4, $2, $4, op(ENV, NOOP, IVSMetaInfo($1), 0), op(ENV, NOOP, IVSMetaInfo($1), 0));  }
	| FOR inline_declaration SC expression SC CLOSEPAREN ENDFOR
		{ $$ = op(ENV, FOR, IVSMetaInfo($1),4, $2, $4, op(ENV, NOOP, IVSMetaInfo($1), 0), op(ENV, NOOP, IVSMetaInfo($1), 0));  }
	| FOR SC expression SC assignment_expression CLOSEPAREN ENDFOR
		{ $$ = op(ENV, FOR, IVSMetaInfo($1),4, op(ENV, NOOP, IVSMetaInfo($1), 0), $3, $5, op(ENV, NOOP, IVSMetaInfo($1), 0));  }
	| FOR assignment_expression SC expression SC assignment_expression CLOSEPAREN ENDFOR
		{ $$ = op(ENV, FOR, IVSMetaInfo($1),4, $2, $4, $6, op(ENV, NOOP, IVSMetaInfo($1), 0));  }
	| FOR inline_declaration SC expression SC assignment_expression CLOSEPAREN ENDFOR
		{ $$ = op(ENV, FOR, IVSMetaInfo($1),4, $2, $4, $6, op(ENV, NOOP, IVSMetaInfo($1), 0));  }
	| FOR error CLOSEPAREN
        { yyerrok; moto_malformedFor(IVSMetaInfo($1),'\0'); $$ = op(ENV, NOOP, IVSMetaInfo($1), 0); }

;

embedded_iterative_statement
	: EWHILE OPENPAREN expression CLOSEPAREN { pushScope(ENV,EWHILE); } embedded_statement { popScope(ENV,EWHILE) ; }
		{ $$ = op(ENV, WHILE, IVSMetaInfo($1), 2, $3, $6); }
	| EWHILE OPENPAREN error CLOSEPAREN 
		{ yyerrok; moto_malformedWhile(IVSMetaInfo($1),'\1'); $$ = op(ENV, NOOP, IVSMetaInfo($1), 0); }
	| EFOR OPENPAREN SC expression SC CLOSEPAREN { pushScope(ENV,EFOR); } embedded_statement { popScope(ENV,EFOR); }
		{ $$ = op(ENV, FOR, IVSMetaInfo($1),4, op(ENV, NOOP, IVSMetaInfo($1), 0), $4, op(ENV, NOOP, IVSMetaInfo($1), 0), $8); }
	| EFOR OPENPAREN assignment_expression SC expression SC CLOSEPAREN 
		{ pushScope(ENV,EFOR); } embedded_statement { popScope(ENV,EFOR); }
		{ $$ = op(ENV, FOR, IVSMetaInfo($1),4, $3, $5, op(ENV, NOOP, IVSMetaInfo($1), 0), $9); }
	| EFOR OPENPAREN inline_declaration SC expression SC CLOSEPAREN 
		{ pushScope(ENV,EFOR); } embedded_statement { popScope(ENV,EFOR); }
		{ $$ = op(ENV, FOR, IVSMetaInfo($1),4, $3, $5, op(ENV, NOOP, IVSMetaInfo($1), 0), $9); }
	| EFOR OPENPAREN SC expression SC assignment_expression CLOSEPAREN 
		{ pushScope(ENV,EFOR); } embedded_statement { popScope(ENV,EFOR); }
		{ $$ = op(ENV, FOR, IVSMetaInfo($1),4, op(ENV, NOOP, IVSMetaInfo($1), 0), $4, $6, $9); }
	| EFOR OPENPAREN assignment_expression SC expression SC assignment_expression CLOSEPAREN 
		{ pushScope(ENV,EFOR); } embedded_statement { popScope(ENV,EFOR); }
		{ $$ = op(ENV, FOR, IVSMetaInfo($1),4, $3, $5, $7, $10); }
	| EFOR OPENPAREN inline_declaration SC expression SC assignment_expression CLOSEPAREN 
		{ pushScope(ENV,EFOR); } embedded_statement { popScope(ENV,EFOR); }
		{ $$ = op(ENV, FOR, IVSMetaInfo($1),4, $3, $5, $7, $10); }
	| EFOR OPENPAREN error CLOSEPAREN
		{ yyerrok; moto_malformedFor(IVSMetaInfo($1),'\1'); $$ = op(ENV, NOOP, IVSMetaInfo($1), 0); }

;

declare_statement
	: DECLARE type variable_declarator_list CLOSEPAREN
		{ $$ = op(ENV, DECLARE, IVSMetaInfo($1), 3, op(ENV, LIST,  IVSMetaInfo($1) ,0), $2, $3); }
	| DECLARE type_qualifier type variable_declarator_list CLOSEPAREN
		{ $$ = op(ENV, DECLARE, IVSMetaInfo($1), 3, $2, $3, $4); }
	| DECLARE error  CLOSEPAREN
		{ yyerrok; moto_malformedDeclare(IVSMetaInfo($1),'\0'); 
		  $$ = op(ENV, NOOP, IVSMetaInfo($1), 0); }
;

embedded_declare_statement
	: type variable_declarator_list  
		{ $$ = op(ENV, DECLARE, UCMetaInfo($1), 3,op(ENV, LIST,  UCMetaInfo($1) ,0), $1,$2); }
	| type_qualifier type variable_declarator_list
    	{ $$ = op(ENV, DECLARE, UCMetaInfo($1), 3,$1,$2,$3); }
;

inline_declaration
	: type variable_declarator_list
		{ $$ = op(ENV, DECLARE_INLINE, UCMetaInfo($1), 3,op(ENV, LIST,  UCMetaInfo($1) ,0), $1,$2); }
	| type_qualifier type variable_declarator_list
   	{ $$ = op(ENV, DECLARE_INLINE, UCMetaInfo($1), 3,$1,$2,$3); }
;

variable_declarator_list
	: variable_declarator { $$ = op(ENV, LIST,  UCMetaInfo($1) ,1,$1); }
	| variable_declarator_list COMMA variable_declarator { $$ = listAdd(ENV,$1,$3); }
;

variable_declarator
	: NAME array_declaration_list
		{ $$ = op(ENV, DECLARATOR, SVSMetaInfo($1), 3, id(ENV,$1), $2, op(ENV, NOOP, SVSMetaInfo($1) ,0) ); }
	| NAME array_declaration_list ASSIGN assignment_expression
		{ $$ = op(ENV, DECLARATOR, SVSMetaInfo($1), 3, id(ENV,$1), $2, $4); }
	| NAME array_declaration_list ASSIGN array_initializer
		{ $$ = op(ENV, DECLARATOR, SVSMetaInfo($1), 3, id(ENV,$1), $2,$4); }	
	| NAME 
		{ $$ = op(ENV, DECLARATOR, SVSMetaInfo($1), 3, id(ENV,$1), op(ENV, LIST, SVSMetaInfo($1) ,0), op(ENV, NOOP, SVSMetaInfo($1) ,0) ); }
	| NAME ASSIGN assignment_expression
		{ $$ = op(ENV, DECLARATOR, SVSMetaInfo($1), 3, id(ENV,$1), op(ENV, LIST, SVSMetaInfo($1) ,0), $3 ); }
	| NAME ASSIGN array_initializer
		{ $$ = op(ENV, DECLARATOR, SVSMetaInfo($1), 3, id(ENV,$1), op(ENV, LIST, SVSMetaInfo($1) ,0), $3); }
;

type
	: NAME 
		{ $$ = op(ENV, TYPE, SVSMetaInfo($1), 2, s(ENV,$1), op(ENV, LIST,  SVSMetaInfo($1) ,0)); }
	| type array_declaration
		{ $$ = op(ENV, ARRAY_TYPE, UCMetaInfo($1), 2, $1, op(ENV, LIST,  UCMetaInfo($2) ,1,$2)); }
;

unified_type
	: type { $$ = $1; }
	| functional_type { $$ = $1; }
;

functional_type		
	: unified_type OPENPAREN CLOSEPAREN
		{ $$ = op(ENV, FUNC_TYPE, UCMetaInfo($1), 2,$1,op(ENV, LIST, UCMetaInfo($1) ,0)); }
	| unified_type OPENPAREN type_list CLOSEPAREN
		{ $$ = op(ENV, FUNC_TYPE, UCMetaInfo($1), 2,$1,$3); }
	| functional_type array_declaration
		{ $$ = op(ENV, ARRAY_TYPE, UCMetaInfo($1), 2, $1, op(ENV, LIST,  UCMetaInfo($2) ,1,$2)); }	
;

type_list
	: unified_type { $$ = op(ENV, LIST,  UCMetaInfo($1) ,1,$1); }
	| type_list COMMA unified_type { $$ = listAdd(ENV,$1,$3); }
;

type_qualifier
	: GLOBAL 
		{ if(anyScope(ENV)) moto_illegalGlobal(IVSMetaInfo($1)); 
		  $$ = op(ENV,GLOBAL,IVSMetaInfo($1),0); }
;

do_statement
	: DO expression CLOSEPAREN 
		{ $$ = op(ENV, DO, IVSMetaInfo($1), 1, $2); }
	| DO error  CLOSEPAREN
		{ yyerrok; moto_malformedDo(IVSMetaInfo($1),'\0'); $$ = op(ENV, NOOP, IVSMetaInfo($1), 0); }
;

embedded_do_statement
	: expression  { $$ = op(ENV, DO, UCMetaInfo($1) , 1, $1); } 
;

print_statement
	: PRINT expression CLOSEPAREN 
		{ $$ = op(ENV, PRINT, IVSMetaInfo($1), 1, $2); }
	| PRINT error  CLOSEPAREN
		{ yyerrok; moto_malformedPrint(IVSMetaInfo($1),'\0'); $$ = op(ENV, NOOP, IVSMetaInfo($1), 0); }
;

embedded_print_statement
	: EPRINT expression  
		{ $$ = op(ENV, PRINT, IVSMetaInfo($1), 1, $2); } 
	| EPRINT error  
		{ yyerrok; moto_malformedPrint(IVSMetaInfo($1),'\1'); $$ = op(ENV, NOOP, IVSMetaInfo($1), 0); }
;

use_statement
	: USE STRING CLOSEPAREN
		{ if(anyScope(ENV)) moto_illegalUse(IVSMetaInfo($1),'\0'); $$ = op(ENV, USE, IVSMetaInfo($1), 1, ls(ENV, $2));  }
	| USE error  CLOSEPAREN
		{ yyerrok; moto_malformedUse(IVSMetaInfo($1),'\0'); $$ = op(ENV, NOOP, IVSMetaInfo($1), 0); }
;

embedded_use_statement
	: EUSE STRING 
		{ if(anyScope(ENV)) moto_illegalUse(IVSMetaInfo($1),'\1'); $$ = op(ENV, USE, IVSMetaInfo($1), 1, ls(ENV, $2)); }
	| EUSE error 
		{ yyerrok; moto_malformedUse(IVSMetaInfo($1),'\1'); $$ = op(ENV, NOOP, IVSMetaInfo($1), 0); }
;

jump_statement
	: BREAK
		{ if(!loopScope(ENV)) moto_illegalBreak(IVSMetaInfo($1),'\0'); $$ = op(ENV, BREAK, IVSMetaInfo($1), 0);  }
	| CONTINUE
		{ if(!loopScope(ENV)) moto_illegalContinue(IVSMetaInfo($1),'\0'); $$ = op(ENV, CONTINUE, IVSMetaInfo($1), 0);  }
;

embedded_jump_statement
	: EBREAK 
		{ if(!loopScope(ENV)) moto_illegalBreak(IVSMetaInfo($1),'\1'); $$ = op(ENV, BREAK, IVSMetaInfo($1), 0); }
	| ECONTINUE 
		{ if(!loopScope(ENV)) moto_illegalContinue(IVSMetaInfo($1),'\1'); $$ = op(ENV, CONTINUE, IVSMetaInfo($1), 0); }
;

statement_list
	: statement { $$ = op(ENV, LIST,  UCMetaInfo($1) ,1, $1); }
	| statement_list statement { $$ = listAdd(ENV,$1,$2) ; }
;

try_block
	: EXCP_TRY { pushScope(ENV,EXCP_TRY); } statement_list try_handlers  
		{ popScope(ENV,EXCP_TRY); $$ = op(ENV, EXCP_TRY, IVSMetaInfo($1), 2, $3, $4); }
	| EXCP_TRY { pushScope(ENV,EXCP_TRY); } try_handlers  
		{ popScope(ENV,EXCP_TRY); $$ = op(ENV, EXCP_TRY, IVSMetaInfo($1), 2, op(ENV, NOOP, IVSMetaInfo($1), 0), $3); }
;
	
try_handlers
	: catch_block_list finally_block EXCP_ENDTRY
		{ $$ = op(ENV, EXCP_TRY_HANDLERS, UCMetaInfo($1), 2, $1, $2 ); }
	| catch_block_list EXCP_ENDTRY
		{ $$ = op(ENV, EXCP_TRY_HANDLERS, UCMetaInfo($1), 2, $1, op(ENV, NOOP, UCMetaInfo($1), 0) ); } 
	| finally_block EXCP_ENDTRY
		{ $$ = op(ENV, EXCP_TRY_HANDLERS, UCMetaInfo($1), 2, op(ENV, LIST,  UCMetaInfo($1) ,0) , $1 ); }
	| EXCP_ENDTRY 
		{ moto_illegalTry(IVSMetaInfo($1) ,'\1'); $$ = op(ENV, NOOP, IVSMetaInfo($1), 0) ; }
;
	
embedded_try_block
	: EXCP_ETRY { pushScope(ENV,EXCP_ETRY); } embedded_block embedded_try_handlers 
		{ popScope(ENV,EXCP_ETRY); $$ = op(ENV, EXCP_TRY, IVSMetaInfo($1), 2, $3, $4); }
;

embedded_try_handlers
	: embedded_catch_block_list 
		{ $$ = op(ENV, EXCP_TRY_HANDLERS, UCMetaInfo($1), 2, $1, op(ENV, NOOP, UCMetaInfo($1), 0) ); }
	| embedded_catch_block_list embedded_finally_block 
		{ $$ = op(ENV, EXCP_TRY_HANDLERS, UCMetaInfo($1), 2, $1, $2 ); }
	| embedded_finally_block 
		{ $$ = op(ENV, EXCP_TRY_HANDLERS, UCMetaInfo($1), 2, op(ENV, LIST,  UCMetaInfo($1) ,0) , $1 ); }
;
	
catch_block_list
	: catch_block { $$ = op(ENV, LIST,  UCMetaInfo($1) ,1, $1); }
	| catch_block_list catch_block { $$ = listAdd(ENV,$1,$2) ; }
;

embedded_catch_block_list
	: embedded_catch_block { $$ = op(ENV, LIST,  UCMetaInfo($1) ,1, $1); }
	| embedded_catch_block_list embedded_catch_block { $$ = listAdd(ENV,$1,$2) ; }
;

catch_block
	: EXCP_CATCH NAME NAME CLOSEPAREN statement_list
		{ $$ = op(ENV, EXCP_CATCH, IVSMetaInfo($1), 3, s(ENV,$2), id(ENV,$3), $5); }
	| EXCP_CATCH NAME NAME CLOSEPAREN
		{ $$ = op(ENV, EXCP_CATCH, IVSMetaInfo($1), 3, s(ENV,$2), id(ENV,$3), $$ = op(ENV, NOOP, IVSMetaInfo($1), 0)); }
	| EXCP_CATCH error CLOSEPAREN statement_list
		{ yyerrok; moto_malformedCatch(IVSMetaInfo($1),'\0'); 
		  $$ = op(ENV, NOOP, IVSMetaInfo($1), 0); }
;


embedded_catch_block
	: EXCP_ECATCH OPENPAREN NAME NAME CLOSEPAREN embedded_block
		{ if(!tryScope(ENV)) moto_illegalCatch(IVSMetaInfo($1),'\1'); 
		  $$ = op(ENV, EXCP_CATCH, IVSMetaInfo($1), 3, s(ENV,$3), id(ENV,$4), $6); }
	| EXCP_ECATCH OPENPAREN error CLOSEPAREN embedded_block
		{ yyerrok; if(!tryScope(ENV)) moto_illegalCatch(IVSMetaInfo($1),'\1'); 
		  moto_malformedCatch(IVSMetaInfo($1),'\1'); $$ = op(ENV, NOOP, IVSMetaInfo($1), 0); }
	| EXCP_ECATCH OPENPAREN NAME NAME CLOSEPAREN error
		{ yyerrok; if(!tryScope(ENV)) moto_illegalCatch(IVSMetaInfo($1),'\1'); 
		  moto_malformedCatch(IVSMetaInfo($1),'\1'); $$ = op(ENV, NOOP, IVSMetaInfo($1), 0); }
;

finally_block
	: EXCP_FINALLY statement_list
		{ if(!tryScope(ENV)) moto_illegalFinally(IVSMetaInfo($1),'\1'); $$ = $2; }
	| EXCP_FINALLY
		{ if(!tryScope(ENV)) moto_illegalFinally(IVSMetaInfo($1),'\1'); 
		  $$ = op(ENV, NOOP, IVSMetaInfo($1), 0); }
;

embedded_finally_block
	: EXCP_EFINALLY embedded_block
		{ if(!tryScope(ENV)) moto_illegalFinally(IVSMetaInfo($1),'\1'); $$ = $2; }
	| EXCP_EFINALLY error
		{ yyerrok; if(!tryScope(ENV)) moto_illegalFinally(IVSMetaInfo($1),'\1');
		  $$ = op(ENV, NOOP, IVSMetaInfo($1), 0); }
;

throw_statement
	: EXCP_THROW expression CLOSEPAREN
		{ $$ = op(ENV, EXCP_THROW, IVSMetaInfo($1), 1, $2); }
	| EXCP_THROW error CLOSEPAREN
		{ yyerrok; moto_malformedThrow(IVSMetaInfo($1),'\0'); $$ = op(ENV, NOOP, IVSMetaInfo($1), 0); }
;

embedded_throw_statement
	: EXCP_ETHROW expression
		{ $$ = op(ENV, EXCP_THROW, IVSMetaInfo($1), 1, $2); }
	| EXCP_ETHROW error 
		{ yyerrok; moto_malformedThrow(IVSMetaInfo($1),'\1'); $$ = op(ENV, NOOP, IVSMetaInfo($1), 0); }
;

embedded_class_statement_list
	: embedded_class_statement { $$ = op(ENV, LIST,  UCMetaInfo($1) ,1, $1); }
	| embedded_class_statement_list embedded_class_statement { $$ = listAdd(ENV,$1,$2) ; }
;

embedded_class_statement
	: embedded_constructor_definition_statement
	| embedded_definition_statement { $$ = $1; }
	| embedded_declare_statement SC { $$ = $1; }
;

embedded_statement_list
	: embedded_statement { $$ = op(ENV, LIST,  UCMetaInfo($1) ,1, $1); }
	| embedded_statement_list embedded_statement { $$ = listAdd(ENV,$1,$2) ; }
;

embedded_statement
	: embedded_conditional_statement { $$ = $1; }
	| embedded_iterative_statement { $$ = $1; }
	| embedded_definition_statement { $$ = $1; } 
	| embedded_class_definition_statement { $$ = $1; }
	| embedded_try_block { $$ = $1; }
	| embedded_declare_statement SC { $$ = $1; }
	| embedded_do_statement SC { $$ = $1; }
	| embedded_jump_statement SC { $$ = $1; }
	| embedded_use_statement SC { $$ = $1; }
	| embedded_print_statement SC { $$ = $1; }
	| embedded_return_statement SC { $$ = $1; }
	| embedded_throw_statement SC { $$ = $1; }
	| { pushScope(ENV,SCOPE); } embedded_block { popScope(ENV,SCOPE); $$ = op(ENV, SCOPE,UCMetaInfo($2),1,$2); }
	| error { do_recover(&yylval,'\1'); yyclearin; if(*motoyytext != '\0') yyerrok; $$ = op(ENV, NOOP,  &moto_getEnv()->meta, 0); }
	| SC { $$ = op(ENV, NOOP, IVSMetaInfo($1), 0); }
;

embedded_block
	: OPENCURLY embedded_statement_list CLOSEDOLLAR statement_list DOLLAROPEN embedded_statement_list CLOSECURLY
		{ $$ = op(ENV, LIST, IVSMetaInfo($1), 3, $2, $4, $6); }
	| OPENCURLY CLOSEDOLLAR statement_list DOLLAROPEN embedded_statement_list CLOSECURLY
		{ $$ = op(ENV, LIST, IVSMetaInfo($1), 2, $3, $5); }
	| OPENCURLY embedded_statement_list CLOSEDOLLAR statement_list DOLLAROPEN CLOSECURLY
		{ $$ = op(ENV, LIST, IVSMetaInfo($1), 2, $2, $4); }
	| OPENCURLY embedded_statement_list CLOSEDOLLAR DOLLAROPEN embedded_statement_list CLOSECURLY
		{ $$ = op(ENV, LIST, IVSMetaInfo($1), 2, $2, $5); }
    | OPENCURLY CLOSEDOLLAR DOLLAROPEN embedded_statement_list CLOSECURLY
		{ $$ = $4; }
	| OPENCURLY CLOSEDOLLAR statement_list DOLLAROPEN CLOSECURLY
		{ $$ = $3; }
	| OPENCURLY embedded_statement_list CLOSEDOLLAR DOLLAROPEN CLOSECURLY
		{ $$ = $2; }
	| OPENCURLY CLOSEDOLLAR DOLLAROPEN CLOSECURLY
		{ $$ = op(ENV, NOOP, IVSMetaInfo($1), 0); }
	| OPENCURLY CLOSECURLY
		{ $$ = op(ENV, NOOP, IVSMetaInfo($1), 0); }
	| OPENCURLY embedded_statement_list CLOSECURLY
		{ $$ = $2; }
;

statement
	: conditional_statement { $$ = $1; }
	| iterative_statement { $$ = $1; }
	| declare_statement { $$ = $1; }
	| definition_statement { $$ = $1; }
	| try_block { $$ = $1; }
	| do_statement { $$ = $1; }
	| jump_statement { $$ = $1; }
	| use_statement { $$ = $1; }
	| print_statement { $$ = $1; }
	| throw_statement { $$ = $1; }
	| return_statement { $$ = $1; }
	| DOLLAROPEN embedded_statement_list CLOSEDOLLAR { $$ = $2 ;}
   | DOLLAROPEN CLOSEDOLLAR {$$ = op(ENV, NOOP, IVSMetaInfo($1), 0);}
	| ANYCHARS { $$ = op(ENV, PRINT, SVSMetaInfo($1), 1, as(ENV, $1)); }
	| WS { $$ = op(ENV, PRINT, SVSMetaInfo($1), 1, as(ENV, $1)); }	
	| error {do_recover(&yylval, '\0'); yyclearin; if(*motoyytext != '\0') yyerrok; $$ = op(ENV, NOOP, &moto_getEnv()->meta, 0);}
;	

%%

static inline MetaInfo* 
IVSMetaInfo(IVS* ivs) {
        return &ivs->meta;
}

static inline MetaInfo* 
SVSMetaInfo(SVS* svs) {
        return &svs->meta;
}

static inline MetaInfo*
UCMetaInfo(UnionCell* uc) {
        if(uc->type == VALUE_TYPE)
           return &uc->valuecell.meta;
        else
           return &uc->opcell.meta;
}

static union unionCell *
nullcell(MotoEnv *env, SVS* svs) {
	UnionCell *p;
        char* value = svs->sval;
	p = (UnionCell *)mman_malloc(env->tree->mpool, sizeof(ValueCell));
	p->type = VALUE_TYPE;					   
	p->valuecell.value = value;
	p->valuecell.kind = NULL_VALUE;					   
	p->valuecell.code = value;

	p->valuecell.meta.filename = svs->meta.filename;
	p->valuecell.meta.macroname = svs->meta.macroname;
        p->valuecell.meta.lineno = svs->meta.lineno;
	p->valuecell.meta.caller = svs->meta.caller;

	return p;
}

static union unionCell *b(MotoEnv *env, SVS* svs) {	
	UnionCell *p;
	char *n;
        char* value = svs->sval;
	p = (UnionCell *)mman_malloc(env->tree->mpool, sizeof(ValueCell));
	p->type = VALUE_TYPE;					   
	n = mman_malloc(env->tree->mpool, sizeof(char));
	if (strcmp(value, "true") == 0) {
		*n = 1;
	}
	else {
		*n = 0;
	}
	p->valuecell.value = n;					   
	p->valuecell.kind = BOOLEAN_VALUE;					   
	p->valuecell.code = value;

        p->valuecell.meta.filename = svs->meta.filename;
        p->valuecell.meta.macroname = svs->meta.macroname;
        p->valuecell.meta.lineno = svs->meta.lineno;
        p->valuecell.meta.caller = svs->meta.caller;

	return p;
}

static union unionCell *c(MotoEnv *env, SVS* svs) {	
	UnionCell *p;
	char *n;
        char* value = svs->sval;
	p = (UnionCell *)mman_malloc(env->tree->mpool, sizeof(ValueCell));
	p->type = VALUE_TYPE;					   
	n = mman_malloc(env->tree->mpool, sizeof(char));
	if (value[1] == '\\') {
		switch (value[2]) {
			case 'x':
				if (strlen(value) == 6) {
					*n = (hexCharToInt(value[3]) << 4) + hexCharToInt(value[4]);
				}
				else {
					*n = hexCharToInt(value[3]);
				}
				break;	
			case '\\':
				*n = '\\';
				break;	
			case '\'':
				*n = '\'';
				break;	
			case 'a':
				*n = '\a';
				break;	
			case 'b':
				*n = '\b';
				break;	
			case 'f':
				*n = '\f';
				break;	
			case 'r':
				*n = '\r';
				break;	
			case 'n':
				*n = '\n';
				break;	
			case 't':
				*n = '\t';
				break;	
			default:
				if (strlen(value) == 6) {
					*n = ((value[2] - '0') << 6) + ((value[3] - '0') << 3) + (value[4] - '0');
				}
				else if (strlen(value) == 5) {
					*n = ((value[2] - '0') << 3) + (value[3] - '0');
				}
				else {
					*n = (value[2] - '0');
				}
				break;	
		}
	}
	else {
		*n = value[1];
	}
	p->valuecell.value = n;					   
	p->valuecell.kind = CHAR_VALUE;					   
	p->valuecell.code = value;

	p->valuecell.meta.filename = svs->meta.filename;
	p->valuecell.meta.macroname = svs->meta.macroname;
	p->valuecell.meta.lineno = svs->meta.lineno;
	p->valuecell.meta.caller = svs->meta.caller;

	return p;
}

extern int depth;
int do_recover(YYSTYPE* lvalp, char embedded ){
	if (!strncmp("$elseif",motoyytext,7))
		moto_elseIfWithoutIf(&lvalp->ivs->meta);
	else if (!strncmp("$else",motoyytext,5))
		moto_elseWithoutIf(&lvalp->ivs->meta,'\0');
	else if (!strncmp("$endif",motoyytext,6))
		moto_endIfWithoutIf(&lvalp->ivs->meta);
	else if (!strncmp("$endwhile",motoyytext,9))
		moto_endWhileWithoutWhile(&lvalp->ivs->meta);
	else if (!strncmp("$endfor",motoyytext,7))
		moto_endForWithoutFor(&lvalp->ivs->meta);
	else if (!strncmp("$endswitch",motoyytext,10))
		moto_endSwitchWithoutSwitch(&lvalp->ivs->meta);
	else if (!strncmp("$enddef",motoyytext,7))
		moto_endDefWithoutDefine(&lvalp->ivs->meta);
	else if (!strncmp("$case",motoyytext,5))
		moto_illegalCase(&moto_getEnv()->meta,'\0');
	else if (!strncmp("$endclass",motoyytext,9))
		moto_endClassWithoutClass(&lvalp->ivs->meta);
	else if (!strncmp("$catch(",motoyytext,7))
		moto_illegalCatch(&lvalp->ivs->meta,'\0');
	else if (!strncmp("$finally",motoyytext,8))
		moto_illegalFinally(&lvalp->ivs->meta,'\0');
	else if (!strncmp("$endtry",motoyytext,7))
		moto_endTryWithoutTry(&lvalp->ivs->meta);
	else if (!strncmp("catch",motoyytext,5))
		moto_illegalCatch(&lvalp->ivs->meta,'\1');
	else if (!strncmp("finally",motoyytext,7))
		moto_illegalFinally(&lvalp->ivs->meta,'\1');
	else if ( lvalp && embedded )
		moto_malformedStatement( &lvalp->ivs->meta,'\1') ; 
	return 1;
}

int recover() {
   if(depth > 1) depth=1;
   return 1;
}

static union unionCell *
integer(MotoEnv *env, SVS *svs) {	
	UnionCell *p;
	ParsedInt u;	
        char* value = svs->sval;
	p = (UnionCell *)mman_malloc(env->tree->mpool, sizeof(ValueCell));
	p->type = VALUE_TYPE;					   
	parse_int(value, &u);
	if (u.t == I32) {
		int32_t *n = mman_malloc(env->tree->mpool, sizeof(int32_t));
		*n = u.i;
		p->valuecell.value = n;					   
		p->valuecell.kind = INT32_VALUE;					   
	}
	else {
		int64_t *n = mman_malloc(env->tree->mpool, sizeof(int64_t));
		*n = u.l;
		p->valuecell.value = n;					   
		p->valuecell.kind = INT64_VALUE;					   
	}
	p->valuecell.code = value;

        p->valuecell.meta.filename = svs->meta.filename;
        p->valuecell.meta.macroname = svs->meta.macroname;
        p->valuecell.meta.lineno = svs->meta.lineno;
        p->valuecell.meta.caller = svs->meta.caller;

	return p;
}

static union unionCell *
fp(MotoEnv *env, SVS * svs) {
	UnionCell *p;
	ParsedFloat u;	
        char* value = svs->sval;
	p = (UnionCell *)mman_malloc(env->tree->mpool, sizeof(ValueCell));
	p->type = VALUE_TYPE;					   
	parse_float(value, &u);
	if (u.t == I32) {
		float *n = mman_malloc(env->tree->mpool, sizeof(float));
		*n = u.f;
		p->valuecell.value = n;					   
		p->valuecell.kind = FLOAT_VALUE;					   
	}
	else {
		double *n = mman_malloc(env->tree->mpool, sizeof(double));
		*n = u.d;
		p->valuecell.value = n;					   
		p->valuecell.kind = DOUBLE_VALUE;					   
	}
	p->valuecell.code = value;

        p->valuecell.meta.filename = svs->meta.filename;
        p->valuecell.meta.macroname = svs->meta.macroname;
        p->valuecell.meta.lineno = svs->meta.lineno;
        p->valuecell.meta.caller = svs->meta.caller;

	return p;
}

/*
 *  Creates a String value cell for a function name or identifier
 */

static union unionCell *
s(MotoEnv *env, SVS *svs) {
	UnionCell *p;
        char* value = svs->sval;
	p = (UnionCell *)mman_malloc(env->tree->mpool, sizeof(ValueCell));
	p->type = VALUE_TYPE;					   
	p->valuecell.kind = STRING_VALUE;					   
	p->valuecell.code = value;
	p->valuecell.value = value;

        p->valuecell.meta.filename = svs->meta.filename;
        p->valuecell.meta.macroname = svs->meta.macroname;
        p->valuecell.meta.lineno = svs->meta.lineno;
        p->valuecell.meta.caller = svs->meta.caller;

	return p;
}

/*
 *  Creates a String value cell for an unquoted (outside of a construct) string
 *  (escapes the code for error reporting)
 */

static union unionCell *
as(MotoEnv *env, SVS *svs) {
	UnionCell *p;
        char* value = svs->sval;
	p = (UnionCell *)mman_malloc(env->tree->mpool, sizeof(ValueCell));
	p->type = VALUE_TYPE;					   
	p->valuecell.kind = STRING_VALUE;					   

	p->valuecell.value = value;
	p->valuecell.code = escapeStrConst(value);

        p->valuecell.meta.filename = svs->meta.filename;
        p->valuecell.meta.macroname = svs->meta.macroname;
        p->valuecell.meta.lineno = svs->meta.lineno;
        p->valuecell.meta.caller = svs->meta.caller;

	mman_track(env->tree->mpool, p->valuecell.code);
	return p;
}

/*
 *  Creates a String valuecell for a quoted string
 *  Unescapes the value for use
 */

static union unionCell *
ls(MotoEnv *env, SVS *svs) {
	UnionCell *p;
	char* value = svs->sval;

	p = (UnionCell *)mman_malloc(env->tree->mpool, sizeof(ValueCell));
	p->type = VALUE_TYPE;					   
	p->valuecell.kind = STRING_VALUE;					   

	p->valuecell.value = unescapeStrConst(value);
	p->valuecell.code = value; 
	
	p->valuecell.meta.filename = svs->meta.filename;
	p->valuecell.meta.macroname = svs->meta.macroname;
	p->valuecell.meta.lineno = svs->meta.lineno;
	p->valuecell.meta.caller = svs->meta.caller;

	mman_track(rtime_getRuntime()->mpool, p->valuecell.value);

	return p;
}

static union unionCell *
id(MotoEnv *env, SVS *svs) {
	UnionCell *p;
   char* name = svs->sval;

	p = (UnionCell *)mman_malloc(env->tree->mpool, sizeof(ValueCell));
	p->type = VALUE_TYPE;					   
	p->valuecell.kind = ID_VALUE;					   
	p->valuecell.code = name;
	p->valuecell.value = name;

	p->valuecell.meta.filename = svs->meta.filename;
	p->valuecell.meta.macroname = svs->meta.macroname;
	p->valuecell.meta.lineno = svs->meta.lineno;
	p->valuecell.meta.caller = svs->meta.caller;

	return p;
}

static union unionCell *
rx(MotoEnv *env, SVS *svs) {
	UnionCell *p;
	char* value = svs->sval;

	p = (UnionCell *)mman_malloc(env->tree->mpool, sizeof(ValueCell));
	p->type = VALUE_TYPE;					   
	p->valuecell.kind = REGEX_VALUE;					   
	p->valuecell.code = value;
	p->valuecell.value = value + 1;
	value[strlen(value) - 1] = '\0';

	p->valuecell.meta.filename = svs->meta.filename;
	p->valuecell.meta.macroname = svs->meta.macroname;
	p->valuecell.meta.lineno = svs->meta.lineno;
	p->valuecell.meta.caller = svs->meta.caller;

	return p;
}

static union unionCell *
op(MotoEnv *env, int opcode, MetaInfo* meta, int opcount, ...) {
	UnionCell *p;
	va_list ap;
	size_t size;
	int i;
	size = sizeof(OpCell) + ((opcount - 1) * sizeof(UnionCell *));

	p = (UnionCell *)mman_malloc(env->tree->mpool, size);
	p->type = OP_TYPE;
	p->opcell.opcode = opcode;
	p->opcell.opcount = opcount;

	p->opcell.meta.lineno = meta->lineno;
	p->opcell.meta.filename = meta->filename;
	p->opcell.meta.macroname = meta->macroname;
	p->opcell.meta.caller = meta->caller;

	va_start(ap, opcount);
	for (i = 0; i < opcount; i++) {
		p->opcell.operands[i] = va_arg(ap, union unionCell *);
	}
	va_end(ap);
	return p;
}

static union unionCell *
listAdd(MotoEnv *env, union unionCell * list, union unionCell* addition) {
	UnionCell *p;
	size_t size;
	size = sizeof(OpCell) + ((list->opcell.opcount) * sizeof(UnionCell *));
	
	mman_untrack(list);
	p = (UnionCell *)realloc(list,size);
	mman_track(env->tree->mpool,p);
	
	p->opcell.operands[p->opcell.opcount] = addition;
	p->opcell.opcount += 1;
	
	return p;
}

static int 
hexCharToInt(char c) {
	int n;
	switch (c) {
		case 'a':
		case 'A':
			n = 10;
			break;
		case 'b':
		case 'B':
			n = 11;
			break;
		case 'c':
		case 'C':
			n = 12;
			break;
		case 'd':
		case 'D':
			n = 13;
			break;
		case 'e':
		case 'E':
			n = 14;
			break;
		case 'f':
		case 'F':
			n = 15;
			break;
		default:
			n = c - '0';
			break;
	}
	return n;
}

int syntax_errors = 0;

extern int peekScope(MotoEnv *env, int verify);
int yyerror (char *s) {
	  // printf("unexpected token: %s %s\n",s, motoyytext);  

	  if (*motoyytext == '\0') { /* EOF */
		  while(stack_size(moto_getEnv()->scope) > 0) {
			  MotoParseScope *scope = stack_peek(moto_getEnv()->scope);
			  int type = scope->type;

			  switch (type) {
				  case FOR: moto_unterminatedFor(&scope->meta,'\0'); break;
				  case IF: moto_unterminatedIf(&scope->meta,'\0'); break;
				  case SWITCH: moto_unterminatedSwitch(&scope->meta,'\0'); break;
				  case WHILE: moto_unterminatedWhile(&scope->meta,'\0'); break;
				  case DEFINE: moto_unterminatedDefinition(&scope->meta,'\0'); break;
				  case CLASSDEF: moto_unterminatedClassDefinition(&scope->meta,'\0'); break;
				  case EXCP_TRY: moto_unterminatedTry(&scope->meta,'\0'); break;
			 
				  case EFOR: moto_unterminatedFor(&scope->meta,'\1'); break;
				  case EIF: moto_unterminatedIf(&scope->meta,'\1'); break;
				  case ESWITCH: moto_unterminatedSwitch(&scope->meta,'\1'); break;
				  case EWHILE: moto_unterminatedWhile(&scope->meta,'\1'); break;
				  case EDEFINE: moto_unterminatedDefinition(&scope->meta,'\1'); break;
				  case ECLASSDEF: moto_unterminatedClassDefinition(&scope->meta,'\1'); break;
				  case EXCP_ETRY: moto_unterminatedTry(&scope->meta,'\1'); break;
			  }
			  free(scope);
			  stack_pop(moto_getEnv()->scope);
		  }
	  }
	  syntax_errors++;
	  if (*motoyytext=='\0') return 1;
	  if (syntax_errors < 10){
		  return 1;
	  } 
	  return 0;
}

void
pushScope(MotoEnv *env, int type) {
        MotoParseScope *scope = emalloc(sizeof(MotoParseScope));
        scope->type = type;
        scope->meta = env->meta;
        stack_push(env->scope, scope);
}

int
popScope(MotoEnv *env, int verify) {

	/* Pop until we see the verify scope we are looking for.
	  Every scope above that is unterminated */
	
	while(stack_size(moto_getEnv()->scope) > 0) {
		MotoParseScope *scope = stack_peek(moto_getEnv()->scope);
		int type = scope->type;
		
		if(type==verify) {
			stack_pop(moto_getEnv()->scope);
			free(scope);
			return 1;
		}
		
		switch (type) {
			case FOR: moto_unterminatedFor(&scope->meta,'\0'); break;
			case IF: moto_unterminatedIf(&scope->meta,'\0'); break;
			case SWITCH: moto_unterminatedSwitch(&scope->meta,'\0'); break;
			case WHILE: moto_unterminatedWhile(&scope->meta,'\0'); break;
			case DEFINE: moto_unterminatedDefinition(&scope->meta,'\0'); break;
			case EXCP_TRY : moto_unterminatedTry(&scope->meta,'\0'); break;
			
			case EFOR: moto_unterminatedFor(&scope->meta,'\1'); break;
			case EIF: moto_unterminatedIf(&scope->meta,'\1'); break;
			case ESWITCH: moto_unterminatedSwitch(&scope->meta,'\1'); break;
			case EWHILE: moto_unterminatedWhile(&scope->meta,'\1'); break;
			case EDEFINE: moto_unterminatedDefinition(&scope->meta,'\1'); break;
			case EXCP_ETRY: moto_unterminatedTry(&scope->meta,'\1'); break;
		}
		
		stack_pop(moto_getEnv()->scope);
		free(scope);
	}
	
	return 1;
}

int
loopScope(MotoEnv *env) {
	int index;
	int size;
	if ((size = stack_size(env->scope)) > 0) {
		index = 1;
		while (index <= size) {
			MotoParseScope *scope = stack_peekAt(env->scope, index);
			int type = scope->type;
			index++;
			if (type == FOR || type == WHILE || type == EFOR || type == EWHILE) {
					  return 1;
			}
		}
	}
	return 0;
}

int
tryScope(MotoEnv *env) {
	int index;
	int size;
	if ((size = stack_size(env->scope)) > 0) {
		index = 1;
		while (index <= size) {
			MotoParseScope *scope = stack_peekAt(env->scope, index);
			int type = scope->type;
			index++;
			if (type == EXCP_TRY || type == EXCP_ETRY) {
					  return 1;
			}
		}
	}
	return 0;
}

int 
classDefinitionScope(MotoEnv *env) {
	MotoParseScope *scope;

    if (stack_size(env->scope)==0)
    	return 0;
    	
	scope = stack_peekAt(env->scope, stack_size(env->scope));
	if (scope->type == CLASSDEF || scope->type == ECLASSDEF )
		return 1;
                              
	return 0;
}

int 
anyScope(MotoEnv* env) {
   return stack_size(env->scope) > 0;
}

int
definitionScope(MotoEnv *env) {
	int index;
	int size;
	if ((size = stack_size(env->scope)) > 0) {
		index = 1;
		while (index <= size) {
			MotoParseScope *scope = stack_peekAt(env->scope, index);
			index++;
			if (scope->type == DEFINE || scope->type == EDEFINE ) {
				return 1;
			}
		}
	}
	return 0;
}

/*=============================================================================
// end of file: $RCSfile: moto.y,v $
==============================================================================*/

