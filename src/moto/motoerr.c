/*=============================================================================

	Copyright (C) 2000 Kenneth Kocienda and David Hakim.
	This file is part of the Moto Programming Language.

	Moto Programming Language is free software; you can redistribute it and/or
	modify it under the terms of the GNU Library General Public License as
	published by the Free Software Foundation; either version 2 of the
	License, or (at your option) any later version.

	Moto Programming Language is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the GNU
	Library General Public License for more details.

	You should have received a copy of the GNU Library General Public
	License along with the Codex C Library; see the file COPYING.	If not,
	write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
	Boston, MA 02111-1307, USA.

	$RCSfile: motoerr.c,v $	  
	$Revision: 1.51 $
	$Author: dhakim $
	$Date: 2003/03/12 16:41:14 $
 
==============================================================================*/
#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define __MOTOERR_FILE_C

#include <stdlib.h>
#include <stdarg.h>
#include <setjmp.h>
#include <string.h>

#include "memsw.h"

#include "fileio.h"
#include "log.h"
#include "err.h"
#include "stack.h"

#include "cell.h"
#include "env.h"
#include "pp.h"
#include "motolex.h"
#include "moto.tab.h"
#include "motoerr.h"
#include "motoutil.h"
#include "motohook.h"
#include "motofunction.h"

excp_declare(MotoException);
excp_declare(MotoCellTypeException);
excp_declare(MotoOpcodeException);

static void motoppiniterr(char *fmt, va_list ap) {
	fprintf(stderr, "*** motopp initialization error\n");
	vfprintf(stderr, fmt, ap);
	exit(-1);
}
static void motopperr(char *fmt, va_list ap) {
	MotoPP *ppenv = motopp_getEnv();
	buf_vprintf(ppenv->err, "%s:%d: *** motopp error\n", ap);
	buf_vprintf(ppenv->err, fmt, ap);
}
static void motoiniterr(char *fmt, va_list ap) {
	fprintf(stderr, "*** moto initialization error\n");
	vfprintf(stderr, fmt, ap);
	exit(-1);
}

static void 
motoyerr(char *fmt, va_list ap) {
	MotoEnv *env = moto_getEnv();
	char *err;
	StringBuffer *buf = buf_createDefault();

	buf_puts(buf,env->errmeta.filename);
	buf_putc(buf,':');
	buf_puti(buf,env->errmeta.lineno);
	buf_puts(buf,": ");

	buf_vprintf(buf, fmt, ap);							/* Fill in error text */
	err = buf_toString(buf);

	if (!sset_contains(env->errs, err)) {
		MetaInfo* caller;
		char* macroname;

		sset_add(env->errs, err);
		buf_puts(env->err, err);

		macroname=env->errmeta.macroname;
		
		for (caller = env->errmeta.caller; caller != NULL; caller=caller->caller) {
			if (macroname == NULL) {
				buf_puts(env->err, "	  >>> ");
				buf_printf(env->err, "in file included from %s:%d\n", 
					caller->filename, caller->lineno);
			} else {
				buf_puts(env->err, "	  >>> ");
				buf_printf(env->err, "in macro %s called from %s:%d\n", 
					macroname, caller->filename, caller->lineno);
			}
			macroname = caller->macroname;
		}
	} else { free(err); }

	buf_free(buf);

	env->errflag = 1;
}

static char FMT[] = "%s:%d: %s";
static char RUN[] = "*** syntax error: possible runaway statement\n";
static char SYN[] = "*** syntax error\n";
static Err errs[] = {

	/* Internal Parse Errors */

	{"InternalOpcodeError: Unrecognized opcode: %d\nThis is a bug moto language bug", FMT, NULL},
	{"InternalCellTypeError: Unrecognized cell type code: %d\nThis is a moto language bug", FMT, NULL},

	/* Preprocessor Errors */

	{"InputFileNotFound", "Input file not found: %s\n", "motoppiniterr"},
	{"BadIfDirective", "%s:%d: Malformed #if directive\n", "motopperr"},
	{"BadElseIfDirective", "%s:%d: Malformed #elseif directive\n", "motopperr"},
	{"BadDefineDirective", "%s:%d: Malformed #define directive\n", "motopperr"},
	{"BadUndefDirective", "%s:%d: Malformed #undef directive\n", "motopperr"},
	{"BadIfdefDirective", "%s:%d: Malformed #ifdef directive\n", "motopperr"},
	{"BadIfndefDirective", "%s:%d: Malformed #ifndef directive\n", "motopperr"},
	{"BadIncludeDirective", "%s:%d: Malformed #include directive\n", "motopperr"},
	{"BadReaddefsDirective", "%s:%d: Malformed #readdefs directive\n", "motopperr"},
	{"BadCallParamList", "%s:%d: Malformed parameter list in macro call\n", "motopperr"},
	{"BadDefineParamList", "%s:%d: Malformed parameter list in #define directive\n", "motopperr"},
	{"UnterminatedMacroCall", "%s:%d: Unterminated macro call\n", "motopperr"},
	{"IncludeFileNotFound", "%s:%d: #include file not found: %s\n", "motopperr"},
	{"ReaddefsFileNotFound", "%s:%d: #readdefs file not found: %s\n", "motopperr"},
	{"PPEndIfWithoutIf", "%s:%d: #endif without #if\n", "motopperr"},
	{"PPElseIfWithoutIf", "%s:%d: #elseif without #if\n", "motopperr"},
	{"PPElseWithoutIf", "%s:%d: #else without #if\n", "motopperr"},
	{"BadExpression", "%s:%d: Malformed conditional expression\n", "motopperr"},
	{"MultiplyDefinedParamName", "%s:%d: Mulitply defined argument name in parameter list: %s\n", "motopperr"},
	
	/* Moto Runtime Errors */
  
	{"MotoRuntimeError","Uncaught %s\n    message: %s\n\n%s","motoyerr"},

	/* Moto Dynamic Loader Errors */
	
	{"FunctionAlreadyDefinedInExtension","Function %s re-defined.\n   Original definition in extension \"%s\"\n", "motoyerr"},
	{"FunctionAlreadyDefined", "Function %s re-defined.\n   Original definition at %s:%d\n", "motoyerr"},
	{"MethodAlreadyDefinedInExtension","Method %s re-defined.\n   Original definition in extension \"%s\"\n", "motoyerr"},
	{"MethodAlreadyDefined", "Method %s re-defined.\n   Original definition at %s:%d\n", "motoyerr"},
	{"ClassAlreadyDefinedInExtension", "Class %s re-defined.\n   Original definition in extension \"%s\"\n", "motoyerr"},
	{"ClassAlreadyDefined", "Class %s re-defined.\n   Original definition at %s:%d\n", "motoyerr"},
	
	/* Moto Verifier Errors */

	{"AmbiguousFunction", "Ambiguous function call: %s\n", "motoyerr"},
	{"CannotCast", "Expression of type <%s> cannot be cast to <%s>\n", "motoyerr"},
	{"CannotCastToVoid", "Expressions cannot be cast to <void>\n", "motoyerr"},
	{"CannotPrint", "Cannot print expression of type <void>\n", "motoyerr"},
	{"CantInvokeMethodOnType", "Cannot invoke a method on this type: <%s>\n", "motoyerr"},
	{"CantDereferenceType", "Cannot dereference this type: <%s>\n", "motoyerr"},
	{"ExpressionNotCallable", "Expression of type <%s> not callable\n", "motoyerr"},	
	{"IllegalUnaryOperation", "Illegal unary operation: %s <%s>\n", "motoyerr"},
	{"IllegalBinaryOperation", "Illegal binary operation: <%s> %s <%s>\n", "motoyerr"},
	{"IllegalTernaryOperands", "Illegal ternary operands: <%s> ? <%s> : <%s>\n", "motoyerr"},
	{"IllegalDeleteOperation", "Only reference types have destructors\n", "motoyerr"},
	{"IllegalIncrement", "Illegal value for increment\n", "motoyerr"},
	{"IllegalDecrement", "Illegal value for decrement\n", "motoyerr"},
	{"IllegalAssignment", "Illegal value for assignment\n", "motoyerr"},
	{"IllegalConditional", "Illegal type for condition: <%s>\n","motoyerr"},
	{"IllegalTypeForArrayIndex", "Illegal type used for array index: <%s>\n","motoyerr"},
	{"IllegalTypeForSubscriptOp", "The subscript operator [] may only be used on Arrays, not variables of type <%s>\n","motoyerr"},
	{"IllegalSubscriptDimension", "The dimensions specified (%d) exceeds the dimensions of the array (%d)\n","motoyerr"},
	{"IllegalSwitchArgument", "Cannot switch on an expression of type <%s> \n", "motoyerr"},
	{"IllegalCaseType", "Case type must match that of switch: <%s> != <%s>\n", "motoyerr"},
	{"IllegalCaseArgument", "Case arguments must be literals \n", "motoyerr"},
	{"IllegalUnknown", "'?' may only be used as an argument to functional typed expressions\n", "motoyerr"},
	{"MultiplyDefinedType", "Multiply defined type <%s>\n", "motoyerr"}, 
	{"MultiplyDeclaredVar", "Multiply declared variable '%s'\n", "motoyerr"},
	{"NoSuchLibrary", "No such library: %s\n", "motoyerr"},
	{"NoSuchFunction", "No such function: %s\n", "motoyerr"},
	{"NoSuchMethod", "No such method: %s::%s\n", "motoyerr"},
	{"NoSuchMember", "No such class member: %s::%s\n", "motoyerr"},
	{"NoSuchConstructor", "No such constructor: %s\n", "motoyerr"},
	{"RegexParseError","Invalid regular expression /%s/: %s\n","motoyerr"}, 
	{"ReturnTypeMustBeSpecified", "A return type must be specified for all functions and methods except constructors.\n", "motoyerr"},
	{"ReturnTypeMustNotBeSpecified", "A return type must not be specified for constructors.\n", "motoyerr"},
	{"TooManyArgumentsToCall", "Too many arguments provided to call: <%s>\n", "motoyerr"},
	{"TooFewArgumentsToCall", "Too few arguments provided to call: <%s>\n", "motoyerr"},	
	{"TypeNotDefined", "Type not defined: <%s>\n", "motoyerr"}, 
	{"TypeNotCatchable", "Cannot catch <%s>. Only exceptions may be caught\n", "motoyerr"},
	{"TypeNotThrowable", "Cannot throw <%s>. Only exceptions may be thrown\n", "motoyerr"},
	{"TypeAlreadyCaught", "The exception of type <%s> will have already been caught by a prior catch\n", "motoyerr"},
	{"VarsCannotBeVoid", "Variables cannot be declared type <void>\n", "motoyerr"},	 
	{"VarNotDeclared", "Variable '%s' not declared before used\n", "motoyerr"},
	{"VarTypeAssignError", "Variable of type <%s> cannot store type <%s>\n", "motoyerr"},
	{"VarTypeReturnError", "Return type <%s> cannot be implicitly cast to type <%s>\n", "motoyerr"},

	/* Lex Errors */

	{"UnmatchedParen", "Unmatched '('\n", "motoyerr"},
	{"UnterminatedString", "Unterminated string\n", "motoyerr"},
	{"SyntaxError", "Syntax Error\n", "motoyerr"},
	{"NoSuchConstruct", "No such construct: %s\n", "motoyerr"},
	{"EmbSyntaxError", "Syntax Error\n", "motoyerr"},

	/* Yacc Errors */

	{"EmbElseWithoutIf", "else without if\n", "motoyerr"},
	{"EmbUnterminatedDefinition", "Unterminated function definition\n", "motoyerr"},
	{"EmbUnterminatedFor", "Unterminated for loop\n", "motoyerr"},
	{"EmbUnterminatedIf", "Unterminated if statement\n", "motoyerr"},
	{"EmbUnterminatedSwitch", "Unterminated switch statement\n", "motoyerr"},
	{"EmbUnterminatedWhile", "Unterminated while loop\n", "motoyerr"},
	{"EmbUnterminatedTry", "Unterminated try block\n", "motoyerr"},
	{"EmbUnterminatedClassDefinition", "Unterminated class definition\n", "motoyerr"},
	
	{"EmbIllegalBreak", "break not within loop\n", "motoyerr"},
	{"EmbIllegalCase", "case not within switch\n", "motoyerr"},
	{"EmbIllegalCatch", "catch not within try block\n", "motoyerr"},
	{"EmbIllegalClassDefinition", "class definition within sub-scope\n", "motoyerr"},
	{"EmbIllegalContinue", "continue not within loop\n", "motoyerr"},
	{"EmbIllegalDefine", "function definition within sub-scope\n", "motoyerr"},
	{"EmbIllegalFinally", "finally not within try block\n", "motoyerr"},
	{"EmbIllegalReturn", "return not within function or method definition\n", "motoyerr"},
	{"EmbIllegalTry", "try without catch or finally\n", "motoyerr"},
	{"EmbIllegalUse", "use within sub-scope\n", "motoyerr"},
	
	{"IllegalBreak", "$break not within loop\n", "motoyerr"},
	{"IllegalCase", "$case not within $switch\n", "motoyerr"},
	{"IllegalCatch", "$catch not within $try ... $endtry\n", "motoyerr"},
	{"IllegalClassDefinition", "$class within sub-scope\n", "motoyerr"},
	{"IllegalContinue", "$continue not within loop\n", "motoyerr"},
	{"IllegalDefine", "$define within sub-scope\n", "motoyerr"},
	{"IllegalFinally", "$finally not within $try ... $endtry\n", "motoyerr"},
	{"IllegalGlobal", "global within sub-scope\n", "motoyerr"},
	{"IllegalReturn", "$return not within function or method defintion\n", "motoyerr"},
	{"IllegalThis", "'this' not used within method defintion\n", "motoyerr"},
	{"IllegalTry", "$try without $catch or $finally\n", "motoyerr"},
	{"IllegalUse", "$use within sub-scope\n", "motoyerr"},
	
	{"MalformedCase", "Malformed $case statement, expected $case(<literal>)\n", "motoyerr"},
	{"MalformedCatch", "Malformed catch statement, expected $catch(<type> <name>)\n", "motoyerr"},
	{"MalformedClassDefinition", "Malformed $class statement, expected $class(<name>)\n", "motoyerr"},
	{"MalformedDeclare", "Malformed $declare statement, expected $declare(<type> <name> (=<expression>)?)\n", "motoyerr"},
	{"MalformedDefine", "Malformed $define statement, expected $(<type> <name>(<args>))\n", "motoyerr"},
	{"MalformedDo", "Malformed $do statement, expected $do(<expr>)\n", "motoyerr"},
	{"MalformedElseIf", "Malformed $elseif statement, expected $elseif(<expr>)\n", "motoyerr"},
	{"MalformedFor", "Malformed $for statement, expected $for((<expr>)?;<expr>;(<expr>)?)\n", "motoyerr"},
	{"MalformedIf", "Malformed $if statement, expected $if(<expr>)\n", "motoyerr"},
	{"MalformedPrint", "Malformed print statement, expected $(<expr>)\n", "motoyerr"},
	{"MalformedReturn", "Malformed $return statement, expected $return(<expr>)\n", "motoyerr"},
	{"MalformedStatement", "Malformed statement\n", "motoyerr"},
	{"MalformedSwitch", "Malformed $switch statement, expected $switch(<expr>)\n", "motoyerr"},
	{"MalformedThrow", "Malformed throw statement, expected $throw(<expression>)\n", "motoyerr"},
	{"MalformedUse", "Malformed $use statement, expected $use(<string literal>)\n", "motoyerr"},
	{"MalformedWhile", "Malformed $while statement, expected $while(<expr>)\n", "motoyerr"},
	
	{"MalformedEmbCase", "Malformed case, expected case <literal>: <statement list>\n", "motoyerr"},
	{"MalformedEmbCatch", "Malformed catch, expected catch(<exception type> <name>) {...}\n", "motoyerr"},
	{"MalformedEmbClassDefintion", "Malformed class defintion, expected class <name> {...} \n", "motoyerr"},
	{"MalformedEmbDeclare", "Malformed declaration, expected <type> <name> (=<expr>)?;\n", "motoyerr"},
	{"MalformedEmbDefine", "Malformed function prototype, expected <type> <name>(<args>)\n", "motoyerr"},
	{"MalformedEmbDo", "Malformed expression, expected <expr>;\n", "motoyerr"},
	{"MalformedEmbFor", "Malformed for statement, expected for((<expr>)?;<expr>;(<expr>)?) <statement>\n", "motoyerr"},
	{"MalformedEmbIf", "Malformed if statement, expected if(<expr>) <statement> (else <statement>)?\n", "motoyerr"},
	{"MalformedEmbPrint", "Malformed print statement, expected print <expr>;\n", "motoyerr"},
	{"MalformedEmbReturn", "Malformed return statement, expected return <expr>;\n", "motoyerr"},
	{"MalformedEmbStatement", "Malformed statement\n", "motoyerr"},
	{"MalformedEmbSwitch", "Malformed switch statement, expected switch(<expr>) <case statement list>\n", "motoyerr"},
	{"MalformedEmbThrow", "Malformed throw, expected throw <expression>\n", "motoyerr"},
	{"MalformedEmbUse", "Malformed use statement, expected use <string literal>;\n", "motoyerr"},
	{"MalformedEmbWhile", "Malformed while statement, expected while(<expr>) <statement>\n", "motoyerr"},
	
	{"EndForWithoutFor", "$endfor without $for\n", "motoyerr"},
	{"EndWhileWithoutWhile", "$endwhile without $while\n", "motoyerr"},
	{"ElseWithoutIf", "$else without $if\n", "motoyerr"},
	{"ElseIfWithoutIf", "$elseif without $if\n", "motoyerr"},
	{"EndIfWithoutIf", "$endif without $if\n", "motoyerr"},
	{"EndDefWithoutDefine", "$enddef without $define\n", "motoyerr"},
	{"EndClassWithoutClass", "$endclass without $class\n", "motoyerr"},
	{"EndSwitchWithoutSwitch", "$endswitch without $switch\n", "motoyerr"},
	{"EndTryWithoutTry", "$endtry without $try\n", "motoyerr"},
	
	{"UnterminatedClassDefinition", "Unterminated $class statement\n", "motoyerr"},
	{"UnterminatedDefinition", "Unterminated $define statement\n", "motoyerr"},
	{"UnterminatedFor", "Unterminated $for loop\n", "motoyerr"},
	{"UnterminatedIf", "Unterminated $if statement\n", "motoyerr"},
	{"UnterminatedSwitch", "Unterminated $switch statement\n", "motoyerr"},
	{"UnterminatedWhile", "Unterminated $while loop\n", "motoyerr"},
	{"UnterminatedTry", "Unterminated $try block\n", "motoyerr"},
	
	{ NULL },
};

/*----------------------------------------------------------------------------- 
 * initialization
 *---------------------------------------------------------------------------*/
void moto_errInit() {
	err_register((Err *)errs);
	err_registerHandler("motopperr", motopperr);
	err_registerHandler("motoppiniterr", motoppiniterr);
	err_registerHandler("motoyerr", motoyerr);
	err_registerHandler("motoiniterr", motoiniterr);
}

/*----------------------------------------------------------------------------- 
 * preprocessor
 *---------------------------------------------------------------------------*/

void 
motopp_abort(MotoPP *ppenv) {
	log_debug(__FILE__, ">>> motopp_abort\n");
	THROW_D("MotoException");
	log_debug(__FILE__, "<<< motopp_abort\n");
}

void 
motopp_inputFileNotFound(char *name) {
	err_error("InputFileNotFound", name);
}

void 
motopp_badIfDirective(MotoPP *ppenv, int lineno) {
	ppenv->errlineno = lineno;
	err_error("BadIfDirective", ppenv->frame->filename, lineno);
	motopp_printStackTrace(ppenv);
	motopp_abort(ppenv);
}
void 
motopp_badElseIfDirective(MotoPP *ppenv, int lineno) {
	ppenv->errlineno = lineno;
	err_error("BadElseIfDirective", ppenv->frame->filename, lineno);
	motopp_printStackTrace(ppenv);
	motopp_abort(ppenv);
}

void 
motopp_badDefineDirective(MotoPP *ppenv, int lineno) {
	ppenv->errlineno = lineno;
	err_error("BadDefineDirective", ppenv->frame->filename, lineno);
	motopp_printStackTrace(ppenv);
	motopp_abort(ppenv);
}

void 
motopp_badUndefDirective(MotoPP *ppenv, int lineno) {
	ppenv->errlineno = lineno;
	err_error("BadUndefDirective", ppenv->frame->filename, lineno);
	motopp_printStackTrace(ppenv);
	motopp_abort(ppenv);
}

void 
motopp_badIfdefDirective(MotoPP *ppenv, int lineno) {
	ppenv->errlineno = lineno;
	err_error("BadIfdefDirective", ppenv->frame->filename, lineno);
	motopp_printStackTrace(ppenv);
	motopp_abort(ppenv);
}

void 
motopp_badIfndefDirective(MotoPP *ppenv, int lineno) {
	ppenv->errlineno = lineno;
	err_error("BadIfndefDirective", ppenv->frame->filename, lineno);
	motopp_printStackTrace(ppenv);
	motopp_abort(ppenv);
}

void 
motopp_badReaddefsDirective(MotoPP *ppenv, int lineno) {
	ppenv->errlineno = lineno;
	err_error("BadReaddefsDirective", ppenv->frame->filename, lineno);
	motopp_printStackTrace(ppenv);
	motopp_abort(ppenv);
}

void 
motopp_badIncludeDirective(MotoPP *ppenv, int lineno) {
	ppenv->errlineno = lineno;
	err_error("BadIncludeDirective", ppenv->frame->filename, lineno);
	motopp_printStackTrace(ppenv);
	motopp_abort(ppenv);
}

void 
motopp_badCallParamList(MotoPP *ppenv, int lineno) {
	ppenv->errlineno = lineno;
	err_error("BadCallParamList", ppenv->frame->filename, lineno);
	motopp_printStackTrace(ppenv);
	motopp_abort(ppenv);
}

void 
motopp_badDefineParamList(MotoPP *ppenv, int lineno) {
	ppenv->errlineno = lineno;
	err_error("BadDefineParamList", ppenv->frame->filename, lineno);
	motopp_printStackTrace(ppenv);
	motopp_abort(ppenv);
}

void 
motopp_unterminatedMacroCall(MotoPP *ppenv, int lineno) {
	ppenv->errlineno = lineno;
	err_error("UnterminatedMacroCall", ppenv->frame->filename, lineno - 1);
	motopp_printStackTrace(ppenv);
	motopp_abort(ppenv);
}

void 
motopp_includeFileNotFound(MotoPP *ppenv, int lineno, char *filename) {
	ppenv->errlineno = lineno;
	err_error("IncludeFileNotFound", ppenv->frame->filename, lineno, filename);
	motopp_printStackTrace(ppenv);
	motopp_abort(ppenv);
}

void 
motopp_readdefsFileNotFound(MotoPP *ppenv, int lineno, char *filename) {
	ppenv->errlineno = lineno;
	err_error("ReaddefsFileNotFound", ppenv->frame->filename, lineno, filename);
	motopp_printStackTrace(ppenv);
	motopp_abort(ppenv);
}

void 
motopp_endifWithoutIf(MotoPP *ppenv, int lineno) {
	ppenv->errlineno = lineno;
	err_error("PPEndIfWithoutIf", ppenv->frame->filename, lineno);
	motopp_printStackTrace(ppenv);
	motopp_abort(ppenv);
}

void 
motopp_elseifWithoutIf(MotoPP *ppenv, int lineno) {
	ppenv->errlineno = lineno;
	err_error("PPElseIfWithoutIf", ppenv->frame->filename, lineno);
	motopp_printStackTrace(ppenv);
	motopp_abort(ppenv);
}

void 
motopp_elseWithoutIf(MotoPP *ppenv, int lineno) {
	ppenv->errlineno = lineno;
	err_error("PPElseWithoutIf", ppenv->frame->filename, lineno);
	motopp_printStackTrace(ppenv);
	motopp_abort(ppenv);
}

void 
motopp_badExpression(MotoPP *ppenv, int lineno) {
	ppenv->errlineno = lineno;
	err_error("BadExpression", ppenv->frame->filename, lineno);
	motopp_printStackTrace(ppenv);
	motopp_abort(ppenv);
}

void 
motopp_multiplyDefinedParamName(MotoPP *ppenv, int lineno, char *name) {
	ppenv->errlineno = lineno;
	err_error("MultiplyDefinedParamName", ppenv->frame->filename, lineno, name);
	motopp_printStackTrace(ppenv);
	motopp_abort(ppenv);
}

void 
motopp_badMacroCall(MotoPP *ppenv, int lineno) {
	ppenv->errlineno = lineno;
	err_error("BadMacroCall", ppenv->frame->filename, lineno);
	motopp_printStackTrace(ppenv);
	motopp_abort(ppenv);
}

void 
motopp_printStackTrace(MotoPP *ppenv) {
	int i;
	int size = stack_size(ppenv->frames);
	log_debug(__FILE__, "Stack size: %d\n", size);
	if (size > 1) {
		buf_putc(ppenv->err, '\n');
	}
	for (i = 1; i <= size; i++) {
		MotoPPFrame *frame = stack_peekAt(ppenv->frames, i);
		MotoPPFrame *pframe;
		MotoMacro *m;
		switch (frame->type) {
			case INCLUDE_FRAME:
				pframe = stack_peekAt(ppenv->frames, i + 1);
				buf_printf(ppenv->err, "	>>> at file included from %s:%d\n", 
							  pframe->filename, frame->lineno);
				break;
			case READDEFS_FRAME:
				pframe = stack_peekAt(ppenv->frames, i + 1);
				buf_printf(ppenv->err, "	>>> at macro library read from %s:%d\n", 
							  pframe->filename, frame->lineno);
				break;
			case MACRO_FRAME:
				pframe = stack_peekAt(ppenv->frames, i + 1);
				m = frame->macro;
				buf_printf(ppenv->err, "	>>> at macro %s called from %s:%d\n", 
								m->name, pframe->filename, frame->lineno);
				break;
			default:
				break;
		}
	}	 
}

void 
motopp_syntaxError(MotoPP *ppenv, int lineno) {
	char *filename;
	if (ppenv->frame != NULL) {
		filename = ppenv->frame->filename;
		buf_printf(ppenv->err, FMT, filename, lineno, SYN);
	}
	else {
		buf_printf(ppenv->err, FMT, "???" , lineno, RUN);
	}
	motopp_printStackTrace(ppenv);
	motopp_abort(ppenv);
}

/*----------------------------------------------------------------------------- 
 * moto language 
 *---------------------------------------------------------------------------*/

void 
moto_abort() {
	MotoEnv *env = moto_getEnv();	  
	log_debug(__FILE__, ">>> moto_abort\n");
	env->errflag = 1;
	THROW("MotoException", "bad mojo");
	log_debug(__FILE__, "<<< moto_abort\n");
}

/*=============================================================================
	begin dynamic loader errors 
=============================================================================*/

void 
moto_functionAlreadyDefinedInExtension(char* motoname,char*libname){
	err_error("FunctionAlreadyDefinedInExtension",motoname,libname);
}

void 
moto_functionAlreadyDefined(char *motoname, const MetaInfo* meta){
	err_error("FunctionAlreadyDefined",motoname,meta->filename,meta->lineno);
}

void 
moto_methodAlreadyDefinedInExtension(char* motoname,char*libname){
	err_error("MethodAlreadyDefinedInExtension",motoname,libname);
}

void 
moto_methodAlreadyDefined(char *motoname, const MetaInfo* meta){
	err_error("MethodAlreadyDefined",motoname,meta->filename,meta->lineno);
}

void 
moto_classAlreadyDefinedInExtension(char* classname, char* libname){
	err_error("ClassAlreadyDefinedInExtension",classname,libname);
}

void 
moto_classAlreadyDefined(char *classname, const MetaInfo* meta){
	err_error("ClassAlreadyDefined",classname,meta->filename,meta->lineno);
}

/*=============================================================================
	begin verifier errors
=============================================================================*/


void 
moto_typeNotCatchable(char* typename){
	err_error("TypeNotCatchable", typename);
}

void 
moto_typeNotThrowable(char* typename){
	err_error("TypeNotThrowable", typename);
}

void 
moto_typeAlreadyCaught(char* typename){
	err_error("TypeAlreadyCaught", typename);
}

void 
moto_multiplyDefinedType(char *name) {
	err_error("MultiplyDefinedType", name);
}

void 
moto_returnTypeMustBeSpecified() {
	err_error("ReturnTypeMustBeSpecified");
}

void 
moto_returnTypeMustNotBeSpecified() {
	err_error("ReturnTypeMustNotBeSpecified");
}

void 
moto_typeNotDefined(char *name) {
	err_error("TypeNotDefined",name);
}

void 
moto_varsCannotBeVoid() {
	err_error("VarsCannotBeVoid");
}

void 
moto_illegalUnknown() {
	err_error("IllegalUnknown");
}

void 
moto_illegalUnaryOperation( int op, char *t) {
	char *symbol = moto_symbolForOp(op);
	err_error("IllegalUnaryOperation",symbol, t);
}

void 
moto_illegalBinaryOperation( int op, char *t1, char *t2) {
	char *symbol = moto_symbolForOp(op);
	err_error("IllegalBinaryOperation",t1, symbol, t2);
}

void 
moto_illegalTernaryOperands( char *t1, char *t2,char* t3) {
	err_error("IllegalTernaryOperands",t1, t2,t3);
}

void 
moto_illegalConditional( char *type) {
	err_error("IllegalConditional",type);
}

void 
moto_illegalDeleteOperation() {
	err_error("IllegalDeleteOperation");
}

void 
moto_illegalSwitchArgument(char *type) {
	err_error("IllegalSwitchArgument", type);
}

void 
moto_illegalCaseType(char *t1, char *t2) {
	err_error("IllegalCaseType",t1, t2);
}

void 
moto_illegalCaseArgument(char *text) {
	err_error("IllegalCaseArgument", text);
}

void 
moto_illegalTypeForArrayIndex(char *type) {
	err_error("IllegalTypeForArrayIndex",type);
}

void 
moto_illegalTypeForSubscriptOp(char *type) {
	err_error("IllegalTypeForSubscriptOp",type);
}

void 
moto_illegalSubscriptDimension(int dim,int adim) {
	err_error("IllegalSubscriptDimension",dim,adim );
}

void 
moto_regexParseError(char* rx,char* reason){
	err_error("RegexParseError",rx,reason);
}

void 
moto_varNotDeclared(char *name) {
	err_error("VarNotDeclared", name);	 
}

void 
moto_varTypeAssignError(char *st1, char *st2) {
	err_error("VarTypeAssignError", st1, st2);
}

void 
moto_varTypeReturnError(char *st1, char *st2) {
	err_error("VarTypeReturnError", st1, st2);
}

void 
moto_cannotCast(char *t1, char *t2) {
	err_error("CannotCast", t1, t2);
}

void 
moto_cannotCastToVoid() {
	err_error("CannotCastToVoid");
}

void 
moto_cannotPrint() {
	err_error("CannotPrint");
}

void 
moto_multiplyDeclaredVar(char *name) {
	err_error("MultiplyDeclaredVar", name);
}

void 
moto_illegalDecrement() {
	err_error("IllegalDecrement");
}

void 
moto_illegalIncrement() {
	err_error("IllegalIncrement");
}

void 
moto_illegalAssignment() {
	err_error("IllegalAssignment");
}

void 
moto_noSuchLibrary(char *name) {
	err_error("NoSuchLibrary", name);
}

void 
moto_noSuchFunction(char *fullname) {
	err_error("NoSuchFunction", fullname);
}

void 
moto_noSuchMethod(char *classname, char *fullname) {
	err_error("NoSuchMethod", classname, fullname);
}

void 
moto_noSuchMember(char *classname, char *membername) {
	err_error("NoSuchMember", classname, membername);
}

void 
moto_noSuchConstructor(char *fullname) {
	err_error("NoSuchConstructor", fullname);
}

void 
moto_tooManyArgumentsToCall(char *type) {
	err_error("TooManyArgumentsToCall", type);
}

void 
moto_tooFewArgumentsToCall(char *type) {
	err_error("TooFewArgumentsToCall", type);
}

void 
moto_expressionNotCallable(char *type) {
	err_error("ExpressionNotCallable", type);
}

void 
moto_ambiguousFunction(char *name) {
	err_error("AmbiguousFunction", name);
}

void 
moto_cantInvokeMethodOnType(char *type) {
	err_error("CantInvokeMethodOnType", type);
}

void 
moto_cantDereferenceType(char *type) {
	err_error("CantDereferenceType", type);
}

/*=============================================================================
	end verifier errors
=============================================================================*/

/*============================================================================= 
	begin lex errors
=============================================================================*/

void 
moto_noSuchConstruct(MetaInfo* meta,char *construct) {
	MotoEnv *env = moto_getEnv();
	env->errmeta = *meta;
	err_error("NoSuchConstruct", construct);
}

void 
moto_unmatchedParen(MetaInfo* meta) {
	MotoEnv *env = moto_getEnv();
	env->errmeta = *meta;
	err_error("UnmatchedParen");
}

void 
moto_unterminatedString(MetaInfo* meta) {
	MotoEnv *env = moto_getEnv();
	env->errmeta = *meta;
	err_error("UnterminatedString");
}

void 
moto_syntaxError(MetaInfo* meta) {
	MotoEnv *env = moto_getEnv();
	env->errmeta = *meta;
	err_error("SyntaxError");
}
/*============================================================================= 
	end lex errors
=============================================================================*/

/*============================================================================= 
	begin yacc errors
=============================================================================*/

void 
moto_endClassWithoutClass(MetaInfo* meta) {
	MotoEnv *env = moto_getEnv();
	env->errmeta = *meta;
	err_error("EndClassWithoutClass");
} 

void 
moto_endForWithoutFor(MetaInfo* meta) {
	MotoEnv *env = moto_getEnv();
	env->errmeta = *meta;
	err_error("EndForWithoutFor");
} 

void 
moto_endWhileWithoutWhile(MetaInfo* meta) {
	MotoEnv *env = moto_getEnv();
	env->errmeta = *meta;
	err_error("EndWhileWithoutWhile");
} 

void 
moto_elseWithoutIf(MetaInfo* meta,char embedded) {
	MotoEnv *env = moto_getEnv();
	env->errmeta = *meta;
	if (embedded)
		err_error("EmbElseWithoutIf");
	else
		err_error("ElseWithoutIf" );
} 

void 
moto_elseIfWithoutIf(MetaInfo* meta) {
	MotoEnv *env = moto_getEnv();
	env->errmeta = *meta;
	err_error("ElseIfWithoutIf");
} 

void 
moto_endDefWithoutDefine(MetaInfo* meta){
	MotoEnv *env = moto_getEnv();
	env->errmeta = *meta;
	err_error("EndDefWithoutDefine");
}	

void 
moto_endIfWithoutIf(MetaInfo* meta) {
	MotoEnv *env = moto_getEnv();
	env->errmeta = *meta;
	err_error("EndIfWithoutIf");
} 

void 
moto_endSwitchWithoutSwitch(MetaInfo* meta) {
	MotoEnv *env = moto_getEnv();
	env->errmeta = *meta;
	err_error("EndSwitchWithoutSwitch");
} 

void 
moto_endTryWithoutTry(MetaInfo* meta) {
	MotoEnv *env = moto_getEnv();
	env->errmeta = *meta;
	err_error("EndTryWithoutTry");
} 

void 
moto_illegalCase(MetaInfo* meta,char embedded) {
	MotoEnv *env = moto_getEnv();
	env->errmeta = *meta;
	if (embedded)
		err_error("EmbIllegalCase");
	else
		err_error("IllegalCase");
}

void 
moto_illegalBreak(MetaInfo* meta,char embedded) {
	MotoEnv *env = moto_getEnv();
	env->errmeta = *meta;
	if (embedded)
		err_error("EmbIllegalBreak");
	else
		err_error("IllegalBreak");
}

void 
moto_illegalReturn(MetaInfo* meta,char embedded) {
	MotoEnv *env = moto_getEnv();
	env->errmeta = *meta;
	if (embedded)
		err_error("EmbIllegalReturn");
	else
		err_error("IllegalReturn");
}

void 
moto_illegalCatch(MetaInfo* meta,char embedded) {
	MotoEnv *env = moto_getEnv();
	env->errmeta = *meta;
	if (embedded)
		err_error("EmbIllegalCatch");
	else
		err_error("IllegalCatch");
}

void 
moto_illegalFinally(MetaInfo* meta,char embedded) {
	MotoEnv *env = moto_getEnv();
	env->errmeta = *meta;
	if (embedded)
		err_error("EmbIllegalFinally");
	else
		err_error("IllegalFinally");
}

void 
moto_illegalDefine(MetaInfo* meta,char embedded) {
	MotoEnv *env = moto_getEnv();
	env->errmeta = *meta;
	if (embedded)
		err_error("EmbIllegalDefine");
	else
		err_error("IllegalDefine");
}

void 
moto_illegalTry(MetaInfo* meta,char embedded) {
	MotoEnv *env = moto_getEnv();
	env->errmeta = *meta;
	if (embedded)
		err_error("EmbIllegalTry");
	else
		err_error("IllegalTry");
}

void 
moto_illegalUse(MetaInfo* meta,char embedded) {
	MotoEnv *env = moto_getEnv();
	env->errmeta = *meta;
	if (embedded)
		err_error("EmbIllegalUse");
	else
		err_error("IllegalUse");
}

void 
moto_illegalGlobal(MetaInfo* meta) {
	MotoEnv *env = moto_getEnv();
	env->errmeta = *meta;
	err_error("IllegalGlobal");
}

void 
moto_illegalContinue(MetaInfo* meta,char embedded) {
	MotoEnv *env = moto_getEnv();
	env->errmeta = *meta;
	if (embedded)
		err_error("EmbIllegalContinue");
	else
		err_error("IllegalContinue");
}

void 
moto_illegalThis(MetaInfo* meta) {
	MotoEnv *env = moto_getEnv();
	env->errmeta = *meta;
	err_error("IllegalThis");
}

void 
moto_illegalClassDefinition(MetaInfo* meta,char embedded) {
	MotoEnv *env = moto_getEnv();
	env->errmeta = *meta;
	if (embedded)
		err_error("EmbIllegalClassDefinition");
	else
		err_error("IllegalClassDefinition");
}

void 
moto_malformedClassDefinition(MetaInfo* meta,char embedded) {
	MotoEnv *env = moto_getEnv();
	env->errmeta = *meta;
	if(embedded)
		err_error("MalformedEmbClassDefinition");
	else
		err_error("MalformedClassDefinition");
}

void 
moto_malformedDeclare(MetaInfo* meta,char embedded) {
	MotoEnv *env = moto_getEnv();	  
	env->errmeta = *meta;
	if(embedded)
		err_error("MalformedEmbDeclare");
	else
		err_error("MalformedDeclare");
}

void 
moto_malformedDefine(MetaInfo* meta,char embedded) {
	MotoEnv *env = moto_getEnv();
	env->errmeta = *meta;
	if(embedded)
		err_error("MalformedEmbDefine");
	else
		err_error("MalformedDefine");
}

void 
moto_malformedDo(MetaInfo* meta,char embedded) {
	MotoEnv *env = moto_getEnv();	  
	env->errmeta = *meta;
	if(embedded)
		err_error("MalformedEmbDo");
	else
		err_error("MalformedDo");
}

void 
moto_malformedPrint(MetaInfo* meta,char embedded) {
	MotoEnv *env = moto_getEnv();
	env->errmeta = *meta;
	if(embedded)
		err_error("MalformedEmbPrint");
	else
		err_error("MalformedPrint");
}

void 
moto_malformedIf(MetaInfo* meta,char embedded) {
	MotoEnv *env = moto_getEnv();
	env->errmeta = *meta;
	if(embedded)
		err_error("MalformedEmbIf");
	else
		err_error("MalformedIf");
}

void 
moto_malformedReturn(MetaInfo* meta,char embedded) {
	MotoEnv *env = moto_getEnv();
	env->errmeta = *meta;
	if(embedded)
		err_error("MalformedEmbReturn");
	else
		err_error("MalformedReturn");
}

void 
moto_malformedStatement(MetaInfo* meta,char embedded) {
	MotoEnv *env = moto_getEnv();
	env->errmeta = *meta;
	if(embedded)
		err_error("MalformedEmbStatement");
	else
		err_error("MalformedStatement");
}

void 
moto_malformedElseIf(MetaInfo* meta,char embedded) {
	MotoEnv *env = moto_getEnv();
	env->errmeta = *meta;
	if(embedded)
		err_error("MalformedEmbElseIf");
	else
		err_error("MalformedElseIf");
}

void 
moto_malformedSwitch(MetaInfo* meta,char embedded) {
	MotoEnv *env = moto_getEnv();
	env->errmeta = *meta;
	if(embedded)
		err_error("MalformedEmbSwitch");
	else
		err_error("MalformedSwitch");
}

void 
moto_malformedCase(MetaInfo* meta,char embedded) {
	MotoEnv *env = moto_getEnv();
	env->errmeta = *meta;
	if(embedded)
		err_error("MalformedEmbCase");
	else
		err_error("MalformedCase");
}

void 
moto_malformedCatch(MetaInfo* meta,char embedded) {
	MotoEnv *env = moto_getEnv();
	env->errmeta = *meta;
	if(embedded)
		err_error("MalformedEmbCatch");
	else
		err_error("MalformedCatch");
}

void 
moto_malformedThrow(MetaInfo* meta,char embedded) {
	MotoEnv *env = moto_getEnv();
	env->errmeta = *meta;
	if(embedded)
		err_error("MalformedEmbThrow");
	else
		err_error("MalformedThrow");
}

void 
moto_malformedWhile(MetaInfo* meta,char embedded) {
	MotoEnv *env = moto_getEnv();
	env->errmeta = *meta;
	if(embedded)
		err_error("MalformedEmbWhile");
	else
		err_error("MalformedWhile");
}

void 
moto_malformedFor(MetaInfo* meta,char embedded) {
	MotoEnv *env = moto_getEnv();
	env->errmeta = *meta;
	if(embedded)
		err_error("MalformedEmbFor");
	else
		err_error("MalformedFor");
}

void 
moto_malformedUse(MetaInfo* meta,char embedded) {
	MotoEnv *env = moto_getEnv();
	env->errmeta = *meta;
	if(embedded)
		err_error("MalformedEmbUse");
	else
		err_error("MalformedUse");
}

void 
moto_unterminatedClassDefinition(MetaInfo* meta,char embedded) {
	MotoEnv *env = moto_getEnv();
	env->errmeta = *meta;
	if (embedded)
		err_error("EmbUnterminatedClassDefinition");
	else
		err_error("UnterminatedClassDefinition");
}

void 
moto_unterminatedDefinition(MetaInfo* meta,char embedded) {
	MotoEnv *env = moto_getEnv();
	env->errmeta = *meta;
	if (embedded)
		err_error("EmbUnterminatedDefinition");
	else
		err_error("UnterminatedDefinition");
}

void 
moto_unterminatedFor(MetaInfo* meta,char embedded) {
	MotoEnv *env = moto_getEnv();
	env->errmeta = *meta;
	if (embedded)
		err_error("EmbUnterminatedFor");
	else
		err_error("UnterminatedFor");
}

void 
moto_unterminatedIf(MetaInfo* meta,char embedded) {
	MotoEnv *env = moto_getEnv();
	env->errmeta = *meta;
	if (embedded)
		err_error("EmbUnterminatedIf");
	else
		err_error("UnterminatedIf");
}

void 
moto_unterminatedSwitch(MetaInfo* meta,char embedded) {
	MotoEnv *env = moto_getEnv();
	env->errmeta = *meta;
	if (embedded)
		err_error("EmbUnterminatedSwitch");
	else
		err_error("UnterminatedSwitch");
}

void 
moto_unterminatedTry(MetaInfo* meta, char embedded) {
	MotoEnv *env = moto_getEnv();
	env->errmeta = *meta;
	if (embedded)
		err_error("EmbUnterminatedTry");
	else
		err_error("UnterminatedTry");
}

void 
moto_unterminatedWhile(MetaInfo* meta,char embedded) {
	MotoEnv *env = moto_getEnv();
	env->errmeta = *meta;
	if (embedded)
		err_error("EmbUnterminatedWhile");
	else
		err_error("UnterminatedWhile");
}

/*============================================================================= 
	end yacc errors
=============================================================================*/

/*=============================================================================
// end of file: $RCSfile: motoerr.c,v $
==============================================================================*/

