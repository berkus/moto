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

	$RCSfile: motoc.c,v $	
	$Revision: 1.91 $
	$Author: dhakim $
	$Date: 2003/07/07 02:05:42 $
 
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
#include "stringbuffer.h"
#include "symboltable.h"
#include "stringset.h"
#include "stack.h"
#include "vector.h"
#include "log.h"
#include "jumptable.h"
#include "symboltable.h"
#include "cdx_function.h"

#include "motolex.h"
#include "moto.tab.h"
#include "motoc.h"
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

static void motoc_and(const UnionCell *p); 
static void motoc_array_lval(const UnionCell *p);
static void motoc_array_rval(const UnionCell *p);
static void motoc_array_new(const UnionCell *p);
static void motoc_assign(const UnionCell *p);
static void motoc_bitop(const UnionCell *p);
static void motoc_bitnot(const UnionCell *p);

static void motoc_cast(const UnionCell *p);
static void motoc_classdef(const UnionCell *p);
static void motoc_comma(const UnionCell *p); 
static void motoc_do(const UnionCell *p);
static void motoc_declare(const UnionCell *p);
static void motoc_define(const UnionCell *p);
static void motoc_delete(const UnionCell *p);
static void motoc_dereference_lval(const UnionCell *p);
static void motoc_dereference_rval(const UnionCell *p);
static void motoc_elseif(const UnionCell *p);
static void motoc_elseiflist(const UnionCell *p);
static void motoc_excp_throw(const UnionCell *p);
static void motoc_excp_try(const UnionCell *p);
static void motoc_fn(const UnionCell *p);
static void motoc_for(const UnionCell *p);
static void motoc_identify(const UnionCell *p);
static void motoc_id(const UnionCell *p);
static void motoc_if(const UnionCell *p);
static void motoc_jump(const UnionCell *p);
static void motoc_list(const UnionCell *p);
static void motoc_math(const UnionCell *p);
static void motoc_new(const UnionCell *p);
static void motoc_not(const UnionCell *p);
static void motoc_or(const UnionCell *p);
static void motoc_incdec(const UnionCell *p);
static void motoc_print(const UnionCell *p);
static void motoc_regex(const UnionCell *p);
static void motoc_rel(const UnionCell *p);
static void motoc_return(const UnionCell *p);
static void motoc_scope(const UnionCell *p);
static void motoc_shiftop(const UnionCell *p);
static void motoc_switch(const UnionCell *p);
static void motoc_ternary(const UnionCell *p);
static void motoc_uminus(const UnionCell *p);
static void motoc_use(const UnionCell *p);
static void motoc_value(const UnionCell *p);
static void motoc_while(const UnionCell *p);

static void motoc_domath(MotoVal *v1, MotoVal *v2, int op) ;
static void motoc_dorel(MotoVal *v1, MotoVal *v2, int op) ;

static char* 
motoc_genAnonymousFunction(MotoFunction* f,MotoType* aftype,int pargc,int* pargi,FuncTypeKind* pargt);
/*-----------------------------------------------------------------------------
 * interpreter
 *---------------------------------------------------------------------------*/

void motoc(const UnionCell *p) {
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
					motoc_value(p);
					break;
				case ID_VALUE:
					motoc_id(p);
					break;
				default:
					THROW_D("MotoCellTypeException");
					break;
			}
			break;
		case OP_TYPE:
			switch(p->opcell.opcode) {
				case AND: motoc_and(p); break;
				case ARRAY_LVAL: motoc_array_lval(p); break;
				case ARRAY_RVAL: motoc_array_rval(p); break;
				case ARRAY_NEW: motoc_array_new(p); break;
				case ASSIGN: motoc_assign(p); break;
				case BITWISE_AND:
				case BITWISE_OR:
				case BITWISE_XOR:
					motoc_bitop(p); break;
				case SHIFT_LEFT:
				case SHIFT_RIGHT:
					motoc_shiftop(p); break;
				case BITWISE_NOT:
					motoc_bitnot(p); break;
				case BREAK: motoc_jump(p); break;
				case CAST: motoc_cast(p); break;
				case CLASSDEF: motoc_classdef(p); break;
				case COMMA: motoc_comma(p); break;
				case CONTINUE: motoc_jump(p); break;
				case DECLARE_INLINE:
				case DECLARE: motoc_declare(p); break;
				case DEFINE: motoc_define(p); break;
				case DELETE: motoc_delete(p); break;
				case DEREFERENCE_LVAL: motoc_dereference_lval(p); break;
				case DEREFERENCE_RVAL: motoc_dereference_rval(p); break;
				case DO: motoc_do(p); break;
				case ELSEIF: motoc_elseif(p); break;
				case ELSEIFLIST: motoc_elseiflist(p); break;
				case EXCP_THROW: motoc_excp_throw(p); break;
				case EXCP_TRY: motoc_excp_try(p); break;
				case FCALL:
				case FN:
				case METHOD: motoc_fn(p); break;
				case IDENTIFY: motoc_identify(p); break;
				case FOR: motoc_for(p); break;
				case IF: motoc_if(p); break;
				case LIST: motoc_list(p); break;
				case MATH_ADD:
				case MATH_DIV:
				case MATH_MOD:
				case MATH_MULT:
				case MATH_SUB: motoc_math(p); break;
				case NEW: motoc_new(p); break;
				case NOOP: break;
				case NOT: motoc_not(p); break;
				case OR: motoc_or(p); break;
				case POSTFIX_DEC:
				case POSTFIX_INC: 
				case PREFIX_DEC:
				case PREFIX_INC: motoc_incdec(p); break;
				case PRINT: motoc_print(p); break;
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
				case REL_NE_S: motoc_rel(p); break;
				case RETURN: motoc_return(p); break;
				case RX_MATCH:
				case RX_NOT_MATCH: motoc_regex(p); break;
				case SCOPE: motoc_scope(p); break;
				case SWITCH: motoc_switch(p); break;
				case TERNARY: motoc_ternary(p); break;
				case UMINUS: motoc_uminus(p); break;
				case USE: motoc_use(p); break;
				case WHILE: motoc_while(p); break;
				default:
					THROW_D("MotoOpcodeException");
					break;
			}
	}
	env->errmeta = curmeta; /* FIXME: this should be in a finally block */
}


/*-----------------------------------------------------------------------------
 * ops
 *---------------------------------------------------------------------------*/

void 
outputIndent(){
	MotoEnv *env = moto_getEnv();
	int indent = env->frameindex+1;

	if (env->flags & MACRO_DEBUG_FLAG) {
		buf_printf(
			env->frame->out,"#line %d \"%s\"\n",
			env->errmeta.lineno,env->errmeta.filename
		);
	}

	while (--indent >= 0)
		buf_puts(env->frame->out, "	");
}

void 
motoc_and(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVal *v1, *v2, *r;
	StringBuffer* sb = buf_createDefault();

	motoc(uc_operand(p, 0));
	motoc(uc_operand(p, 1));
	v2 = opstack_pop(env);
	v1 = opstack_pop(env);

	buf_printf(sb,"(%s&&%s)",(char*)v1->codeval.value,(char*)v2->codeval.value);

	r = moto_createVal(env, "boolean",0);
	r->codeval.value = buf_toString(sb);

	buf_free(sb);

	opstack_push(env, r);
}

static char* 
arr_gen_get(MotoVal* arr, char** indexExp,int level){
	StringBuffer* sb = buf_createDefault();

	/* Output the return type */

	if(level == arr->type->dim) {
		if(arr->type->atype->kind == REF_TYPE) {
			buf_puts(sb,"(*(");
			buf_puts(sb, mtype_toCType(arr->type->atype) );
			buf_puts(sb,"*)(excp_file=__FILE__,excp_line=__LINE__,");
		} else {
			buf_puts(sb,"(*(excp_file=__FILE__,excp_line=__LINE__,");
		}
	} else {
		buf_puts(sb, "(*(UnArray**)(excp_file=__FILE__,excp_line=__LINE__," );
	}

	/* Output the call to sub() */

	if(level == arr->type->dim) {
		switch(arr->type->atype->kind) {
			case BOOLEAN_TYPE: buf_puts(sb,"inline_bsub("); break;
			case BYTE_TYPE: buf_puts(sb,"inline_ysub("); break;
			case CHAR_TYPE: buf_puts(sb,"inline_csub("); break;
			case INT32_TYPE: buf_puts(sb,"inline_isub("); break;
			case INT64_TYPE: buf_puts(sb,"inline_lsub("); break;
			case FLOAT_TYPE: buf_puts(sb,"inline_fsub("); break;
			case DOUBLE_TYPE: buf_puts(sb,"inline_dsub("); break;
			case REF_TYPE: buf_puts(sb,"inline_rsub("); break;
			default: THROW_D("MotoException"); break;
		}
	} else {
		buf_puts(sb, "inline_rsub(" );
	}
	
	if(level == 1) {
		buf_puts(sb,arr->codeval.value);
	} else {
		buf_puts(sb,arr_gen_get(arr,indexExp,level - 1));
	}

	buf_putc(sb,',');
	buf_puts(sb,indexExp[level-1]);
	buf_puts(sb,")))");
	
	return buf_toString(sb);
}

static void
motoc_array_rval(const UnionCell *p) {
	motoc_array_lval(p);
}

static void
motoc_array_lval(const UnionCell *p) {

	MotoEnv *env = moto_getEnv();
	MotoVal *v1,*v2,*rval,*r;
	char** indexExpressionArray;

	/* Evaluate the first operand */
	motoc(uc_operand(p, 0));
	v1 = opstack_pop(env);

	/* Evaluate the LIST of ARRAY_INDEX elements */
	motoc(uc_operand(p, 1));  /* array_index list */
	v2 = opstack_pop(env);

	if ((env->rval_uc != NULL) && 
		(v1->type->kind == REF_TYPE) && (!isArray(v1))
		) {

		UnionCell * rval_uc = env->rval_uc;
		env->rval_uc = NULL;
			
		motoc(rval_uc);
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

	/* Test for presence of [] rval overloaded op */
	if(motox_try_overloaded_method_op(
		OPENBRACKET,v1,v2,NULL) == MOTO_OK) {
		return;
	}

	/* Switch on the relationship between the # of subscript operations
		and the arrays declared dimensions to create the appropriately 
		typed MotoVal */
	r = moto_createVal(env, v1->type->atype->name, 
			 v1->type->dim - 1);

	/* Build an array of the specified indexes */
	indexExpressionArray = (char**)emalloc(sizeof(char*)* (1));
	indexExpressionArray[0] = v2->codeval.value;

	r->codeval.value = arr_gen_get(v1,indexExpressionArray,1);

	moto_freeVal(env,v1);
	moto_freeVal(env,v2);

	opstack_push(env, r);
}

static void
motoc_array_new(const UnionCell *p){
	MotoEnv *env = moto_getEnv();
	MotoFrame* frame;
	MotoVal *r;
	MotoType* type;
	int stacksize,i;
	char** arrayIndexExpressions;
	int dimspec=0 , dimunspec=0; /* The fully specified array dimensions and the not
										fully specified array dimensions */
	StringBuffer* sb = buf_createDefault();

	/* Grab the array subtype from the first operand */
	type = motox_extractType(uc_operand(p, 0));

	/* Record the current position on the stack */

	frame = env->frame;
	stacksize = stack_size(frame->opstack);

	/* Evaluate the LIST of ARRAY_INDEX elements */
	motoc(uc_operand(p, 1));  /* array_index list */

	dimspec = stack_size(frame->opstack) - stacksize;

	/* Dimensions not fully specified therefore add the number of
		unspecified dimensions */

	dimunspec = uc_opcount(uc_operand(p, 2));

	/* Construct a new Array type MotoVal */

	r = moto_createVal(env, type->name,dimspec + dimunspec);

	/* Build the array of dimension bounds */

	arrayIndexExpressions = (char**)emalloc(sizeof(int)*dimspec);
	i = dimspec-1;

	while(stack_size(frame->opstack) > stacksize){
		MotoVal* indexMotoVal = opstack_pop(env);

		arrayIndexExpressions[i] = indexMotoVal->codeval.value;
		i--;
		/* FIXME moto_freeVal(env,indexMotoVal); */
	}

	/* Generate code for the array instantiation */

	buf_puts(sb,"(UnArray*)mman_track(rtime_getMPool(),arr_create(");
	buf_puti(sb,dimspec);
	buf_putc(sb,',');
	buf_puti(sb,dimunspec);
	buf_puts(sb,",0,(");

	for(i=0;i<dimspec;i++){
		if (i>0) buf_putc(sb,','); 
		buf_printf(sb,"_MDS_[%d]=%s",i,arrayIndexExpressions[i]);
	}

	buf_puts(sb,",_MDS_)");
	switch(r->type->atype->kind){
		case BOOLEAN_TYPE: buf_puts(sb,",ARR_BOOLEAN_TYPE))"); break;
		case BYTE_TYPE: buf_puts(sb,",ARR_BYTE_TYPE))"); break;
		case CHAR_TYPE: buf_puts(sb,",ARR_CHAR_TYPE))"); break;
		case INT32_TYPE: buf_puts(sb,",ARR_INT32_TYPE))"); break;
		case INT64_TYPE: buf_puts(sb,",ARR_INT64_TYPE))"); break;
		case FLOAT_TYPE: buf_puts(sb,",ARR_FLOAT_TYPE))"); break;
		case DOUBLE_TYPE: buf_puts(sb,",ARR_DOUBLE_TYPE))"); break;
		case VOID_TYPE: buf_puts(sb,",ARR_VOID_TYPE))"); break;
		case REF_TYPE: buf_puts(sb,",ARR_REF_TYPE))"); break;
	}

	r->codeval.value=buf_toString(sb);

	free(arrayIndexExpressions);
	opstack_push(env, r);
}

static void
motoc_arrayInit(const UnionCell *p,MotoType* type){
	MotoEnv *env = moto_getEnv();
	MotoVal *r,*val;
	int i;
	UnionCell* arrayElementsListUC;
	UnionCell* arrayElementUC;
	char * elements;
	StringBuffer* sb = buf_createDefault();
	
	/* Construct a new Array type MotoVal with subtype type and dimension dim */
	r = moto_createVal(env, type->name, 0);

	/* Grab the array elements */
	arrayElementsListUC = uc_operand(p, 0);
	
	/* Generate code for the array instantiation */
	buf_puts(sb,"(UnArray*)mman_track(rtime_getMPool(),arr_create_and_init(");
	buf_puti(sb,type->dim);
	buf_putc(sb,',');
	buf_puti(sb,0);
	buf_puts(sb,",0,(");

	/* Number of array elements */
	elements = emalloc(10);
	sprintf(elements, "%d", arrayElementsListUC->opcell.opcount);
	buf_printf(sb,"_MDS_[%d]=%s",0,elements);	

	buf_puts(sb,",_MDS_)");

	/* Type of array */	
	switch(r->type->atype->kind){
		case BOOLEAN_TYPE: buf_puts(sb,",ARR_BOOLEAN_TYPE,"); break;
		case BYTE_TYPE: buf_puts(sb,",ARR_BYTE_TYPE,"); break;
		case CHAR_TYPE: buf_puts(sb,",ARR_CHAR_TYPE,"); break;
		case INT32_TYPE: buf_puts(sb,",ARR_INT32_TYPE,"); break;
		case INT64_TYPE: buf_puts(sb,",ARR_INT64_TYPE,"); break;
		case FLOAT_TYPE: buf_puts(sb,",ARR_FLOAT_TYPE,"); break;
		case DOUBLE_TYPE: buf_puts(sb,",ARR_DOUBLE_TYPE,"); break;
		case VOID_TYPE: buf_puts(sb,",ARR_VOID_TYPE,"); break;
		case REF_TYPE: buf_puts(sb,",ARR_REF_TYPE,"); break;
	}

	/* Number of initializers */
	buf_printf(sb,"%d",arrayElementsListUC->opcell.opcount);
			
	for(i=0;i<arrayElementsListUC->opcell.opcount;i++){
		arrayElementUC = uc_operand(arrayElementsListUC,i);
		
		if(arrayElementUC->type == OP_TYPE && uc_opcode(arrayElementUC) == ARRAY_INIT) {
			motoc_arrayInit(arrayElementUC,mttab_get(env->types,type->atype->name,type->dim-1));
		} else 
			motoc(arrayElementUC);
		
		val = opstack_pop(env);

		if (type->dim == 1) {
			MotoVal* dup = moto_createVal(env, type->atype->name,type->dim-1);
			buf_printf(sb,",((%s)(%s))", moto_valToCType(dup),(char*)val->codeval.value);
			moto_freeVal(env,dup);
		} else {
			buf_printf(sb,",%s", val->codeval.value);
		}
		moto_freeVal(env, val);
			
	}
	
	buf_puts(sb,"))");

	r->codeval.value=buf_toString(sb);
	
	opstack_push(env, r);
}

void 
motoc_assign(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVal *v1,*v2,*r;
	int op = uc_operand(p, 1)->opcell.opcode;
	StringBuffer* sb = buf_createDefault();
	int rvalop = 0;

	/* Save rval UnionCell */
	env->rval_uc = uc_operand(p, 2);

	motoc(uc_operand(p, 0));
	v1 = opstack_pop(env);

	if (env->rval_uc != NULL) {
		motoc(uc_operand(p, 2));
		env->rval_uc = NULL;
		rvalop = 1;
	}
	v2 = opstack_pop(env);

	/* Do a math operation if need be */

	switch (op) {
		case MATH_ADD:	  /* add-assignment */
		case MATH_SUB:	  /* sub-assignment */
		case MATH_MULT:  /* mult-assignment */
		case MATH_DIV:	  /* div-assignment */
		case MATH_MOD:	  /* mod-assignment */
			if(motox_try_overloaded_method_op(
				op,v1,v2,NULL) != MOTO_OK) {
				motoc_domath(v1, v2, op);
			}
			v2 = opstack_pop(env);
			break;
	}

	if (rvalop) {
		buf_printf(sb,"(%s=%s)",v1->codeval.value,v2->codeval.value);
	} else {
		buf_printf(sb,"(%s)",v1->codeval.value,v2->codeval.value);
	}
	
	r = moto_cloneVal(env, v1);
	r->codeval.value = buf_toString(sb);

	buf_free(sb);

	opstack_push(env, r);

}

static void 
motoc_bitop(const UnionCell* p) {
	MotoEnv *env = moto_getEnv();
	MotoVal *r=NULL,*v1,*v2;
	int op;
	StringBuffer* sb;
	char* cop;
	
	sb = buf_createDefault();
	
	motoc(uc_operand(p, 0));
	motoc(uc_operand(p, 1));
	v2 = opstack_pop(env);
	v1 = opstack_pop(env);
	op = uc_opcode(p);
	
	cop="";

	if (v1->type->kind == INT64_TYPE || v1->type->kind == INT64_TYPE) 
		r = moto_createVal(env, "long",0); 
	else if (v1->type->kind == INT32_TYPE || v1->type->kind == INT32_TYPE)
		r = moto_createVal(env, "int",0);
	else if (v1->type->kind == CHAR_TYPE || v1->type->kind == CHAR_TYPE) 
		r = moto_createVal(env, "char",0);
	else if (v1->type->kind == BYTE_TYPE || v1->type->kind == BYTE_TYPE) 
		r = moto_createVal(env, "byte",0);
	else
		{ THROW_D("MotoException"); } 
		
	switch (op) {
		case BITWISE_AND: cop = "&";break;
		case BITWISE_OR: cop = "|";break;
		case BITWISE_XOR: cop = "^";break;
		default: { THROW_D("MotoOpcodeException"); } break;
	}

	buf_printf(sb,"(%s %s %s)", (char*)v1->codeval.value, cop, (char*)v2->codeval.value);

	r->codeval.value = buf_toString(sb);
	buf_free(sb);

	/* clean up and push result onto opstack */
	opstack_push(env, r);
}

static void 
motoc_bitnot(const UnionCell* p) {
	MotoEnv *env = moto_getEnv();
	MotoVal *r,*v1;
	StringBuffer* sb;
	
	sb = buf_createDefault();
	
	motoc(uc_operand(p, 0));
	v1 = opstack_pop(env);
	
	r = moto_createVal(env, v1->type->name,0); 

	buf_printf(sb,"(~%s)", (char*)v1->codeval.value);

	r->codeval.value = buf_toString(sb);
	buf_free(sb);

	/* clean up and push result onto opstack */
	opstack_push(env, r);
}

static void 
motoc_shiftop(const UnionCell* p) {
	MotoEnv *env = moto_getEnv();
	MotoVal *r,*v1,*v2;
	int op;
	StringBuffer* sb;
	char* cop;
	
	sb = buf_createDefault();
	
	motoc(uc_operand(p, 0));
	motoc(uc_operand(p, 1));
	v2 = opstack_pop(env);
	v1 = opstack_pop(env);
	op = uc_opcode(p);
	
	cop="";

	r = moto_createVal(env, v1->type->name,0); 

	switch (op) {
		case SHIFT_LEFT: cop = "<<";break;
		case SHIFT_RIGHT: cop = ">>";break;
		default: break;
	}

	buf_printf(sb,"(%s %s %s)", (char*)v1->codeval.value, cop, (char*)v2->codeval.value);

	r->codeval.value = buf_toString(sb);
	buf_free(sb);

	/* clean up and push result onto opstack */
	opstack_push(env, r);
}

void 
motoc_cast(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVal *val;
	MotoVal *dup;
	StringBuffer* sb;
	MotoType* type;
	
	/* Extract the cast MotoType */
	type = motox_extractType(uc_operand(p, 0));
	
	/* Evaluate the cast expression */
	/* See if the cast expression is an inline array initializer */
	if (uc_operand(p,1)->type == OP_TYPE && uc_opcode(uc_operand(p,1)) == ARRAY_INIT) {
		
		motoc_arrayInit(uc_operand(p,1),type);
		/* Just leave the MotoVal from arrayInit on the stack as it is guarenteed 
		   suitable for return */
		   
	} else {
		dup = moto_createVal(env, type->name, 0);

		motoc(uc_operand(p, 1));
		val = opstack_pop(env);
			
		sb = buf_createDefault();
		buf_printf(sb,"((%s)(%s))", moto_valToCType(dup),(char*)val->codeval.value);
		dup->codeval.value=buf_toString(sb);
		buf_free(sb);
	
		opstack_push(env, dup);
	}
	
}

void
motoc_classdef(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoClassDefinition* mcd;
	char* classn;
	
	/* Grab the Class name (operand 1) */
	classn = moto_strdup(env, uc_str(p, 0));
	
	/* Get the MotoClassDefinition for this class name */
	mcd = moto_getMotoClassDefinition(env,classn);
	
	/* Set the Environment Variable for the ÔCurrent Class DefinitionÕ 
		to this MotoClassDefinition */
	env->ccdef = mcd;
	
	/* Create a private frame */
	moto_createFrame(env, FN_FRAME);

	/* Generate the C code for the implicit constructor */
	motoc(uc_operand(p, 1));

	/* Set the MotoClassDefinition code to the of the definition frame 
		Luckily this will not have variable declarations
		in it */
	mcd->code = buf_toString(env->frame->out);
	
	/* Free the definition frame */
	moto_freeFrame(env);

	/* Clear the fcodebuffer for the next guy */ 
	/* FIXME!!!: sorry ... I peed in it */
	buf_clear(env->fcodebuffer);
	
	/* UnSet the Environment Variable for the ÔCurrent Class DefinitionÕ */
	env->ccdef = NULL;
}

void 
motoc_comma(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVal *v1, *v2, *r;
	StringBuffer* sb = buf_createDefault();

	motoc(uc_operand(p, 0));
	motoc(uc_operand(p, 1));
	v2 = opstack_pop(env);
	v1 = opstack_pop(env);

	buf_printf(sb,"(%s, %s)",(char*)v1->codeval.value,(char*)v2->codeval.value);

	r = moto_createVal(env, v2->type->name, 0);
	r->codeval.value = buf_toString(sb);

	buf_free(sb);

	opstack_push(env, r);
}

void
motoc_define(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoFunction *f = NULL;
	UnionCell *argListUC,*declaration;
	int i,argc;
	char **argtypes;
	char *fn;
	char* motoname;

	/* Create a 'definition' frame */
	moto_createFrame(env, FN_FRAME);

	/* Generate all the things we need to retrieve the MotoFunction
		for this definition i.e. the fname, the argcount, and the
		argtypes array */

	declaration = uc_operand(p,0);
	if(declaration->type == OP_TYPE)
		fn = uc_str(declaration, 2);
	else
		fn = uc_str(p,0);

	argListUC = uc_operand(p,1);
	argc = uc_opcount(argListUC);

	/* Generate the Moto name for this function / method / or constructor */
	motoname = moto_createMotonameForFn((env->ccdef != NULL?env->ccdef->classn:NULL),fn);

	if(argc==0) {
		argtypes = NULL;
	} else {
		argtypes = emalloc(argc * sizeof(char *));
	}

	/* While we're at it, push arguments onto the definition 
		frame by declaring them */

	for(i=0;i<argc;i++){
		UnionCell* argdec = uc_operand(argListUC,i);
		UnionCell* atype_uc = uc_operand(argdec,0);
		
		int argdecDim;
		char *aname;
		MotoType* atype;
		MotoVar* avar;

		atype = motox_extractType(atype_uc);
		aname = uc_str(argdec,1);
		argdecDim = uc_opcount(uc_operand(argdec, 2));

		/* Declare the argument variable */
		avar = moto_declareVar(env, aname,atype->name, argdecDim, '\0');

		argtypes[i] = avar->type->name;
	}

	/* If this is a method definition then we need to push on all the MotoClassDefinition vars 
		as well as the special 'this' var */
	
	if(env->ccdef != NULL){
		MotoClassDefinition* mcd = env->ccdef;
		
		Enumeration *e = stab_getKeys(mcd->memberTypes);
		while(enum_hasNext(e)){
			char* mname,*mtypen;
			MotoVar* var;
			
			mname= (char*)enum_next(e);
			
			/* Fake variable shadowing by simply not loading in a member var
				with the same name as an argument */
				
			if(moto_getFrameVar(env,mname) != NULL)
				continue; 
				
			mtypen = mcd_getMemberType(mcd,mname);
			
			var = moto_declareVar(env, mname, mtypen, 0, '\0');
			var->isMemberVar = '\1';
		}
		enum_free(e);
		
		/* Declare the special this variable */
		
		moto_declareVar(env , "this", mcd->classn, 0,'\0');
		
	}

	/* Get the function this definition corresponds to */
	ftab_cacheGet(env->ftable, &f, motoname, argc, argtypes);

	/* Generate a new scope ID and push it on the scope ID stack */
	istack_push(env->scopeIDStack,env->curScopeID++);

	/* Generate a holder variable for the return value */
	if(mttab_get(env->types, f->rtype,0)->kind != VOID_TYPE) {
		MotoVal* r = moto_createVal(env, f->rtype,0);
		outputIndent();
		
		/* FIXME: This is probably bad */
		if(env->ccdef != NULL && 
			!strcmp(motoname,moto_createMotonameForFn(env->ccdef->classn,env->ccdef->classn)))
			buf_printf(env->frame->out,"%s _MOTO_RETURN_VALUE=this;\n", 
				moto_valToCType(r));
		else
			buf_printf(env->frame->out,"%s _MOTO_RETURN_VALUE=%s;\n",
				moto_valToCType(r), moto_defaultValForCType(r));
	}
	
	/* Push on a scoping frame */
	moto_createFrame(env, SUB_FRAME);

	/* Generate the C code for the function body */
	motoc(uc_operand(p, 2));

	/* Free the scoping frame */
	moto_freeFrame(env);

	/* Ouput end of scope label */

	outputIndent();
	buf_printf(env->frame->out,"SCOPE_%d_END: ;\n",istack_pop(env->scopeIDStack));
	outputIndent();
	if(mttab_get(env->types, f->rtype,0)->kind != VOID_TYPE) 
		buf_printf(env->frame->out,"_MOTO_ACTION=NOOP_ACTION; return _MOTO_RETURN_VALUE;\n");
	else
		buf_printf(env->frame->out,"_MOTO_ACTION=NOOP_ACTION; return;\n");
	outputIndent();
	
	/* Free the definition frame */
	
	moto_freeFrame(env);
	
	/* Map the function to it's definition in env->fdefs */
	htab_put(env->fdefs,f,buf_toString(env->fcodebuffer));

	/* Clear the fcodebuffer for the next guy */
	buf_clear(env->fcodebuffer);
}

static void 
motoc_declare(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	int i;
	/* the type and name of the id being declared */
	char *varn;
	/* whether it is a global variable being declared */
	char global = '\0';
	MotoVar* var;
	MotoType* type;

	MotoVal *r;
	StringBuffer* sb = buf_createDefault();
	
	UnionCell *type_uc,*type_qualifier_uc,*declarator_list_uc;
	
	/* Extract the type_qualifier, type, and variable declarator list */

	type_qualifier_uc = uc_operand(p,0);
	type_uc = uc_operand(p,1);
	declarator_list_uc = uc_operand(p,2);

	/* Extract the type name and dimension, then retrieve the mototype */
	type = motox_extractType(type_uc);
	
	/* Discover if the type qualifer is "global" */
	global = uc_opcode(type_qualifier_uc) == GLOBAL;
	
	/* Iterate over the declarators */
	
	for(i=0;i<declarator_list_uc->opcell.opcount;i++) {
		UnionCell* declarator_uc = uc_operand(declarator_list_uc,i);
		int vdim;
		
		/* Get the variable name and dimension */
		varn = moto_strdup(env, uc_str(declarator_uc, 0));
		vdim = uc_opcount(uc_operand(declarator_uc, 1));
		
		/* Declare the variable */
		var = moto_declareVar(env, varn, type->name, vdim, global);
	
		/* FIXME: I have never made such an awful hack as this! */
		/* The problem is when outputting the implicit constructor for a moto class I NEED
			to have the is member var thingy set to true! */
		if(env->ccdef != NULL && env->frame->type == FN_FRAME) {
			var->isMemberVar = '\1';
		}

		/* see if the declaration is "fancy" (i.e. it includes an assignment) */
		if (uc_opcode(uc_operand(declarator_uc, 2)) != NOOP) {

			/* See if the assignment is an inline array initializer */
			if (uc_operand(declarator_uc, 2)->type == OP_TYPE 
								 && uc_opcode(uc_operand(declarator_uc, 2)) == ARRAY_INIT) {
				/* If it is, verify that the inlined array elements are of the appropriate
				type and dimensions */
				
				MotoVal *val;
				motoc_arrayInit(uc_operand(declarator_uc, 2),var->type );
				val = opstack_pop(env);

				buf_printf(env->frame->out,
					global ? "_G_%s=%s;\n" : "%s=%s;\n",
					var->n,
					(char*)val->codeval.value
				);

				moto_freeVal(env,val);

			} else {

				/* Declaration includes an assignment */
				MotoVal *val;
	 
				motoc(uc_operand(declarator_uc, 2));
				val = opstack_pop(env);
	 
				outputIndent();
	
				if(var->isMemberVar)
					buf_puts(env->frame->out,"this->");
		
				switch (p->opcell.opcode) {
					case DECLARE_INLINE:
						buf_printf(sb, "(%s=%s)", var->n,(char*)val->codeval.value);
						break;
					case DECLARE: 
					default:
						buf_printf(
							env->frame->out,
							global ? "_G_%s=%s;\n" : "%s=%s;\n",
							var->n,
							(char*)val->codeval.value
						); 
						break;
				}
			}	
		}

		/* For inline declarations push an appropriately typed MotoVal onto the stack */
	   
		if (p->opcell.opcode == DECLARE_INLINE)  {
			r = moto_createVal(env, type->name, 0);
			r->codeval.value = buf_toString(sb);
			buf_free(sb);
			opstack_push(env, r);
		}
		
	}
}

void 
motoc_delete(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVal *r = NULL;
	MotoVal *val = NULL;
	StringBuffer* sb = buf_createDefault();

	motoc(uc_operand(p, 0));
	val = opstack_pop(env);

	buf_printf(sb,"mman_free(%s)",(char*)val->codeval.value);

	/* FIXME: I have my doubts about this assignment ... shouldn't it be a void type ? */
	r = moto_createVal(env, "Object",0);
	r->codeval.value = buf_toString(sb);

	buf_free(sb);
	opstack_push(env, r);
}

static void 
motoc_dereference_lval(const UnionCell *p) {
	motoc_dereference_rval(p);
}

/* Given a class name find and generate code for the destructor function for that class.
 * If not destructor is found generate code identifying STD_FREE_FN */
 
static char*
motoc_genDestructor(char* classn){
	MotoEnv* env = moto_getEnv();
	MotoFunction *destructor;
	int code;
	char* dname;
	
	StringBuffer* sb = buf_createDefault();
	dname = emalloc((2 * strlen(classn)) + 7);
	sprintf(dname, "%s::~%s", classn, classn);
	code = ftab_cacheGet(env->ftable, &destructor, dname, 0, NULL);
	if(code == FTAB_OK) 
		buf_printf(sb,"(void(*)(void *))%s",mfn_csymbol(destructor));
	else 
		buf_printf(sb,"(void(*)(void *))STD_FREE_FN");
	dname= buf_toString(sb);
	buf_free(sb);
	
	return dname;
}

static void 
motoc_dereference_rval(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVal *r = NULL;
	MotoVal *instanceVal = NULL;
	StringBuffer* sb = buf_createDefault();
	MotoClassDefinition* mcd;
	char* varn, *typen;
	
	/* Evaluate Operand 0 to get the motoval associated with the structure */
	motoc(uc_operand(p, 0));
	instanceVal = opstack_pop(env);
	
	/* Retreive the MotoClassDefinition for the type of the associated motoval */
	mcd = moto_getMotoClassDefinition(env,instanceVal->type->name);
		
	/* Get the name of the member variable to dereference to from operand 1 */
	varn = moto_strdup(env, uc_str(p, 1));
	
	/* If the MotoClassDefinition defines no such member variable */
	if(mcd == NULL || mcd_getMemberType(mcd,varn) == NULL){
		MotoFunction* mfn = NULL;
		MotoType* type;
		char* dname = NULL;
		char* fname = moto_createMotonameForFn(instanceVal->type->name,varn);
		
		ftab_fnForName(env->ftable,&mfn,fname);
		type = mfn_type(mfn);
		
		/* Instantate and push onto the stack a motoval of the functional type */
		r = moto_createVal(env, type->name,0);
		
		/* Grab the destructor */
		if(mfn->tracked) dname = motoc_genDestructor(mfn->rtype);
			
		/* Construct the generate C code */
		sb = buf_createDefault();

		buf_printf(sb, "((Function*)mman_track(rtime_getMPool(),func_create(0,\"%s\",&%s,%s)))",
			/* Pass the name of the function to be pushed onto the trace stack */
				mfn->motoname,
			/* Pass the function pointer */
				motoc_genAnonymousFunction(mfn,type,0,NULL,NULL),
			/* Pass the code value for self if this is a method */
				mfn_isMethod(mfn)?instanceVal->codeval.value:"NULL" 
		);	
		r->codeval.value = buf_toString(sb);
		
	} else {
		/* Otherwise, create the return val of appropriate type and dimension */	 
		typen = mcd_getMemberType(mcd,varn);
		r = moto_createVal(env, typen,0);
		
		/* Set the codeval of the return val to instanceVal.code + '->' + op2 name */
		buf_printf(sb,"((%s)CHK_NULL_D(%s))->%s",
			moto_valToCType(instanceVal),(char*)instanceVal->codeval.value,varn);
		
		r->codeval.value=buf_toString(sb);
		buf_free(sb);
	}
	
	opstack_push(env, r);
}

static void 
motoc_do(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVal *val;

	motoc(p->opcell.operands[0]);
	val = opstack_pop(env);

	outputIndent();
	buf_printf(env->frame->out,"%s;\n",(char*)val->codeval.value);
}

static void 
motoc_elseif(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVal *val;

	motoc(uc_operand(p, 0));
	val = opstack_pop(env);

	outputIndent();
	buf_printf(env->frame->out,"}\n",(char*)val->codeval.value);
	outputIndent();
	buf_printf(env->frame->out,"else if(%s) {\n",(char*)val->codeval.value);

	moto_createFrame(env, SUB_FRAME);
	motoc(uc_operand(p, 1));
	moto_freeFrame(env);
}

static void 
motoc_elseiflist(const UnionCell *p) {

	motoc(uc_operand(p, 0));
	motoc(uc_operand(p, 1));
}

static void 
motoc_excp_throw(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVal *exceptionVal;
	
	/* Evalute the expression being thrown */
	motoc(uc_operand(p, 0));
	exceptionVal = opstack_pop(env);

	/* Output excp_marshal(expression,__FILE__,__LINE__); */
	outputIndent();
	buf_printf(env->frame->out,"_MOTO_ACTION=NOOP_ACTION,excp_marshal((%s),__FILE__,__LINE__);\n",
		(char*)exceptionVal->codeval.value);
}


static void 
motoc_excp_try(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	
	UnionCell *tryBlockCode = uc_operand(p,0);
	UnionCell *handlers = uc_operand(p,1);
	UnionCell *catchBlocks = uc_operand(handlers,0);
	UnionCell *finallyBlockCode = uc_operand(handlers,1);

	int i;
	
	/* Output "TRY {" */
	outputIndent();
	buf_printf(env->frame->out,"TRY {\n");
	
	/* Generate a new scope ID and push it on the scope ID stack */
	istack_push(env->scopeIDStack,env->curScopeID++);
	
	/* Push on a new sub-frame */
	moto_createFrame(env, SUB_FRAME);
	
	/* Generate the 'try' code */
	motoc(tryBlockCode);
	
	/* Pop off the sub-frame */
	moto_freeFrame(env);
	
	/* Ouput end of scope label */
	outputIndent();
	buf_printf(env->frame->out,"SCOPE_%d_END: ;\n",istack_pop(env->scopeIDStack));
	
	/* Output '}' */
	outputIndent();
	buf_printf(env->frame->out,"}\n");
	
	/* Foreach catch block */
	for(i=0;i<uc_opcount(catchBlocks);i++) {
		UnionCell* currentCatchBlock = uc_operand(catchBlocks,i);
		char* exceptionTypeName = uc_str(currentCatchBlock,0);
		char* exceptionName = uc_str(currentCatchBlock,1);
		UnionCell* catchBlockCode = uc_operand(currentCatchBlock,2);
		
		/* Output "CATCH("<Exception Type>") {" or "CATCH_ALL {" if the exception type
			is 'Exception' */
		
		outputIndent();	
		if(!strcmp(exceptionTypeName,"Exception")) {
			buf_printf(env->frame->out,"CATCH_ALL {\n");
		} else {
			buf_printf(env->frame->out,"CATCH(\"%s\") {\n",exceptionTypeName);
		}
		
		/* Generate a new scope ID and push it on the scope ID stack */
		istack_push(env->scopeIDStack,env->curScopeID++);
		
		/* Push on a new sub-frame */
		moto_createFrame(env, SUB_FRAME);
					
		/* Declare the exception variable in the catch block */
		moto_declareVar(env, exceptionName, exceptionTypeName, 0,'\0');
		
		/* Output an assignment of the current exception to this var */
		outputIndent();
		buf_printf(env->frame->out,"%s=excp_getCurrentException();\n",exceptionName);
		
		/* Generate the current 'catch' code */
		motoc(catchBlockCode);
		
		/* Pop off the sub-frame */
		moto_freeFrame(env);
		
		/* Ouput end of scope label */
		outputIndent();
		buf_printf(env->frame->out,"SCOPE_%d_END: ;\n",istack_pop(env->scopeIDStack));
	
		/* Output '}' */
		outputIndent();
		buf_printf(env->frame->out,"}\n");
	}
	
	/* Output "FINALLY {" */
	outputIndent();
	buf_printf(env->frame->out,"FINALLY {\n");
	
	/* Generate a new scope ID and push it on the scope ID stack */
	istack_push(env->scopeIDStack,env->curScopeID++);
	
	/* Push on a new sub-frame */
	moto_createFrame(env, SUB_FRAME);
		
	/* Generate the 'finally' code */
	motoc(finallyBlockCode);
	
	/* Pop off the sub-frame */
	moto_freeFrame(env);
	
	/* Ouput end of scope label */
	outputIndent();
	buf_printf(env->frame->out,"SCOPE_%d_END: ;\n",istack_pop(env->scopeIDStack));

	/* Output jump to Parent Scope */
	if(istack_size(env->scopeIDStack) > 0) {
		outputIndent();
		buf_printf(env->frame->out,"if(!TRYBLK.code && _MOTO_ACTION)\n");
		outputIndent();
		buf_printf(env->frame->out,"	 {excp_try_i--;goto SCOPE_%d_END;}\n", istack_peek(env->scopeIDStack));
	}
			
	/* Output "} END_TRY" */
	outputIndent();
	buf_printf(env->frame->out,"} END_TRY\n");
	
}

/* Peek at argc motovals on the stack and fill the argv and types
	arrays with the appropriate values */

static void 
motoc_fillArgs(int argc,void** argv, char** types){
	int i;
	MotoVal* val;
	MotoFrame* frame= moto_getEnv()->frame;

	for (i = 1; i <= argc; i++) {
		val = (MotoVal *)stack_peekAt(frame->opstack, i);
		argv[argc - i] = val->codeval.value;
		types[argc - i] = val->type->name;
	}
}

static void 
motoc_callDDF(MotoVal* self, MotoVal *callee, UnionCell * arglistUC){
	MotoEnv *env = moto_getEnv();
	MotoVal *r;
	StringBuffer *sb = buf_createDefault();
	int i,argc;
	void *argv[20];
	char *types[20];

	/* Motoc the arguments */
	motoc(arglistUC);
	argc = arglistUC->opcell.opcount;
	motoc_fillArgs(argc,argv,types);
	
	/* Generate the cast to the appropriate functional type */
	buf_puts(sb,"(((");
	
	/* FIXME: ick, so since void returning hofs could actually be returning 
		objects we better fudge the return type */
		
	buf_puts(sb,callee->type->atype->kind != VOID_TYPE ? mtype_toCType(callee->type->atype) : "void*");
	
	buf_puts(sb,"(*)(Function*");
	for(i=0;i<callee->type->argc;i++){
		buf_puts(sb,",");
		buf_puts(sb,mtype_toCType(callee->type->argt[i]));
	}
	buf_printf(sb,"))CURRENT_FUNCTION->fn)");
	
	/* Generate the function call itself */
	buf_puts(sb,"(CURRENT_FUNCTION");
	for (i = 0; i < argc; i++) 
		buf_printf(sb,",%s",argv[i]);
	buf_puts(sb,")");
	
	/* FIXME: I HATE the fact that the functions in ctype return ints! It pisses me off! 
		(unsigned char)isalpha('A') should not be null!!! damnit damnit damnit */
	if(callee->type->atype->kind == BOOLEAN_TYPE)
		buf_printf(sb,">0)");
	else
		buf_printf(sb,")");
	
	r = moto_createVal(env,callee->type->atype->name,0);
	r->codeval.value = buf_toString(sb);
	
	/* Wrap function call with mman_trackf if the function returns a tracked retun value ...
		FIXME: This code is problematic ... if one of the arguments is another virtual function 
		call, it won't work (the free function will be wrong) */
		
	buf_clear(sb);

	buf_printf(sb,"(%s)",r->codeval.value);
   
	r->codeval.value = buf_toString(sb);
	
	buf_clear(sb);
	/* Wrap function call with stack trace maintenance code */
	buf_printf(sb,"(CURRENT_FUNCTION=%s,CHK_NULL_C(CURRENT_FUNCTION),",callee->codeval.value);
	buf_printf(sb,"ST_PUSH(CURRENT_FUNCTION->fname,ST_FUNCTION),");
	switch(r->type->kind){
		case BOOLEAN_TYPE: buf_puts(sb,"stacktrace_bpop("); break;
		case BYTE_TYPE: buf_puts(sb,"stacktrace_ypop("); break;
		case CHAR_TYPE: buf_puts(sb,"stacktrace_cpop("); break;
		case INT32_TYPE: buf_puts(sb,"stacktrace_ipop("); break;
		case INT64_TYPE: buf_puts(sb,"stacktrace_lpop("); break;
		case FLOAT_TYPE: buf_puts(sb,"stacktrace_fpop("); break;
		case DOUBLE_TYPE: buf_puts(sb,"stacktrace_dpop("); break;
		case REF_TYPE: buf_puts(sb,"stacktrace_rpop("); break;
		case VOID_TYPE: buf_puts(sb,"("); break;
		default: THROW_D("MotoException"); break;
	}
	/* Generate the function call itself */
	buf_printf(sb,"%s",r->codeval.value);

	/* Close out the stacktrace wrapping */
	if(r->type->kind == VOID_TYPE) buf_printf(sb,"),stacktrace_vpop())");
	else buf_printf(sb,"))");

	r->codeval.value = buf_toString(sb);
	buf_free(sb);
		   
	/* Free MotoVals used in call */
	for (i = 0; i < argc; i++) 
		moto_freeVal(env,opstack_pop(env));
	
	opstack_push(env,r);
}

MotoVal*
motoc_callee(const UnionCell* p, MotoVal* self){
	MotoEnv *env = moto_getEnv();
	MotoVal* callee;
	UnionCell* calleeUC = uc_operand(p, uc_opcode(p) == METHOD ?1:0 );
		
	if ( uc_opcode(p) == METHOD) {
		/* Retreive the MotoClassDefinition for the type of the associated motoval */
		MotoClassDefinition* mcd = moto_getMotoClassDefinition(env,self->type->name);
		char* typen = mcd_getMemberType(mcd,calleeUC->valuecell.value);
		StringBuffer* sb = buf_createDefault();
			
		callee = moto_createVal(env, typen,0);

		/* Set the codeval of the return val to instanceVal.code + '->' + op2 name */
		buf_printf(sb,"((%s)CHK_NULL_D(%s))->%s",
			moto_valToCType(self),(char*)self->codeval.value,calleeUC->valuecell.value);

		callee->codeval.value=buf_toString(sb);
		buf_free(sb);
	} else {
		/* Motov the expression being called */
		motoc(calleeUC);
		callee = opstack_pop(env);
	}
	
	return callee;
}

MotoVal* 
motoc_callADF(MotoVal* self, void** argv,MotoFunction* f, int freevals){
	MotoEnv *env = moto_getEnv();
	MotoVal * r;
	int i,argc=f->argc;
	int isMethod = mfn_isMethod(f);
	StringBuffer * sb,* mb;

	if(isMethod){

		/* surround "self" with CHK_NULL */
		mb=buf_createDefault();
		buf_printf(mb,"((%s)CHK_NULL_M(%s))",
				moto_valToCType(self),self->codeval.value);

		/* shift arguments to make room for the self pointer */
		for (i = argc - 1; i >= 0; i--) argv[i + 1] = argv[i];
		argv[0] = buf_toString(mb);
		argc++;
		buf_free(mb);
	}

	/* call function */
	sb=buf_createDefault();

	/* allocate the function return value */
	r = moto_createVal(env, f->rtype,0);
		
	/* Wrap function call with mman trackf if the function returns a tracked retun value */
	if(f->tracked)
		buf_printf(sb,"((%s)mman_trackf(rtime_getMPool(),",
			moto_valToCType(r));
	
	/* Wrap function call with stack trace maintenance code */
	buf_printf(sb,"(ST_PUSH(\"%s\",%s),",f->motoname,isMethod?"ST_METHOD":"ST_FUNCTION");
	
	switch(r->type->kind){
		case BOOLEAN_TYPE: buf_puts(sb,"stacktrace_bpop("); break;
		case BYTE_TYPE: buf_puts(sb,"stacktrace_ypop("); break;
		case CHAR_TYPE: buf_puts(sb,"stacktrace_cpop("); break;
		case INT32_TYPE: buf_puts(sb,"stacktrace_ipop("); break;
		case INT64_TYPE: buf_puts(sb,"stacktrace_lpop("); break;
		case FLOAT_TYPE: buf_puts(sb,"stacktrace_fpop("); break;
		case DOUBLE_TYPE: buf_puts(sb,"stacktrace_dpop("); break;
		case REF_TYPE: buf_puts(sb,"stacktrace_rpop("); break;
		case VOID_TYPE: buf_puts(sb,"("); break;
		default: THROW_D("MotoException"); break;
	}
		
	/* Generate the function call itself */
	buf_printf(sb,"%s(",mfn_csymbol(f));
	for (i = 0; i < argc; i++) 
		buf_printf(sb,"%s%s",i!=0?",":"",argv[i]);
	buf_printf(sb,")");
		
	/* FIXME: I HATE the fact that the functions in ctype return ints! It pisses me off! 
	   (unsigned char)isalpha('A') should not be null!!! damnit damnit damnit */
	if(r->type->kind == BOOLEAN_TYPE)
		buf_printf(sb,">0");
	
	/* Close out the stacktrace wrapping */
	if(r->type->kind == VOID_TYPE) buf_printf(sb,"),stacktrace_vpop())");
	else buf_printf(sb,"))");
	   
	/* Close out the trackf wrapping */
	if(f->tracked) buf_printf(sb,",%s))",motoc_genDestructor(f->rtype));
	
	r->codeval.value = buf_toString(sb);
		
	buf_free(sb);
	
	if(isMethod) argc--;

	/* Free MotoVals used in call */
	if (freevals > 0)
		for (i = 0; i < argc; i++)
			moto_freeVal(env,opstack_pop(env));
		
	return r;
	
}

void 
motoc_fn(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	int argc;					/* Number of arguments to the function */
	void **argv = NULL;		/* The code for each argument to the function */
	char **types = NULL;		/* The type of each argument to the fucntions */

	MotoFunction *f = NULL;
	MotoFrame *frame = NULL;
	MotoVal *r = NULL;
	MotoVal *self = NULL;	/* Value representing the object itself */
	char *fbasename;
	int calleeIsDDF = 0;
	
	UnionCell* arglistUC = uc_operand(p, 1 + (uc_opcode(p) == METHOD?1:0) );

	frame = env->frame;

	/* If this is a method call evaluate the self operand */
	
	if(uc_opcode(p) == METHOD) {
		motoc(uc_operand(p, 0));
		self = opstack_pop(env);	
	} else if (uc_opcode(p) == FN && moto_getVar(env,"this") != NULL) {
		/* This could still be a method call, lets get a self value just in case */
		self = moto_getVarVal(env,moto_getVar(env,"this"));
		self->codeval.value=estrdup("this");
	}
	
	/* Figure out if the callee is a dynamically defined function (DDF) */
	calleeIsDDF = motox_calleeIsDDF(p,self);
	
	if(calleeIsDDF) {
		/* Retrieve the callee */
		MotoVal* callee = motoc_callee(p,self);
		
		motoc_callDDF(self,callee,arglistUC);
		
		moto_freeVal(env,callee);
	} else {		

		/* Get the base function name */
		fbasename = uc_str(p, 0 + (uc_opcode(p) == METHOD?1:0));
		
		motoc(arglistUC);
		argc = arglistUC->opcell.opcount;
		types = mman_malloc(env->mpool,(argc + 1) * sizeof(char *));
		argv = mman_malloc(env->mpool, (argc + 1) * sizeof(char *));
		motoc_fillArgs(argc,argv,types);
		
		/* Look up the function or method being identified */
		motox_lookupMethodOrFn(&f, self,fbasename, argc, types, uc_opcode(p));

		/* Call function or method */
		r = motoc_callADF(self, argv, f, 1);
		
		stack_push(frame->opstack, r);
	}
}

void 
motoc_for(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVal *val;
	char *vinit,*vcond,*vinc;


	outputIndent();
	buf_printf(env->frame->out,"{\n");	
	
	moto_createFrame(env, SUB_FRAME);
	
	if (uc_opcode(uc_operand(p, 0)) == NOOP) {
		vinit = "";
	}
	else {
		motoc(uc_operand(p, 0));
		val = opstack_pop(env);
		vinit = val->codeval.value;
	}
	
	if (uc_opcode(uc_operand(p, 1)) == NOOP) {
		vcond = "";
	}
	else {
		motoc(uc_operand(p, 1));
		val = opstack_pop(env);
		vcond = val->codeval.value;
	}

	if (uc_opcode(uc_operand(p, 2)) == NOOP) {
		vinc = "";
	}
	else {
		motoc(uc_operand(p, 2));
		val = opstack_pop(env);
		vinc = val->codeval.value;
	}

	outputIndent();
	buf_printf(env->frame->out,"for(%s;%s;%s){\n", vinit, vcond, vinc);

	/* Generate a new scope ID and push it on the scope ID stack */
	istack_push(env->scopeIDStack,env->curScopeID++);
	
	if (uc_opcode(uc_operand(p, 3)) != NOOP) {
		moto_createFrame(env, SUB_FRAME);
		motoc(uc_operand(p, 3));
		moto_freeFrame(env);
	}

	/* Pop off and ouput end of scope label */
	outputIndent();
	buf_printf(env->frame->out,"	 if(0) {\n");
	outputIndent();
	buf_printf(env->frame->out,"		 SCOPE_%d_END: ;\n",istack_pop(env->scopeIDStack));
	outputIndent();
	buf_printf(env->frame->out,"		 if(_MOTO_ACTION == BREAK_ACTION){_MOTO_ACTION==NOOP_ACTION;break;}\n");
	outputIndent();
	buf_printf(env->frame->out,"		 else if(_MOTO_ACTION == CONTINUE_ACTION){_MOTO_ACTION==NOOP_ACTION;continue;}\n");
	/* Output jump to Parent Scope */
	if(istack_size(env->scopeIDStack) > 0) {
		outputIndent();
		buf_printf(env->frame->out,"		 else {goto SCOPE_%d_END;}\n", istack_peek(env->scopeIDStack));
	}
	outputIndent();
	buf_printf(env->frame->out,"	 }\n");

	outputIndent();
	buf_printf(env->frame->out,"}\n");
	
	moto_freeFrame(env);
 	
	outputIndent();
	buf_printf(env->frame->out,"}\n");
	

}

/* 
 * Generate an anonymous function of type aftype, to call f and return its C identifier 
 */
 
static char* 
motoc_genComposedFunction(MotoType* ftype,MotoType* aftype,int pargc,int* pargi,FuncTypeKind* pargt){
	MotoEnv* env = moto_getEnv();
	/* Generate an anonymous function for identification */
	int i,j;
	char* afname; 
	MotoFunction* af;
	StringBuffer* sb = buf_createDefault();
	char** argtypes = emalloc(sizeof(char*) * (aftype->argc+1));
	char** argnames = emalloc(sizeof(char*) * (aftype->argc+1));
	
	buf_clear(sb);
	buf_printf(sb,"_%s_%d","ANON",env->constantCount);
	afname=buf_toString(sb);
	env->constantCount++;
	
	/* FIXME: Ok ... its not sooo bad ... but it is definitely kinda bad */
	argtypes[0] = "int()"; /* We need something that will force the C prototype to emit a function* */
	argnames[0] = "_F_";
	
	for(i=1;i<aftype->argc+1;i++){
		argtypes[i] = aftype->argt[i-1]->name;
		buf_clear(sb);
		buf_printf(sb,"%s%d","_ARG_",i);
		argnames[i] = buf_toString(sb);
	}
		
	af = mfn_create(afname,aftype->atype->name,aftype->argc+1,argtypes,
		argnames,NULL, '\0', NULL,NULL, NULL, NULL 
	);
	
	/* Generate the code for the anonymous function */
	buf_clear(sb);

	/* If this anonymous function returns anything, output 'return' */
	if(aftype->atype->kind != VOID_TYPE) buf_puts(sb,"return ");

	/* Output the return type of the parent function call */
	buf_printf(sb,"((%s",mtype_toCType(ftype->atype));

	/* Output the arg types expected by the parent function call */
	buf_puts(sb,"(*)(Function*");
	for(i=0;i<ftype->argc;i++) buf_printf(sb,",%s",mtype_toCType(ftype->argt[i]));
	
    buf_puts(sb,"))_F_->parent->fn)");
	
	/* Generate the function call itself */
	
	buf_puts(sb,"(_F_->parent");
			
	for (i=0,j=0; i+j<ftype->argc; i++) {
		while (j<pargc && pargi[j] == i+j) { 
			buf_putc(sb,',');
			switch(pargt[j]){
				case F_BOOLEAN_TYPE: buf_printf(sb,"barg(_F_,%d)",j); break;
				case F_BYTE_TYPE: buf_printf(sb,"yarg(_F_,%d)",j); break;
				case F_CHAR_TYPE: buf_printf(sb,"carg(_F_,%d)",j); break;
				case F_INT32_TYPE: buf_printf(sb,"iarg(_F_,%d)",j);break;
				case F_INT64_TYPE: buf_printf(sb,"larg(_F_,%d)",j); break;
				case F_FLOAT_TYPE: buf_printf(sb,"farg(_F_,%d)",j); break;
				case F_DOUBLE_TYPE: buf_printf(sb,"darg(_F_,%d)",j); break;
				case F_REF_TYPE: buf_printf(sb,"rarg(_F_,%d)",j); break;
				default: THROW_D("MotoException");
			};
			j++ ;
		}
		if(i+j == ftype->argc) break;
		buf_putc(sb,',');
		buf_printf(sb,"_ARG_%d",i+1);
	}
		
	buf_puts(sb,")");
	
	/* FIXME: I HATE the fact that the functions in ctype return ints! It pisses me off! 
	   (unsigned char)isalpha('A') should not be null!!! damnit damnit damnit */
	if(ftype->atype->kind == BOOLEAN_TYPE)
		buf_printf(sb,">0");

	buf_puts(sb,";");
	
	/* Add the anonymous function and its definition to env->adefs */
	htab_put(env->adefs,af, buf_toString(sb));
	buf_free(sb);
	
	return mfn_csymbol(af);
}

/* 
 * Generate an anonymous function of type aftype, to call f and return its C identifier 
 */
 
static char* 
motoc_genAnonymousFunction(MotoFunction* f,MotoType* aftype,int pargc,int* pargi,FuncTypeKind* pargt){
	MotoEnv* env = moto_getEnv();
	/* Generate an anonymous function for identification */
	int i,j;
	char* afname; 
	MotoFunction* af;
	StringBuffer* sb = buf_createDefault();
	char** argtypes = emalloc(sizeof(char*) * (aftype->argc+1));
	char** argnames = emalloc(sizeof(char*) * (aftype->argc+1));
	
	buf_clear(sb);
	buf_printf(sb,"_%s_%d","ANON",env->constantCount);
	afname=buf_toString(sb);
	env->constantCount++;
	
	/* FIXME: Ok ... its not sooo bad ... but it is definitely kinda bad */
	argtypes[0] = "int()"; /* We need something that will force the C prototype to emit a function* */
	argnames[0] = "_F_";
	
	for(i=1;i<aftype->argc+1;i++){
		argtypes[i] = aftype->argt[i-1]->name;
		buf_clear(sb);
		buf_printf(sb,"%s%d","_ARG_",i);
		argnames[i] = buf_toString(sb);
	}
		
	af = mfn_create(afname,aftype->atype->name,aftype->argc+1,argtypes,
		argnames,NULL, '\0', NULL,NULL, NULL, NULL 
	);
	
	/* Generate the code for the anonymous function */
	buf_clear(sb);

	/* If this anonymous function returns anything, output 'return' */
	if(aftype->atype->kind != VOID_TYPE) buf_puts(sb,"return ");

	/* If the function this anonymous function is calling returns something that's tracked
	   wrap function call with mman trackf if the function returns a tracked retun value */
	if(f->tracked)
		buf_printf(sb,"((%s)mman_trackf(rtime_getMPool(),",
			mtype_toCType(mttab_get(env->types,f->rtype,0)));
	
	/* Generate the function call itself */
	buf_printf(sb,"%s(",mfn_csymbol(f));

	if(mfn_isMethod(f))
		buf_puts(sb,"_F_->self");
			
	for (i=0,j=0; i+j<f->argc; i++) {
		while (j<pargc && pargi[j] == i+j) { 
			if(i+j>0 || mfn_isMethod(f)) buf_putc(sb,',');
			switch(pargt[j]){
				case F_BOOLEAN_TYPE: buf_printf(sb,"barg(_F_,%d)",j); break;
				case F_BYTE_TYPE: buf_printf(sb,"yarg(_F_,%d)",j); break;
				case F_CHAR_TYPE: buf_printf(sb,"carg(_F_,%d)",j); break;
				case F_INT32_TYPE: buf_printf(sb,"iarg(_F_,%d)",j);break;
				case F_INT64_TYPE: buf_printf(sb,"larg(_F_,%d)",j); break;
				case F_FLOAT_TYPE: buf_printf(sb,"farg(_F_,%d)",j); break;
				case F_DOUBLE_TYPE: buf_printf(sb,"darg(_F_,%d)",j); break;
				case F_REF_TYPE: buf_printf(sb,"rarg(_F_,%d)",j); break;
				default: THROW_D("MotoException");
			};
			j++ ;
		}
		if(i+j == f->argc) break;
		if(i+j>0 || mfn_isMethod(f)) buf_putc(sb,',');
		buf_printf(sb,"_ARG_%d",i+1);
	}
		
	buf_puts(sb,")");
	
	/* FIXME: I HATE the fact that the functions in ctype return ints! It pisses me off! 
	   (unsigned char)isalpha('A') should not be null!!! damnit damnit damnit */
	if(mttab_get(env->types,f->rtype,0)->kind == BOOLEAN_TYPE)
		buf_printf(sb,">0");
   
   /* Close out the trackf wrapping */
	if(f->tracked) buf_printf(sb,",%s))",motoc_genDestructor(f->rtype));

	buf_puts(sb,";");
	
	/* Add the anonymous function and its definition to env->adefs */
	htab_put(env->adefs,af, buf_toString(sb));
	buf_free(sb);
	
	return mfn_csymbol(af);
}

void motoc_identify(const UnionCell *wrapper) {
	MotoEnv *env = moto_getEnv();
	char* fbasename;
	UnionCell* p,* arglistUC;
	UnionCell* fbasenameUC;	
	char *types[20]; 
	int i,argc;
	StringBuffer * sb;
	int pargc = 0,pargi[20]; 	/* The number of args that are 'known' and their indexes */
	MotoVal* pargv[20]; 		/* The 'known' argument moto vals */
	FuncTypeKind pargt[20];		/* The 'known' argument moto val types */	
	MotoType* ftype,*rftype;
	MotoFunction* f;
	MotoVal* r, *self = NULL;
	char* pargicode, *pargtcode, *pargvcode;
	int calleeIsDDF = 0;
	MotoVal* callee = NULL;

	p = uc_operand(wrapper,0);
		
	fbasenameUC = uc_operand(p,0 + (uc_opcode(p) == METHOD ? 1:0));
	arglistUC = uc_operand(p,1 + (uc_opcode(p) == METHOD ? 1:0));

	/* If this is a method call evaluate the self operand */
	if(uc_opcode(p) == METHOD) {
		motoc(uc_operand(p, 0));
		self = opstack_pop(env);
		sb = buf_createDefault();
		buf_printf(sb,"((%s)CHK_NULL_M(%s))",
				moto_valToCType(self),self->codeval.value);
		self->codeval.value=buf_toString(sb);		
	} else if (uc_opcode(p) == FN && moto_getVar(env,"this") != NULL) {
		/* This could still be a method call, lets get a self value just in case */
		self = moto_getVarVal(env,moto_getVar(env,"this"));
		self->codeval.value=estrdup("this");
	}
	
	argc = uc_opcount(arglistUC);
	
	/* For each argument */
	for(i=0;i<argc;i++){
		UnionCell* argUC;
		
		/* Extract and argument type */
		argUC = uc_operand(arglistUC,i);
		if(uc_opcode(argUC) == TYPED_UNKNOWN)
			types[i] = motox_extractType(uc_operand(argUC,0))->name;
		else if (uc_opcode(argUC) == UNTYPED_UNKNOWN)
			types[i] = NULL;
		else {
			/* Generare the argument and retrieve its type */
			MotoVal* aval;
			motoc(argUC);
			aval = opstack_pop(env);
			types[i] = aval->type->name;

			/* Add the 'known' argument index to pargi, val to pargc, and increment pargc */
			pargi[pargc] = i;
			pargv[pargc] = aval;
			pargt[pargc] = aval->type->kind;
			pargc++;
		}					
	}

	/* Figure out if the callee is a dynamically defined function (DDF) */
	calleeIsDDF = motox_calleeIsDDF(p,self);
	
	if(calleeIsDDF){
	
		/* Retrieve the callee */
		callee = motoc_callee(p,self);
		
		/* Retrieve the DDF's type */
		ftype = callee->type;
		
	} else {
	
		/* Extract the function or method name */
		fbasename = fbasenameUC->valuecell.value;
			
		/* Look up the function or method being identified */
		motox_lookupMethodOrFn(&f, self,fbasename, argc, types, uc_opcode(p));
		
		/* Retrieve the function's type */
		ftype = mfn_type(f);
	}
	
	/* Generate a new type by removing any applied arguments from the function's type */
	rftype = mttab_apply(env->types,ftype,pargc,pargi);
		
	/* Instantate and push onto the stack a motoval of the functional type */
	r = moto_createVal(env, rftype->name,0);
	
	sb = buf_createDefault();
	/* Generate C code for the partial argument index array */
	buf_puts(sb,"(");
	for(i=0;i<pargc;i++) 
		buf_printf(sb,"_PARGI_[%d]=%d,",i,pargi[i]);
	buf_puts(sb,"_PARGI_)");
	pargicode = buf_toString(sb);
	
	/* Generate C code for the partial argument types array */
	buf_clear(sb);
	buf_puts(sb,"(");
	for(i=0;i<pargc;i++){
		switch(pargt[i]){
			case F_BOOLEAN_TYPE: buf_printf(sb,"_PARGT_[%d]=F_BOOLEAN_TYPE,",i); break;
			case F_BYTE_TYPE: buf_printf(sb,"_PARGT_[%d]=F_BYTE_TYPE,",i); break;
			case F_CHAR_TYPE: buf_printf(sb,"_PARGT_[%d]=F_CHAR_TYPE,",i); break;
			case F_INT32_TYPE: buf_printf(sb,"_PARGT_[%d]=F_INT32_TYPE,",i); break;
			case F_INT64_TYPE: buf_printf(sb,"_PARGT_[%d]=F_INT64_TYPE,",i); break;
			case F_FLOAT_TYPE: buf_printf(sb,"_PARGT_[%d]=F_FLOAT_TYPE,",i); break;
			case F_DOUBLE_TYPE: buf_printf(sb,"_PARGT_[%d]=F_DOUBLE_TYPE,",i); break;
			case F_VOID_TYPE: buf_printf(sb,"_PARGT_[%d]=F_VOID_TYPE,",i); break;
			case F_REF_TYPE: buf_printf(sb,"_PARGT_[%d]=F_REF_TYPE,",i); break;
		};
	}
	buf_puts(sb,"_PARGT_)");

	pargtcode = buf_toString(sb);

	/* Generate C code for the partial argument values array */
	buf_clear(sb);

	for(i=0;i<pargc;i++)
		buf_printf(sb,",%s",pargv[i]->codeval.value);
		
	pargvcode = buf_toString(sb);

	/* Generate C code for the partial function creation*/
	buf_clear(sb);
	
	if(!calleeIsDDF){
		buf_printf(sb, "((Function*)mman_track(rtime_getMPool(),func_createPartialWithValues(0,\"%s\",&%s,%s,%s,%d,%s,%s%s)))",
			/* Pass the name of the function to be pushed onto the trace stack */
				f->motoname,
			/* Pass the function pointer */
				motoc_genAnonymousFunction(f,rftype,pargc,pargi,pargt),
			/* Pass the parent function pointer */					
				"NULL",	
			/* Pass the code value for self if this is a method */
				mfn_isMethod(f)?self->codeval.value:"NULL" ,
			/* Pass the number of arguments already filled in */
				pargc ,
			/* Pass the indexes of the arguments already filled in */	
				pargicode,
			/* Pass the types of the arguments already filled in */	
				pargtcode,
			/* Pass the values of the arguments already filled in */	
				pargvcode			
		);	
	} else {
		buf_printf(sb, "((Function*)mman_track(rtime_getMPool(),func_createPartialWithValues(0,\"%s\",&%s,%s,%s,%d,%s,%s%s)))",
			/* Pass the name of the function to be pushed onto the trace stack */
				"<Anonymous>",
			/* Pass the function pointer */
				motoc_genComposedFunction(ftype,rftype,pargc,pargi,pargt),
			/* Pass the parent function pointer */	
				callee->codeval.value,	
			/* Pass the code value for self if this is a method */
				"NULL" ,
			/* Pass the number of arguments already filled in */
				pargc ,
			/* Pass the indexes of the arguments already filled in */	
				pargicode,
			/* Pass the types of the arguments already filled in */	
				pargtcode,
			/* Pass the values of the arguments already filled in */	
				pargvcode			
		);	
	}
	
	r->codeval.value = buf_toString(sb);
			
	buf_free(sb);
	if(self != NULL)
		moto_freeVal(env,self);
		
	opstack_push(env, r);
}

void 
motoc_list(const UnionCell *p) {
	int i;

	for(i=0;i<p->opcell.opcount;i++)
		motoc(p->opcell.operands[i]);
}

int 
escapedStrlen(char* s){
	int i,j, size = strlen(s);

	for (i=0,j=0;i<size;j++,i++) 
		if(s[i]=='\\') i++;

	return j;
}

static void 
motoc_id(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	char *varn = p->valuecell.value;
	MotoVar *var = moto_getVar(env, varn);
	MotoVal* val;
	StringBuffer* sb = buf_createDefault();
	
	if(var != NULL){
		val = moto_cloneVal(env,var->vs);
	 
		/* For class member vars prepend 'this->' */
		if(var->isMemberVar)
			buf_puts(sb,"this->");	
		
		/* For global vars prepend '_G_' */
		if(moto_isVarGlobal(env,var->n))
			buf_puts(sb,"_G_");
			
		buf_puts(sb,var->n);
		
		val->codeval.value = buf_toString(sb);
	} else {
		MotoFunction* mfn = NULL;
		MotoType* type;
		char *fname,* varn = p->valuecell.value,* dname = NULL;
		MotoVal* self = NULL;
		int code = FTAB_NONE;


		/* Check to see if a method with this name is defined */
		if (moto_getVar(env,"this") != NULL) {
			/* Get the self MotoVal */
			self = moto_getVarVal(env,moto_getVar(env,"this"));
			
			/* Generate the Moto name for a possible method */
			fname = moto_createMotonameForFn(self->type->name, varn);
	
			/* Retrieve the MXFN record */
			code = ftab_fnForName(env->ftable,&mfn,fname);
			free(fname);
		} 
		
		/* If there is no method, see if there is a function with this name */
		if(code == FTAB_NONE) ftab_fnForName(env->ftable,&mfn,varn);
		
		type = mfn_type(mfn);
		
		/* Instantate and push onto the stack a motoval of the functional type */
		val = moto_createVal(env, type->name,0);
		
		/* Grab the destructor */
		if(mfn->tracked) dname = motoc_genDestructor(mfn->rtype);
			
		/* Construct the generate C code */
		sb = buf_createDefault();
		buf_printf(sb, "((Function*)mman_track(rtime_getMPool(),func_create(0,\"%s\",&%s,%s)))",
			/* Pass the name of the function to be pushed onto the trace stack */
			mfn->motoname, 				
			/* Pass the function pointer */
			motoc_genAnonymousFunction(mfn,type,0,NULL,NULL),	
			/* Pass the code value for self if this is a method */
			mfn_isMethod(mfn)?"this":"NULL"
		);
		
		val->codeval.value = buf_toString(sb);
	}
	buf_free(sb);
	
	opstack_push(env, val);
}

static void 
motoc_if(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVal *val;

	motoc(uc_operand(p, 0));

	val = opstack_pop(env);
  
	outputIndent();
	buf_printf(env->frame->out,"if(%s) {\n",(char*)val->codeval.value);

	/* if expression */
	motoc(uc_operand(p, 0));
	val = opstack_pop(env);

	/* if statement block */
	if (uc_opcode(uc_operand(p, 1)) != NOOP) {
		moto_createFrame(env, SUB_FRAME);
		motoc(uc_operand(p, 1));
		moto_freeFrame(env);
	}

	/* else if statement block */
	if (uc_opcode(uc_operand(p, 2)) != NOOP) {
		motoc(uc_operand(p, 2));
		outputIndent();
		buf_printf(env->frame->out,"}\n");
	}
	
	/* else statement block */
	if (uc_opcode(uc_operand(p, 3)) != NOOP) {
		if (uc_opcode(uc_operand(p, 2)) == NOOP) {
			outputIndent();
			buf_puts(env->frame->out,"}\n"); 
		}
		outputIndent();
		buf_printf(env->frame->out,"else {\n"); 
		moto_createFrame(env, SUB_FRAME);
		motoc(uc_operand(p, 3));
		moto_freeFrame(env);
		outputIndent();
		buf_printf(env->frame->out,"}\n");
	} 

	/* make sure there's a closing paren */
	if (uc_opcode(uc_operand(p, 2)) == NOOP && 
		 uc_opcode(uc_operand(p, 3)) == NOOP) {
		outputIndent();
		buf_puts(env->frame->out,"}\n"); 
	}
}

static void 
motoc_jump(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	outputIndent();
	if(uc_opcode(p) == BREAK)
		buf_printf(env->frame->out,"{_MOTO_ACTION=BREAK_ACTION; TRYBLK.code = 0; goto SCOPE_%i_END;}\n",
			istack_peek(env->scopeIDStack));
	else
		buf_printf(env->frame->out,"{_MOTO_ACTION=CONTINUE_ACTION; TRYBLK.code = 0; goto SCOPE_%i_END;}\n",
			istack_peek(env->scopeIDStack));
}

static void 
motoc_print(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVal* val;

	motoc(uc_operand(p, 0));
	val= opstack_pop(env);

	outputIndent();

	switch (val->type->kind) {

		case BOOLEAN_TYPE:
			buf_printf(env->frame->out,"buf_puts(out,%s?\"true\":\"false\");\n",(char*)val->codeval.value);
			break;
		case BYTE_TYPE:
			buf_printf(env->frame->out,"buf_puty(out,%s);\n",(char*)val->codeval.value); 
			break;
		case CHAR_TYPE:
			buf_printf(env->frame->out,"buf_putc(out,%s);\n",(char*)val->codeval.value); 
			break;
		case INT32_TYPE:	
			buf_printf(env->frame->out,"buf_puti(out,%s);\n",(char*)val->codeval.value); 
			break;
		case INT64_TYPE:	
			buf_printf(env->frame->out,"buf_putl(out,%s);\n",(char*)val->codeval.value); 
			break;
		case FLOAT_TYPE:	
			buf_printf(env->frame->out,"buf_putf(out,%s);\n",(char*)val->codeval.value); 
			break;
		case DOUBLE_TYPE: 
			buf_printf(env->frame->out,"buf_putd(out,%s);\n",(char*)val->codeval.value); 
			break;
		case REF_TYPE:
			if (isType("String", val)) {	/* Static String */
				if(uc_operand(p, 0)->type == VALUE_TYPE && uc_operand(p, 0)->valuecell.kind != ID_VALUE) {

					if(strlen((char*)val->codeval.value) > 1024){ /* FIXME : Nasty bug fix to print out giant blocks
																		of static text */

						char* curpos = unescapeStrConst((char*)val->codeval.value);
						while(strlen(curpos) > 1023){
							char tmp[1024];
							strncpy(tmp,curpos,1023);
							tmp[1023] = '\0';
							buf_printf(env->frame->out,"buf_put(out,\"%s\",%d);\n",
								escapeStrConst(tmp),1023);
							curpos += 1023;
						}
						buf_printf(env->frame->out,"buf_put(out,\"%s\",%d);\n",
							escapeStrConst(curpos),strlen(curpos));
					} else {
						buf_printf(env->frame->out,"buf_put(out,%s,%d);\n",
							(char*)val->codeval.value,escapedStrlen((char*)val->codeval.value)-2);
					}
				} else
					buf_printf(env->frame->out,"buf_puts(out,%s);\n",(char*)val->codeval.value);
			} else if (isArray(val) && val->type->atype->kind == CHAR_TYPE && val->type->dim == 1){			
				buf_printf(env->frame->out,"buf_putca(out,%s);\n",(char*)val->codeval.value);
			} else if (isArray(val) && val->type->atype->kind == BYTE_TYPE && val->type->dim == 1){			
				buf_printf(env->frame->out,"buf_putya(out,%s);\n",(char*)val->codeval.value);
			} else if (!strcmp(val->type->name,"null")) {
				buf_puts(env->frame->out, "buf_puts(out,\"null\");\n");
			} else
				buf_printf(env->frame->out,"buf_printf(out,\"[%s: %p]\");\n",
					val->type->name,(char*)val->codeval.value);
			break;
		default:
			buf_printf(env->frame->out,"buf_printf(out,\"[%s: %p]\");",
				val->type->name,(char*)val->codeval.value);
			break;
	}
}

static void 
motoc_math(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVal *v1;
	MotoVal *v2;
	int op = uc_opcode(p);

	motoc(uc_operand(p, 0));
	motoc(uc_operand(p, 1));
	v2 = opstack_pop(env);
	v1 = opstack_pop(env);
	if(motox_try_overloaded_function_op(op,v1,v2) != MOTO_OK) {
		motoc_domath(v1, v2, op);
	}
}

void 
motoc_new(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	int i;
	int code;						  /* Possible error code from function lookup */
	char *fname = NULL;			  /* The moto function name to be looked up */

	int argc;						  /* Number of arguments to the function */
	void **argv = NULL;			  /* The code for each argument to the function */
	char **types = NULL;			  /* The type of each argument to the fucntions */

	MotoFunction *f = NULL;
	MotoFrame *frame = NULL;
	MotoVal *r = NULL;
	StringBuffer* sb;

	char *classname = NULL;
	MotoType *type = NULL;

	UnionCell* arglistUC = uc_operand(p, 1 );

	frame = env->frame;

	classname = uc_str(p, 0);

	/* Fill in the arguments */
	motoc(arglistUC);	 /* argument list */
	argc = arglistUC->opcell.opcount;
	argv = mman_malloc(env->mpool, argc * sizeof(char *));
	types = mman_malloc(env->mpool, argc * sizeof(char *));
	motoc_fillArgs(argc,argv,types);
	
	/* Generate the Moto name for this Constructor */
	fname = moto_createMotonameForFn(classname,classname);
	
	/* Retrieve the MXFN record */
	code = ftab_cacheGet(env->ftable, &f, fname, argc, types);

	free(fname);

	/* make sure this type is registered */
	type = mttab_get(env->types, classname,0);

	/* Generate constructor call */

	sb=buf_createDefault();

	r = moto_createVal(env,classname,0);

	/* Are we dealing with an MDC ? */
	if (moto_getMotoClassDefinition(env,classname)!= NULL) {
		
		/* If so we could be dealing with an implicit constructor */	
		if (f==NULL) {
			/* FIXME: Not sure if we should to do trace stack maintenance here or not */
			buf_printf(sb,"_%s_%s___()",classname,classname);
		} else {
		
			/* Wrap function call with stack trace maintenance code */
			buf_printf(sb,"(ST_PUSH(\"%s\",%s),",f->motoname,"ST_METHOD");
			buf_puts(sb,"stacktrace_rpop("); 
		   
			buf_printf(sb,"%s(_%s_%s___()",mfn_csymbol(f),classname,classname);
			
			for (i = 0; i < argc; i++)
				buf_printf(sb,",%s",argv[i]);
			buf_printf(sb,")");			
		
			/* Close out the stacktrace wrapping */
			buf_printf(sb,"))");
			
		}
	} else {
		if(f->tracked)
			buf_printf(sb,"((%s)mman_trackf(rtime_getMPool(),",
				moto_valToCType(r));
	
		/* Wrap function call with stack trace maintenance code */
		buf_printf(sb,"(ST_PUSH(\"%s\",%s),",f->motoname,"ST_METHOD");
		switch(r->type->kind){
			case BOOLEAN_TYPE: buf_puts(sb,"stacktrace_bpop("); break;
			case BYTE_TYPE: buf_puts(sb,"stacktrace_ypop("); break;
			case CHAR_TYPE: buf_puts(sb,"stacktrace_cpop("); break;
			case INT32_TYPE: buf_puts(sb,"stacktrace_ipop("); break;
			case INT64_TYPE: buf_puts(sb,"stacktrace_lpop("); break;
			case FLOAT_TYPE: buf_puts(sb,"stacktrace_fpop("); break;
			case DOUBLE_TYPE: buf_puts(sb,"stacktrace_dpop("); break;
			case REF_TYPE: buf_puts(sb,"stacktrace_rpop("); break;
			case VOID_TYPE: buf_puts(sb,"("); break;
			default: THROW_D("MotoException"); break;
		}
	
		buf_printf(sb,"%s(",mfn_csymbol(f));
		for (i = 0; i < argc; i++)
			buf_printf(sb,"%s%s",i!=0?",":"",argv[i]);
		buf_printf(sb,")");
	
		/* Close out the stacktrace wrapping */
		if(r->type->kind == VOID_TYPE) buf_printf(sb,"),stacktrace_vpop())");
		else buf_printf(sb,"))");
   
   	/* Close out the trackf wrapping */
		if(f->tracked) buf_printf(sb,",%s))",motoc_genDestructor(f->rtype));
	}
	
	/* r->type = type; */
	r->codeval.value = buf_toString(sb);

	buf_free(sb);

	/* free MotoVals used in call */

	for (i = 0; i < argc; i++) {
		stack_pop(frame->opstack);
	}

	stack_push(frame->opstack, r);


}

void 
motoc_not(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVal *val,*r;
	StringBuffer* sb = buf_createDefault();

	motoc(uc_operand(p, 0));
	val = opstack_pop(env);

	buf_printf(sb,"(!%s)",(char*)val->codeval.value);

	r = moto_createVal(env, "boolean",0);
	r->codeval.value = buf_toString(sb);
	buf_free(sb);

	opstack_push(env, r);
}

void 
motoc_or(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVal *v1, *v2, *r;
	StringBuffer* sb = buf_createDefault();

	motoc(uc_operand(p, 0));
	motoc(uc_operand(p, 1));
	v2 = opstack_pop(env);
	v1 = opstack_pop(env);

	buf_printf(sb,"(%s||%s)",(char*)v1->codeval.value,(char*)v2->codeval.value);

	r = moto_createVal(env, "boolean",0);
	r->codeval.value = buf_toString(sb);

	buf_free(sb);

	opstack_push(env, r);
}

void 
motoc_incdec(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVal *val,*r = NULL;
	StringBuffer* sb = buf_createDefault();
	int incdec = uc_opcode(p);

	motoc(uc_operand(p, 0));
	val = opstack_pop(env);

	/* If this is NOT an overloaded operator ... */
	if(motox_try_overloaded_method_op(incdec,val,NULL,NULL) != MOTO_OK) {	
	
		switch (val->type->kind) {
			case BYTE_TYPE: r = moto_createVal(env, "byte",0); break;
			case CHAR_TYPE: r = moto_createVal(env, "char",0); break;
			case INT32_TYPE: r = moto_createVal(env, "int",0); break;
			case INT64_TYPE: r = moto_createVal(env, "long",0); break;
			case FLOAT_TYPE: r = moto_createVal(env, "float",0); break;
			case DOUBLE_TYPE: r = moto_createVal(env, "double",0); break;
			default: THROW_D("MotoException"); break;
		}

		if (incdec == POSTFIX_INC) 
			buf_printf(sb,"(%s++)",val->codeval.value);
		else if(incdec == POSTFIX_DEC)
			buf_printf(sb,"(%s--)",val->codeval.value);
		else if (incdec == PREFIX_INC) 
			buf_printf(sb,"(++%s)",val->codeval.value);
		else /* if(incdec == PREFIX_DEC) */
			buf_printf(sb,"(--%s)",val->codeval.value);
	
		moto_freeVal(env,val);	 
		r->codeval.value=buf_toString(sb);

		buf_free(sb);
	
		opstack_push(env,r);
	}

}

static void 
motoc_regex(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVal *v1,*v2,*r;
	StringBuffer* sb = buf_createDefault();

	motoc(uc_operand(p, 0));
	motoc(uc_operand(p, 1));

	v2 = opstack_pop(env);
	v1 = opstack_pop(env);

	r = moto_createVal(env, "boolean",0);

	switch (uc_opcode(p)) {
		case RX_MATCH:
			buf_printf(sb,"(regex_matchesSubstring(%s,%s))",(char*)v2->codeval.value,(char*)v1->codeval.value);
			break;
		case RX_NOT_MATCH:
			buf_printf(sb,"(!regex_matchesSubstring(%s,%s))",(char*)v2->codeval.value,(char*)v1->codeval.value);
			break;
	}

	r->codeval.value=buf_toString(sb);
	buf_free(sb);
	opstack_push(env, r);


}

static void 
motoc_rel(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVal *v1;
	MotoVal *v2;
	int op = uc_opcode(p);

	motoc(uc_operand(p, 0));
	motoc(uc_operand(p, 1));
	v2 = opstack_pop(env);
	v1 = opstack_pop(env);

	if(motox_try_overloaded_function_op(op,v1,v2) != MOTO_OK) {
		motoc_dorel(v1, v2, op);
	}

}

static void
motoc_return(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVal *val = NULL;

	if(uc_opcount(p) != 0) {
		motoc(uc_operand(p, 0));
		val = opstack_pop(env);
	
		outputIndent();
		buf_printf(env->frame->out,"{_MOTO_RETURN_VALUE=%s;_MOTO_ACTION=RETURN_ACTION; TRYBLK.code = 0;goto SCOPE_%i_END;}\n",
			(char*)val->codeval.value,istack_peek(env->scopeIDStack));
	} else {
		outputIndent();
		buf_printf(env->frame->out,"{_MOTO_ACTION=RETURN_ACTION; TRYBLK.code = 0;goto SCOPE_%i_END;}\n",
			istack_peek(env->scopeIDStack));
	}
}

void 
motoc_value(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVal *val = NULL;
	StringBuffer *sb;

	sb = buf_createDefault();



	switch (p->valuecell.kind) {
		case NULL_VALUE:
			val = moto_createVal(env,"null",0);
			buf_printf(sb, "NULL");
			break;
		case BOOLEAN_VALUE:
			val = moto_createVal(env, "boolean",0);
			if (*((char *)p->valuecell.value)) {
				buf_printf(sb,"'\\1'");
			} else {
				buf_printf(sb,"'\\0'");
			}
			break;
		case CHAR_VALUE:
			val = moto_createVal(env, "char",0);
			buf_printf(sb,"%s",p->valuecell.code);
			break;
		case INT32_VALUE:
			val = moto_createVal(env, "int",0);
			buf_printf(sb,"%s",p->valuecell.code);
			break;
		case INT64_VALUE:
			val = moto_createVal(env, "long",0);
			buf_printf(sb,"%s",p->valuecell.code);
			break;
		case FLOAT_VALUE:
			val = moto_createVal(env, "float",0);
			buf_printf(sb,"%s",p->valuecell.code);
			break;
		case DOUBLE_VALUE:
			val = moto_createVal(env, "double",0);
			buf_printf(sb,"%s",p->valuecell.code);
			break;
		case REGEX_VALUE:
			 val = moto_createVal(env,"Regex",0);
			buf_printf(sb,
				"((Regex*)mman_trackf(rtime_getMPool(),regex_create(\"%s\"),(void(*)(void *))regex_free))",
				escapeStrConst(p->valuecell.value));
			break;
		case STRING_VALUE:
			val = moto_createVal(env,"String",0);
			
			/* WARNING: This cannot be changed to buf_printf! buf_printf
				has a 4k limit when replacing string constants */

			buf_putc(sb,'"');
			buf_puts(sb,escapeStrConst(p->valuecell.value));
			buf_putc(sb,'"');
			
			break;
		default:
			THROW_D("MotoCellTypeException");
			break;
	}
	val->codeval.value = buf_toString(sb);
	buf_free(sb);

	opstack_push(env, val);


}

void 
motoc_scope(const UnionCell *p){
	MotoEnv *env = moto_getEnv();

	outputIndent();
	buf_printf(env->frame->out,"{\n");

	moto_createFrame(env, SUB_FRAME);
	motoc(uc_operand(p, 0));
	moto_freeFrame(env);

	outputIndent();
	buf_printf(env->frame->out,"}\n");
}

void 
motoc_switch(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVal *switchExp;
	StringBuffer* identifier = NULL; 
	UnionCell *caseStatementListUC;
	SymbolTable* caseVals = NULL;
	int i;
	
	/* Generate code for the expression to switch on */
	motoc(uc_operand(p, 0));
	switchExp = opstack_pop(env);

	outputIndent();

	if (isString(switchExp)) {
		identifier = buf_createDefault();
		buf_printf(identifier,"C%d",env->constantCount);
		env->constantCount += 1;

		buf_printf(
			env->frame->out,
			"switch(mjtab_strmatch(&%s,%s)) {\n",
			buf_data(identifier), 
			(char*)switchExp->codeval.value
		);
		
		/* We generate code for jumptables to switch on strings */
		sset_add(env->includes, "\"jumptable.h\"");
		caseVals = stab_createDefault();
	} else {
		buf_printf(env->frame->out,"switch(%s) {\n",(char*)switchExp->codeval.value);
	}

	moto_createFrame(env, SUB_FRAME);
		
	/* Generate code for each of the case blocks */
	
	caseStatementListUC = uc_operand(p, 2);
	for(i=0;i<caseStatementListUC->opcell.opcount;i++){
		UnionCell *caseStatementUC = uc_operand(caseStatementListUC,i);
		MotoVal* caseExp;
		
		/* Since we are looping over cases ourselves we need to set the
		global meta info properly */
		
		MetaInfo curmeta = env->errmeta;
		env->errmeta = *uc_meta(caseStatementUC);
				
		/* a case with an expression to test */
	
		motoc(uc_operand(caseStatementUC, 0));
		caseExp = opstack_pop(env);
	
		outputIndent();
	
		if (isString(caseExp)) {
			int numCaseVals = stab_size(caseVals);
			
			/* FIXME: this whole int* thing is nasty */
			int* i = emalloc(sizeof(int));
			*i=numCaseVals;
			buf_printf(env->frame->out,"case %d: {\n",*i);
			stab_put(caseVals,unescapeStrConst((char*)caseExp->codeval.value),i);
		} else {
			buf_printf(env->frame->out,"case %s: {\n",(char*)caseExp->codeval.value);
		} 
	
		/* Generate code for this particular case block */
		moto_createFrame(env, SUB_FRAME);
		motoc(uc_operand(caseStatementUC, 1));
		moto_freeFrame(env);
	
		outputIndent();
		buf_printf(env->frame->out,"} break;\n");
		
		/* Re-set the global err meta */
		env->errmeta = curmeta;
	}

	/* Output the code for the default block */
	if (uc_opcode(uc_operand(p, 1)) != NOOP) {
		outputIndent();
		buf_puts(env->frame->out,"default: {\n");
		moto_createFrame(env, SUB_FRAME);
		motoc(uc_operand(p, 1));
		moto_freeFrame(env);
		outputIndent();
		buf_puts(env->frame->out,"} break;\n");
	}
	
	/* If the switch type was a string, generate code for the jumptable */
	if (isString(switchExp)) {
		char** caseValArray;
		int numCaseVals;
		Enumeration* e; 
		JumpTable* jtab;
		MJTable* mjtab;

		numCaseVals = stab_size(caseVals);
		caseValArray = numCaseVals > 0 ? emalloc(sizeof(char*) * numCaseVals) : NULL;

		e=stab_getKeys(caseVals);
		while(enum_hasNext(e)){
			char* curCaseVal = (char*)enum_next(e);
			int* i = stab_get(caseVals, curCaseVal);

			caseValArray[*i] = curCaseVal;
			free(i);
		}
		enum_free(e);

		stab_free(caseVals);

		jtab = jtab_create(numCaseVals,caseValArray);
		mjtab = mjtab_create(jtab);

		mjtab_generateCode( mjtab,env->constantPool,buf_data(identifier));
		
		buf_free(identifier);
		jtab_free(jtab);
		/* mjtab_free(mjtab); */
	}

	moto_freeFrame(env);

	outputIndent();
	buf_printf(env->frame->out,"}\n");

	

}

void 
motoc_ternary(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVal *valc,*valt,*valf,*val;
	StringBuffer * sb = buf_createDefault();

	motoc(uc_operand(p, 0));
	valc = opstack_pop(env);
	motoc(uc_operand(p, 1));
	valt = opstack_pop(env);
	motoc(uc_operand(p, 2));
	valf = opstack_pop(env);

	buf_printf(sb,"(%s?%s:%s)",(char*)valc->codeval.value,(char*)valt->codeval.value,(char*)valf->codeval.value);

	val = moto_cloneVal(env, valt);
	val->codeval.value = buf_toString(sb);

	buf_free(sb); 

	opstack_push(env, val);
}

void 
motoc_uminus(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVal *val,*r;
	StringBuffer *sb = buf_createDefault();

	motoc(uc_operand(p, 0));
	val = opstack_pop(env);

	buf_printf(sb,"(-%s)",(char*)val->codeval.value);
	
	r = moto_createVal(env, val->type->name,0);
	r->codeval.value = buf_toString(sb);
	buf_free(sb);

	opstack_push(env, r);
}

void 
motoc_use(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	char *usename = uc_str(p, 0);


	moto_use(env, usename);


}

void 
motoc_while(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVal *v;


	motoc(uc_operand(p, 0));
	v = opstack_pop(env);

	outputIndent();
	buf_printf(env->frame->out,"while(%s){\n",(char*)v->codeval.value);

	/* Generate a new scope ID and push it on the scope ID stack */
	istack_push(env->scopeIDStack,env->curScopeID++);
	
	if (uc_opcode(uc_operand(p, 1)) != NOOP) {
		moto_createFrame(env, SUB_FRAME);
		motoc(uc_operand(p, 1));
		moto_freeFrame(env);
	}

	/* Pop off and ouput end of scope label */
	outputIndent();
	buf_printf(env->frame->out,"	 if(0) {\n");
	outputIndent();
	buf_printf(env->frame->out,"		 SCOPE_%d_END: ;\n",istack_pop(env->scopeIDStack));
	outputIndent();
	buf_printf(env->frame->out,"		 if(_MOTO_ACTION == BREAK_ACTION){_MOTO_ACTION==NOOP_ACTION;break;}\n");
	outputIndent();
	buf_printf(env->frame->out,"		 else if(_MOTO_ACTION == CONTINUE_ACTION){_MOTO_ACTION==NOOP_ACTION;continue;}\n");
	/* Output jump to Parent Scope */
	if(istack_size(env->scopeIDStack) > 0) {
		outputIndent();
		buf_printf(env->frame->out,"		 else {goto SCOPE_%d_END;}\n", istack_peek(env->scopeIDStack));
	}
	outputIndent();
	buf_printf(env->frame->out,"	 }\n");
	
	outputIndent();
	buf_printf(env->frame->out,"}\n");


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

static void 
motoc_dorel(MotoVal *v1, MotoVal *v2, int op) {
	MotoEnv *env = moto_getEnv();
	MotoVal *r;

	StringBuffer* sb = buf_createDefault();



	if (v1->type->kind == REF_TYPE || v2->type->kind == REF_TYPE) {

		if (op == REL_EQ_M) {
			buf_printf(sb,"(%s==%s)",(char*)v1->codeval.value,(char*)v2->codeval.value);

		} else if (op == REL_NE_M) {
			buf_printf(sb,"(%s!=%s)",(char*)v1->codeval.value,(char*)v2->codeval.value);

		} else if (isString(v1) || isString(v2)) {

			if (isString(v1) && isString(v2)) {
				switch (op) {
					case REL_EQ_S: 
						buf_printf(sb,"(estrcmp(%s,%s)==0)",(char*)v1->codeval.value,(char*)v2->codeval.value);
						break;
					case REL_NE_S: 
						buf_printf(sb,"(estrcmp(%s,%s)!=0)",(char*)v1->codeval.value,(char*)v2->codeval.value); 
						break;
					case REL_GT_S:
						buf_printf(sb,"(estrcmp(%s,%s)>0)",(char*)v1->codeval.value,(char*)v2->codeval.value); 
						break;
					case REL_GTE_S: 
						buf_printf(sb,"(estrcmp(%s,%s)>=0)",(char*)v1->codeval.value,(char*)v2->codeval.value); 
						break;
					case REL_LT_S: 
						buf_printf(sb,"(estrcmp(%s,%s)<0)",(char*)v1->codeval.value,(char*)v2->codeval.value); 
						break;
					case REL_LTE_S: 
						buf_printf(sb,"(estrcmp(%s,%s)<=0)",(char*)v1->codeval.value,(char*)v2->codeval.value); 
						break;
				} 
			}
		}
	} else if (v1->type->kind == BOOLEAN_TYPE || v2->type->kind == BOOLEAN_TYPE) {
		if (v1->type->kind == v2->type->kind) {
			switch (op) {
				case REL_EQ_M: 
					buf_printf(sb,"(%s==%s)",(char*)v1->codeval.value,(char*)v2->codeval.value);
					break;
				case REL_NE_M: 
					buf_printf(sb,"(%s!=%s)",(char*)v1->codeval.value,(char*)v2->codeval.value);
					break;
			}
		}
	} else { 

		char* cop="";
		switch (op) {
			case REL_EQ_M: cop = "==";break;
			case REL_NE_M: cop = "!=";break;
			case REL_GT_M: cop = ">";break;
			case REL_GTE_M: cop = ">=";break;
			case REL_LT_M: cop = "<";break;
			case REL_LTE_M: cop = "<=";break;
		}

#ifdef CAST_INT64_TO_INT32_FIRST
		buf_printf(sb,"(%s%s%s%s%s)",
			v1->type->kind==INT64_TYPE && v2->type->kind==FLOAT_TYPE ? "(int32_t)":"",
			(char*)v1->codeval.value,
			cop,
			v1->type->kind==FLOAT_TYPE && v2->type->kind==INT64_TYPE ? "(int32_t)":"",
			(char*)v2->codeval.value
		);
#else		 
		buf_printf(sb,"(%s%s%s)", (char*)v1->codeval.value, cop, (char*)v2->codeval.value);
#endif

	}

	r = moto_createVal(env, "boolean",0);
	r->codeval.value = buf_toString(sb);
	buf_free(sb);

	/* push result onto opstack */
	opstack_push(env, r);

}

static void 
motoc_domath(MotoVal *v1, MotoVal *v2, int op) {
	MotoEnv *env = moto_getEnv();
	MotoVal *r=NULL;
	StringBuffer* sb;

	sb = buf_createDefault();



	if (op == MATH_ADD && (isString(v1) || isString(v2))) {
		r = moto_createVal(env,"String",0);

		buf_puts(sb,"(mxstr_mstrcat(");
		
		switch(v1->type->kind){
			case BOOLEAN_TYPE:
				buf_printf(sb,"(char*)mman_track(rtime_getMPool(),boolean_toString(%s))",(char*)v1->codeval.value); break;
			case BYTE_TYPE:
				buf_printf(sb,"(char*)mman_track(rtime_getMPool(),byte_toString(%s))",(char*)v1->codeval.value); break;
			case CHAR_TYPE:
				buf_printf(sb,"(char*)mman_track(rtime_getMPool(),char_toString(%s))",(char*)v1->codeval.value); break;
			case INT32_TYPE:
				buf_printf(sb,"(char*)mman_track(rtime_getMPool(),int_toString(%s))",(char*)v1->codeval.value); break;
			case INT64_TYPE:
				buf_printf(sb,"(char*)mman_track(rtime_getMPool(),long_toString(%s))",(char*)v1->codeval.value); break;
			case FLOAT_TYPE:
				buf_printf(sb,"(char*)mman_track(rtime_getMPool(),float_toString(%s))",(char*)v1->codeval.value); break;
			case DOUBLE_TYPE:
				buf_printf(sb,"(char*)mman_track(rtime_getMPool(),double_toString(%s))",(char*)v1->codeval.value); break;
			case REF_TYPE:
			default:
				if (isType("String", v1) || isType("null",v1)) 
					buf_puts(sb,(char*)v1->codeval.value);
				else if (isArray(v1) && v1->type->atype->kind == CHAR_TYPE && v1->type->dim == 1)
					buf_printf(sb,"(char*)mman_track(rtime_getMPool(),carr_toString(%s))",(char*)v1->codeval.value); 
				else if (isArray(v1) && v1->type->atype->kind == BYTE_TYPE && v1->type->dim == 1)
					buf_printf(sb,"(char*)mman_track(rtime_getMPool(),yarr_toString(%s))",(char*)v1->codeval.value);			
				else 
					buf_printf(sb, "mxstr_mstrcat(mxstr_mstrcat(\"[%s: \",(char*)mman_track(rtime_getMPool(),obj_toString(%s))),\"]\")",
						v1->type->name,v1->codeval.value);

			break;
			
		}
		
		buf_putc(sb,',');
		
		switch(v2->type->kind){
			case BOOLEAN_TYPE:
				buf_printf(sb,"(char*)mman_track(rtime_getMPool(),boolean_toString(%s))",(char*)v2->codeval.value); break;
			case BYTE_TYPE:
				buf_printf(sb,"(char*)mman_track(rtime_getMPool(),byte_toString(%s))",(char*)v2->codeval.value); break;
			case CHAR_TYPE:
				buf_printf(sb,"(char*)mman_track(rtime_getMPool(),char_toString(%s))",(char*)v2->codeval.value); break;
			case INT32_TYPE:
				buf_printf(sb,"(char*)mman_track(rtime_getMPool(),int_toString(%s))",(char*)v2->codeval.value); break;
			case INT64_TYPE:
				buf_printf(sb,"(char*)mman_track(rtime_getMPool(),long_toString(%s))",(char*)v2->codeval.value); break;
			case FLOAT_TYPE:
				buf_printf(sb,"(char*)mman_track(rtime_getMPool(),float_toString(%s))",(char*)v2->codeval.value); break;
			case DOUBLE_TYPE:
				buf_printf(sb,"(char*)mman_track(rtime_getMPool(),double_toString(%s))",(char*)v2->codeval.value); break;
			case REF_TYPE:
			default:
				if (isType("String", v2) || isType("null",v2)) 
					buf_puts(sb,(char*)v2->codeval.value);
				else if (isArray(v2) && v2->type->atype->kind == CHAR_TYPE && v2->type->dim == 1)
					buf_printf(sb,"(char*)mman_track(rtime_getMPool(),carr_toString(%s))",(char*)v2->codeval.value); 
				else if (isArray(v2) && v2->type->atype->kind == BYTE_TYPE && v2->type->dim == 1)
					buf_printf(sb,"(char*)mman_track(rtime_getMPool(),yarr_toString(%s))",(char*)v2->codeval.value);	
				else 
					buf_printf(sb, "mxstr_mstrcat(mxstr_mstrcat(\"[%s: \",(char*)mman_track(rtime_getMPool(),obj_toString(%s))),\"]\")",
						v2->type->name,v2->codeval.value);

			break;
		}
		
		buf_puts(sb,"))");

	}
	else {
		char* cop="";

		if (v1->type->kind == DOUBLE_TYPE || v2->type->kind == DOUBLE_TYPE) 
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
		else
			{ THROW_D("MotoException"); }
			
		switch (op) {
			case MATH_ADD: cop = "+";break;
			case MATH_SUB: cop = "-";break;
			case MATH_MULT: cop = "*";break;
			case MATH_DIV: cop = "/";
				{
					StringBuffer* tbuf = buf_createDefault();
					switch(v2->type->kind){
						case DOUBLE_TYPE: buf_printf(tbuf,"(excp_file=__FILE__,excp_line=__LINE__,inline_checkForDoubleZero(%s))",v2->codeval.value);break;
						case FLOAT_TYPE: buf_printf(tbuf,"(excp_file=__FILE__,excp_line=__LINE__,inline_checkForFloatZero(%s))",v2->codeval.value);break;
						case INT64_TYPE: buf_printf(tbuf,"(excp_file=__FILE__,excp_line=__LINE__,inline_checkForLongZero(%s))",v2->codeval.value);break;
						case INT32_TYPE: buf_printf(tbuf,"(excp_file=__FILE__,excp_line=__LINE__,inline_checkForIntZero(%s))",v2->codeval.value);break;
						case CHAR_TYPE: buf_printf(tbuf,"(excp_file=__FILE__,excp_line=__LINE__,inline_checkForCharZero(%s))",v2->codeval.value);break;
						case BYTE_TYPE: buf_printf(tbuf,"(excp_file=__FILE__,excp_line=__LINE__,inline_checkForByteZero(%s))",v2->codeval.value);break;
						default : break;
					}
					v2->codeval.value = buf_toString(tbuf);
				}
				break;
			case MATH_MOD: cop = "%";break;
			default : { THROW_D("MotoOpcodeException"); } break;
		}

#ifdef CAST_INT64_TO_INT32_FIRST
		buf_printf(sb,"(%s%s %s %s%s)",
			v1->type->kind==INT64_TYPE && v2->type->kind==FLOAT_TYPE ? "(int32_t)":"",
			(char*)v1->codeval.value,
			cop,
			v1->type->kind==FLOAT_TYPE && v2->type->kind==INT64_TYPE ? "(int32_t)":"",
			(char*)v2->codeval.value
		);
#else 
		buf_printf(sb,"(%s %s %s)", (char*)v1->codeval.value, cop, (char*)v2->codeval.value);
#endif
 
	}

	r->codeval.value = buf_toString(sb);
	buf_free(sb);

	/* clean up and push result onto opstack */

	opstack_push(env, r);
}

/*=============================================================================
// end of file: $RCSfile: motoc.c,v $
==============================================================================*/

