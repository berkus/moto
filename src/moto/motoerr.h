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

   $RCSfile: motoerr.h,v $   
   $Revision: 1.36 $
   $Author: dhakim $
   $Date: 2003/03/12 16:41:14 $
 
==============================================================================*/
#ifndef __MOTOERR_H
#define __MOTOERR_H

#include "exception.h"
#include "motocodes.h"
#include "env.h"
#include "pp.h"
#include "type.h"

excp_extern(MotoException);
excp_extern(MotoCellTypeException);
excp_extern(MotoOpcodeException);

/*----------------------------------------------------------------------------- 
 * initialization
 *---------------------------------------------------------------------------*/
void moto_errInit();

/*----------------------------------------------------------------------------- 
 * preprocessor
 *---------------------------------------------------------------------------*/
void motopp_printStackTrace(MotoPP *ppenv);
void motopp_abort(MotoPP *ppenv);
void motopp_inputFileNotFound(char *name);
void motopp_badIfDirective(MotoPP *ppenv, int lineno);
void motopp_badElseIfDirective(MotoPP *ppenv, int lineno);
void motopp_badDefineDirective(MotoPP *ppenv, int lineno);
void motopp_badUndefDirective(MotoPP *ppenv, int lineno);
void motopp_badIfdefDirective(MotoPP *ppenv, int lineno);
void motopp_badIfndefDirective(MotoPP *ppenv, int lineno);
void motopp_badReaddefsDirective(MotoPP *ppenv, int lineno);
void motopp_badIncludeDirective(MotoPP *ppenv, int lineno);
void motopp_badDefineParamList(MotoPP *ppenv, int lineno);
void motopp_badCallParamList(MotoPP *ppenv, int lineno);
void motopp_unterminatedMacroCall(MotoPP *ppenv, int lineno);
void motopp_includeFileNotFound(MotoPP *ppenv, int lineno, char *filename);
void motopp_readdefsFileNotFound(MotoPP *ppenv, int lineno, char *filename);
void motopp_endifWithoutIf(MotoPP *ppenv, int lineno);
void motopp_elseifWithoutIf(MotoPP *ppenv, int lineno);
void motopp_elseWithoutIf(MotoPP *ppenv, int lineno);
void motopp_badExpression(MotoPP *ppenv, int lineno);
void motopp_multiplyDefinedParamName(MotoPP *ppenv, int lineno, char *name); 
void motopp_badMacroCall(MotoPP *ppenv, int lineno);
void motopp_syntaxError(MotoPP *ppenv, int lineno);

/*----------------------------------------------------------------------------- 
 * moto language 
 *---------------------------------------------------------------------------*/
void moto_abort();
void moto_functionErr(char *errtext);
void moto_printErr(char *fmt, ...);


/* Dynamic loader errors */
void moto_functionAlreadyDefinedInExtension(char* motoname,char*libname);
void moto_functionAlreadyDefined(char *motoname, const MetaInfo* meta);
void moto_methodAlreadyDefinedInExtension(char* motoname,char*libname);
void moto_methodAlreadyDefined(char *motoname, const MetaInfo* meta);
void moto_classAlreadyDefinedInExtension(char* classname, char* libname);
void moto_classAlreadyDefined(char *classname, const MetaInfo* meta);

/* Verifier errors */

void moto_ambiguousFunction(char *name);
void moto_cannotCast(char *t1, char *t2);
void moto_cannotCastToVoid();
void moto_cannotPrint();
void moto_cantInvokeMethodOnType(char *type);
void moto_cantDereferenceType(char *type);
void moto_expressionNotCallable(char *type);
void moto_illegalUnaryOperation(int op, char *t);
void moto_illegalBinaryOperation(int op, char *t1, char *t2);
void moto_illegalTernaryOperands(char *t1, char *t2,char* t3);
void moto_illegalDeleteOperation();
void moto_illegalSwitchArgument(char *type);
void moto_illegalCaseType(char *t1, char *t2);
void moto_illegalCaseArgument(char *text);
void moto_illegalTypeForArrayIndex(char *type);
void moto_illegalTypeForSubscriptOp(char *type);
void moto_illegalSubscriptDimension(int dim,int adim);
void moto_illegalConditional(char *type);
void moto_illegalDecrement();
void moto_illegalIncrement();
void moto_illegalAssignment();
void moto_illegalUnknown();
void moto_multiplyDeclaredVar(char *name);
void moto_multiplyDefinedType( char *name);
void moto_noSuchLibrary(char *name);
void moto_noSuchFunction(char *fullname);
void moto_noSuchMethod(char *classname, char *fullname);
void moto_noSuchMember(char *classname, char *membername);
void moto_noSuchConstructor(char *fullname);
void moto_regexParseError(char* rx,char* reason);
void moto_returnTypeMustBeSpecified();
void moto_returnTypeMustNotBeSpecified();
void moto_tooManyArgumentsToCall(char* type);
void moto_tooFewArgumentsToCall(char* type);
void moto_typeNotCatchable(char* typename);
void moto_typeNotDefined(char *name);
void moto_typeNotThrowable(char* typename);
void moto_typeAlreadyCaught(char* typename);
void moto_varsCannotBeVoid();
void moto_varNotDeclared(char *name);
void moto_varTypeAssignError(char *st1, char *st2);
void moto_varTypeReturnError(char *st1, char *st2);

/* Lex Errors */

void moto_unmatchedParen(MetaInfo* meta);
void moto_unterminatedString(MetaInfo* meta);
void moto_syntaxError(MetaInfo* meta);
void moto_noSuchConstruct(MetaInfo* meta,char *construct);

/* Yacc Errors */

void moto_endClassWithoutClass(MetaInfo* meta);
void moto_endForWithoutFor(MetaInfo* meta);
void moto_endWhileWithoutWhile(MetaInfo* meta);
void moto_elseWithoutIf(MetaInfo* meta,char embedded);
void moto_elseIfWithoutIf(MetaInfo* meta);
void moto_endIfWithoutIf(MetaInfo* meta);
void moto_endDefWithoutDefine(MetaInfo* meta);
void moto_endCaseWithoutCase(MetaInfo* meta);
void moto_endSwitchWithoutSwitch(MetaInfo* meta);
void moto_endTryWithoutTry(MetaInfo* meta);
void moto_unterminatedClassDefinition(MetaInfo* meta,char embedded);
void moto_unterminatedDefinition(MetaInfo* meta,char embedded);
void moto_unterminatedFor(MetaInfo* meta,char embedded);
void moto_unterminatedIf(MetaInfo* meta,char embedded);
void moto_unterminatedSwitch(MetaInfo* meta,char embedded);
void moto_unterminatedTry(MetaInfo* meta, char embedded) ;
void moto_unterminatedWhile(MetaInfo* meta,char embedded);
void moto_illegalBreak(MetaInfo* meta,char embedded);
void moto_illegalCase(MetaInfo* meta,char embedded);
void moto_illegalCatch(MetaInfo* meta,char embedded);
void moto_illegalClassDefinition(MetaInfo* meta,char embedded);
void moto_illegalContinue(MetaInfo* meta,char embedded);
void moto_illegalDefine(MetaInfo* meta,char embedded);
void moto_illegalFinally(MetaInfo* meta,char embedded);
void moto_illegalGlobal(MetaInfo* meta);
void moto_illegalReturn(MetaInfo* meta,char embedded);
void moto_illegalThis(MetaInfo* meta);
void moto_illegalTry(MetaInfo* meta,char embedded);
void moto_illegalUse(MetaInfo* meta,char embedded);
void moto_malformedCase(MetaInfo* meta,char embedded);
void moto_malformedCatch(MetaInfo* meta, char embedded);
void moto_malformedClassDefinition(MetaInfo* meta,char embedded);
void moto_malformedDefine(MetaInfo* meta,char embedded);
void moto_malformedDeclare(MetaInfo* meta,char embedded);
void moto_malformedDo(MetaInfo* meta,char embedded);
void moto_malformedElseIf(MetaInfo* meta,char embedded);
void moto_malformedFor(MetaInfo* meta,char embedded);
void moto_malformedIf(MetaInfo* meta,char embedded);
void moto_malformedPrint(MetaInfo* meta,char embedded);
void moto_malformedReturn(MetaInfo* meta,char embedded);
void moto_malformedSwitch(MetaInfo* meta,char embedded);
void moto_malformedStatement(MetaInfo* meta,char embedded);
void moto_malformedThrow(MetaInfo* meta, char embedded);
void moto_malformedUse(MetaInfo* meta,char embedded);
void moto_malformedWhile(MetaInfo* meta,char embedded);


#endif

/*=============================================================================
// end of file: $RCSfile: motoerr.h,v $
==============================================================================*/

