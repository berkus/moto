/*=============================================================================

	Copyright (C) 2003 David Hakim.
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

	$RCSfile: motox.c,v $	
	$Revision: 1.6 $
	$Author: dhakim $
	$Date: 2003/05/29 02:14:08 $

==============================================================================*/
#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>

#include "memsw.h"

#include "excpfn.h"
#include "motohook.h"
#include "moto.tab.h"
#include "motox.h"
#include "motoi.h"
#include "motoc.h"
#include "motoutil.h"
#include "motoerr.h"

/* Recursively extracts a MotoType from and TYPE, ARRAY_TYPE, or DEF cell */

MotoType* 
motox_extractType(const UnionCell *p){
	MotoEnv *env = moto_getEnv();
	int dim=0;
	char* typen;
	MotoType* type;
	
	if(uc_opcode(p) == TYPE) {
		/* Extract the type name and dimension, then retrieve the MotoType */
		
		typen = uc_str(p, 0);
		dim = uc_opcount(uc_operand(p, 1));
		type = mttab_get(env->types, typen,dim);
		
	} else if(uc_opcode(p) == ARRAY_TYPE) {
		/* Extract the base type and dimension, then retrieve the MotoType */
		
		type = motox_extractType(uc_operand(p, 0));
		dim = uc_opcount(uc_operand(p, 1));
		type = mttab_get(env->types, type->name,dim);
		
	} else /* if (uc_opcode(p) == DEF) */ {
		UnionCell* argListUC = uc_operand(p,1);
		int i,argc = argListUC->opcell.opcount;
		MotoType* argt[20];

		/* Extract the return type */
		type = motox_extractType(uc_operand(p,0));
		
		for(i=0;i<argc;i++)
			argt[i] = motox_extractType(uc_operand(argListUC,i));
		
		type = mttab_getHOFType(env->types,type,argc,argt);
		
	} 
	
	return type;
}

/**
 * Try to locate a moto function or method with the specified base name, arguments,  
 * and cononical argument types. 
 *
 * First, try finding a method using the type of 'self' as the classname if self is != null.
 *
 * If no method is found and the opcode allows for the possiblity that we are interested in 
 * functions as well than try looking for one of those. In the end return 'code' and set f
 * if we find anything.
 **/
 
int
motox_lookupMethodOrFn(
	MotoFunction **f, 
	MotoVal* self,
	char* fbasename, 
	int argc, 
	char** types, 
	int opcode
){
	MotoEnv* env = moto_getEnv();
	int code = FTAB_NONE;
	char fname[256];
	
	if(self != NULL) {
		/* Generate the Moto name for this function */
		moto_createMotonameForFnInBuffer(self->type->name,fbasename,fname);
	
		/* Retrieve the MXFN record */
		code = ftab_cacheGet(env->ftable, f, fname, argc, types);
	}
	
	/* If we got nothing then lets try it as a function call if we can */
	if(code == FTAB_NONE && opcode == FN ) {
		/* Generate the Moto name for this function */
		moto_createMotonameForFnInBuffer(NULL, fbasename,fname);
	
		/* Retrieve the MXFN record */
		code = ftab_cacheGet(env->ftable, f, fname, argc, types);
	}
	return code;
}

/**
 * Returns true if the callee specified is a dynamically defined function (identified 
 * function or partially applied function or method) 
 * 
 * A callee is considered dynamically defined iff:
 * 1) The opcode of the callee is FN but the function name being called is a variable
 * 2) The opcode of the callee is FN but we are inside a class definition and 
 * 	the function name being called is a class member variable 
 * 3) The opcode of the callee is METHOD but the method name being called is a
 * 	class member variable
 * 4) The opcode of the callee is FCALL
 **/
 
int 
motox_calleeIsDDF(const UnionCell* p, MotoVal* self){
	MotoEnv* env = moto_getEnv();
	UnionCell* calleeUC = uc_operand(p, uc_opcode(p) == METHOD?1:0 );
	int calleeIsDDF = 0;
	
	if( uc_opcode(p) == FN) {
		if(moto_getVar(env, calleeUC->valuecell.value)) {
			calleeIsDDF = 1;
		} else if(self != NULL) {
			MotoClassDefinition* mcd = moto_getMotoClassDefinition(env,self->type->name);
			char* typen = NULL;
			
			if(mcd != NULL) typen = mcd_getMemberType(mcd,calleeUC->valuecell.value);
			if(typen != NULL) calleeIsDDF = 1;	
		}
	} else if (uc_opcode(p) == METHOD) {
		MotoClassDefinition* mcd = moto_getMotoClassDefinition(env,self->type->name);
		char* typen = NULL;
		
		if(mcd != NULL) typen = mcd_getMemberType(mcd,calleeUC->valuecell.value);
		if(typen != NULL) calleeIsDDF = 1;
	} else if (uc_opcode(p) == FCALL) {
		calleeIsDDF = 1;
	}
	
	return calleeIsDDF;
}

/* Fill the argv and types arrays with the appropriate values from the args array */
void 
motox_fillArgs(int argc,MotoVal** args, void** argv,char** types){
	int i;
	MotoVal* val;

	for (i = 0; i < argc; i++) {
		val = args[i];
		switch(val->type->kind) {
			case BOOLEAN_TYPE : argv[i] = &val->booleanval.value; break;
			case BYTE_TYPE		: argv[i] = &val->byteval.value; break;
			case CHAR_TYPE		: argv[i] = &val->charval.value; break;
			case INT32_TYPE	: argv[i] = &val->intval.value; break;
			case INT64_TYPE	: argv[i] = &val->longval.value; break;
			case FLOAT_TYPE	: argv[i] = &val->floatval.value; break;
			case DOUBLE_TYPE	: argv[i] = &val->doubleval.value; break;
			case REF_TYPE		: argv[i] = val->refval.value; break;
			case VOID_TYPE		: /* FIXME: This should never happen */ break;
		}

		types[i] = val->type->name;
	}
}

/** 
 * Try to locate a suitable operator method for the given operator.
 * If it finds it:
 * 	- in verifier mode it creates a MotoVal of the respective type
 * 	- in interpreter mode it executes the function and creates the MotoVal
 **/ 

int 
motox_try_overloaded_method_op(int op,MotoVal* v1, MotoVal* v2, MotoVal *v3){

	MotoEnv *env = moto_getEnv();
	int code=FTAB_NONE;
	int argc=0;					
	MotoFunction *f = NULL;
	MotoVal * r = NULL;
	char * fbasename;
	char *types[20];		
	MotoVal* args[20];
	void *argv[20];
	int i;

	/* If this is not an object... */
	if ((v1->type->kind != REF_TYPE) || isArray(v1)) {
		/* Nothing to do */
		return FTAB_NONE;
	}

	/* If the underlying object cannot be identified ... */
	if ((v1->refval.value == NULL) && (env->mode == INTERPRETER_MODE)) {
		/* Clean Up and throw a NullPointerException */
		THROW("NullPointerException","A method may not be called on null");
	}

	/* operator name */
	switch (op) {
		case MATH_ADD:
			fbasename = "_pluseq"; break;	
		case MATH_SUB:
			fbasename = "_minuseq"; break;	
		case MATH_MULT:
			fbasename = "_timeseq"; break;		
		case INC:
		case POSTFIX_INC:
		case PREFIX_INC:
			fbasename = "_inc"; break;		
		case DEC:
		case POSTFIX_DEC:
		case PREFIX_DEC:
			fbasename = "_dec"; break;		
		case OPENBRACKET:
			fbasename = "_arrindex"; break;		
		default:
			fbasename = NULL;
	}

	/* types for arguments */	
	args[0] = v2;
	types[0] = v2 ? v2->type->name : NULL;
	args[1] = v3;
	types[1] = v3 ? v3->type->name : NULL;

	for(i = 0; i < 2; i++) {
		argc += args[i] ? 1 : 0;
	}
		
	/* fill in the types array for function or method identification */
	motox_fillArgs(argc,args,argv,types); 	
	
	if (env->mode == COMPILER_MODE) {
		for (i = 0; i < argc; i++) {
			argv[i] = moto_strdup(env, args[i]->codeval.value);
		}
	}
	
	/* look up the function or method for the operator */
	code = motox_lookupMethodOrFn(&f, v1, fbasename, argc, types, METHOD);
	
	/* if we found the operator we execute op function  */
	if (code == FTAB_OK) {
			
		switch(env->mode) {

			case DEPENDENCY_MODE:
			case VERIFIER_MODE:
				/* call function or method */
				r = moto_createVal(env,f->rtype,0);
				break;
			case INTERPRETER_MODE:
				/* Call function or method */
				if(f->fptr != NULL) {
					r = motoi_callEDF(v1,args,f);
				} else {
					r = motoi_callMDF(v1,args,f);
				}		
				break;
			case COMPILER_MODE:
				r = motoc_callADF(v1,argv,f, 0);
				break;
		}

		if (v2) moto_freeVal(env,v2);
		opstack_push(env,r);

	}

	return code;
}

/** 
 * Try to locate a suitable operator function for the given operator.
 * If it finds it:
 * 	- in verifier mode it creates a MotoVal of the respective type
 * 	- in interpreter mode it executes the function and creates the MotoVal
 **/
int 
motox_try_overloaded_function_op(int op,MotoVal* v1, MotoVal* v2){

	MotoEnv *env = moto_getEnv();
	int code=FTAB_NONE;
	int argc=0;					
	MotoFunction *f = NULL;
	MotoVal * r = NULL;
	char * fbasename;
	char *types[20];		
	MotoVal* args[20];
	void *argv[20];
	
	/* operator name */
	switch (op) {
		case REL_GT_S:
			fbasename = "_gt"; break;
		case REL_GTE_S:
			fbasename = "_gte"; break;			
		case REL_LT_S:
			fbasename = "_lt"; break;			
		case REL_LTE_S:
			fbasename = "_lte"; break;
		case REL_EQ_S:
			fbasename = "_equal"; break;			
		case REL_NE_S:
			fbasename = "_notequal"; break;			
		case MATH_ADD:
			fbasename = "_plus"; break;			
		case MATH_SUB:
			fbasename = "_minus"; break;			
		case MATH_MULT:
			fbasename = "_times"; break;			
		default:
			return FTAB_NONE; break;			
	}

	/* types for arguments */	
	argc = 2;
	types[0] = v1->type->name;
	args[0] = v1;
	types[1] = v2->type->name;
	args[1] = v2;
	
	/* fill in the types array for function or method identification */
	motox_fillArgs(argc,args,argv,types);

	/* look up the function or method for the operator */
	code = motox_lookupMethodOrFn(&f, NULL, fbasename, argc, types, FN);
	
	/* if we found the operator we execute op function  */
	if (code == FTAB_OK) {

		switch(env->mode) {

			case DEPENDENCY_MODE:
			case VERIFIER_MODE:
				/* call function or method */
				r = moto_createVal(env,f->rtype,0);
				break;
			case INTERPRETER_MODE:
				/* Call function or method */
				if(f->fptr != NULL) {
					r = motoi_callEDF(NULL,args,f);
				} else {
					r = motoi_callMDF(NULL,args,f);
				}		
				break;
			case COMPILER_MODE:
				r = motoc_callADF(NULL,argv,f, 0);
				break;
		}

		opstack_push(env,r);

	}
	
	return code;
}

/*=============================================================================
// end of file: $RCSfile: motox.c,v $
==============================================================================*/
