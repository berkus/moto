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

	$RCSfile: motov.c,v $	
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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "memsw.h"
#include "excpfn.h"
#include "magic.h"
#include "stringbuffer.h"
#include "symboltable.h"
#include "stringset.h"
#include "stack.h"
#include "vector.h"
#include "log.h"
#include "jumptable.h"
#include "symboltable.h"
#include "nfa.h"
#include "mxstring.h"

#include "motolex.h"
#include "moto.tab.h"
#include "motov.h"
#include "motox.h"
#include "motoutil.h"
#include "motoerr.h"
#include "dl.h"
#include "env.h"
#include "motohook.h"

#define T ("true")
#define F ("false")

extern int yyerror(char *s);
extern int errno;												
extern int yylineno;												

static void motov_popFillAndFreeArgs(int argc,char** types);

static void motov_and(const UnionCell *p); 
static void motov_array_lval(const UnionCell *p);
static void motov_array_rval(const UnionCell *p);
static void motov_array_new(const UnionCell *p);
static void motov_assign(const UnionCell *p);
static void motov_bitop(const UnionCell *p);
static void motov_bitnot(const UnionCell *p);
static void motov_cast(const UnionCell *p);
static void motov_classdef(const UnionCell *p);
static void motov_comma(const UnionCell *p); 
static void motov_do(const UnionCell *p);
static void motov_declare(const UnionCell *p);
static void motov_define(const UnionCell *p);
static void motov_dereference_lval(const UnionCell *p);
static void motov_dereference_rval(const UnionCell *p);
static void motov_delete(const UnionCell *p);
static void motov_elseif(const UnionCell *p);
static void motov_elseiflist(const UnionCell *p);
static void motov_excp_try(const UnionCell *p);
static void motov_excp_throw(const UnionCell *p);
static void motov_identify(const UnionCell *p);
static void motov_fn(const UnionCell *p);
static void motov_for(const UnionCell *p);
static void motov_id(const UnionCell *p);
static void motov_if(const UnionCell *p);
static void motov_incdec(const UnionCell *p);
static void motov_jump(const UnionCell *p);
static void motov_list(const UnionCell *p);
static void motov_math(const UnionCell *p);
static void motov_new(const UnionCell *p);
static void motov_not(const UnionCell *p);
static void motov_or(const UnionCell *p);
static void motov_print(const UnionCell *p);
static void motov_regex(const UnionCell *p);
static void motov_rel(const UnionCell *p);
static void motov_return(const UnionCell *p);
static void motov_scope(const UnionCell *p);
static void motov_shiftop(const UnionCell *p);
static void motov_switch(const UnionCell *p);
static void motov_ternary(const UnionCell *p);
static void motov_uminus(const UnionCell *p);
static void motov_use(const UnionCell *p);
static void motov_unknown(const UnionCell *p);
static void motov_value(const UnionCell *p);
static void motov_while(const UnionCell *p);

static void motov_domath(MotoVal *v1, MotoVal *v2, int op) ;
static void motov_dorel(MotoVal *v1, MotoVal *v2, int op) ;

/*-----------------------------------------------------------------------------
 * interpreter
 *---------------------------------------------------------------------------*/

void motov(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MetaInfo curmeta = env->errmeta;
	env->errmeta = *uc_meta(p);

	switch(p->type) {
		case VALUE_TYPE:

			switch(p->valuecell.kind) {
				case NULL_VALUE:
				case BOOLEAN_VALUE:
				case CHAR_VALUE:
				case INT32_VALUE:
				case INT64_VALUE:
				case FLOAT_VALUE:
				case DOUBLE_VALUE:
				case STRING_VALUE:
				case REGEX_VALUE:
					motov_value(p);
					break;
				case ID_VALUE:
					motov_id(p);
					break;
				default:
					THROW_D("MotoCellTypeException");
					break;
			}
			break;
		case OP_TYPE:

			switch(p->opcell.opcode) {
				case AND: motov_and(p); break;
				case ARRAY_NEW: motov_array_new(p); break;
				case ARRAY_LVAL: motov_array_lval(p); break;
				case ARRAY_RVAL: motov_array_rval(p); break;
				case ASSIGN: motov_assign(p); break;
				case BITWISE_AND:
				case BITWISE_OR:
				case BITWISE_XOR:
					motov_bitop(p); break;
				case SHIFT_LEFT:
				case SHIFT_RIGHT:
					motov_shiftop(p); break;
				case BITWISE_NOT:
					motov_bitnot(p); break;
				case BREAK: motov_jump(p); break;
				case CAST: motov_cast(p); break;
				case CLASSDEF: motov_classdef(p); break;
				case COMMA: motov_comma(p); break;
				case CONTINUE: motov_jump(p); break;
				case DEFINE: motov_define(p); break;
				case DECLARE: 
				case DECLARE_INLINE: motov_declare(p); break;
				case DELETE: motov_delete(p); break;
				case DEREFERENCE_RVAL: motov_dereference_rval(p); break;
				case DEREFERENCE_LVAL: motov_dereference_lval(p); break;
				case DO: motov_do(p); break;
				case ELSEIF: motov_elseif(p); break;
				case ELSEIFLIST: motov_elseiflist(p); break;
				case EXCP_TRY: motov_excp_try(p); break;
				case EXCP_THROW: motov_excp_throw(p); break;
				case FCALL:
				case FN: 
				case METHOD: motov_fn(p); break;
				case IDENTIFY: motov_identify(p); break;
				case FOR: motov_for(p); break;
				case IF: motov_if(p); break;
				case LIST: motov_list(p); break;
				case MATH_ADD:
				case MATH_DIV:
				case MATH_MOD:
				case MATH_MULT:
				case MATH_SUB: motov_math(p); break;
				case NEW: motov_new(p); break;
				case NOOP:
					break;
				case NOT: motov_not(p); break;
				case OR: motov_or(p); break;
				case POSTFIX_DEC:
				case POSTFIX_INC:
				case PREFIX_DEC:
				case PREFIX_INC: motov_incdec(p); break;
				case PRINT: motov_print(p); break;
				case REL_EQ_M:
				case REL_EQ_S:
				case REL_GT_M:
				case REL_GT_S:
				case REL_GTE_M:
				case REL_GTE_S:
				case REL_LT_M:
				case REL_LT_S:
				case REL_LTE_M:
				case REL_LTE_S:
				case REL_NE_M:
				case REL_NE_S: motov_rel(p); break;
				case RETURN: motov_return(p); break;
				case RX_MATCH:
				case RX_NOT_MATCH: motov_regex(p); break;
				case SCOPE: motov_scope(p); break;
				case SWITCH: motov_switch(p); break;
				case TERNARY: motov_ternary(p); break;
				case TYPED_UNKNOWN:
				case UNTYPED_UNKNOWN:
					motov_unknown(p); break;
				case UMINUS: motov_uminus(p); break;
				case USE: motov_use(p); break;
				case WHILE: motov_while(p); break;				 
				default:
					THROW_D("MotoOpcodeException");					 
					break;
			}
	}

	/* FIXME: this should really occur in a finally block */
	env->errmeta = curmeta; /* Restore the saved meta information so errors
										don't get reported on later lines than they should */
}


/*-----------------------------------------------------------------------------
 * ops
 *---------------------------------------------------------------------------*/

void motov_and(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVal *v1, *v2, *r;

	motov(uc_operand(p, 0));
	motov(uc_operand(p, 1));
	v2 = opstack_pop(env);
	v1 = opstack_pop(env);
	if (v1->type->kind != BOOLEAN_TYPE || v2->type->kind != BOOLEAN_TYPE) 
		moto_illegalBinaryOperation(uc_opcode(p), v1->type->name, v2->type->name);
	moto_freeVal(env,v2);
	moto_freeVal(env,v1);

	r = moto_createVal(env, "boolean",0);

	opstack_push(env, r);
}

static void
motov_arrayIndexCheck(MotoVal * val) {

	MotoEnv *env = moto_getEnv();
	MotoVal *r;

	/* Verify the type of the expression is an int or a long */
	if (val->type->kind != INT32_TYPE && val->type->kind != INT64_TYPE){
		moto_illegalTypeForArrayIndex(val->type->name);
	}

	r = moto_createVal(env, "int",0);
	opstack_push(env, r);

}

static void 
motov_array_rval(const UnionCell *p) {
	motov_array_lval(p);
}

static void 
motov_array_lval(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVal *v1,*v2,*rval,*r;
	MotoFrame *frame = NULL;
	int stacksize; /* the position of the subscript op in the opstack */

	/* Evaluate the first operand */
	motov(uc_operand(p, 0));
	
	/* Verify that it was declared as an Array */
	v1 = opstack_pop(env);			

	/* Evaluate the ARRAY_INDEX element */
	motov(uc_operand(p, 1)); 		
	v2 = opstack_pop(env);	

	if (env->rval_uc != NULL) {

		UnionCell * rval_uc = env->rval_uc;
		env->rval_uc = NULL;
	
		motov(rval_uc);
		env->rval_uc = rval_uc;

		rval = opstack_pop(env);
		opstack_push(env, rval);
	
		if(motox_try_overloaded_method_op(
			OPENBRACKET,v1,v2,rval) == MOTO_OK) {
			env->rval_uc = NULL;
			return;
		}
		moto_freeVal(env, opstack_pop(env));

	}

	/* Test for [] rval overloaded operator */
	if(motox_try_overloaded_method_op(
		OPENBRACKET,v1,v2,NULL) == MOTO_OK) {
		return;
	}

	/* Record the current position on the stack */
	frame = env->frame;
	stacksize = stack_size(frame->opstack);		

	/* Verify v1 is an array ... */
	if (!isArray(v1)){
		moto_illegalTypeForSubscriptOp( v1->type->name);
	
		/* Recover */
		moto_freeVal(env,v1);
		v1 = moto_createVal(env, "int" ,1);
	}
	
	/* Verify the subscript of the array */
	motov_arrayIndexCheck(v2);
	
	/* Switch on the relationship between the # of subscript operations
		and the arrays declared dimensions */ 

	if(stack_size(frame->opstack) - stacksize > v1->type->dim) {

		/* The dimensions subscripted exceeds the dimensions of the array */
		moto_illegalSubscriptDimension(stack_size(frame->opstack) - stacksize,v1->type->dim); 

		/* Recover */
		r = moto_createVal(env, v1->type->atype->name,0);

	} else if (stack_size(frame->opstack) - stacksize == v1->type->dim) {

		/* The dimensions subscripted equals the dimensions of the array */
		r = moto_createVal(env, v1->type->atype->name,0);		 
	
	} else /* stack_size(frame->opstack) - stacksize < v1->type->dim */ {

		/* The dimensions subscripted are less than the dimensions of the array */
		r = moto_createVal(env,v1->type->atype->name, 
			 v1->type->dim - (stack_size(frame->opstack) - stacksize));

	}

	/* Pop off the element pushed onto the stack */
	moto_freeVal(env,opstack_pop(env));
	
	opstack_push(env, r);
	
	moto_freeVal(env,v1);
	moto_freeVal(env,v2);

}

MotoType *
motov_typeForName(char *typen) {
	MotoEnv *env = moto_getEnv();
	MotoType *type = NULL;
	int i;
	
	// printf("### Looking Up '%s'\n",typen);
	
	type = (MotoType *)stab_get(env->types, typen);
	if(type == NULL) {
		
		if(typen[strlen(typen)-1] == ')') {
		
			/* This is an HOF Type */
				
		    char* atypen[20];
		    MotoType* argt[20];
		    
		    int argc = 0;
		    char* rtypen;
		    MotoType* rtype;
			int depth = 0;
			int eindex = strlen(typen)-1;
			int sindex = eindex;
			
			for(;sindex > 0;sindex --){
				if(typen[sindex] == '(' && depth == 1){
					if(eindex > sindex+1)
						atypen[argc++] = mman_track(env->mpool,mxstr_substring(typen,sindex+1,eindex));
					break;
				} else if(typen[sindex] == '(' && depth > 1) {
					depth --;
				} else if(typen[sindex] == ')' ) {
					depth ++;
				} else if(typen[sindex] == ',' && depth == 1) {
					atypen[argc++] = mman_track(env->mpool,mxstr_substring(typen,sindex+1,eindex));
					eindex = sindex;
				}	
			} 
			
			/* Get the actual types for the arguments and reverse the array at the same time */
			for(i=0;i<argc;i++) {
				argt[argc - i - 1] = mttab_typeForName(env->types,atypen[i]);
				if(argt[argc - i - 1] == NULL) return NULL;
			}
			
			/* Get the actual return type */ 
			rtypen = mman_track(env->mpool,mxstr_substring(typen,0,sindex));
			rtype = mttab_typeForName(env->types,rtypen);
			if(rtype == NULL) return NULL;
			
			type = mttab_getHOFType(env->types,rtype,argc,argt);
			
		} else if(typen[strlen(typen)-1] == ']') {
		
			/* This is an Array Type */
		
			char* atypen;
			MotoType* atype;
			
			int dim = 0;
			while(typen[strlen(typen) - 2*dim -1] == ']' && typen[strlen(typen) - 2*dim - 2] == '[')
				dim ++;
			
			atypen = mman_track(env->mpool,mxstr_substring(typen,0,strlen(typen) - 2*dim));
			atype = mttab_typeForName(env->types,atypen);
			
			if(atype == NULL) return NULL;
			
			type = mttab_get(env->types,atypen,dim);
		} else {
			moto_typeNotDefined(typen);
			type = mttab_get(env->types, "Object",0); /* recover */
		}
	}	
	
	return type;
}

MotoType* motov_extractType(const UnionCell *p){
	MotoEnv *env = moto_getEnv();
	int dim=0;
	char* typen;
	MotoType* type;
	
	if(uc_opcode(p) == TYPE) {
		/* Extract the type name and dimension, then retrieve the MotoType */
		
		typen = uc_str(p, 0);
		type = motov_typeForName(typen);
		
		/* Check the variable type */
		
		if (type == NULL) {
			moto_typeNotDefined(typen);
			type = mttab_get(env->types, "Object",0); /* recover */
		} 
	} else if(uc_opcode(p) == ARRAY_TYPE) {
		/* Extract the base type and dimension, then retrieve the MotoType */
		
		type = motov_extractType(uc_operand(p, 0));
		dim = uc_opcount(uc_operand(p, 1));
		type = mttab_get(env->types, type->name,dim);
		
	} else /* if (uc_opcode(p) == DEF) */ {
		UnionCell* argListUC = uc_operand(p,1);
		int i,argc = argListUC->opcell.opcount;
		MotoType* argt[20];
		
		/* Extract the return type */
		type = motov_extractType(uc_operand(p,0));
		
		for(i=0;i<argc;i++)
			argt[i] = motov_extractType(uc_operand(argListUC,i));
		
		type = mttab_getHOFType(env->types,type,argc,argt);
		
	} 
	
	return type;
}

static void 
motov_array_new(const UnionCell *p){
	MotoEnv *env = moto_getEnv();
	MotoType *type;
	MotoVal *r,*v;
	MotoFrame* frame;
	int stacksize,dim=0;

	/* Grab the array subtype from the first operand */
	type = motov_extractType(uc_operand(p, 0));

	/* Record the current position on the stack */

	frame = env->frame;
	stacksize = stack_size(frame->opstack);

	/* Evaluate the ARRAY_INDEX element */
	motov(uc_operand(p, 1));
	v = opstack_pop(env);	

	/* Verify the subscript of the array */
	motov_arrayIndexCheck(v);

	moto_freeVal(env, v);

	dim = stack_size(frame->opstack) - stacksize;

	/* Pop off and free the elements pushed onto the stack */
	while(stack_size(frame->opstack) > stacksize)
		moto_freeVal(env,opstack_pop(env));

	/* Add the number of unspecified dimensions */
	dim += uc_opcount(uc_operand(p, 2));

	/* Construct a new Array type MotoVal */
	r = moto_createVal(env, type->name,dim);

	opstack_push(env, r);
}

static void
motov_arrayInit(const UnionCell *p,MotoType* type){
	MotoEnv *env = moto_getEnv();
	MotoVal *r,*eval,*val;
	int i;
	UnionCell* arrayElementsListUC;
	UnionCell* arrayElementUC;
	
	/* Construct a new array element type MotoVal for type checking */
	eval = moto_createVal(env, type->atype->name,type->dim-1);
		
	/* Verify the array elements */
	arrayElementsListUC = uc_operand(p, 0);
	for(i=0;i<arrayElementsListUC->opcell.opcount;i++){
		arrayElementUC = uc_operand(arrayElementsListUC,i);
		
		if(arrayElementUC->type == OP_TYPE && uc_opcode(arrayElementUC) == ARRAY_INIT) {
			if(type->dim==1) {
				/* The dimensions subscripted exceeds the dimensions of the array */
				moto_illegalSubscriptDimension(1,0); 
				/* Recover */
				opstack_push(env,moto_cloneVal(env,eval));
			} else
				motov_arrayInit(arrayElementUC,eval->type);
		} else 
			motov(arrayElementUC);
		
		val = opstack_pop(env);
		
		/* Check the cast between eval and val */
		if (!mtype_checkCast(val->type,eval->type)) 
			moto_cannotCast(val->type->name,eval->type->name);
			
		moto_freeVal(env,val);
	}

	moto_freeVal(env,eval);
		
	/* Construct a new Array type MotoVal for return */
	r = moto_createVal(env, type->name,0);
	
	opstack_push(env, r);
}

void motov_assign(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVal *lval,*rval;
	UnionCell* lvop = uc_operand(p,0);

	int op = uc_operand(p, 1)->opcell.opcode;

	/* Save rval UnionCell */
	env->rval_uc = uc_operand(p, 2);

	/* Get the LValue from the left side */
	motov(uc_operand(p, 0));
	lval = opstack_pop(env);

	/* Get the RValue from the right side */
	if (env->rval_uc != NULL) {
		motov(uc_operand(p, 2));
		env->rval_uc = NULL;
	}
	rval = opstack_pop(env);

	/* If this is a math assignment do a math operation and 
	re-retrieve the RValue */
	switch (op) {
		case ASSIGN		:	break;	/* normal assignment */
		case MATH_ADD	:				/* add-assignment */
		case MATH_SUB	:				/* sub-assignment */
		case MATH_MULT :				/* mult-assignment */
		case MATH_DIV	:				/* div-assignment */
		case MATH_MOD	:				/* mod-assignment */
			if(motox_try_overloaded_method_op(op,lval,rval,NULL) != MOTO_OK) {
				motov_domath(lval, rval, op);
				moto_freeVal(env,rval);
			}
			rval = opstack_pop(env);
			break;
		default:
			moto_illegalAssignment();
			break;
	}

	if(
		(lvop->type == VALUE_TYPE && lvop->valuecell.kind == ID_VALUE ) ||
		(lvop->type == OP_TYPE && (lvop->opcell.opcode == ARRAY_LVAL || lvop->opcell.opcode == DEREFERENCE_LVAL))
	) {
 
		/* Need to check the array types here */
		if (mtype_checkAssign(lval->type, rval->type) != 1)
			moto_varTypeAssignError(lval->type->name, rval->type->name);

	} else {
		// throw not an lval err
		moto_illegalAssignment();
	}
	
	moto_freeVal(env,rval);

	opstack_push(env,lval);
}

static void motov_bitop(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVal *r = NULL, *v1,*v2;
	int op;
	
	motov(uc_operand(p, 0));
	motov(uc_operand(p, 1));
	v2 = opstack_pop(env);
	v1 = opstack_pop(env);
	op = uc_opcode(p);

	if (v1->type->kind != INT32_TYPE && v1->type->kind != INT64_TYPE &&
		 v1->type->kind != CHAR_TYPE && v1->type->kind != BYTE_TYPE ) {

		 moto_illegalBinaryOperation(op,v1->type->name,v2->type->name);
		 r = moto_createVal(env, "int",0); /* recover */

	} else if (v2->type->kind != INT32_TYPE && v2->type->kind != INT64_TYPE &&
		 v2->type->kind != CHAR_TYPE && v2->type->kind != BYTE_TYPE ) {

		 moto_illegalBinaryOperation(op,v1->type->name,v2->type->name);
		 r = moto_createVal(env, "int",0); /* recover */

	} else if (v1->type->kind == INT64_TYPE || v2->type->kind == INT64_TYPE) 
		r = moto_createVal(env, "long",0); 
	else if (v1->type->kind == INT32_TYPE || v2->type->kind == INT32_TYPE)
		r = moto_createVal(env, "int",0);
	else if (v1->type->kind ==	 CHAR_TYPE || v2->type->kind == CHAR_TYPE)
		r = moto_createVal(env, "char",0);
	else if (v1->type->kind ==	 BYTE_TYPE || v2->type->kind == BYTE_TYPE)
		r = moto_createVal(env, "byte",0);
	else { THROW_D("MotoException"); }
	
	/* clean up and push result onto opstack */
	moto_freeVal(env,v1);
	moto_freeVal(env,v2);
	opstack_push(env, r);
}

static void motov_shiftop(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVal *r, *v1,*v2;
	int op;
	
	motov(uc_operand(p, 0));
	motov(uc_operand(p, 1));
	v2 = opstack_pop(env);
	v1 = opstack_pop(env);
	op = uc_opcode(p);

	if (v1->type->kind != INT32_TYPE && v1->type->kind != INT64_TYPE &&
		v1->type->kind != CHAR_TYPE && v1->type->kind != BYTE_TYPE ) {

		moto_illegalBinaryOperation(op,v1->type->name,v2->type->name);
		r = moto_createVal(env, "int",0); /* recover */

	} else if (v2->type->kind != INT32_TYPE && v2->type->kind != INT64_TYPE &&
		v2->type->kind != CHAR_TYPE && v2->type->kind != BYTE_TYPE ) {

		moto_illegalBinaryOperation(op,v1->type->name,v2->type->name);
		r = moto_createVal(env, v1->type->name ,0); /* recover */

	} else
		r = moto_createVal(env, v1->type->name ,0);

	/* clean up and push result onto opstack */
	moto_freeVal(env,v1);
	moto_freeVal(env,v2);
	opstack_push(env, r);
}
void motov_bitnot(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVal *val,*r;

	motov(uc_operand(p, 0));
	val = opstack_pop(env);

	if(val->type->kind != INT32_TYPE && val->type->kind != INT64_TYPE 
		&& val->type->kind != CHAR_TYPE && val->type->kind != BYTE_TYPE)
		moto_illegalUnaryOperation(uc_opcode(p) , val->type->name);

	moto_freeVal(env,val);

	r = moto_createVal(env, "int",0);
	opstack_push(env, r);
}


void motov_cast(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVal *val,*dup;
	MotoType *type;
	
	/* Extract and verify the cast MotoType*/
	type = motov_extractType(uc_operand(p, 0));
	
	/* Verify the type is defined and is not the void type */
	if (type->kind == VOID_TYPE) {
		moto_cannotCastToVoid();
		type = mttab_get(env->types, "Object",0); /* recover */
	} 
	
	/* Create a motoval of the specified time and dimension */
	dup = moto_createVal(env, type->name,0);
	
	/* See if the cast expression is an inline array initializer */
	if (uc_operand(p,1)->type == OP_TYPE && uc_opcode(uc_operand(p,1)) == ARRAY_INIT) {
		/* If it is, verify that the inlined array elements are of the appropriate
		type and dimensions */
		
		if(type->dim == 0){
			/* The dimensions subscripted exceeds the dimensions of the array */
			moto_illegalSubscriptDimension(1,0); 
		} else {
			motov_arrayInit(uc_operand(p,1),type);
			moto_freeVal(env,opstack_pop(env));
		}
	} else {		
		/* Evaluate the cast expression */
		motov(uc_operand(p, 1));
		val = opstack_pop(env);
	
		/* Try the cast */
		
		if (!mtype_checkCast(dup->type,val->type)) 
			moto_cannotCast(val->type->name,dup->type->name);

		moto_freeVal(env,val);
	}
	
	opstack_push(env,dup);
}

void motov_classdef(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	char* classn;
	
	/* Push on a new private frame */
	moto_createFrame(env, FN_FRAME); // FIXME: this is a hack in that function frames are the only way to do private frames right now
		
	/* Set the environment's current MotoClassDefinition to the current class definition */
	classn = moto_strdup(env, uc_str(p, 0));
	env->ccdef = moto_getMotoClassDefinition(env,classn);
	
	/* Call motov on the statement list */
	motov(uc_operand(p,1));
	
	/* Pop off the private frame */
	moto_freeFrame(env);
	
	/* Un-set the environmentÕs current MotoClassDefinition */
	env->ccdef = NULL;
}

void motov_comma(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVal *v1, *v2, *r;

	motov(uc_operand(p, 0));
	motov(uc_operand(p, 1));

	v2 = opstack_pop(env);
	v1 = opstack_pop(env);

	r = moto_createVal(env, v2->type->name, 0);
	
	moto_freeVal(env,v2);
	moto_freeVal(env,v1);

	opstack_push(env, r);
}

void motov_declare(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	int i;
	/* the type and name of the id being declared */
	char *varn;
	/* whether it is a global variable being declared */
	char global = '\0';
	MotoType *type;
	
	UnionCell *type_uc,*type_qualifier_uc,*declarator_list_uc;

	/* Extract the type_qualifier, type, and variable declarator list */

	type_qualifier_uc = uc_operand(p,0);
	type_uc = uc_operand(p,1);
	declarator_list_uc = uc_operand(p,2);	

	/* Discover if the type qualifer is "global" */
	global = uc_opcode(type_qualifier_uc) == GLOBAL;
	
	/* Extract the MotoType */
	type = motov_extractType(type_uc);
	
	/* Make sure the type isn't 'void' */
	if(type->kind == VOID_TYPE) {
		moto_varsCannotBeVoid();
		type = mttab_get(env->types, "Object",0); /* recover */
	}

	/* Iterate over the declarators */
	
	for(i=0;i<declarator_list_uc->opcell.opcount;i++) {
		MotoVar *var = NULL;
		MotoType *vart = NULL;
		UnionCell* declarator_uc = uc_operand(declarator_list_uc,i);
		int vdim = 0;
		
		/* Get the variable name, dimension and type */
		
		varn = moto_strdup(env, uc_str(declarator_uc, 0));
		vdim = uc_opcount(uc_operand(declarator_uc, 1));
		vart = mttab_get(env->types, type->name,vdim);
		
		/* Check to see if its already been declared */
	
		if(global)
			var = moto_getGlobalVar(env, varn);
		else
			var = moto_getFrameVar(env, varn);
	
		if (var == NULL) {
			/* OK: variable not declared in this frame -- so declare it */
			var = moto_declareVar(env, varn,vart->name,0, global);
		} else {
			moto_multiplyDeclaredVar(varn);
		}
	
		/* See if the declaration is "fancy" (i.e. it includes an assignment) */
		if (uc_opcode(uc_operand(declarator_uc, 2)) != NOOP) {
			
			/* See if the assignment is an inline array initializer */
			if (uc_operand(declarator_uc, 2)->type == OP_TYPE && uc_opcode(uc_operand(declarator_uc, 2)) == ARRAY_INIT) {
				/* If it is, verify that the inlined array elements are of the appropriate
				type and dimensions */
				
				if(vart->dim == 0){
					/* The dimensions subscripted exceeds the dimensions of the array */
					moto_illegalSubscriptDimension(1,0); 
				} else {
					motov_arrayInit(uc_operand(declarator_uc, 2),vart );
					moto_freeVal(env,opstack_pop(env));
				}
				
			} else {
				/* If it is just a run of the mill assignment than verify it as such */
				
				MotoVal *val;
		 
				motov(uc_operand(declarator_uc, 2));
				val = opstack_pop(env);
				
				if (mtype_checkAssign( var->type, val->type)!= 1) 
					moto_varTypeAssignError(var->type->name,val->type->name);
	
				moto_freeVal(env,val);
			}
		}
		
		/* For inline declarations push an appropriately typed MotoVal onto the stack 
		   Note that we want to do this regardless of whether an assignment took place
		   and that the type and dimension should be that of the newly declared MotoVar */
		   
		if (p->opcell.opcode == DECLARE_INLINE) 
			opstack_push(env,moto_getVarVal(env, var));
	}
}


void motov_define(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	char *aname,*fn;
	MotoType *type,*atype;
	UnionCell *declaration_uc,*argListUC,*type_uc;
	int argc,i,dim = 0;
 
	/* Extract the declaration */
	
	declaration_uc = uc_operand(p,0);
	
	if(declaration_uc->type == OP_TYPE) {
	
		/* Extract the type operand and the function name */
		
		type_uc = uc_operand(declaration_uc, 1);
		fn = uc_str(declaration_uc, 2);

		/* Extract, verify and retrieve the mototype */
	
		type = motov_extractType(type_uc);
		
		if(env->ccdef != NULL && !estrcmp(fn,env->ccdef->classn)) {
			/* Constructor */
			moto_returnTypeMustNotBeSpecified();
		}
	
	} else {
		fn = uc_str(p,0);
		
		type = mttab_get(env->types, uc_str(p,0),0);
		
		if(!(env->ccdef != NULL && !estrcmp(fn,env->ccdef->classn))) {
			/* Not Constructor */
			moto_returnTypeMustBeSpecified();
			type = mttab_get(env->types,"int",0);
		}
	}
 
	/* Extract the arguments */
	argListUC = uc_operand(p,1);
	argc = uc_opcount(argListUC);	 

	/* Create a 'definition' frame */ 
	moto_createFrame(env, FN_FRAME);
	
	/* Push arguments onto the frame by declaring them */
	for(i=0;i<argc;i++){
		UnionCell* argdec = uc_operand(argListUC,i);
		UnionCell* atype_uc = uc_operand(argdec,0);
		
		int argdecDim ;
		MotoVar* avar;
		
		atype = motov_extractType(atype_uc);
		aname = uc_str(argdec,1);
		argdecDim = uc_opcount(uc_operand(argdec, 2));
			  
		avar = moto_declareVar(env, aname, atype->name, argdecDim,'\0');
	}

	/* Declare the special rvalue argument */
	moto_declareVar(env, "$rvalue",type->name, dim, '\0');
	
	/* If this is a method definition then we need to push on all the MotoClassDefinition vars 
		as well as the special 'this' var */
	
	if(env->ccdef != NULL){
		MotoClassDefinition* mcd = env->ccdef;
		
		Enumeration *e = stab_getKeys(mcd->memberTypes);
		while(enum_hasNext(e)){
			char* mname,*mtypen;
			
			mname= (char*)enum_next(e);
			
			/* Fake variable shadowing by simply not loading in a member var
				with the same name as an argument */
				
			if(moto_getFrameVar(env,mname) != NULL)
				continue; 
				
			mtypen = mcd_getMemberType(mcd,mname);
			
			moto_declareVar(env, mname, mtypen, 0, '\0');
		}
		enum_free(e);
		
		/* Declare the special this variable */
		
		moto_declareVar(env , "this", mcd->classn, 0,'\0');
		
	}

	/* Create a scoping frame */
	moto_createFrame(env, SUB_FRAME);

	/* Call motov on the statement list */
	motov(uc_operand(p, 2)); 

	/* Pop off the sub frame */
	moto_freeFrame(env);

	/* Pop off the function frame */
	moto_freeFrame(env);
}

void motov_delete(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVal *r = NULL;
	MotoVal *val = NULL;

	motov(uc_operand(p, 0));
	val = opstack_pop(env);

	if(val->type->kind != REF_TYPE)
		moto_illegalDeleteOperation();

	moto_freeVal(env,val);

	r = moto_createVal(env, "void",0);

	opstack_push(env, r);
}

void motov_dereference_rval(const UnionCell *p){
	MotoEnv *env = moto_getEnv();
	MotoVal *instanceVal,*r;
	MotoClassDefinition* mcd;
	char* varn, *typen;

	/* Evaluate Operand 0 to get the motoval associated with the structure */
	motov(p->opcell.operands[0]);
	instanceVal = opstack_pop(env); 
	
	/* If Operand 0 is not of type->kind REF_TYPE */
	if(instanceVal->type->kind != REF_TYPE) {
	
		/* throw error(ÒThe dereference operator Ô.Õ	 may only be used on ObjectsÓ) */
		moto_cantDereferenceType(instanceVal->type->name);
		r = moto_createVal(env, "int",0);
	
	} else {
	
		/* Retreive the MotoClassDefinition for the type of the associated motoval */
		mcd = moto_getMotoClassDefinition(env,instanceVal->type->name);
		
		/* Get the name of the member variable to dereference to from operand 1 */
		varn = uc_str(p, 1);
		
		/* If the MotoClassDefinition defines no such member variable */
		if(mcd == NULL || mcd_getMemberType(mcd,varn) == NULL){
			
			/* Ok ... check to see if a method with this name is defined and if so 
			that there is only one method with this name */
			MotoFunction* mfn;
			char fname[256];
			
			moto_createMotonameForFnInBuffer(instanceVal->type->name,varn,fname);
			
			if(ftab_fnForName(env->ftable,&mfn,fname) == MOTO_OK){
				MotoType* type = mfn_type(mfn);
				r = moto_createVal(env, type->name,0);
			} else {	
				/* Throw error (ÒClass %s has no member named %sÓ) */
				moto_noSuchMember(instanceVal->type->name,varn);
				r = moto_createVal(env, "int",0);
			}
		
		} else {
			typen = mcd_getMemberType(mcd,varn);
			
			r = moto_createVal(env, typen,0);
		}
	}
	
	/* Free the instanceVal */
	moto_freeVal(env,instanceVal);
	
	/* Push the MotoVal associated with the dereference onto the OpStack */
	opstack_push(env,r);
}

void 
motov_dereference_lval(const UnionCell *p){
	/* Just call motov_dereference rval */
	motov_dereference_rval(p);
}

void 
motov_do(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVal *val;

	motov(p->opcell.operands[0]);

	val = opstack_pop(env); 
	moto_freeVal(env,val); 
}

void 
motov_elseif(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVal *val;

	motov(uc_operand(p, 0));
	val = opstack_pop(env);

	if (val->type->kind != BOOLEAN_TYPE) {
		moto_illegalConditional(val->type->name);
	}
  
	moto_freeVal(env,val);

	moto_createFrame(env, SUB_FRAME);
	motov(uc_operand(p, 1));
	moto_freeFrame(env);
}

void 
motov_elseiflist(const UnionCell *p) {
	motov(uc_operand(p, 0));
	motov(uc_operand(p, 1));
}

static void 
motov_excp_try(const UnionCell *p){
	MotoEnv *env = moto_getEnv();
	
	UnionCell *tryBlockCode = uc_operand(p,0);
	UnionCell *handlers = uc_operand(p,1);
	UnionCell *catchBlocks = uc_operand(handlers,0);
	UnionCell *finallyBlockCode = uc_operand(handlers,1);
	
	StringSet *caughtExceptions = sset_createDefault();
	int caughtSuperException = 0;
	
	Enumeration* e;
	int i;
	 
	/* Push on a new sub-frame */
	moto_createFrame(env, SUB_FRAME);
	
	/* Verify the 'try' code */
	motov(tryBlockCode);
	
	/* Pop off the sub-frame */
	moto_freeFrame(env);
	
	/* Foreach catch block */
	for(i=0;i<uc_opcount(catchBlocks);i++) {
		UnionCell* currentCatchBlock = uc_operand(catchBlocks,i);
		char* exceptionTypeName = uc_str(currentCatchBlock,0);
		char* exceptionName = uc_str(currentCatchBlock,1);
		MotoType* exceptionType = mttab_get(env->types,exceptionTypeName,0);
		UnionCell* catchBlockCode = uc_operand(currentCatchBlock,2);
		
		/* Push on a new sub-frame */
		moto_createFrame(env, SUB_FRAME);
	
		/* Verify the exception type exists and is a subclass of Exception*/
		if (exceptionType == NULL) {
			/* If it doesn't exist give a type not defined error */
			moto_typeNotDefined(exceptionTypeName);
			/* Recover */
			exceptionType = mttab_get(env->types,"Exception",0);
		} else if (exceptionType->kind != REF_TYPE || 
			!mxstr_endsWith(exceptionTypeName,"Exception")) { // FIXME: need inheritance check 
			/* If it is not a subclass of Exception throw a type not catchable error */
			moto_typeNotCatchable(exceptionTypeName);
			/* Recover */
			exceptionType = mttab_get(env->types,"Exception",0);
		}
			
		/* Add the exception type to the handled exceptions stringset
			If it was already present throw a typeAlreadyCaught err */
			
		if (sset_contains(caughtExceptions, exceptionTypeName) || caughtSuperException){
			moto_typeAlreadyCaught(exceptionTypeName);
		} else {
			sset_add(caughtExceptions, estrdup(exceptionTypeName));
		}

		/* If exception type is "Exception" then don't even try to catch future exceptions */
		if (!estrcmp(exceptionTypeName,"Exception"))
			caughtSuperException = 1;
					
		/* Declare the exception variable in the catch block */
		moto_declareVar(env, exceptionName, exceptionTypeName, 0,'\0');
		
		/* Verify the current 'catch' code */
		motov(catchBlockCode);
		
		/* Pop off the sub-frame */
		moto_freeFrame(env);
	}
	
	/* Push on a new sub-frame */
	moto_createFrame(env, SUB_FRAME);
		
	/* Verify the 'finally' code */
	motov(finallyBlockCode);
	
	/* Pop off the sub-frame */
	moto_freeFrame(env);
	
	/* Clean Up */
	for(e = sset_elements(caughtExceptions);enum_hasNext(e);) 
		free(enum_next(e));
	enum_free(e);
	sset_free(caughtExceptions);
	
	
}

static void 
motov_excp_throw(const UnionCell *p){
	MotoEnv *env = moto_getEnv();
	MotoVal *exceptionVal;
	
	/* Evaluate the expression operand */
	motov(uc_operand(p,0));
	exceptionVal = opstack_pop(env);
	
	/* Verify that whatÕs being thrown is an exception */
	if (exceptionVal->type->kind != REF_TYPE || 
		!mxstr_endsWith(exceptionVal->type->name,"Exception")) { // FIXME: need inheritance check 
		/* If it is not a subclass of Exception throw a type not throwable error */
		moto_typeNotThrowable(exceptionVal->type->name);
	}
	
	/* Clean Up */
	moto_freeVal(env,exceptionVal);
}

void 
motov_for(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVal *vcond;

	moto_createFrame(env, SUB_FRAME);
	if (uc_opcode(uc_operand(p, 0)) != NOOP) {
		motov(uc_operand(p, 0));
		moto_freeVal(env,opstack_pop(env));
	}
	motov(uc_operand(p, 1));
	vcond = opstack_pop(env);
	if (vcond->type->kind != BOOLEAN_TYPE) { 
		moto_illegalConditional(vcond->type->name);
	}
	moto_freeVal(env,vcond);
	if (uc_opcode(uc_operand(p, 2)) != NOOP) {
		motov(uc_operand(p, 2));
		moto_freeVal(env,opstack_pop(env));
	}
	if (uc_opcode(uc_operand(p, 3)) != NOOP) {
		moto_createFrame(env, SUB_FRAME);
		motov(uc_operand(p, 3));
		moto_freeFrame(env);
	}
	moto_freeFrame(env);

}

static void 
motov_list(const UnionCell *p) {
	int i;
	for(i=0;i<p->opcell.opcount;i++)
		motov(p->opcell.operands[i]);
}

static void 
motov_id(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	char *varn;
	MotoVar *var;
	MotoVal* val;

	varn = p->valuecell.value;
	var = moto_getVar(env, varn);

	if (var == NULL) {
		MotoFunction* mfn = NULL;
		MotoVar* self = NULL;
		int code = FTAB_NONE;
		
		/* Check to see if a method with this name is defined */
		if ((self = moto_getVar(env,"this")) != NULL) {
			/* Generate the Moto name for a possible method */
			char fname[256];
			moto_createMotonameForFnInBuffer(self->type->name, varn,fname);
	
			/* Retrieve the MXFN record */
			code = ftab_fnForName(env->ftable,&mfn,fname);
		} 
		
		/* If there is no method, see if there is a function with this name */
		if(code == FTAB_NONE) code = ftab_fnForName(env->ftable,&mfn,varn);
		
		if (code == FTAB_OK){
			MotoType* type = mfn_type(mfn);
			val = moto_createVal(env, type->name,0);
		} else if (code == FTAB_MULTIPLE) {
			/* Multiple functions or methods with the same name */
			moto_ambiguousFunction(varn);
			val = moto_createVal(env, "Object",0); /* recover */
		} else {	
			moto_varNotDeclared(varn);
			val = moto_createVal(env, "Object",0); /* recover */
		}
	} else {
		val = moto_cloneVal(env, var->vs);
	}

	opstack_push(env, val);

}

static void 
motov_if(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVal *val;

	motov(uc_operand(p, 0));
	val = opstack_pop(env);

	if (val->type->kind != BOOLEAN_TYPE) {
		moto_illegalConditional(val->type->name);
	}

	moto_freeVal(env,val);
 
	moto_createFrame(env, SUB_FRAME);
	motov(uc_operand(p, 1));
	moto_freeFrame(env);

	if (uc_opcode(uc_operand(p, 2)) != NOOP) {
		motov(uc_operand(p, 2));
	} 
	if (uc_opcode(uc_operand(p, 3)) != NOOP) {
		moto_createFrame(env, SUB_FRAME);
		motov(uc_operand(p, 3));
		moto_freeFrame(env);
	} 

}

static void 
motov_incdec(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVal *r = NULL;
	MotoVal *val;
	int incdec = uc_opcode(p);
	UnionCell* lvop = uc_operand(p, 0);
	
	motov(uc_operand(p, 0));
	val = opstack_pop(env);

	/* If this is NOT an overloaded operator ... */	
	if(motox_try_overloaded_method_op(incdec,val,NULL,NULL) != MOTO_OK) {
		
		/* Verify that we are assigning to an LVal */
		if (
			(lvop->type == VALUE_TYPE && lvop->valuecell.kind == ID_VALUE ) ||
			(lvop->type == OP_TYPE && (lvop->opcell.opcode == ARRAY_LVAL || lvop->opcell.opcode == DEREFERENCE_LVAL))
		) {	
			switch (val->type->kind) {
				case BYTE_TYPE: r = moto_createVal(env, "byte",0); break;
				case CHAR_TYPE: r = moto_createVal(env, "char",0); break;
				case INT32_TYPE: r = moto_createVal(env, "int",0); break;
				case INT64_TYPE: r = moto_createVal(env, "long",0); break;
				case FLOAT_TYPE: r = moto_createVal(env, "float",0); break;
				case DOUBLE_TYPE: r = moto_createVal(env, "double",0); break;
				default: moto_illegalUnaryOperation(uc_opcode(p) , val->type->name);
				r = moto_createVal(env, "int",0); /* recover */
			}
		
		} else {
			if (incdec == POSTFIX_INC || incdec == PREFIX_INC) {
				moto_illegalIncrement();
			} else {
				/* POSTFIX_DEC || PREFIX_DEC */
				moto_illegalDecrement();
			}
	
			r = moto_createVal(env, "int",0); /* recover */

		}
		opstack_push(env,r);
	}	
	
	moto_freeVal(env,val);
	
}

void motov_jump(const UnionCell *p) {
}

void motov_print(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVal * v;
	motov(uc_operand(p, 0));
	v = (MotoVal*)opstack_pop(env);

	/* FIXME: Not entirely sure this should be an error .. but,
		better safe than sorry */

	if(v->type->kind == VOID_TYPE){
		moto_cannotPrint();
	}

	moto_freeVal(env,v);
}

static void motov_math(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVal *v1;
	MotoVal *v2;
	int op = uc_opcode(p);

	motov(uc_operand(p, 0));
	motov(uc_operand(p, 1));
	v2 = opstack_pop(env);
	v1 = opstack_pop(env);

	if(motox_try_overloaded_function_op(op,v1,v2) != MOTO_OK) {
		motov_domath(v1, v2, op);
	}

	moto_freeVal(env,v1);
	moto_freeVal(env,v2);
}

/* Peek at argc motovals on the stack and fill the argv and types
	arrays with the appropriate values */

static void 
motov_popFillAndFreeArgs(int argc,char** types){
	int i;
	MotoVal* val;
	MotoEnv* env= moto_getEnv();

	for (i = argc-1; i >= 0; i--) {
		val = (MotoVal *)opstack_pop(env);
		types[i] = val->type->name;
		moto_freeVal(env,val);
	}
}

static char* 
motov_fnErrorName(char* basename,int argc,char** types){
	StringBuffer * d = buf_createDefault();
	int i;
	char* fname;
	
	buf_puts(d, basename);
	buf_putc(d, '(');
	for (i = 0; i < argc; i++) {
		if (i > 0) buf_puts(d, ", ");
		buf_printf(d,"<%s>",types[i]);
	}
	buf_putc(d, ')');

	fname = buf_toString(d);
	buf_free(d);
	
	return fname;
}

static void 
motov_fcall(MotoVal* self, MotoVal *callee, UnionCell * arglistUC){
	MotoEnv *env = moto_getEnv();
	MotoVal *r;
	int i,argc;
	char** types;
	
	/* Is it a functional type ? */
	if(callee->type->atype == NULL || callee->type->dim > 0 ) {
		/* If not generate error: Cannot call non-functional expression */
		moto_expressionNotCallable(callee->type->name);
		/* Recover */
		r = moto_createVal(env,"int",0);
	} else {
	
		/* Motov the arguments */
		motov(arglistUC);
		argc = arglistUC->opcell.opcount;
		
		types = emalloc(argc*sizeof(char*));
		motov_popFillAndFreeArgs(argc,types);
		
		/* Verify the argument counts match up */
		if(argc > callee->type->argc)
			moto_tooManyArgumentsToCall(callee->type->name);
		else if (argc < callee->type->argc)
			moto_tooFewArgumentsToCall(callee->type->name);
		else {	
			/* For each type argument */
			for(i=0;i<argc;i++){
				/* Verify the passed argument is castable */
				if(!mtype_checkCast(mttab_get(env->types,types[i],0),callee->type->argt[i])) 
					moto_cannotCast(types[i],callee->type->argt[i]->name);
			}
		}
		
		free(types);
			
		/* Return the typeÕs atype */
		r = moto_createVal(env,callee->type->atype->name,0);
	}
	opstack_push(env,r);
}

MotoVal*
motov_callee(const UnionCell* p, MotoVal* self){
	MotoEnv *env = moto_getEnv();
	MotoVal* callee;
	UnionCell* calleeUC = uc_operand(p, uc_opcode(p) == METHOD ? 1:0 );
		
	if (uc_opcode(p) == METHOD) {
		/* Retreive the MotoClassDefinition for the type of the associated motoval */
		MotoClassDefinition* mcd = moto_getMotoClassDefinition(env,self->type->name);
		char* typen = mcd_getMemberType(mcd,calleeUC->valuecell.value);
			
		callee = moto_createVal(env, typen,0);
	} else {
		/* Motov the expression being called */
		motov(calleeUC);
		callee = opstack_pop(env);
	}
	
	return callee;
}

static void 
motov_fn(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	int code=FTAB_NONE;		/* Possible error code from function lookup */
	char *fname = NULL;		/* The moto function name to be looked up */
	char *fbasename;			/* The base function name to be looked up */
	int argc=0;					/* Number of arguments to the function */
	char **types = NULL;		/* The type of each argument to the fucntion */

	MotoFunction *f = NULL;
	MotoVal *r = NULL;
	
	MotoVal *self = NULL;	/* Value representing the object itself */
	int calleeIsDDF = 0; /* Should be set to true if the callee UnionCell 
										in an expression of a functional type */
	
	UnionCell* arglistUC = uc_operand(p, 1 + (uc_opcode(p) == METHOD?1:0) );
	
	/* If this is a method call evaluate the self operand */
		
	if (uc_opcode(p) == METHOD) {
		motov(uc_operand(p, 0));
		self = opstack_pop(env);
		if (self->type->kind != REF_TYPE) 
			moto_cantInvokeMethodOnType(self->type->name);
	} else if (uc_opcode(p) == FN && moto_getVar(env,"this") != NULL) {
		/* This may still be a method call, lets get a self value just in case */
		self = moto_getVarVal(env,moto_getVar(env,"this"));
	}
			
	/* Figure out if the callee is a dynamically defined function (DDF) */
	calleeIsDDF = motox_calleeIsDDF(p,self);
	
	if(calleeIsDDF) {
		MotoVal* callee;
		
		/* Retrieve the callee */
		callee = motov_callee(p,self);
		
		motov_fcall(self,callee,arglistUC);
		
		moto_freeVal(env,callee);
	} else {
		
		/* Get the base function name */
		fbasename = uc_str(p, 0 + (uc_opcode(p) == METHOD?1:0));
		
		/* Fill in the arguments */
		motov(arglistUC);	 /* argument list */
		argc = arglistUC->opcell.opcount;
		types = mman_malloc(env->mpool, argc * sizeof(char *));
		motov_popFillAndFreeArgs(argc,types);
	
		/* Look up the function or method being identified */
		code = motox_lookupMethodOrFn(&f, self,fbasename, argc, types, uc_opcode(p));
		
		/* If we still have nothing, or are confused by too many possible fdefs ... throw an err */
		
		if(code ==	FTAB_NONE || code == FTAB_MULTIPLE){  
	
			fname = motov_fnErrorName(uc_str(p, 0+ (uc_opcode(p) == METHOD?1:0)),argc,types);
			
			if (code == FTAB_NONE)
				if(uc_opcode(p) == METHOD)
					moto_noSuchMethod(self->type->name, fname);
				else
					moto_noSuchFunction(fname);
			else
				moto_ambiguousFunction(fname);
	
			free(fname);
			r = moto_createVal(env, "int",0);
			
		} else {
			MotoType *type = mttab_get(env->types, f->rtype,0);
			if(type == NULL) {
				moto_typeNotDefined(f->rtype);
				r = moto_createVal(env, "Object",0); /* Recover */
			} else {
				r = moto_createVal(env, f->rtype,0);
			}
		}
		
		if(self != NULL)
			moto_freeVal(env,self);
			
		opstack_push(env, r);
	}
}

static void 
motov_identify(const UnionCell *wrapper) {
	MotoEnv *env = moto_getEnv();
	char* fbasename;
	UnionCell* p, * arglistUC;
	UnionCell* fbasenameUC;
	char *fname = NULL,*types[20];
	int pargc = 0,pargi[20]; /* The number of args that are 'known' and their indexes */
	int i,code = FTAB_NONE,argc;
	MotoType* ftype, *rftype;
	MotoFunction* f;
	MotoVal* r,*self = NULL;
	int calleeIsDDF = 0;
	
	p = uc_operand(wrapper,0);
	
	fbasenameUC = uc_operand(p,0 + (uc_opcode(p) == METHOD ? 1:0));
	arglistUC = uc_operand(p,1 + (uc_opcode(p) == METHOD ? 1:0));
	
	/* If this is a method identifier evaluate the self operand */	
	if (uc_opcode(p) == METHOD) {
		motov(uc_operand(p, 0));
		self = opstack_pop(env);
		if (self->type->kind != REF_TYPE) 
			moto_cantInvokeMethodOnType(self->type->name);
	} else if (uc_opcode(p) == FN && moto_getVar(env,"this") != NULL) {
		/* This may still be a method call, lets get a self value just in case */
		self = moto_getVarVal(env,moto_getVar(env,"this"));
	}
	
	/* Fill in the arg types */
	argc = uc_opcount(arglistUC);
	
	/* For each argument */
	for(i=0;i<argc;i++){
		UnionCell* argUC;
		
		/* Extract the argument type */
		argUC = uc_operand(arglistUC,i);
		if(uc_opcode(argUC) == TYPED_UNKNOWN)
			types[i] = motov_extractType(uc_operand(argUC,0))->name;
		else if (uc_opcode(argUC) == UNTYPED_UNKNOWN)
			types[i] = NULL;
		else {
			/* Verify the argument and retrieve its type */
			MotoVal* aval;
			motov(argUC);
			aval = opstack_pop(env);
			types[i] = aval->type->name;
			moto_freeVal(env,aval);
			
			/* Add the 'known' argument index to pargi and increment pargc */
			pargi[pargc] = i;
			pargc++;
		}	
	}

	/* Extract the function or method name */
	fbasename = fbasenameUC->valuecell.value;
	
	/* Figure out if the callee is a dynamically defined function (DDF) */
	calleeIsDDF = motox_calleeIsDDF(p,self);
	
	if(calleeIsDDF){
	
		/* Retrieve the callee */
		MotoVal* callee = motov_callee(p,self);
		
		/* Retrieve the DDF's type */
		ftype = callee->type;
		
		/* Verify that it's a callable type */
		if(callee->type->atype == NULL || callee->type->dim > 0 ) {
		
			/* If not generate error: Cannot identify non-functional expression */
			moto_expressionNotCallable(callee->type->name);
			
			/* Recover */
			rftype = mttab_get(env->types, "Object",0);
			
		} else {			
		
			/* Generate a new type by removing any applied arguments from the function's type */
			rftype = mttab_apply(env->types,ftype,pargc,pargi);
		}
			
	} else {
			
		/* Look up the function or method being identified */
		code = motox_lookupMethodOrFn(&f, self,fbasename, argc, types, uc_opcode(p));
		
		/* If we have nothing, or are confused by too many possible fdefs ... throw an err */
		if(code ==	FTAB_NONE || code == FTAB_MULTIPLE){  
			fname = motov_fnErrorName(fbasename,argc,types);
			
			if (code == FTAB_NONE)
				if(uc_opcode(p) == METHOD)
					moto_noSuchMethod(self->type->name, fname);
				else
					moto_noSuchFunction(fname);
			else
				moto_ambiguousFunction(fname);
	
			free(fname);
			
			/* Recover */
			rftype = mttab_get(env->types, "Object",0);
		} else {
			/* Retrieve the function's type */
			ftype = mfn_type(f);
			
			/* Generate a new type by removing any applied arguments from the function's type */
			rftype = mttab_apply(env->types,ftype,pargc,pargi);
		}
	}
	
	/* Instantate and push onto the stack a motoval of the functional type */
	r = moto_createVal(env, rftype->name,0);
	opstack_push(env, r);
}

void motov_new(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	int code;					/* Possible error code from function lookup */
	char fname[256];			/* The moto function name to be looked up */

	int argc=0;					/* Number of arguments to the function */
	char **types = NULL;		/* The type of each argument to the fucntions */

	MotoFunction *f = NULL;
	MotoVal *r = NULL;

	char *classname = NULL;
	MotoType *type = NULL;

	classname = uc_str(p, 0);

	/* Make sure this type is registered */
	
	type = mttab_get(env->types, classname,0);
	if (type == NULL) {
		moto_typeNotDefined(classname);
		type = mttab_get(env->types, "Object",0); /* recover */
	}
	
	/* Discover the argument count */
	argc = uc_operand(p, 1)->opcell.opcount;

	/* We only need to bother checking for the existense of a no-arg
		constructor if the Class was externally defined */
				
	if(argc > 0 || type->isExternallyDefined) {
			
		motov(uc_operand(p, 1));
		types = mman_malloc(env->mpool, argc * sizeof(char *));
		motov_popFillAndFreeArgs(argc,types);
	
		/* Generate the Moto name for this Constructor */
		moto_createMotonameForFnInBuffer(classname,classname,fname);
	
		/* Retrieve the MXFN record */
		code = ftab_cacheGet(env->ftable, &f, fname, argc, types);
		
		if(code ==	FTAB_NONE || code == FTAB_MULTIPLE){
			char* ferrname = motov_fnErrorName(uc_str(p, 0),argc,types);
	
			if (code == FTAB_NONE )
				moto_noSuchConstructor(ferrname);
			else
				moto_ambiguousFunction(ferrname);
				
			free(ferrname);
		}
	}
	
	r = moto_createVal(env, type->name,0);
	opstack_push(env, r);
}

void motov_not(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVal *val,*r;

	motov(uc_operand(p, 0));
	val = opstack_pop(env);

	if(val->type->kind != BOOLEAN_TYPE)
		moto_illegalUnaryOperation(uc_opcode(p) , val->type->name);

	moto_freeVal(env,val);

	r = moto_createVal(env, "boolean",0);
	opstack_push(env, r);
}

void motov_or(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVal *v1, *v2, *r;

	motov(uc_operand(p, 0));
	motov(uc_operand(p, 1));
	v2 = opstack_pop(env);
	v1 = opstack_pop(env);
	if (v1->type->kind != BOOLEAN_TYPE || v2->type->kind != BOOLEAN_TYPE)
		moto_illegalBinaryOperation(uc_opcode(p), v1->type->name, v2->type->name);
	moto_freeVal(env,v2);
	moto_freeVal(env,v1);

	r = moto_createVal(env, "boolean",0);

	opstack_push(env, r);
}

static void motov_regex(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVal *v1;
	MotoVal *v2;
	MotoVal *r;

	motov(uc_operand(p, 0));
	motov(uc_operand(p, 1));

	v2 = opstack_pop(env);
	v1 = opstack_pop(env);

	r = moto_createVal(env, "boolean",0);

	if(!isString(v1) || !isRegex(v2))
		moto_illegalBinaryOperation(uc_opcode(p), v1->type->name, v2->type->name);
	
	switch (uc_opcode(p)) {
		case RX_MATCH: break;
		case RX_NOT_MATCH: break;
		default:
			moto_illegalBinaryOperation(uc_opcode(p), v1->type->name, v2->type->name);
			break;
	}

	moto_freeVal(env,v2);
	moto_freeVal(env,v1);

	opstack_push(env, r);

}

static void motov_rel(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVal *v1;
	MotoVal *v2;
	int op = uc_opcode(p);

	motov(uc_operand(p, 0));
	motov(uc_operand(p, 1));

	v2 = opstack_pop(env);
	v1 = opstack_pop(env);
	
	if(motox_try_overloaded_function_op(op,v1,v2) != MOTO_OK) {
		motov_dorel(v1, v2, op);
	}	
	
	moto_freeVal(env,v2);
	moto_freeVal(env,v1);
}

static void motov_return(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVar *dest;
	MotoVal *val;

	/* Get the type of the return value */
	
	dest = moto_getVar(env,"$rvalue");

	/* Verify the type of the argument to return matches (or is 
	implicitly castable to the type of the rvalue variable.
	If not generate an error */

	if(uc_opcount(p) != 0) {
		motov(uc_operand(p,0));
		val = opstack_pop(env);
	
		if (mtype_checkAssign( dest->type, val->type)!= 1) 
			moto_varTypeReturnError(dest->type->name,val->type->name);
		
		moto_freeVal(env,val);
	} else {
		if (dest->type->kind != VOID_TYPE) {
			moto_varTypeReturnError(dest->type->name,"void");
		}
	}
}

void motov_value(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVal *val = NULL;

	switch (p->valuecell.kind) {
		case NULL_VALUE:
			val = moto_createVal(env,"null",0);
			break;
		case BOOLEAN_VALUE:
			val = moto_createVal(env, "boolean",0);
			break;
		case CHAR_VALUE:
			val = moto_createVal(env, "char",0);
			break;
		case INT32_VALUE:
			val = moto_createVal(env, "int",0);
			break;
		case INT64_VALUE:
			val = moto_createVal(env, "long",0);
			break;
		case FLOAT_VALUE:
			val = moto_createVal(env, "float",0);
			break;
		case DOUBLE_VALUE:
			val = moto_createVal(env, "double",0);
			break;

		/* Create build in object types */

		case REGEX_VALUE:
			TRY {
				NFA* nfa;
				nfa_init();
				nfa = parse(p->valuecell.code + 1,0,strlen(p->valuecell.code + 1));
				nfa_free(nfa);
			} CATCH("RegexParseException") {
				moto_regexParseError(p->valuecell.code + 1,excp_getCurrentException()->msg);
			} END_TRY
			val = moto_createVal(env,"Regex",0);
			break;
		case STRING_VALUE:
			val = moto_createVal(env,"String",0);
			break;
		default:
			THROW_D("MotoCellTypeException");
			break;
	}

	opstack_push(env, val);

}

void motov_scope(const UnionCell *p){
	MotoEnv *env = moto_getEnv();

	moto_createFrame(env, SUB_FRAME);
	motov(uc_operand(p, 0));
	moto_freeFrame(env);
}

void motov_switch(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVal *switchVal;
	UnionCell *caseStatementListUC;
	MotoType* switchType;
	int i;

	motov(uc_operand(p, 0));
	switchVal = opstack_pop(env);

	/* Verify that the switch arg is an atomic type or a String */
	if (switchVal->type->kind == REF_TYPE && !isString(switchVal)) 
		moto_illegalSwitchArgument(switchVal->type->name);

	switchType = switchVal->type;
	moto_freeVal(env,switchVal);
	
	/* Verify the default case's statement list */
	moto_createFrame(env, SUB_FRAME);
	motov(uc_operand(p, 1));
	moto_freeFrame(env);
		
	/* Verify the other cases */
	caseStatementListUC = uc_operand(p, 2);
	for(i=0;i<caseStatementListUC->opcell.opcount;i++){
		MotoVal *caseVal;
		UnionCell *caseStatementUC = uc_operand(caseStatementListUC,i);
		
		/* Since we are looping over cases ourselves we need to set the
		global meta info properly */
		
		MetaInfo curmeta = env->errmeta;
		env->errmeta = *uc_meta(caseStatementUC);
	
		/* Verify the case value */
		motov(uc_operand(caseStatementUC, 0));
		caseVal = opstack_pop(env);
	
		/* If the case type != switch type print an error */
		if (caseVal->type != switchType)
			moto_illegalCaseType(caseVal->type->name, switchType->name);
		
		/* Make sure the case argument doesn't vary */
		if ( uc_operand(caseStatementUC, 0)->type != VALUE_TYPE || 
			(uc_operand(caseStatementUC, 0)->type == VALUE_TYPE && 
			 uc_operand(caseStatementUC, 0)->valuecell.kind == ID_VALUE))
			moto_illegalCaseArgument((char*)caseVal->refval.value);
		
	
		moto_freeVal(env,caseVal);
		
		/* Verify this case block's statement list */
		moto_createFrame(env, SUB_FRAME);
		motov(uc_operand(caseStatementUC, 1));
		moto_freeFrame(env);

		/* Re-set the global err meta */		
		env->errmeta = curmeta;
	}

}

void motov_ternary(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVal *valc,*valt,*valf,*val;
	int operandsOk = 1;

	motov(uc_operand(p, 0));
	valc = opstack_pop(env);

	if (valc->type->kind != BOOLEAN_TYPE) {
		operandsOk = 0;
	}

	motov(uc_operand(p, 1));
	valt = opstack_pop(env);
	motov(uc_operand(p, 2));
	valf = opstack_pop(env);

	val = moto_cloneVal(env, valt);

	if (valt->type->kind == valf->type->kind){
		if( valt->type->kind == REF_TYPE && valt->type != valf->type ) {
			if( strcmp(valt->type->name,"null") == 0 ) {
				val->type=valf->type;
			} else if ( strcmp(valf->type->name,"null") == 0 ) {
				/* val->type=valt->type ; -- this is the default */
			} else if (strcmp(valt->type->name,"Object") == 0 ) {
				/* val->type=valt->type ; -- this is the default */
			} else if (strcmp(valf->type->name,"Object") == 0 ) {
				val->type=valf->type; /* implicit expanding cast */
			} else {
				operandsOk = 0;
			}
		}
	} else {
	 
		/* if it is not the case that they are both numeric types */

		if( 
			  !( 
				  (
					  valt->type->kind == INT32_TYPE || 
					  valt->type->kind == INT64_TYPE ||
					  valt->type->kind == FLOAT_TYPE || 
					  valt->type->kind == DOUBLE_TYPE 
				  ) && (
					  valf->type->kind == INT32_TYPE || 
					  valf->type->kind == INT64_TYPE ||
					  valf->type->kind == FLOAT_TYPE || 
					  valf->type->kind == DOUBLE_TYPE 
				  ) 
			  ) 
		  )
		{
			operandsOk = 0;
		} 
	} 

	if(!operandsOk){
		moto_illegalTernaryOperands(
			valc->type->name, valt->type->name, valf->type->name);
	}

	moto_freeVal(env,valc);
	moto_freeVal(env,valt);
	moto_freeVal(env,valf);

	opstack_push(env, val);
}

void motov_uminus(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVal *val,*r;

	motov(uc_operand(p, 0));
	val = opstack_pop(env);

	if (val->type->kind != INT32_TYPE && val->type->kind != INT64_TYPE &&
		 val->type->kind != FLOAT_TYPE && val->type->kind != DOUBLE_TYPE ) {

		moto_illegalUnaryOperation(uc_opcode(p) , val->type->name);
		r = moto_createVal(env, "int",0);

	} else
		r = moto_createVal(env, val->type->name,0);

	moto_freeVal(env,val);
	opstack_push(env, r);
}

void motov_unknown(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVal* r;
	
	moto_illegalUnknown();
	
	/* Recover */
	r = moto_createVal(env, "int",0);

	opstack_push(env, r);	
}

void motov_use(const UnionCell *p) {
	char *usename = uc_str(p, 0);
	char *libpath;

	if ((libpath = mxdl_find(usename)) == NULL) 
		moto_noSuchLibrary(usename);

	if (libpath != NULL) free(libpath);
}

void motov_while(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVal *v;
	motov(uc_operand(p, 0));
	v = opstack_pop(env);

	if (v->type->kind != BOOLEAN_TYPE) {
		moto_illegalConditional(v->type->name);
	}

	moto_freeVal(env,v);

	moto_createFrame(env, SUB_FRAME);
	motov(uc_operand(p, 1));
	moto_freeFrame(env);

}

/*-------------------------------------------------------------------------
 * Type Conversions for Relational and Arithmetic Operations
 *
 * Conversions of operands in arithmetic operations is done in a manner
 * similar to that described in K&R Second Edition, Appendix A, Section A6.
 * The differences in this rule set are due to the fact that Moto has
 * fewer numeric types than does C. The "usual arithmetic conversions" for
 * Moto:
 *
 * If either operand is double, the other is converted to double.
 * Otherwise, if either operand is float, the other is converted to float.
 * Otherwise, if either operand is long, the other is converted to long.
 * Otherwise, both operands have type int.
 *------------------------------------------------------------------------*/

static void motov_dorel(MotoVal *v1, MotoVal *v2, int op) {
	MotoEnv *env = moto_getEnv();
	MotoVal *r;

	if (v1->type->kind == REF_TYPE || v2->type->kind == REF_TYPE) {
		if (v1->type->kind != v2->type->kind) {
			moto_illegalBinaryOperation(op, v1->type->name, v2->type->name);
		} else if ( 
			(v1->type != v2->type) && 
			!(isObject(v1) || isObject(v2)) && 
			!(strcmp(v1->type->name,"null") == 0) &&
			!(strcmp(v2->type->name,"null") == 0)
		){
			moto_illegalBinaryOperation(op, v1->type->name, v2->type->name);

		} else if (op == REL_EQ_M) {

		} else if (op == REL_NE_M) {

		} else if (isString(v1) || isString(v2)) {

			if (isString(v1) && isString(v2)) {
				switch (op) {
					case REL_EQ_S: break;
					case REL_NE_S: break;
					case REL_GT_S: break;
					case REL_GTE_S: break;
					case REL_LT_S: break;
					case REL_LTE_S: break;
					default: 
						moto_illegalBinaryOperation(op, v1->type->name, v2->type->name);
				} 
			} else {
				moto_illegalBinaryOperation(op, v1->type->name, v2->type->name);
			}
		} else {
			moto_illegalBinaryOperation(op, v1->type->name, v2->type->name);
		}
	} else if (v1->type->kind == BOOLEAN_TYPE || v2->type->kind == BOOLEAN_TYPE) {
		if (v1->type->kind == v2->type->kind) {
			switch (op) {
				case REL_EQ_M: 
					break;
				case REL_NE_M: 
					break;
				case REL_GT_M:
				case REL_GTE_M:
				case REL_LT_M:
				case REL_LTE_M:
					moto_illegalBinaryOperation(op, v1->type->name, v2->type->name);
					break;
				default:
					moto_illegalBinaryOperation(op, v1->type->name, v2->type->name);
			}
		} else {
			moto_illegalBinaryOperation(op, v1->type->name, v2->type->name);
		}
	} else {		  
		switch (op) {
			case REL_EQ_M: break;
			case REL_NE_M: break;
			case REL_GT_M: break;
			case REL_GTE_M: break;
			case REL_LT_M: break;
			case REL_LTE_M: break;
			default: 
				moto_illegalBinaryOperation(op, v1->type->name, v2->type->name);
		}
	}

	r = moto_createVal(env, "boolean",0);

	/* push result onto opstack */
	opstack_push(env, r);
}

static void motov_domath(MotoVal *v1, MotoVal *v2, int op) {
	MotoEnv *env = moto_getEnv();
	MotoVal *r	 = NULL;

	if (op == MATH_ADD && (isString(v1) || isString(v2))) {
		r = moto_createVal(env,"String",0);
	}
	else {

		if (v1->type->kind != INT32_TYPE && v1->type->kind != INT64_TYPE &&
			 v1->type->kind != DOUBLE_TYPE && v1->type->kind != FLOAT_TYPE &&
			 v1->type->kind != CHAR_TYPE && v1->type->kind != BYTE_TYPE) {

			 moto_illegalBinaryOperation(op,v1->type->name,v2->type->name);
			 r = moto_createVal(env, "int",0); /* recover */
 
		} else if (v2->type->kind != INT32_TYPE && v2->type->kind != INT64_TYPE &&
			 v2->type->kind != DOUBLE_TYPE && v2->type->kind != FLOAT_TYPE &&
			 v2->type->kind != CHAR_TYPE && v2->type->kind != BYTE_TYPE ) {

			 moto_illegalBinaryOperation(op,v1->type->name,v2->type->name);
			 r = moto_createVal(env, "int",0); /* recover */

		} else if (v1->type->kind == DOUBLE_TYPE || v2->type->kind == DOUBLE_TYPE) 
			r = moto_createVal(env, "double",0);
		else if (v1->type->kind == FLOAT_TYPE || v2->type->kind == FLOAT_TYPE) 
			r = moto_createVal(env, "float",0);
		else if (v1->type->kind == INT64_TYPE || v1->type->kind == INT64_TYPE) 
			r = moto_createVal(env, "long",0); 
		else if (v1->type->kind == INT32_TYPE || v1->type->kind == INT32_TYPE)
			r = moto_createVal(env, "int",0);
		else if (v1->type->kind == CHAR_TYPE || v1->type->kind == CHAR_TYPE)
			r = moto_createVal(env, "int",0);
		else if (v1->type->kind == BYTE_TYPE || v1->type->kind == BYTE_TYPE)
			r = moto_createVal(env, "byte",0);
 
		switch (op) {
			case MATH_ADD:	 break;
			case MATH_SUB:	 break;
			case MATH_MULT: break;
			case MATH_DIV:	 break;
			case MATH_MOD:
				if(v1->type->kind == DOUBLE_TYPE || v2->type->kind == DOUBLE_TYPE ||
					v1->type->kind == FLOAT_TYPE	|| v2->type->kind == FLOAT_TYPE ) {

					moto_illegalBinaryOperation(op, v1->type->name, v2->type->name);
				}
				break;
			default: 
				moto_illegalBinaryOperation(op, v1->type->name, v2->type->name);
		}
	}

	/* clean up and push result onto opstack */

	opstack_push(env, r);
}


/*=============================================================================
// end of file: $RCSfile: motov.c,v $
==============================================================================*/

