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

	$RCSfile: motoi.c,v $	
	$Revision: 1.116 $
	$Author: scorsi $
	$Date: 2003/06/14 15:05:32 $

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
#include <setjmp.h>

#include "memsw.h"

#include "exception.h"
#include "stacktrace.h"
#include "excpfn.h"
#include "stringbuffer.h"
#include "symboltable.h"
#include "stringset.h"
#include "stack.h"
#include "vector.h"
#include "log.h"
#include "mxstring.h"
#include "runtime.h"
#include "cdx_function.h"
#include "cdx_regex.h"

#include "motox.h"
#include "motolex.h"
#include "moto.tab.h"
#include "motoutil.h"
#include "motoerr.h"
#include "motoi.h"
#include "dl.h"
#include "env.h"
#include "motohook.h"
#include "mxarr.h"

static void motoi_and(const UnionCell *p);
static void motoi_array_lval(const UnionCell *p);
static void motoi_array_rval(const UnionCell *p);
static void motoi_array_new(const UnionCell *p);
static void motoi_assign(const UnionCell *p);
static void motoi_bitop(const UnionCell *p);
static void motoi_bitnot(const UnionCell *p);
static void motoi_cast(const UnionCell *p);
static void motoi_classdef(const UnionCell *p);
static void motoi_comma(const UnionCell *p);
static void motoi_do(const UnionCell *p);
static void motoi_declare(const UnionCell *p);
static void motoi_define(const UnionCell *p);
static void motoi_dereference_lval(const UnionCell *p);
static void motoi_dereference_rval(const UnionCell *p);
static void motoi_delete(const UnionCell *p);
static void motoi_elseif(const UnionCell *p);
static void motoi_elseiflist(const UnionCell *p);
static void motoi_excp_throw(const UnionCell *p);
static void motoi_excp_try(const UnionCell *p);
static void motoi_identify(const UnionCell *p);
static void motoi_fn(const UnionCell *p);
static void motoi_for(const UnionCell *p);
static void motoi_id(const UnionCell *p);
static void motoi_if(const UnionCell *p);
static void motoi_jump(const UnionCell *p);
static void motoi_list(const UnionCell *p);
static void motoi_new(const UnionCell *p);
static void motoi_not(const UnionCell *p);
static void motoi_or(const UnionCell *p);
static void motoi_incdec(const UnionCell *p);
static void motoi_print(const UnionCell *p);
static void motoi_regex(const UnionCell *p);
static void motoi_scope(const UnionCell *p);
static void motoi_shift(const UnionCell *p);
static void motoi_switch(const UnionCell *p);
static void motoi_ternary(const UnionCell *p);
static void motoi_uminus(const UnionCell *p);
static void motoi_use(const UnionCell *p);
static void motoi_value(const UnionCell *p);
static void motoi_while(const UnionCell *p);

static void motoi_math(const UnionCell *p);
static void motoi_rel(const UnionCell *p);
static void motoi_return(const UnionCell *p);

static void buf_putmv(StringBuffer* buf,MotoVal* val);
static void motoi_domath(MotoVal *v1, MotoVal *v2, int op);
static void motoi_dorel(MotoVal *v1, MotoVal *v2, int op);
static void motoi_trackReturnValue(MotoFunction* f, MotoVal* r);
static void motoi_convertArgs(int argc, void **argv, char **at, char **vt,MotoVal** castvals);

/*-----------------------------------------------------------------------------
 * interpreter
 *---------------------------------------------------------------------------*/

void motoi(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MetaInfo curmeta = env->errmeta;
	env->errmeta = *uc_meta(p);

	switch (p->type) {
		case VALUE_TYPE:

			switch (p->valuecell.kind) {
				case NULL_VALUE:
				case BOOLEAN_VALUE:
				case CHAR_VALUE:
				case INT32_VALUE:
				case INT64_VALUE:
				case FLOAT_VALUE:
				case DOUBLE_VALUE:
				case STRING_VALUE:
				case REGEX_VALUE:
					motoi_value(p);
					break;
				case ID_VALUE:
					motoi_id(p);
					break;
				default:
					THROW_D("MotoCellTypeException");
					break;
			}
			break;
		case OP_TYPE:

			switch (uc_opcode(p)) {
				case AND: motoi_and(p); break;
				case ARRAY_LVAL: motoi_array_lval(p); break;
				case ARRAY_RVAL: motoi_array_rval(p); break;
				case ARRAY_NEW: motoi_array_new(p); break;
				case ASSIGN: motoi_assign(p); break;
				case BITWISE_AND:
				case BITWISE_OR:
				case BITWISE_XOR:
					motoi_bitop(p); break;
				case SHIFT_LEFT:
				case SHIFT_RIGHT:
					motoi_shift(p); break;
				case BITWISE_NOT:
					motoi_bitnot(p); break;
				case BREAK: motoi_jump(p); break;
				case CAST: motoi_cast(p); break;
				case CLASSDEF: motoi_classdef(p); break;
				case COMMA: motoi_comma(p); break;
				case CONTINUE: motoi_jump(p); break;
				case DECLARE:
				case DECLARE_INLINE: motoi_declare(p); break;
				case DEFINE: motoi_define(p); break;
				case DELETE: motoi_delete(p); break;
				case DEREFERENCE_RVAL: motoi_dereference_rval(p); break;
				case DEREFERENCE_LVAL: motoi_dereference_lval(p); break;
				case DO: motoi_do(p); break;
				case ELSEIF: motoi_elseif(p); break;
				case ELSEIFLIST: motoi_elseiflist(p); break;
				case EXCP_THROW: motoi_excp_throw(p); break;
				case EXCP_TRY: motoi_excp_try(p); break;
				case FCALL:
				case FN: 
				case METHOD: motoi_fn(p); break;				
				case FOR: motoi_for(p); break;
				case IDENTIFY: motoi_identify(p); break;
				case IF: motoi_if(p); break;
				case LIST: motoi_list(p); break;
				case MATH_ADD:
				case MATH_DIV:
				case MATH_MOD:
				case MATH_MULT:
				case MATH_SUB: motoi_math(p); break;
				case NEW: motoi_new(p); break;
				case NOOP: break;
				case NOT: motoi_not(p); break;
				case OR: motoi_or(p); break;
				case POSTFIX_DEC:					  
				case POSTFIX_INC:
				case PREFIX_DEC:					 
				case PREFIX_INC: motoi_incdec(p); break;
				case PRINT: motoi_print(p); break;
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
				case REL_NE_S: motoi_rel(p); break;
				case RETURN: motoi_return(p); break;
				case RX_MATCH:
				case RX_NOT_MATCH: motoi_regex(p); break;
				case SCOPE: motoi_scope(p); break;
				case SWITCH: motoi_switch(p); break;
				case TERNARY: motoi_ternary(p); break;
				case UMINUS: motoi_uminus(p); break;
				case USE: motoi_use(p); break;
				case WHILE: motoi_while(p); break;
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

#define MOTO_THROW motoi_getFillAndThrow

static void 
motoi_getFillAndThrow( char* etype , char*s, ...){
	MotoEnv *env = moto_getEnv();
	Exception* e = excp_getProcessException();
	int maxchars = MAX_EMSG_SIZE - 6;
	int nchars;
	char* tmp;
	
	va_list ap;
	va_start(ap, s);
	
	e->line = env->errmeta.lineno;
	e->file = env->errmeta.filename;
   
	strcpy(e->t,etype);
	
	nchars = vsnprintf(excp_e.msg, maxchars, s, ap);
	if (nchars < 0 || nchars >= maxchars) {
		strcat(e->msg, " ...\n");
	}
	
   tmp = stacktrace_toString(e);
   strncpy(excp_e.stacktrace,  tmp,maxchars);
   if (strlen(excp_e.stacktrace) >= maxchars) {
      strcat(e->stacktrace, " ...\n");
   }  
   free (tmp);
   
	extb_throw(e);
}

static void 
motoi_and(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVal *val1,*val2 = NULL;
	MotoVal *r;

	r = moto_createVal(env, "boolean",0);

	r->booleanval.value = '\0';

	motoi(uc_operand(p, 0));
	val1 = opstack_pop(env);

	if (bv(val1)) {
		motoi(uc_operand(p, 1));
		val2 = opstack_pop(env);
		if (bv(val2)) {
			r->booleanval.value ='\1';
		}
	} 

	moto_freeVal(env,val1);
	if(val2 != NULL)
		moto_freeVal(env,val2);

	opstack_push(env, r);
}

static void
motoi_array_rval(const UnionCell *p){
	MotoEnv *env = moto_getEnv();
	MotoVar* var;
	MotoVal* r;

	motoi_array_lval(p);
	var = (MotoVar*)opstack_pop(env);
	r = moto_getVarVal(env,var);
	moto_freeVar(env,var);
	opstack_push(env,r);
	
}

static void
motoi_array_lval(const UnionCell *p){
	MotoEnv *env = moto_getEnv();
	MotoVal *val, *ival,*rval;
	MotoVar *rvar;
	UnArray *ua;
	int dims;

	/* Evaluate the first operand to get the Array*/
	motoi(uc_operand(p, 0));
	val = opstack_pop(env);

	/* Evaluate the ARRAY_INDEX element */
	motoi(uc_operand(p, 1));
	ival = opstack_pop(env);
				
	if (ival->type->kind == INT32_TYPE)
		dims = iv(ival);
	else /* (ival->type->kind == INT64_TYPE) */
		dims = lv(ival);	
	
	if ((env->rval_uc != NULL) && 
		(val->type->kind == REF_TYPE) && (!isArray(val))
		) {

		UnionCell * rval_uc = env->rval_uc;
		env->rval_uc = NULL;

		motoi(rval_uc);
		env->rval_uc = rval_uc;

		rval = opstack_pop(env);
		opstack_push(env, rval);
	
		if(motox_try_overloaded_method_op(
			OPENBRACKET,val,ival,rval) == MOTO_OK) {
			MotoVal * ret = opstack_pop(env);
				rvar = moto_createVar(
				env,NULL,
				ret->type->name,
				0,
				'\0',
				NULL
			);
			moto_setVarVal(env, rvar, ret);
			opstack_push(env, (MotoVal *) rvar);
			env->rval_uc = NULL;
			return;
		}
		moto_freeVal(env, opstack_pop(env));
		
	}
	
	/* Test rval array overloaded operator */
	if(motox_try_overloaded_method_op(
		OPENBRACKET,val,ival,NULL) == MOTO_OK) {
			
		MotoVal * ret = opstack_pop(env);
			rvar = moto_createVar(
			env,NULL,
			ret->type->name,
			0,
			'\0',
			NULL
		);
		moto_setVarVal(env, rvar, ret);
		opstack_push(env, (MotoVal *) rvar);
		return;
	}

	rvar = moto_createVar(
		env,NULL,
		val->type->atype->name,
		val->type->dim - 1,
		'\1',
		NULL
	);
	ua = val->refval.value;
	
	/* Clean up val now since we might need to throw an exception */
	moto_freeVal(env,val);
		
	if (ua == NULL){
		/* Clean up and throw NullPointerException */
		moto_freeVar(env,rvar);
		MOTO_THROW("NullPointerException","Attempt to subscript null");
	}
	if(dims < 0 || dims >= ua->meta.length){
		/* Clean up and throw ArrayBoundsException */
		moto_freeVar(env,rvar);
		MOTO_THROW("ArrayBoundsException","Attempt to subscript array outside of declared bounds");
	}

	if (val->type->dim == 1){
		switch (val->type->atype->kind){
			case INT32_TYPE: rvar->address = &ua->ia.data[dims]; break;
			case INT64_TYPE: rvar->address = &ua->la.data[dims]; break;
			case FLOAT_TYPE: rvar->address = &ua->fa.data[dims]; break;
			case DOUBLE_TYPE: rvar->address = &ua->da.data[dims]; break;
			case BOOLEAN_TYPE: rvar->address = &ua->ba.data[dims]; break;
			case BYTE_TYPE: rvar->address = &ua->ya.data[dims]; break;
			case CHAR_TYPE: rvar->address = &ua->ca.data[dims]; break;
			case REF_TYPE: rvar->address = &ua->ra.data[dims]; break;
			default: {THROW_D("MotoCellTypeException");}
		}
	} else {
		rvar->address = &ua->aa.data[dims];
	}
	
	moto_freeVal(env,ival);

	opstack_push(env, (MotoVal*)rvar);
}

static void
motoi_array_new(const UnionCell *p){
	MotoEnv *env = moto_getEnv();
	MotoType* type;
	MotoFrame* frame;
	MotoVal *r;
	int stacksize, *dimarr,i;
	int dimspec=0 , dimunspec=0; /* The fully specified array dimensions and the not
										fully specified array dimensions */

	/* Grab the array subtype from the first operand */
	type = motox_extractType(uc_operand(p, 0));

	/* Record the current position on the stack */

	frame = env->frame;
	stacksize = stack_size(frame->opstack);

	/* Evaluate the LIST of ARRAY_INDEX elements */
	motoi(uc_operand(p, 1));  /* array_index list */
	
	dimspec = stack_size(frame->opstack) - stacksize;

	/* Add the number of unspecified dimensions */
	dimunspec = uc_opcount(uc_operand(p, 2));

	/* Construct a new Array type MotoVal */
	r = moto_createVal(env, type->name, dimspec + dimunspec);
	
	/* Build the array of dimension bounds */
	dimarr = (int*)emalloc(sizeof(int)*dimspec);
	i = dimspec-1;
	
	while(stack_size(frame->opstack) > stacksize){
		MotoVal* indexMotoVal = opstack_pop(env);

		if (indexMotoVal->type->kind == INT32_TYPE)
			dimarr[i] = iv(indexMotoVal);
		else /* (indexMotoVal->type->kind == INT64_TYPE) */
			dimarr[i] = lv(indexMotoVal);

		i--;

		moto_freeVal(env,indexMotoVal);
	}

	r->refval.value = arr_create(dimspec,dimunspec,0,dimarr,r->type->atype->kind);
	mman_track(rtime_getMPool(),r->refval.value);

	free(dimarr);
	opstack_push(env, r);
}

static void
motoi_arrayInit(const UnionCell *p,MotoType* type){
	MotoEnv *env = moto_getEnv();
	MotoVal *r,*val;
	int i;
	UnionCell* arrayElementsListUC;
	UnionCell* arrayElementUC;
	UnArray* ua;
	int dimensions[1];
	MotoType* etype;
	
	/* Construct a new Array type MotoVal with subtype type and dimension dim */
	r = moto_createVal(env, type->name,0);
	
	/* Retrieve the array element type */
	etype = mttab_get(env->types,type->atype->name,type->dim-1);
	
	/* Grab the array elements */
	arrayElementsListUC = uc_operand(p, 0);
	
	/* Allocate and track the array storage */
	dimensions[0] = arrayElementsListUC->opcell.opcount;
	r->refval.value = mman_track(rtime_getMPool(),
		arr_create(1,type->dim-1,0,dimensions,type->kind) );
	ua = r->refval.value;
			
	for(i=0;i<arrayElementsListUC->opcell.opcount;i++){
		arrayElementUC = uc_operand(arrayElementsListUC,i);
		
		if(arrayElementUC->type == OP_TYPE && uc_opcode(arrayElementUC) == ARRAY_INIT) {
			motoi_arrayInit(arrayElementUC,etype);
		} else 
			motoi(arrayElementUC);
		
		val = opstack_pop(env);
		
		if(etype->dim == 0) {
			/* Create a temporary of the right type */
			MotoVal* dup = moto_createVal(env, etype->name,0);
				
			/* Cast popped expression into the temporary*/
			moto_castVal(env,dup,val);
		
			/* Now assign to the array element */
			switch (etype->kind){
				case INT32_TYPE: ua->ia.data[i] = iv(dup); break;
				case INT64_TYPE: ua->la.data[i] = lv(dup); break;
				case FLOAT_TYPE: ua->fa.data[i] = fv(dup); break;
				case DOUBLE_TYPE: ua->da.data[i] = dv(dup); break;
				case BOOLEAN_TYPE: ua->ba.data[i] = bv(dup); break;
				case BYTE_TYPE: ua->ya.data[i] = yv(dup); break;
				case CHAR_TYPE: ua->ca.data[i] = cv(dup); break;
				case REF_TYPE: ua->ra.data[i] = dup->refval.value; break;
				default: {THROW_D("MotoCellTypeException");}
			}
			moto_freeVal(env,dup);
		} else
			ua->aa.data[i] = val->refval.value;
			
		moto_freeVal(env,val);
			
	}
	
	opstack_push(env, r);
}

MotoVal* 
motoi_callMDF(MotoVal* self,MotoVal** args, MotoFunction* f){
	MotoEnv *env = moto_getEnv();
	int i,argc=f->argc;
	/* This is a function or method defined in Moto ! */
	MotoVar* rvar, *this;
	MotoVal* r;
	int isMethod = mfn_isMethod(f);
	
	/* Grab the function definition cell and the arglistcell from it */
	const UnionCell* fdcell = f->opcell;
	const UnionCell* argListUC = uc_operand(fdcell,1);

	/* Create a function execution frame */
	moto_createFrame(env, FN_FRAME);

	/* Push arguments onto the frame by declaring them */
	for(i=0;i<argc;i++){
		UnionCell* argdec = uc_operand(argListUC,i);
		UnionCell* atype_uc = uc_operand(argdec,0);
		
		int argdecDim ;
		
		char *aname; 
		MotoVar* avar;
		MotoType* atype;
		
		atype = motox_extractType(atype_uc);
		aname = uc_str(argdec,1);
		argdecDim = uc_opcount(uc_operand(argdec, 2));

		/* Declare the argument variable */
		avar = moto_declareVar(env, aname, atype->name, argdecDim, '\0');

		moto_setVarVal(env, avar, args[i]);
	}

	/* Declare the special rvalue variable */
	rvar = moto_declareVar(env, "$rvalue", f->rtype, 0,'\0');

	if(isMethod) {
		Enumeration *e;
		MotoClassInstance* mci = (MotoClassInstance*)self->refval.value;
		MotoClassDefinition* mcd = moto_getMotoClassDefinition(env,self->type->name);
		
		/* Declare the special 'this' variable */
		this = moto_declareVar(env, "this", self->type->name, self->type->dim,'\0');
		moto_setVarVal(env,this,self);
	
		/* Push on any class variables that haven't already been defined */
	
		e = vec_elements(mcd->memberVarNames);
		while(enum_hasNext(e)){
			char* varn=(char*)enum_next(e);
			
			/* Fake variable shadowing by simply not loading in a member var
				with the same name as an argument */
				
			if(moto_getFrameVar(env,varn) == NULL) {
				MotoVar* var = moto_createVar(
					env,varn,
					mcd_getMemberType(mcd,varn),
					0,
					'\1',
					mci+mcd_getMemberOffset(mcd,varn)
				);
				stab_put(env->frame->symtab, var->n, var);
			}
		}
		enum_free(e);	  
	}

	stacktrace_push(env->errmeta.filename, env->errmeta.lineno, f->motoname,isMethod?ST_METHOD:ST_FUNCTION);

	/* Create a sub execution frame */
	moto_createFrame(env, SUB_FRAME);

	/* Mark off the spot on the stack to return to */
	{
		int frames = vec_size(env->frames);
		int framed = 0;

		TRY {
			/* Call motoi on statement list of the opcell matching the function prototype */
			motoi(uc_operand(fdcell,2));

		} CATCH ("MotoReturnException") {
			/* Pop the stack down to where we were before the return */
			framed = vec_size(env->frames) - frames;

			for (;framed > 0;framed--)
				moto_freeFrame(env);

		} END_TRY
	}

	stacktrace_pop();

	/* Free the sub frame */
	moto_freeFrame(env);

	/* Set the return value to the value stored in rvar */
	r = moto_getVarVal(env,rvar);

	/* Free the function frame */
	moto_freeFrame(env);

	return r;
} 

MotoVal* 
motoi_callEDF(MotoVal* self, MotoVal** args,MotoFunction* f){
	MotoEnv *env = moto_getEnv();
	void *argv[20];
	char *types[20];
	MotoVal * castvals[20];
	MotoVal * r;
	int i,argc=f->argc;
	int isMethod = mfn_isMethod(f);
	
	motox_fillArgs(argc,args, argv,types);

	/* Do numeric conversion on args */
	if(argc > 0) motoi_convertArgs(argc, argv, f->argtypes, types, castvals);
			
	/* Create an appropriately typed return type */
	r = moto_createVal(env,f->rtype,0);

	/* If this is a method call (but not a constructor call)  shift arguments up one and insert 'self' */
	if(isMethod && !mfn_isConstructor(f)){
		for (i=argc ; i>0; i--) argv[i] = argv[i-1];
		argv[0] = self->refval.value;
		argc++;
	}
	
	/* Push an entry onto the stack trace stack */					
	stacktrace_push(env->errmeta.filename, env->errmeta.lineno, f->motoname,ST_FUNCTION|ST_EXTENSION);

	switch(r->type->kind) {
		case BOOLEAN_TYPE : (*(f->fptr))(env, argc, argv, &r->booleanval.value); break;
		case BYTE_TYPE : (*(f->fptr))(env, argc, argv, &r->byteval.value); break;
		case CHAR_TYPE : (*(f->fptr))(env, argc, argv, &r->charval.value); break;
		case INT32_TYPE	: (*(f->fptr))(env, argc, argv, &r->intval.value); break;
		case INT64_TYPE	: (*(f->fptr))(env, argc, argv, &r->longval.value); break;
		case FLOAT_TYPE	: (*(f->fptr))(env, argc, argv, &r->floatval.value); break;
		case DOUBLE_TYPE: (*(f->fptr))(env, argc, argv, &r->doubleval.value); break;
		case REF_TYPE	: (*(f->fptr))(env, argc, argv, &r->refval.value); break;
		case VOID_TYPE : (*(f->fptr))(env, argc, argv, NULL); break;
	}

	/* Pop the top entry from the stack trace stack */
	stacktrace_pop();

	/* If this was a method call then shift arguments back down one removing 'self' */
	if(isMethod && !mfn_isConstructor(f)) {for (i=0;i<argc;i++){ argv[i]=argv[i+1];} argc--;}
	
	/* track memory if tracked keyword used in .i file */
	/* if type has a dynamically loadable c language destructor, use that */
	if(f->tracked && r->refval.value != NULL)
		motoi_trackReturnValue(f,r);
				
	/* Free any castVals used in call */
	for (i = 0; i < argc; i++) { if (castvals[i] != NULL) moto_freeVal(env,castvals[i]); }
											
	return r;
	
}

static void 
motoi_assign(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVal *rval = NULL;
	MotoVal *r = NULL;
	UnionCell *lval = uc_operand(p,0);
	int op = uc_operand(p, 1)->opcell.opcode;
	MotoVar * dest = NULL;

	/* Save rval UnionCell */
	env->rval_uc = uc_operand(p, 2);

	/* First ... either get value name or process operand 0 */

	if (lval->type == VALUE_TYPE && lval->valuecell.kind == ID_VALUE) {
	
		char *destn = uc_str(p, 0);
		dest = moto_getVar(env, destn);

	} else if((lval->type == OP_TYPE && lval->opcell.opcode == ARRAY_LVAL) ||
		 (lval->type == OP_TYPE && lval->opcell.opcode == DEREFERENCE_LVAL)) {
		
		dest = NULL;
		
		/* Evaluate the first operand to get the Member MotoVar */
		motoi(lval);

		dest= (MotoVar*)opstack_pop(env);
	
	}

	/* Now the rval part ... */

	if (op == ASSIGN) {
		if (env->rval_uc != NULL) {
			motoi(uc_operand(p, 2));
			env->rval_uc = NULL;
		}
	}
	else {

		/* do a math operation and leave the result on the top of the stack */
		MotoVal *v1;		
		MotoVal *v2;		
				
		switch (op) {
			case MATH_ADD:	  /* add-assignment */
			case MATH_SUB:	  /* sub-assignment */
			case MATH_MULT:  /* mult-assignment */
			case MATH_DIV:	  /* div-assignment */
			case MATH_MOD:	  /* mod-assignment */
				if (env->rval_uc != NULL) {
					motoi(uc_operand(p, 2));
					env->rval_uc = NULL;
				}
				
				/* FIXME!!! nasty but nice */
				
				if( lval->opcell.opcode == DEREFERENCE_LVAL )
					motoi_dereference_rval(uc_operand(p, 0));
				else
					motoi(uc_operand(p, 0));

				v1 = opstack_pop(env);
				v2 = opstack_pop(env);

				if(motox_try_overloaded_method_op(op,v1,v2,NULL) != MOTO_OK) {
					motoi_domath(v1, v2, op);
					moto_freeVal(env,v1);
					moto_freeVal(env,v2);
				}
			break;
		}	

	}
	rval = opstack_pop(env);


	/* Set dest to the rval value */
	moto_setVarVal(env, dest, rval);
	moto_freeVal(env,rval);

	r = moto_getVarVal(env,dest);

	if((lval->type == OP_TYPE && lval->opcell.opcode == ARRAY_LVAL) ||
		 (lval->type == OP_TYPE && lval->opcell.opcode == DEREFERENCE_LVAL)) {
		moto_freeVar(env,dest);
	}

	opstack_push(env, r);	  
	
}

static void motoi_bitop(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVal *r=NULL, *v1,*v2;
	int op;
	signed char yop1 = 0;
	signed char yop2 = 0;
	char cop1 = 0;
	char cop2 = 0;
	int32_t iop1 = 0;
	int32_t iop2 = 0;
	int64_t lop1 = 0;
	int64_t lop2 = 0;

	signed char *yptr;
	char	  *cptr;
	int32_t *iptr;
	int64_t *lptr;
	
	motoi(uc_operand(p, 0));
	motoi(uc_operand(p, 1));
	v2 = opstack_pop(env);
	v1 = opstack_pop(env);
	op = uc_opcode(p);
	
	switch (v1->type->kind) {
		case BYTE_TYPE:		yop1=cop1=iop1=yv(v1); lop1=yv(v1); break;
		case CHAR_TYPE:		yop1=cop1=iop1=cv(v1); lop1=cv(v1); break;
		case INT32_TYPE:	yop1=cop1=iop1=iv(v1); lop1=iv(v1); break;
		case INT64_TYPE:	yop1=cop1=iop1=lv(v1); lop1=lv(v1); break;
		default: { THROW_D("MotoException") ; } break;
	}
	switch (v2->type->kind) {
		case BYTE_TYPE:		yop2=cop2=iop2=yv(v2); lop2=yv(v2); break;
		case CHAR_TYPE:		yop2=cop2=iop2=cv(v2); lop2=cv(v2); break;
		case INT32_TYPE:	yop2=cop2=iop2=iv(v2); lop2=iv(v2); break;
		case INT64_TYPE:	yop2=cop2=iop2=lv(v2); lop2=lv(v2); break;
		default: { THROW_D("MotoException") ; } break;
	}

	if (v1->type->kind == INT64_TYPE || v1->type->kind == INT64_TYPE) {
		r = moto_createVal(env, "long",0);
		lptr = &r->longval.value;
		switch (op) {
			case BITWISE_AND:	 *(lptr) = lop1 & lop2; break;
			case BITWISE_OR:	 *(lptr) = lop1 | lop2; break;
			case BITWISE_XOR:	 *(lptr) = lop1 ^ lop2; break;
			default: { THROW_D("MotoOpcodeException") ; } break;
		}
	}
	else if (v1->type->kind == INT32_TYPE || v1->type->kind == INT32_TYPE) { 
		r = moto_createVal(env, "int",0);
		iptr = &r->intval.value;
		switch (op) {
			case BITWISE_AND:	 *(iptr) = iop1 & iop2; break;
			case BITWISE_OR:	 *(iptr) = iop1 | iop2; break;
			case BITWISE_XOR: *(iptr) = iop1 ^ iop2; break;
			default: { THROW_D("MotoOpcodeException") ; } break;
		}
	}
	else if (v1->type->kind == CHAR_TYPE || v1->type->kind == CHAR_TYPE) { 
		r = moto_createVal(env, "char",0);
		cptr = &r->charval.value;
		switch (op) {
			case BITWISE_AND:	 *(cptr) = cop1 & cop2; break;
			case BITWISE_OR:	 *(cptr) = cop1 | cop2; break;
			case BITWISE_XOR: *(cptr) = cop1 ^ cop2; break;
			default: { THROW_D("MotoOpcodeException") ; } break;
		}
	}
	else if (v1->type->kind == BYTE_TYPE || v1->type->kind == BYTE_TYPE) { 
		r = moto_createVal(env, "byte",0);
		yptr = &r->byteval.value;
		switch (op) {
			case BITWISE_AND:	 *(yptr) = yop1 & yop2; break;
			case BITWISE_OR:	 *(yptr) = yop1 | yop2; break;
			case BITWISE_XOR: *(yptr) = yop1 ^ yop2; break;
			default: { THROW_D("MotoOpcodeException") ; } break;
		}
	}
	else { THROW_D("MotoException") ; }
	
	moto_freeVal(env,v1);
	moto_freeVal(env,v2);
	opstack_push(env, r);

}

static void motoi_bitnot(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVal *r=NULL, *v1;
	int op;
	
	motoi(uc_operand(p, 0));
	v1 = opstack_pop(env);
	op = uc_opcode(p);
	
	r = moto_createVal(env, v1->type->name ,0);	  
	switch (v1->type->kind) {
		case BYTE_TYPE:
			r->byteval.value = ~yv(v1) ; break;
		case CHAR_TYPE:
			r->charval.value = ~cv(v1) ; break;
		case INT32_TYPE:			
			r->intval.value = ~iv(v1) ; break;
		case INT64_TYPE:	
			r->longval.value = ~lv(v1) ; break;
		default: { THROW_D("MotoException") ; } break;
	}
	
	moto_freeVal(env,v1);

	opstack_push(env, r);
}	

void motoi_cast(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVal *val;		 
	MotoVal *dup;	 
	MotoType* type;
	
	/* Extract the cast MotoType */
	type = motox_extractType(uc_operand(p, 0));
	
	/* See if the cast expression is an inline array initializer */
	if (uc_operand(p,1)->type == OP_TYPE && uc_opcode(uc_operand(p,1)) == ARRAY_INIT) {
		
		motoi_arrayInit(uc_operand(p,1),type);
		/* Just leave the MotoVal from arrayInit on the stack as it is guarenteed 
		   suitable for return */
		   
	} else {
		/* Construct a motoval suitable for return */
		dup = moto_createVal(env, type->name,0);
				
		/* Evaluate the cast expression */
		
		motoi(uc_operand(p, 1));
		val = opstack_pop(env);
		moto_castVal(env,dup,val);
		moto_freeVal(env, val);

		opstack_push(env, dup);
	}
}

void motoi_classdef(const UnionCell *p) {
	/* Don't do a damn thing :) */
}

static void 
motoi_comma(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVal *v1, *v2 = NULL;
	MotoVal *r;

	motoi(uc_operand(p, 0));
	motoi(uc_operand(p, 1));
	v2 = opstack_pop(env);
	v1 = opstack_pop(env);

	r = moto_createVal(env, v2->type->name,0);

	switch(v2->type->kind) {
		case BOOLEAN_TYPE		: r->booleanval.value = v2->booleanval.value; break;
		case BYTE_TYPE		: r->byteval.value = v2->byteval.value; break;
		case CHAR_TYPE		: r->charval.value = v2->charval.value; break;
		case INT32_TYPE		: r->intval.value = v2->intval.value; break;
		case INT64_TYPE		: r->longval.value = v2->longval.value; break;
		case FLOAT_TYPE		: r->floatval.value = v2->floatval.value; break;
		case DOUBLE_TYPE	: r->doubleval.value = v2->doubleval.value; break;
		case REF_TYPE		: r->refval.value = v2->refval.value; break;
		case VOID_TYPE		: /* FIXME: This should never happen */; break;
	}

	moto_freeVal(env, v1);
	moto_freeVal(env, v2);

	opstack_push(env, r);
}

void motoi_declare(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	int i;
	/* the type and name of the id being declared */
	char *varn;
	/* whether it is a global variable being declared */
	char global = '\0';
	MotoVar *var;
	MotoType *type;
	
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

		/* see if the declaration is "fancy" (i.e. it includes an assignment) */
		if (uc_opcode(uc_operand(declarator_uc, 2)) != NOOP) {
		
			/* See if the assignment is an inline array initializer */
			if (uc_operand(declarator_uc, 2)->type == OP_TYPE && uc_opcode(uc_operand(declarator_uc, 2)) == ARRAY_INIT) {
				/* If it is, verify that the inlined array elements are of the appropriate
				type and dimensions */
				
				MotoVal *val;
				motoi_arrayInit(uc_operand(declarator_uc, 2),var->type );
				val = opstack_pop(env);
				moto_setVarVal(env, var, val);
				moto_freeVal(env,val);

			} else {
			
				/* Declaration includes an ordinary assignment */
				MotoVal *val;
				motoi(uc_operand(declarator_uc, 2));
				val = opstack_pop(env);
				moto_setVarVal(env, var, val);
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

void motoi_define(const UnionCell *p) {
	/* Don't do a damn thing :) */
}

void motoi_dereference_lval(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoClassInstance* mci;
	MotoClassDefinition* mcd;
	MotoVal* val;
	char* varn;
	MotoVar* rvar;
	
	/* Evaluate operand 0 to get the MotoVal associated with the structure */
	motoi(uc_operand(p, 0));
	
	/* Retreive the MotoClassInstance from the MotoVal */
	val = opstack_pop(env);
	mcd = moto_getMotoClassDefinition(env,val->type->name);
	mci = (MotoClassInstance*)val->refval.value;
	
	/* If the MotoClassInstance is null throw a NullPointerException */
	if (mci == NULL) {
		/* Clean up and throw a NullPointerException */ 
		moto_freeVal(env,val);
		MOTO_THROW("NullPointerException","Attempt to dereference null");
	}
	
	/* Get the name of the member variable to dereference to from operand 1 */
	varn = moto_strdup(env, uc_str(p, 1));
	
	/* If the MotoClassDefinition defines no such member variable */
	if(mcd == NULL || mcd_getMemberType(mcd,varn) == NULL){

		/* Ok ... get an identifier method with this name */
		MotoFunction* mfn;		
		char fname[256];
		
		moto_createMotonameForFnInBuffer(val->type->name,varn,fname);
		ftab_fnForName(env->ftable,&mfn,fname);

		rvar = moto_createVar(
			env,varn,
			mfn_type(mfn)->name,
			0,
			'\0',
			NULL
		);
		rvar->vs->refval.value = mman_track(
			rtime_getMPool(),
			func_create(F_INTERPRETED,mfn->motoname,mfn,val->refval.value)
		);
		
	} else {
		
		/* Retrieve the data from the MotoClassInstance with this name and push it onto the OpStack */
		rvar = moto_createVar(
			env,varn,
			mcd_getMemberType(mcd,varn),
			0,
			'\1',
			mci +mcd_getMemberOffset(mcd,varn)
		);
	}
	
	opstack_push(env,(MotoVal*)rvar); // FIXME: hack alert ... pushing vars onto opstack
	
	moto_freeVal(env,val);
}
	
void motoi_dereference_rval(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVar *var;
	MotoVal *val;
	
	motoi_dereference_lval(p);
	var = (MotoVar*)opstack_pop(env); // FIXME: hack alert ... pushing vars onto opstack
	val = moto_getVarVal(env,var);
	moto_freeVar(env,var);
	opstack_push(env,val);
}

void motoi_do(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVal* val;

	motoi(uc_operand(p, 0));	

	if (stack_size(env->frame->opstack) > 0){
		val = opstack_pop(env);
		moto_freeVal(env,val);
	}
}

void motoi_elseif(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVal *val;

	if (uc_opcode(p) == NOOP) {
		val = moto_createVal(env, "boolean",0);
	} else {
		motoi(uc_operand(p, 0));
		val = opstack_pop(env);
		if (bv(val)) {
			moto_createFrame(env, SUB_FRAME);
			motoi(uc_operand(p, 1));
			moto_freeFrame(env);
		}
	}
	opstack_push(env, val);

}

void motoi_elseiflist(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVal *val;

	motoi(uc_operand(p, 0));
	val = opstack_peek(env);
	if (!bv(val)) {
		motoi(uc_operand(p, 1));
	}

}

void motoi_excp_throw(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVal *exceptionVal;
	Exception *exception;
	
	/* Evaluate the expression operand */
	motoi(uc_operand(p,0));
	exceptionVal = opstack_pop(env);
	
	/* Retrieve the exception itself */
	exception=exceptionVal->refval.value;
	
	/* Clean Up the exceptionVal */
	moto_freeVal(env,exceptionVal);
	
	/* Set the exceptions internals */
	exception->line = env->errmeta.lineno;
	exception->file = env->errmeta.filename;
	exception->stacktrace = stacktrace_toString(exception);
	
	/* Throw the exception */
	extb_throw(exception);	 
}

void motoi_excp_try(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();

	UnionCell *tryBlockCode = uc_operand(p,0);
	UnionCell *handlers = uc_operand(p,1);
	UnionCell *catchBlocks = uc_operand(handlers,0);
	UnionCell *finallyBlockCode = uc_operand(handlers,1);

	/* Mark off where we are on the stack */
	int frames = vec_size(env->frames);

	/* Begin our own TRY block */
	TRY {

		/* Push on a new sub-frame */
		moto_createFrame(env, SUB_FRAME);

		/* Evaluate the 'try' code */
		motoi(tryBlockCode);

		/* Pop off the sub-frame */
		moto_freeFrame(env);

	/* If it was any sort of MotoException that was thrown don't 
		even try to catch it */

	} CATCH_ALL_BUT("Moto") {
		Exception* exception;
		char exceptionHandled = '\0';
		int i;

		{
			/* Pop the stack down to where we were before the throw */
			int framed = vec_size(env->frames) - frames;
			for (;framed > 0;framed--) moto_freeFrame(env);
		}

		/* Retrieve the caught exception */
		exception = excp_getCurrentException();

		/* Foreach catch block */
		for(i=0;i<uc_opcount(catchBlocks);i++) {
			UnionCell* currentCatchBlock = uc_operand(catchBlocks,i);
			char* exceptionTypeName = uc_str(currentCatchBlock,0);
			char* exceptionName = uc_str(currentCatchBlock,1);
			UnionCell* catchBlockCode = uc_operand(currentCatchBlock,2);

			/* If catch block exception type matches caught exception type
				Or the catch block catches all exceptions */
			if(!estrcmp(exceptionTypeName,"Exception") || 
				!estrcmp(exceptionTypeName,exception->t)) {

				MotoVar* exceptionVar;

				/* Push on a new sub-frame */
				moto_createFrame(env, SUB_FRAME);

				/* Declare the exception variable in the catch block */
				exceptionVar = moto_declareVar(env, exceptionName, exceptionTypeName, 0,'\0');

				/* Set its value to that of caught exception */
				exceptionVar->vs->refval.value = exception;

				/* Evaluate the 'catch' code */
				motoi(catchBlockCode);

				/* Pop off the sub-frame */
				moto_freeFrame(env);

				/* Record the fact that this exception has been handled */
				exceptionHandled = '\1';

				/* Don't try to match any more Execption types, we're done */
				break;
			}
		}

		/* If we were not able to handle the Exception than we better re-throw */
		if(!exceptionHandled)
			extb_throw(exception); 

	} FINALLY {

		/* Push on a new sub-frame */
		moto_createFrame(env, SUB_FRAME);

		/* Evaluate the 'finally' code */
		motoi(finallyBlockCode);	

		/* Pop off the sub-frame */
		moto_freeFrame(env);
	} END_TRY
}

static void 
motoi_popArgs(int argc,MotoVal** args){
	MotoEnv* env = moto_getEnv();
	int i;
	
	for(i=argc-1;i>=0;i--)
		args[i] = opstack_pop(env);
}

void motoi_for(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVal *v;
	char b;
			
	moto_createFrame(env, SUB_FRAME);

	/* initial assignment */
	if (uc_opcode(uc_operand(p, 0)) != NOOP) {
		motoi(uc_operand(p, 0));
		v = opstack_pop(env);
		moto_freeVal(env, v);
	}

	/* conditional */
	if (uc_opcode(uc_operand(p, 1)) == NOOP) {
		b = 1;
	} else {
		motoi(uc_operand(p, 1));	
		v = opstack_pop(env);
		b = bv(v);
		moto_freeVal(env, v);
	}
	while (b) {
		/* 
		 * create a new frame, and:
		 *		o save this spot in case of $break or $continue
		 *		o save the current number of frames in case we break 
		 *		  or continue from down in the stack 
		 */
		int frames = 0;
		int framed = 0;
		
		frames = vec_size(env->frames);
		
		TRY {
			/* Normal loop */
			
			moto_createFrame(env, SUB_FRAME);
			motoi(uc_operand(p, 3)); 
			moto_freeFrame(env);

			if (uc_opcode(uc_operand(p, 2)) != NOOP) {
				motoi(uc_operand(p, 2));  
				v = opstack_pop(env);
				moto_freeVal(env, v);
			} 

			if (uc_opcode(uc_operand(p, 1)) == NOOP) {
				b = 1;
			}
			else {
				motoi(uc_operand(p, 1));	
				v = opstack_pop(env);
				b = bv(v);
				moto_freeVal(env, v);
			}

		} CATCH ("MotoBreakException") {
			
			/* Pop the stack down to where we were before the break */
			framed = vec_size(env->frames) - frames;
			for (;framed > 0;framed--) moto_freeFrame(env);

			/* push a zero so loop breaks */
			b = 0;

		} CATCH ("MotoContinueException") {
			
			/* Pop the stack down to where we were before the continue */
			framed = vec_size(env->frames) - frames;
			for (;framed > 0;framed--) moto_freeFrame(env);
			
			/* continuing...evaluate next loop condition */

			if (uc_opcode(uc_operand(p, 2)) != NOOP) {
				motoi(uc_operand(p, 2));
				v = opstack_pop(env);
				moto_freeVal(env, v);
			}

			if (uc_opcode(uc_operand(p, 1)) == NOOP) {
				b = 1;
			}
			else {
				motoi(uc_operand(p, 1));	
				v = opstack_pop(env);
				b = bv(v);
				moto_freeVal(env, v);
			}

		} END_TRY
	}
	moto_freeFrame(env);

}

static MotoVal*
motoi_callee(const UnionCell* p, MotoVal* self){
	MotoEnv *env = moto_getEnv();
	MotoVal* callee;
	UnionCell* calleeUC = uc_operand(p,0 + (uc_opcode(p) == METHOD ? 1:0));
		
	if (uc_opcode(p) == METHOD) {
		char* typen;
		MotoVar* calleeVar;
		char* varn = calleeUC->valuecell.value;
		MotoClassInstance* mci = self->refval.value;
		MotoClassDefinition* mcd = moto_getMotoClassDefinition(env,self->type->name);
		
		/* Retreive the MotoClassDefinition for the type of the associated motoval */
		typen = mcd_getMemberType(mcd,varn);

		/* Retrieve the data from the MotoClassInstance with this name and push 
			it onto the OpStack */
		calleeVar = moto_createVar(
			env,varn,mcd_getMemberType(mcd,varn),0,'\1',mci +mcd_getMemberOffset(mcd,varn));

		callee = moto_getVarVal(env,calleeVar);
		moto_freeVar(env,calleeVar);

	} else {
		/* Motoi the expression being called */
		motoi(calleeUC);
		callee = opstack_pop(env);
	}
	
	return callee;
}

static MotoVal* 
motoi_getDDFArgVal(Function* ddf, int i, MotoType* type){
	MotoEnv* env = moto_getEnv();
	MotoVal* arg = moto_createVal(env,type->name,0);
	
	switch(arg->type->kind) {
		case BOOLEAN_TYPE	: arg->booleanval.value = *(unsigned char*)func_getArg(ddf,i); break;
		case BYTE_TYPE		: arg->byteval.value = *(signed char*)func_getArg(ddf,i); break;
		case CHAR_TYPE		: arg->charval.value = *(char*)func_getArg(ddf,i); break;
		case INT32_TYPE		: arg->intval.value = *(int32_t*)func_getArg(ddf,i); break;
		case INT64_TYPE		: arg->longval.value = *(int64_t*)func_getArg(ddf,i); break;
		case FLOAT_TYPE		: arg->floatval.value = *(float*)func_getArg(ddf,i); break;
		case DOUBLE_TYPE	: arg->doubleval.value = *(double*)func_getArg(ddf,i); break;
		case REF_TYPE		: arg->refval.value =*(void**)func_getArg(ddf,i); break;
		case VOID_TYPE		: THROW_D("MotoException"); break;
	}
	
	return arg;
}

static void 
motoi_setDDFArgVal(Function* ddf, int i, MotoVal* dup){
	switch(dup->type->kind) {
		case BOOLEAN_TYPE : func_setArg( ddf, i, F_BOOLEAN_TYPE,&dup->booleanval.value); break;
		case BYTE_TYPE	: func_setArg( ddf, i, F_BYTE_TYPE,&dup->byteval.value); break;
		case CHAR_TYPE	: func_setArg( ddf, i, F_CHAR_TYPE,&dup->charval.value); break;
		case INT32_TYPE	: func_setArg( ddf, i, F_INT32_TYPE,&dup->intval.value); break;
		case INT64_TYPE	: func_setArg( ddf, i, F_INT64_TYPE,&dup->longval.value); break;
		case FLOAT_TYPE	: func_setArg( ddf, i, F_FLOAT_TYPE,&dup->floatval.value); break;
		case DOUBLE_TYPE: func_setArg( ddf, i, F_DOUBLE_TYPE,&dup->doubleval.value); break;
		case REF_TYPE	: func_setArg( ddf, i, F_REF_TYPE,&dup->refval.value); break;
		case VOID_TYPE	: THROW_D("MotoException"); break;
	}
}

static void 
motoi_identify(const UnionCell *wrapper) {
	MotoEnv *env = moto_getEnv();
	char* fbasename;
	UnionCell* p,* arglistUC;
	UnionCell* fbasenameUC;
	char *types[20];
	int i,argc;
	int pargc = 0,pargi[20]; 	/* The number of args that are 'known' and their indexes */
	MotoVal* pargv[20]; 		/* The 'known' argument moto vals */
	FuncTypeKind pargt[20];		/* The 'known' argument moto val types */
	MotoType* ftype,*rftype;
	MotoFunction* f;
	MotoVal* r, *self = NULL;
	int calleeIsDDF = 0;
	
	p = uc_operand(wrapper,0);
	fbasenameUC = uc_operand(p,0 + (uc_opcode(p) == METHOD ? 1:0));
	arglistUC = uc_operand(p,1 + (uc_opcode(p) == METHOD ? 1:0));

	/* If this is a method call evaluate self */
	if (uc_opcode(p) == METHOD) {
		motoi(uc_operand(p, 0));
		self = opstack_pop(env);
		if (self->refval.value == NULL) {
			/* Clean Up and throw a NullPointerException */
			MOTO_THROW("NullPointerException","A method may not be identified on null");
		}
	} else if (uc_opcode(p) == FN && moto_getVar(env,"this") != NULL) {
		/* This may still be a method call, lets get a self value just in case */
		self = moto_getVarVal(env,moto_getVar(env,"this"));
	}
			
	/* Fill in the arg types */
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
			/* Verify the argument and retrieve its type */
			MotoVal* aval;
			motoi(argUC);
			aval = opstack_pop(env);
			types[i] = aval->type->name;

			/* Add the 'known' argument index to pargi, val to pargc, and increment pargc */
			pargi[pargc] = i;
			pargv[pargc] = aval;
			pargc++;
		}	
	}

	/* Figure out if the callee is a dynamically defined function (DDF) */
	calleeIsDDF = motox_calleeIsDDF(p,self);
	
	if(calleeIsDDF){
		Function* ddf;
		
		/* Retrieve the callee */
		MotoVal* callee = motoi_callee(p,self);
		
		/* Retrieve the DDF and the MotoFunction its associated with */
		ddf = callee->refval.value;
		f = ddf->fn;
		
		/* Compose f(X) st f(X) = g(h(X)) */
		/* Retrieve pargc, pargi, and pargt from the DDF then apply the deltas to that */
		{	
			int i,j,k;
			
			int newpargc = 0,newpargi[20]; 	/* Combined number of args that are 'known' and their indexes */
			MotoVal* newpargv[20]; 			/* Combined 'known' argument MotoVals */
	
			int opargc = 0,opargi[20]; 		/* Original number of args that are 'known' and their indexes */
			MotoVal* opargv[20]; 			/* Original 'known' argument MotoVals */
			
			opargc = ddf->pargc;
			for(i=0;i<opargc;i++){
				opargi[i] = ddf->pindex[i].index;
				
				/* Create a motoval for the DDF arg at index i */
				opargv[i] = motoi_getDDFArgVal(ddf,i,callee->type->argt[i]);
			}
			
			/* i = index in f */
			/* j = index in opargs */
			/* k = index in pargs */
			/* j+k = index in newpargs */
			
			newpargc = opargc+pargc;
			
			for(i=0,j=0,k=0;i+j < newpargc ;i++) {
				if (opargi[j]==i+j+k) 
					{newpargi[j+k] = i+j+k; newpargv[j+k] = opargv[j]; i--; j++; continue;}
				if (pargi[k]+j==i+j+k)
					{newpargi[j+k] = i+j+k; newpargv[j+k] = pargv[k]; i--; k++;}
			}
			
			/* move newp* back into p* */
			
			pargc = newpargc;
			for(i=0;i<pargc;i++){pargi[i] = newpargi[i]; pargv[i] = newpargv[i];}
	
		}

		/* Is this a function or a method ? */
		if(mfn_isMethod(f)){
			self=moto_createVal(env,f->classn,0);
			self->refval.value = ddf->self;
		}
					
		/* Retrieve the function's type */
		ftype = mfn_type(f);
		
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
	
	/* Fill pargt with the canonical types of the arguments applied in f */
	for(i=0;i<pargc;i++) pargt[i] = mttab_get(env->types,f->argtypes[pargi[i]],0)->kind;
	
	if(mfn_isMethod(f)){
		r->refval.value= mman_track(rtime_getMPool(),
			func_createPartial(F_INTERPRETED,f->motoname,f,self->refval.value,pargc,pargi,pargt));
	} else {
		r->refval.value= mman_track(rtime_getMPool(),
			func_createPartial(F_INTERPRETED,f->motoname,f,NULL,pargc,pargi,pargt));
	}
	
	for(i=0;i<pargc;i++){
		Function* ddf = r->refval.value;
		MotoVal* val = pargv[i];
		MotoVal* dup = moto_createVal(env,f->argtypes[pargi[i]],0);
		
		/* Cast the known argument value to the appropriate argument type */
		moto_castVal(env,dup,val);
		
		/* Set the value of the argument in the DDF */
		motoi_setDDFArgVal(ddf, i, dup);
		
		moto_freeVal(env,val);
		moto_freeVal(env,dup);		
	}
	
	if(self != NULL)
		moto_freeVal(env,self);
		
	opstack_push(env, r);
}

static void 
motoi_id(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVar *var = NULL;
	MotoVal *val;

	var = moto_getVar(env, p->valuecell.value);
	if(var != NULL){
		val = moto_getVarVal(env,var);
	} else {
		char fname[256],* varn = p->valuecell.value;
		MotoFunction* mfn = NULL;
		MotoType* type;
		MotoVal* self = NULL;
		int code = FTAB_NONE;
		
		/* Check to see if a method with this name is defined */
		if (moto_getVar(env,"this") != NULL) {
			/* Get the self MotoVal */
			self = moto_getVarVal(env,moto_getVar(env,"this"));
			
			/* Generate the Moto name for a possible method */
			moto_createMotonameForFnInBuffer(self->type->name, varn,fname);
	
			/* Retrieve the MXFN record */
			code = ftab_fnForName(env->ftable,&mfn,fname);
		} 
		
		/* If there is no method, see if there is a function with this name */
		if(code == FTAB_NONE) ftab_fnForName(env->ftable,&mfn,varn);
		
		type = mfn_type(mfn);
		val = moto_createVal(env, type->name,0);

		if(mfn_isMethod(mfn)){
			val->refval.value= mman_track(rtime_getMPool(),
				func_create(F_INTERPRETED,mfn->motoname,mfn,self->refval.value));
		} else {
			val->refval.value= mman_track(rtime_getMPool(),
				func_create(F_INTERPRETED,mfn->motoname,mfn,NULL));
		}
	}

	opstack_push(env, val);

}

/*
	op1: if expression
	op2: if statement_list
	op3: elseif block
	op4: else statement_list
*/
static void 
motoi_if(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVal *val;
	char b;

	motoi(uc_operand(p, 0));
	val = opstack_pop(env);
	b = bv(val);
	moto_freeVal(env, val);

	if (b) {
		moto_createFrame(env, SUB_FRAME);
		motoi(uc_operand(p, 1));	
		moto_freeFrame(env);
	}
	else if (uc_opcode(uc_operand(p, 2)) != NOOP) {
		motoi(uc_operand(p, 2));
		val = opstack_pop(env);			  
		b = bv(val);
		moto_freeVal(env, val);
		if (!b && uc_opcode(uc_operand(p, 3)) != NOOP) {
			moto_createFrame(env, SUB_FRAME);
			motoi(uc_operand(p, 3));	
			moto_freeFrame(env);
		}
	}
	else if (uc_opcode(uc_operand(p, 3)) != NOOP) {
		moto_createFrame(env, SUB_FRAME);
		motoi(uc_operand(p, 3));	
		moto_freeFrame(env);
	}

}

static void 
motoi_incdec(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVal *val = NULL;
	MotoVal *r = NULL;
	MotoVar* var = NULL;
	int incdec = uc_opcode(p);
	UnionCell* lval = uc_operand(p,0);

	/* Retrieve the original variable value by executing operand 0 */
	if (lval->type == VALUE_TYPE && lval->valuecell.kind == ID_VALUE) {
		motoi_id(uc_operand(p,0));
		val = opstack_pop(env);
	} else if 
		( (lval->type == OP_TYPE && lval->opcell.opcode == DEREFERENCE_LVAL)
		||
		(lval->type == OP_TYPE && lval->opcell.opcode == ARRAY_LVAL)) {
		motoi(uc_operand(p,0));
		var=(MotoVar*)opstack_pop(env);
		val=moto_getVarVal(env,var);
	}
	
	/* If this is NOT an overloaded operator ... */
	if(motox_try_overloaded_method_op(incdec,val,NULL,NULL) != MOTO_OK) {	
	
		/* Set the return value to the result of that execution */
	
		/* Set it before if this was a prefix operation */
		if(incdec == POSTFIX_INC || incdec == POSTFIX_DEC)	 
			r=moto_cloneVal(env,val);

		/* Increment the value by 1 */
		switch (val->type->kind) {
			case BYTE_TYPE:
				val->byteval.value = yv(val) 
					+ (incdec == POSTFIX_INC || incdec == PREFIX_INC ? 1 : -1);
				break;
			case CHAR_TYPE:
				val->charval.value = cv(val) 
					+ (incdec == POSTFIX_INC || incdec == PREFIX_INC ? 1 : -1);
				break;
			case INT32_TYPE:
				val->intval.value = iv(val) 
					+ (incdec == POSTFIX_INC || incdec == PREFIX_INC ? 1 : -1);
				break;
			case INT64_TYPE:
				val->longval.value = lv(val) 
					+ (incdec == POSTFIX_INC || incdec == PREFIX_INC ? 1 : -1);
				break;
			case FLOAT_TYPE:
				val->floatval.value = fv(val) 
					+ (incdec == POSTFIX_INC || incdec == PREFIX_INC ? 1 : -1);
				break;
			case DOUBLE_TYPE:
				val->doubleval.value = dv(val) 
					+ (incdec == POSTFIX_INC || incdec == PREFIX_INC ? 1 : -1);
				break;
			default:
				break;
		}
	
		/* Set it after if this was a postfix operation */
		if(incdec == PREFIX_INC || incdec == PREFIX_DEC)  
			r=moto_cloneVal(env,val);
	
		if (lval->type == VALUE_TYPE && lval->valuecell.kind == ID_VALUE) {
			char *destn = uc_str(p, 0);
			var = moto_getVar(env, destn);
	
			moto_setVarVal(env, var, val);

		} else if((lval->type == OP_TYPE && lval->opcell.opcode == ARRAY_LVAL) 
					||
				(lval->type == OP_TYPE && lval->opcell.opcode == DEREFERENCE_LVAL)) {
			/* Set the vars value to the new value */
			moto_setVarVal(env, var, val);
			moto_freeVar(env,var);
		}
		opstack_push(env, r);
	}

	moto_freeVal(env,val);
}

static void 
motoi_jump(const UnionCell *p) {
	/* Throw a MotoBreakException or a MotoContinueException as appropriate */
	
	if(uc_opcode(p)==BREAK)
		THROW_D("MotoBreakException");
	else if(uc_opcode(p)==CONTINUE)
		THROW_D("MotoContinueException");
	else
		THROW_D("MotoException");

}

static void 
motoi_list(const UnionCell *p) {
	/* MotoEnv *env = moto_getEnv();
	Stack* s; 
	UnionCell *c; */

	int i;
	for(i=0;i<p->opcell.opcount;i++)
		motoi(p->opcell.operands[i]);

}

static void 
motoi_math(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVal *v1 = NULL;
	MotoVal *v2 = NULL;
	int op = uc_opcode(p);

	motoi(uc_operand(p, 0));
	motoi(uc_operand(p, 1));
	v2 = opstack_pop(env);
	v1 = opstack_pop(env);

	if(motox_try_overloaded_function_op(op,v1,v2) != MOTO_OK) {
		motoi_domath(v1, v2, op);
	}

	moto_freeVal(env, v1);
	moto_freeVal(env, v2);

}

/* Insert args already defined in the DDF into the args array */
static int 
motoi_apply(Function* ddf, MotoFunction* mfn, int argc, MotoVal** args){
	MotoEnv* env= moto_getEnv();
	int i,j;
	for(i=0;i<ddf->pargc;i++){
		int argi = ddf->pindex[i].index;
		
		/* Create a MotoVal for the DDF arg at index i */
		MotoVal* arg = motoi_getDDFArgVal(ddf,i,mttab_get(env->types,mfn->argtypes[argi],0));
		
		/* Move the other args forward one */	
		for(j=argc;j>argi;j--) args[j] = args[j-1];
		
		/* Increment argc */
		argc++;
		args[argi] = arg;	
	}
	return argc;
}


static void 
motoi_callDDF(MotoVal *callee, UnionCell * arglistUC){
	MotoEnv *env = moto_getEnv();
	MotoVal *r, *self=NULL;
	MotoVal * args[20];
	Function* ddf;
	MotoFunction *mfn;
	int i,argc;
	
	/* Verify the callee is not null */
	if (callee->refval.value == NULL) {
		/* Clean Up and throw a NullPointerException */
		MOTO_THROW("NullPointerException","Cannot call null");
	}
	
	ddf = (Function*)callee->refval.value;
	mfn = (MotoFunction*)ddf->fn;

	/* Motoi the arguments */
	motoi(arglistUC);
	argc = arglistUC->opcell.opcount;
	motoi_popArgs(argc,args);
	
	/* Is this a function or a method ? */
	if(mfn_isMethod(mfn)){
		self=moto_createVal(env,mfn->classn,0);
		self->refval.value = ((Function*)callee->refval.value)->self;
	}
	
	/* Insert args already defined in the DDF into the args array */
	argc = motoi_apply(ddf,mfn,argc,args);
	
	if(mfn->fptr != NULL) 
		r= motoi_callEDF(self,args,mfn);
	else 
		r= motoi_callMDF(self,args,mfn);

	if(mfn_isMethod(mfn)){
		moto_freeVal(env,self);	
	}
		
	/* Free MotoVals used in call */
	for (i = 0; i < argc; i++) moto_freeVal(env,args[i]);
		
	opstack_push(env,r);		
}

static void 
motoi_fn(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	int i;
	int argc = 0;
	char *fbasename = NULL;
	void *argv[20];
	char *types[20];
	MotoFunction *mfn = NULL;
	MotoFrame *frame = env->frame;
	MotoVal *self = NULL;
	MotoVal *r = NULL;
	int calleeIsDDF = 0; /* Should be set to true if the callee UnionCell 
										in an expression of a functional type */
	
	UnionCell* calleeUC = uc_operand(p, uc_opcode(p) == METHOD?1:0 );
	UnionCell* arglistUC = uc_operand(p, 1 + (uc_opcode(p) == METHOD?1:0) );
	
	/* If this is a method call evaluate self */
	if (uc_opcode(p) == METHOD) {
		motoi(uc_operand(p, 0));
		self = opstack_pop(env);
		if (self->refval.value == NULL) {
			/* Clean Up and throw a NullPointerException */
			MOTO_THROW("NullPointerException","A method may not be called on null");
		}
	} else if (uc_opcode(p) == FN && moto_getVar(env,"this") != NULL) {
		/* This may still be a method call, lets get a self value just in case */
		self = moto_getVarVal(env,moto_getVar(env,"this"));
	}

	/* Figure out if the callee is a dynamically defined function (DDF) */
	calleeIsDDF = motox_calleeIsDDF(p,self);
	
	if(calleeIsDDF) {

		/* Evaluate and retrieve the callee */
		MotoVal* callee = motoi_callee(p,self);
			
		motoi_callDDF(callee,arglistUC);
		
		moto_freeVal(env,callee);		
	} else {	
		MotoVal* args[20];
		
		/* Evaluate the arguments to this function or method call */
		motoi(arglistUC);
		
		/* Retrieve the number of arguments passed to this function or method call */
		argc = arglistUC->opcell.opcount;
		
		/* Pop the evaluated arguments off the stack and into an args array */
		motoi_popArgs(argc,args);
		
		/* Fill in the types array for function or method identification */
		motox_fillArgs(argc,args,argv,types);
	
		/* Extract the function or method name */
		fbasename = calleeUC->valuecell.value;
	
		/* Look up the function or method being identified */
		motox_lookupMethodOrFn(&mfn, self,fbasename, argc, types, uc_opcode(p));
	
		/* Call function or method */
		if(mfn->fptr != NULL) {
			r = motoi_callEDF(self,args,mfn);
		} else {
			r = motoi_callMDF(self,args,mfn);
		}
	
		/* Free MotoVals used in call */
		for (i=0; i<argc ; i++) moto_freeVal(env, args[i]);
	
		/* If this was a method call then free the self MotoVal */
		if(self != NULL) moto_freeVal(env,self);
	
		/* Push the return value of the function or method onto the Stack */
		stack_push(frame->opstack, r);
	}
}


static void 
motoi_new(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	int i;
	int argc = 0;
	int code;
	char *classname = NULL;
	char fname[256];
	MotoVal* args[20];
	void *argv[20];
	char *types[20];
	MotoFunction *mfn = NULL;
	MotoVal *r = NULL;
	MotoType *type = NULL;

	classname = uc_str(p, 0);

	/* make sure this type is registered */
	type = mttab_get(env->types, classname,0);
	
	/* Allocate space for the return value */
	r = moto_createVal(env, classname,0);
		
	/* Check if we are constructing a moto defined class */
	if(moto_getMotoClassDefinition(env,classname) != NULL) {

		MotoClassInstance* mci;
		Enumeration *e;
		
		/* Retrieve the MotoClassDefinition for the class being constructed */
		MotoClassDefinition* mcd = moto_getMotoClassDefinition(env,classname);
		
		/* Create a new private stack frame */
		moto_createFrame(env, FN_FRAME);
		
		/* Call the opcell associated with the MotoClassDefinition 
			(this will declare all and potentailly define all the member variables) */
		motoi(mcd->definition);
		
		/* Instantiate a new MotoClassInstance */
		mci = mci_create(mcd);
		mman_track(rtime_getMPool(), mci);
		
		/* Migrate the values of all the MotoVars out of the current Frame and into 
			the MotoClassInstance */
		
		e = vec_elements(mcd->memberVarNames);
		while(enum_hasNext(e)){
			char* varn = (char*)enum_next(e);
			MotoVar* mvar = moto_createVar(
				env,varn,
				mcd_getMemberType(mcd,varn),
				0,
				'\1',
				mci +mcd_getMemberOffset(mcd,varn)
			);
			MotoVar* fvar = moto_getFrameVar(env,varn);
			MotoVal* val = moto_getVarVal(env,fvar);
			
			moto_setVarVal(env, mvar, val);
			
			moto_freeVar(env,mvar);
			moto_freeVal(env,val);
		}
		enum_free(e);
		
		/* Free the private frame */
		moto_freeFrame(env);
		
		r->refval.value=mci;
	}
	
	/* Evaluate the arguments to this constructor call */
	motoi(uc_operand(p, 1));
	
	/* Retrieve the number of arguments passed to this constructor call */
	argc = uc_operand(p, 1)->opcell.opcount;
	
	/* Pop the evaluated arguments off the stack and into an args array */
	motoi_popArgs(argc,args);
	
	/* Fill in the types array for constructor identification */
	motox_fillArgs(argc,args,argv,types);

	/* Generate the Moto name for this Constructor */
	moto_createMotonameForFnInBuffer(classname,classname,fname);
	
	/* Retrieve the MXFN record */
	code = ftab_cacheGet(env->ftable, &mfn, fname, argc, types);

	/* Potentially no matching constructor was found, this would be the case
		If we were relying on the implicit constructor for instance. */

	if(mfn != NULL) {
		/* Call the constructor */
		if(mfn->fptr != NULL) 
			r = motoi_callEDF(r,args,mfn);
		else 
			motoi_callMDF(r,args,mfn);
	}
	
	/* Free the MotoVals used in call */
	for (i = 0; i < argc; i++) moto_freeVal(env, args[i]);
		
	opstack_push(env, r);
}


static void 
motoi_delete(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVal *val;

	motoi(uc_operand(p, 0));
	val = opstack_pop(env);

	mman_free(val->refval.value);
	moto_freeVal(env, val);
}

static void 
motoi_not(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVal *val;

	motoi(uc_operand(p, 0));
	val = opstack_peek(env);
	val->booleanval.value = !bv(val);

}

static void 
motoi_or(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVal *val1=NULL,*val2=NULL;
	MotoVal *r;

	r = moto_createVal(env, "boolean",0);
	r->booleanval.value = '\0';

	motoi(uc_operand(p, 0));
	val1 = opstack_pop(env);
	if (bv(val1)) {
		r->booleanval.value = '\1';
	}
	else {
		motoi(uc_operand(p, 1));
		val2 = opstack_pop(env);
		if (bv(val2)) {
			r->booleanval.value = 1;
		}
	}
	
	moto_freeVal(env,val1);
	if(val2 != NULL)
		moto_freeVal(env,val2);

	opstack_push(env, r);

}

void 
motoi_print(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVal *val;

	motoi(uc_operand(p, 0));
	val = opstack_pop(env);

	buf_putmv(env->frame->out, val);

	moto_freeVal(env,val);
}

static void 
motoi_regex(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVal *v1;
	MotoVal *v2;
	MotoVal *r;
	char *s;
	Regex *rx;

	motoi(uc_operand(p, 0));
	motoi(uc_operand(p, 1));
	v2 = opstack_pop(env);
	v1 = opstack_pop(env);
	r = moto_createVal(env, "boolean",0);

	switch (uc_opcode(p)) {
		case RX_MATCH:
			s = v1->refval.value;
			rx = (Regex*)v2->refval.value;
			r->booleanval.value = 
			regex_matchesSubstring(rx,(char*)s);
			break;
		case RX_NOT_MATCH:
			s = v1->refval.value;
			rx = (Regex*)v2->refval.value;
			r->booleanval.value = 
			!regex_matchesSubstring(rx,(char*)s);
			break;
		default: /* Will never get here if the verifier did it's job */
			break ;
	} 
	
	/* push result onto opstack */
	moto_freeVal(env, v1);
	moto_freeVal(env, v2);
	opstack_push(env, r);

}

static void 
motoi_rel(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVal *v1;
	MotoVal *v2;
	int op = uc_opcode(p);

	motoi(uc_operand(p, 0));
	motoi(uc_operand(p, 1));
	v2 = opstack_pop(env);
	v1 = opstack_pop(env);

	if(motox_try_overloaded_function_op(op,v1,v2) != MOTO_OK) {
		motoi_dorel(v1, v2, op);
	}

	moto_freeVal(env, v1);
	moto_freeVal(env, v2);

}

static void 
motoi_return(const UnionCell *p){
	MotoEnv *env = moto_getEnv();
	MotoVal *val;
	MotoVar *rvar;

	if(uc_opcount(p) != 0) {
		/* Execute the expression operand */
		motoi(uc_operand(p, 0));
	
		/* Set value of rvalue */
		val = opstack_pop(env);
		rvar = moto_getVar(env, "$rvalue");
		moto_setVarVal(env, rvar, val);	
		moto_freeVal(env, val);
	}

	/* Throw a MotoReturnException */
	THROW_D("MotoReturnException");
}

static void 
motoi_shift(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVal *r=NULL, *v1,*v2;
	int op,bits=0;
	
	motoi(uc_operand(p, 0));
	motoi(uc_operand(p, 1));
	v2 = opstack_pop(env);
	v1 = opstack_pop(env);
	op = uc_opcode(p);

	switch (v2->type->kind) {
		case BYTE_TYPE:	bits = yv(v2); break;
		case CHAR_TYPE:	bits = cv(v2); break;
		case INT32_TYPE:	bits = iv(v2); break;
		case INT64_TYPE:	bits = lv(v2); break;
		default: { THROW_D("MotoException") ; } break;
	}
	
	r = moto_createVal(env, v1->type->name,0);	
	
	switch (v1->type->kind) {
		case BYTE_TYPE:	 
			r->byteval.value = (op==SHIFT_LEFT ?yv(v1) << bits : yv(v1) >> bits ); break;
		case CHAR_TYPE:	 
			r->charval.value = (op==SHIFT_LEFT ?cv(v1) << bits : cv(v1) >> bits ); break;
		case INT32_TYPE:	
			r->intval.value = (op==SHIFT_LEFT ?iv(v1) << bits : iv(v1) >> bits ); break;
		case INT64_TYPE: 
			r->longval.value = (op==SHIFT_LEFT ?lv(v1) << bits : lv(v1) >> bits ); break;
		default: { THROW_D("MotoException") ; } break;
	}

	moto_freeVal(env,v1);
	moto_freeVal(env,v2);
	opstack_push(env, r);
}	

void 
motoi_scope(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();

	moto_createFrame(env, SUB_FRAME);
	motoi(uc_operand(p, 0));
	moto_freeFrame(env);
}

void 
motoi_switch(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVal *switchVal;	  
	UnionCell *caseStatementListUC;
	char matchedCase = 0;
	int i;
	
	/* Interpret value being switched on */
	motoi(uc_operand(p, 0));
	switchVal = opstack_pop(env);
	
	/* Find the appropriate case to execute */
	caseStatementListUC = uc_operand(p, 2);
	for(i=0;i<caseStatementListUC->opcell.opcount;i++){
		MotoVal *caseVal;
		UnionCell *caseStatementUC = uc_operand(caseStatementListUC,i);
		MotoVal *comparisonResult;	  

		/* Since we are looping over cases ourselves we need to set the
		global meta info properly */
		
		MetaInfo curmeta = env->errmeta;
		env->errmeta = *uc_meta(caseStatementUC);
		
		motoi(uc_operand(caseStatementUC, 0));
		caseVal = opstack_pop(env);
		
		/* Determine if the switchVal equals the caseVal */
		if (isString(switchVal)) 
			motoi_dorel(switchVal, caseVal, REL_EQ_S);
		else 
			motoi_dorel(switchVal, caseVal, REL_EQ_M);
	
		moto_freeVal(env, caseVal);
		comparisonResult = opstack_pop(env);
		
		/* Record we made the match */
		matchedCase = bv(comparisonResult);
		moto_freeVal(env,comparisonResult);
		
		/* if this the appropriate case block to execute, execute it, then break */
		if (matchedCase) {
			moto_createFrame(env, SUB_FRAME);
			motoi(uc_operand(caseStatementUC, 1));
			moto_freeFrame(env);
			
			break;
		}

		/* Re-set the global err meta */
		env->errmeta = curmeta;
	}
	
	/* If the switch val never matched a case execute the default block */
	if (!matchedCase) {
		moto_createFrame(env, SUB_FRAME);
		motoi(uc_operand(p, 1));
		moto_freeFrame(env);
	}

	moto_freeVal(env, switchVal);
}

void 
motoi_ternary(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVal *val;
	char condition;
	
	motoi(uc_operand(p, 0));
	val = opstack_pop(env);
	condition = bv(val);
	moto_freeVal(env,val); 
	
	if (condition) 
		motoi(uc_operand(p, 1));	
	else 
		motoi(uc_operand(p, 2));	
	
}

void 
motoi_uminus(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVal *val;

	motoi(uc_operand(p, 0));
	val = opstack_peek(env);

	switch (val->type->kind) {
		case INT32_TYPE: val->intval.value *= -1; break;
		case INT64_TYPE: val->longval.value *= -1; break;
		case FLOAT_TYPE: val->floatval.value *= -1; break;
		case DOUBLE_TYPE: val->doubleval.value *= -1; break;
		default:
			break;
	}

}

void 
motoi_use(const UnionCell *p) {
	/* NOOP : motod handles this */
}

void motoi_value(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVal *val = NULL;

	switch (p->valuecell.kind) {
		case NULL_VALUE:
			val = moto_createVal(env,"null",0);
			break;
		case BOOLEAN_VALUE:
			val = moto_createVal(env, "boolean",0);
			val->booleanval.value = *(unsigned char *)p->valuecell.value;
			break;
		case CHAR_VALUE:
			val = moto_createVal(env, "char",0);
			val->charval.value = *(char *)p->valuecell.value;
			break;
		case INT32_VALUE:
			val = moto_createVal(env, "int",0);
			val->intval.value = *(int32_t *)p->valuecell.value;
			break;
		case INT64_VALUE:
			val = moto_createVal(env, "long",0);
			val->longval.value = *(int64_t *)p->valuecell.value;
			break;
		case FLOAT_VALUE:
			val = moto_createVal(env, "float",0);
			val->floatval.value = *(float *)p->valuecell.value;
			break;
		case DOUBLE_VALUE:
			val = moto_createVal(env, "double",0);
			val->doubleval.value = *(double *)p->valuecell.value;
			break;

		/* Built in object types */

		case REGEX_VALUE:
			val = moto_createVal(env,"Regex",0);
			val->refval.value = (void*)mman_trackf(rtime_getMPool(),regex_create(p->valuecell.value),(void(*)(void *))regex_free);
			break;
		case STRING_VALUE:
			val = moto_createVal(env,"String",0);
			val->refval.value = p->valuecell.value;
			break;
		default:
			break;
	}
	opstack_push(env, val);

}

void 
motoi_while(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MotoVal *v;	  
	char b;

	motoi(uc_operand(p, 0));
	v = opstack_pop(env);
	b = bv(v);
	moto_freeVal(env, v);
	
	while (b) {
		/* 
		 * create a new frame, and:
		 *		o save this spot in case of $break or $continue
		 *		o save the current number of frames in case we break 
		 *		  or continue from down in the stack 
		 */

		int frames = 0;
		int framed = 0;
		
		frames = vec_size(env->frames);

		TRY {
			
			/* normal loop */
			moto_createFrame(env, SUB_FRAME);
			motoi(uc_operand(p, 1));	
			moto_freeFrame(env);
			
			motoi(uc_operand(p, 0));	
				
		} CATCH ("MotoBreakException") {
		
			/* Pop the frame stack down to where we were before the break */
			framed = vec_size(env->frames) - frames;
			for (;framed > 0;framed--) moto_freeFrame(env);
			
			/* push a zero so loop breaks */
			v = moto_createVal(env, "boolean",0);
			v->booleanval.value='\0';
			opstack_push(env, v);
			
		} CATCH ("MotoContinueException") {

			/* Pop the frame stack down to where we were before the continue */
			framed = vec_size(env->frames) - frames;
			for (;framed > 0;framed--) moto_freeFrame(env);
			
			/* continuing...evaluate next loop condition */
			motoi(uc_operand(p, 0));	

		} END_TRY
		
		v = opstack_pop(env);
		b = bv(v);
		moto_freeVal(env, v);
	}
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

static void motoi_dorel(MotoVal *v1, MotoVal *v2, int op) {
	MotoEnv *env = moto_getEnv();
	MotoVal *r;
	char *cptr;

	unsigned char bop1 = 0;
	unsigned char bop2 = 0;
	signed char yop1 = 0;
	signed char yop2 = 0;
	char cop1 = 0;
	char cop2 = 0;
	int32_t iop1 = 0;
	int32_t iop2 = 0;
	int64_t lop1 = 0;
	int64_t lop2 = 0;
	float fop1 = 0;
	float fop2 = 0;
	double dop1 = 0;
	double dop2 = 0;

	r = moto_createVal(env, "boolean",0);
	cptr = &r->booleanval.value;

	if (v1->type->kind == REF_TYPE || v2->type->kind == REF_TYPE) {
		switch (op) {
			case REL_EQ_M: *(cptr) = (v1->refval.value == v2->refval.value); break;
			case REL_NE_M: *(cptr) = (v1->refval.value != v2->refval.value); break;
			case REL_EQ_S: *(cptr) = estrcmp(v1->refval.value, v2->refval.value) == 0; break;
			case REL_NE_S: *(cptr) = estrcmp(v1->refval.value, v2->refval.value) != 0; break;
			case REL_GT_S: *(cptr) = estrcmp(v1->refval.value, v2->refval.value) > 0; break;
			case REL_GTE_S: *(cptr) = estrcmp(v1->refval.value, v2->refval.value) >= 0; break;
			case REL_LT_S: *(cptr) = estrcmp(v1->refval.value, v2->refval.value) < 0; break;
			case REL_LTE_S: *(cptr) = estrcmp(v1->refval.value, v2->refval.value) <= 0; break;
			default: break;
		}
	}
	else if (v1->type->kind == BOOLEAN_TYPE || v2->type->kind == BOOLEAN_TYPE) {
		if (v1->type->kind == v2->type->kind) {
			bop1 = bv(v1);
			bop2 = bv(v2);
			switch (op) {
				case REL_EQ_M: *(cptr) =  bop1 == bop2; break;
				case REL_NE_M: *(cptr) =  bop1 != bop2; break;
				default: break;
			}
		}
	}
	else { /* Numeric types may Require Casts */
		switch (v1->type->kind) {
			case BYTE_TYPE:	yop1=cop1=iop1=yv(v1); lop1=yv(v1); fop1=yv(v1); dop1=yv(v1); break;
			case CHAR_TYPE:	yop1=cop1=iop1=cv(v1); lop1=cv(v1); fop1=cv(v1); dop1=cv(v1); break;
			case INT32_TYPE:	yop1=cop1=iop1=iv(v1); lop1=iv(v1); fop1=iv(v1); dop1=iv(v1); break;
#ifdef CAST_INT64_TO_INT32_FIRST
			case INT64_TYPE:	yop1=cop1=iop1=lv(v1); lop1=lv(v1); fop1=(int32_t)lv(v1); dop1=lv(v1); break;
#else
			case INT64_TYPE:	yop1=cop1=iop1=lv(v1); lop1=lv(v1); fop1=lv(v1); dop1=lv(v1); break;
#endif
			case DOUBLE_TYPE: yop1=cop1=iop1=dv(v1); lop1=dv(v1); fop1=dv(v1); dop1=dv(v1); break;
			case FLOAT_TYPE:	yop1=cop1=iop1=fv(v1); lop1=fv(v1); fop1=fv(v1); dop1=fv(v1); break;
			default: break;
		}
		switch (v2->type->kind) {
			case BYTE_TYPE:	yop2=cop2=iop2=yv(v2); lop2=yv(v2); fop2=yv(v2); dop2=yv(v2); break;
			case CHAR_TYPE:	yop2=cop2=iop2=cv(v2); lop2=cv(v2); fop2=cv(v2); dop2=cv(v2); break;
			case INT32_TYPE:	yop2=cop2=iop2=iv(v2); lop2=iv(v2); fop2=iv(v2); dop2=iv(v2); break;
#ifdef CAST_INT64_TO_INT32_FIRST
			case INT64_TYPE:	yop2=cop2=iop2=lv(v2); lop2=lv(v2); fop2=(int32_t)lv(v2); dop2=lv(v2); break;
#else
			case INT64_TYPE:	yop2=cop2=iop2=lv(v2); lop2=lv(v2); fop2=lv(v2); dop2=lv(v2); break;
#endif
			case DOUBLE_TYPE: yop2=cop2=iop2=dv(v2); lop2=dv(v2); fop2=dv(v2); dop2=dv(v2); break;
			case FLOAT_TYPE:	yop2=cop2=iop2=fv(v2); lop2=fv(v2); fop2=fv(v2); dop2=fv(v2); break;
			default: break;
		}

		if (v1->type->kind == DOUBLE_TYPE || v2->type->kind == DOUBLE_TYPE) {
			switch (op) {
				case REL_EQ_M:	 *(cptr) = dop1 == dop2; break;
				case REL_NE_M:	 *(cptr) = dop1 != dop2; break;
				case REL_GT_M:	 *(cptr) = dop1 > dop2; break;
				case REL_GTE_M: *(cptr) = dop1 >= dop2; break;
				case REL_LT_M:	 *(cptr) = dop1 < dop2; break;
				case REL_LTE_M: *(cptr) = dop1 <= dop2; break;
				default: break;
			}
		}
		else if (v1->type->kind == FLOAT_TYPE || v2->type->kind == FLOAT_TYPE) {
			switch (op) {
				case REL_EQ_M:	 *(cptr) = fop1 == fop2; break;
				case REL_NE_M:	 *(cptr) = fop1 != fop2; break;
				case REL_GT_M:	 *(cptr) = fop1 > fop2; break;
				case REL_GTE_M: *(cptr) = fop1 >= fop2; break;
				case REL_LT_M:	 *(cptr) = fop1 < fop2; break;
				case REL_LTE_M: *(cptr) = fop1 <= fop2; break;
				default: break;
			}
		}
		else if (v1->type->kind == INT64_TYPE || v1->type->kind == INT64_TYPE) {
			switch (op) {
				case REL_EQ_M:	 *(cptr) = lop1 == lop2; break;
				case REL_NE_M:	 *(cptr) = lop1 != lop2; break;
				case REL_GT_M:	 *(cptr) = lop1 > lop2; break;
				case REL_GTE_M: *(cptr) = lop1 >= lop2; break;
				case REL_LT_M:	 *(cptr) = lop1 < lop2; break;
				case REL_LTE_M: *(cptr) = lop1 <= lop2; break;
				default: break;
			}
		}
		else if (v1->type->kind == INT32_TYPE && v1->type->kind == INT32_TYPE) {
			switch (op) {
				case REL_EQ_M:	 *(cptr) = iop1 == iop2; break;
				case REL_NE_M:	 *(cptr) = iop1 != iop2; break;
				case REL_GT_M:	 *(cptr) = iop1 > iop2; break;
				case REL_GTE_M: *(cptr) = iop1 >= iop2; break;
				case REL_LT_M:	 *(cptr) = iop1 < iop2; break;
				case REL_LTE_M: *(cptr) = iop1 <= iop2; break;
				default: break;
			}
		} else if (v1->type->kind == CHAR_TYPE && v1->type->kind == CHAR_TYPE) {
			switch (op) {
				case REL_EQ_M:	 *(cptr) = cop1 == cop2; break;
				case REL_NE_M:	 *(cptr) = cop1 != cop2; break;
				case REL_GT_M:	 *(cptr) = cop1 > cop2; break;
				case REL_GTE_M: *(cptr) = cop1 >= cop2; break;
				case REL_LT_M:	 *(cptr) = cop1 < cop2; break;
				case REL_LTE_M: *(cptr) = cop1 <= cop2; break;
				default: break;
			}
		} else if (v1->type->kind == BYTE_TYPE && v1->type->kind == BYTE_TYPE) {
			switch (op) {
				case REL_EQ_M:	 *(cptr) = yop1 == yop2; break;
				case REL_NE_M:	 *(cptr) = yop1 != yop2; break;
				case REL_GT_M:	 *(cptr) = yop1 > yop2; break;
				case REL_GTE_M: *(cptr) = yop1 >= yop2; break;
				case REL_LT_M:	 *(cptr) = yop1 < yop2; break;
				case REL_LTE_M: *(cptr) = yop1 <= yop2; break;
				default: break;
			}
		} else { THROW_D("MotoException"); }
	}

	opstack_push(env, r);

}

static void buf_putmv(StringBuffer* buf,MotoVal* val){

	switch (val->type->kind) {
		case BOOLEAN_TYPE: buf_putb(buf, bv(val)); break;
		case BYTE_TYPE: buf_puty(buf, yv(val)); break;
		case CHAR_TYPE: buf_putc(buf, cv(val)); break;
		case INT32_TYPE: buf_puti(buf, iv(val)); break;
		case INT64_TYPE: buf_putl(buf, lv(val)); break;
		case FLOAT_TYPE: buf_putf(buf, fv(val)); break;
		case DOUBLE_TYPE: buf_putd(buf, dv(val)); break;
		case REF_TYPE:
			if (isString(val)) {
				if(val->refval.value != NULL)
					buf_puts(buf, val->refval.value);
				else
					buf_puts(buf, "null");
			} else if (isArray(val) && val->type->atype->kind == CHAR_TYPE && val->type->dim == 1){			
				buf_putca(buf, (UnArray*)val->refval.value);
			} else if (isArray(val) && val->type->atype->kind == BYTE_TYPE && val->type->dim == 1){			
				buf_putya(buf, (UnArray*)val->refval.value);
			} else if (!strcmp(val->type->name,"null")) {
				buf_puts(buf, "null");
			} else {
				buf_puts(buf, "[");
				buf_puts(buf, val->type->name);
				buf_puts(buf, ": ");
				buf_putp(buf, val->refval.value);
				buf_puts(buf, "]");
			}
			break;
		default:
			buf_puts(buf, "[");
			buf_puts(buf, val->type->name);
			buf_puts(buf, ": ");
			buf_putp(buf, val->refval.value);
			buf_puts(buf, "]");
			break;
	}
}


static void 
motoi_domath(MotoVal *v1, MotoVal *v2, int op) {
	MotoEnv *env = moto_getEnv();
	MotoVal *r = NULL;

	signed char yop1 = 0;
	signed char yop2 = 0;
	char cop1 = 0;
	char cop2 = 0;
	int32_t iop1 = 0;
	int32_t iop2 = 0;
	int64_t lop1 = 0;
	int64_t lop2 = 0;
	float fop1 = 0;
	float fop2 = 0;
	double dop1 = 0;
	double dop2 = 0;

	signed char *yptr;
	/* char	  *cptr; */
	int32_t *iptr;
	int64_t *lptr;
	float	  *fptr;
	double  *dptr;

	if (op == MATH_ADD && (isString(v1) || isString(v2 ))) {
		StringBuffer* sb = (StringBuffer*)opool_grab(env->bufpool);
		
		r = moto_createVal(env,"String",0);
		buf_putmv(sb,v1);
		buf_putmv(sb,v2); 
		r->refval.value = buf_toString(sb);
		mman_track(rtime_getMPool(),r->refval.value);  /* Strings go in the Runtime Pool */
		opool_release(env->bufpool,sb);
	}
	else {
		switch (v1->type->kind) {
			case BYTE_TYPE:	yop1=cop1=iop1=yv(v1); lop1=yv(v1); fop1=yv(v1); dop1=yv(v1); break;
			case CHAR_TYPE:	yop1=cop1=iop1=cv(v1); lop1=cv(v1); fop1=cv(v1); dop1=cv(v1); break;
			case INT32_TYPE:	yop1=cop1=iop1=iv(v1); lop1=iv(v1); fop1=iv(v1); dop1=iv(v1); break;
#ifdef CAST_INT64_TO_INT32_FIRST
			case INT64_TYPE:	yop1=cop1=iop1=lv(v1); lop1=lv(v1); fop1=(int32_t)lv(v1); dop1=lv(v1); break;
#else
			case INT64_TYPE:	yop1=cop1=iop1=lv(v1); lop1=lv(v1); fop1=lv(v1); dop1=lv(v1); break;
#endif
			case DOUBLE_TYPE: yop1=cop1=iop1=dv(v1); lop1=dv(v1); fop1=dv(v1); dop1=dv(v1); break;
			case FLOAT_TYPE:	yop1=cop1=iop1=fv(v1); lop1=fv(v1); fop1=fv(v1); dop1=fv(v1); break;
			default: { THROW_D("MotoException"); } break;
		}
		switch (v2->type->kind) {
			case BYTE_TYPE:	yop2=cop2=iop2=yv(v2); lop2=yv(v2); fop2=yv(v2); dop2=yv(v2); break;
			case CHAR_TYPE:	yop2=cop2=iop2=cv(v2); lop2=cv(v2); fop2=cv(v2); dop2=cv(v2); break;
			case INT32_TYPE:	yop2=cop2=iop2=iv(v2); lop2=iv(v2); fop2=iv(v2); dop2=iv(v2); break;
#ifdef CAST_INT64_TO_INT32_FIRST
			case INT64_TYPE:	yop2=cop2=iop2=lv(v2); lop2=lv(v2); fop2=(int32_t)lv(v2); dop2=lv(v2); break;
#else
			case INT64_TYPE:	yop2=cop2=iop2=lv(v2); lop2=lv(v2); fop2=lv(v2); dop2=lv(v2); break;
#endif
			case DOUBLE_TYPE: yop2=cop2=iop2=dv(v2); lop2=dv(v2); fop2=dv(v2); dop2=dv(v2); break;
			case FLOAT_TYPE:	yop2=cop2=iop2=fv(v2); lop2=fv(v2); fop2=fv(v2); dop2=fv(v2); break;
			default: { THROW_D("MotoException"); } break;
		}

		if (v1->type->kind == DOUBLE_TYPE || v2->type->kind == DOUBLE_TYPE) {
			r = moto_createVal(env, "double",0);
			dptr = &r->doubleval.value;
			switch (op) {
				case MATH_ADD:	 *(dptr) = dop1 + dop2; break;
				case MATH_SUB:	 *(dptr) = dop1 - dop2; break;
				case MATH_MULT: *(dptr) = dop1 * dop2; break;
				case MATH_DIV:	 
					if (dop2 == 0){
						/* Clean Up and throw a runtime exception */
						moto_freeVal(env,r); 
						MOTO_THROW("MathException","Attempt to divide by zero"); 
					}
					*(dptr) = dop1 / dop2; 
					break;
				default: { THROW_D("MotoOpcodeException"); } break;
			}
		}
		else if (v1->type->kind == FLOAT_TYPE || v2->type->kind == FLOAT_TYPE) {
			r = moto_createVal(env, "float",0);
			fptr = &r->floatval.value;
			switch (op) {
				case MATH_ADD:	 *(fptr) = fop1 + fop2; break;
				case MATH_SUB:	 *(fptr) = fop1 - fop2; break;
				case MATH_MULT: *(fptr) = fop1 * fop2; break;
				case MATH_DIV:	 
					if (dop2 == 0){
						/* Clean Up and throw a runtime exception */
						moto_freeVal(env,r); 
						MOTO_THROW("MathException","Attempt to divide by zero"); 
					}
					*(fptr) = fop1 / fop2; 
					break;
				default: { THROW_D("MotoOpcodeException"); } break;
			}
		}
		else if (v1->type->kind == INT64_TYPE || v1->type->kind == INT64_TYPE) {
			r = moto_createVal(env, "long",0);
			lptr = &r->longval.value;
			switch (op) {
				case MATH_ADD:	 *(lptr) = lop1 + lop2; break;
				case MATH_SUB:	 *(lptr) = lop1 - lop2; break;
				case MATH_MULT: *(lptr) = lop1 * lop2; break;
				case MATH_DIV:	 
					if (lop2 == 0){
						/* Clean Up and throw a runtime exception */
						moto_freeVal(env,r); 
						MOTO_THROW("MathException","Attempt to divide by zero"); 
					}
					*(lptr) = lop1 / lop2; 
					break;
				case MATH_MOD:	 *(lptr) = lop1 % lop2; break;
				default: { THROW_D("MotoOpcodeException"); } break;
			}
		}
		else if (v1->type->kind == INT32_TYPE || v1->type->kind == INT32_TYPE) { 
			r = moto_createVal(env, "int",0);
			iptr = &r->intval.value;
			switch (op) {
				case MATH_ADD:	 *(iptr) = iop1 + iop2; break;
				case MATH_SUB:	 *(iptr) = iop1 - iop2; break;
				case MATH_MULT: *(iptr) = iop1 * iop2; break;
				case MATH_DIV:	 
					if (iop2 == 0){
						/* Clean Up and throw a runtime exception */
						moto_freeVal(env,r); 
						MOTO_THROW("MathException","Attempt to divide by zero"); 
					}
					*(iptr) = iop1 / iop2; 
					break;
				case MATH_MOD:	 *(iptr) = iop1 % iop2; break;
				default: { THROW_D("MotoOpcodeException"); } break;
			}
		}
		else if (v1->type->kind == CHAR_TYPE || v1->type->kind == CHAR_TYPE) { 
			r = moto_createVal(env, "int",0);
			iptr = &r->intval.value;
			switch (op) {
				case MATH_ADD:	 *(iptr) = cop1 + cop2; break;
				case MATH_SUB:	 *(iptr) = cop1 - cop2; break;
				case MATH_MULT: *(iptr) = cop1 * cop2; break;
				case MATH_DIV:	 
					if (cop2 == 0){
						/* Clean Up and throw a runtime exception */
						moto_freeVal(env,r); 
						MOTO_THROW("MathException","Attempt to divide by zero"); 
					}
					*(iptr) = cop1 / cop2; 
					break;
				case MATH_MOD:	 *(iptr) = cop1 % cop2; break;
				default: { THROW_D("MotoOpcodeException"); } break;
			}
		}
		else if (v1->type->kind == BYTE_TYPE || v1->type->kind == BYTE_TYPE) { 
			r = moto_createVal(env, "byte",0);
			yptr = &r->byteval.value;
			switch (op) {
				case MATH_ADD:	 *(yptr) = yop1 + yop2; break;
				case MATH_SUB:	 *(yptr) = yop1 - yop2; break;
				case MATH_MULT: *(yptr) = yop1 * yop2; break;
				case MATH_DIV:	 
					if (yop2 == 0){
						/* Clean Up and throw a runtime exception */
						moto_freeVal(env,r); 
						MOTO_THROW("MathException","Attempt to divide by zero"); 
					}
					*(yptr) = yop1 / yop2; 
					break;
				case MATH_MOD:	 *(yptr) = yop1 % yop2; break;
				default: { THROW_D("MotoOpcodeException"); } break;
			}
		} else { THROW_D("MotoException"); }
	}

	opstack_push(env, r);

}

static void 
motoi_trackReturnValue(MotoFunction* f, MotoVal* r){
	MotoEnv* env = moto_getEnv();
	MotoFunction *mxfn;
	int code;
	StringBuffer* dname;

	dname = (StringBuffer*)opool_grab(env->bufpool);

	buf_puts(dname,f->rtype);
	buf_puts(dname,"::~");
	buf_puts(dname,f->rtype);

	/* allocate enough room for the destructor name */
	code = ftab_cacheGet(env->ftable, &mxfn, buf_data(dname), 0, NULL);
	if(code == FTAB_OK) {
		void(*free_fn)(void *);
		moto_loadDestructor(mxfn->handle, f->rtype, &free_fn);
		if (free_fn != NULL) {
			mman_trackf(rtime_getMPool(), r->refval.value, free_fn);
		}
		else {
			mman_track(rtime_getMPool(), r->refval.value);
		}
	}
	else {
		mman_track(rtime_getMPool(), r->refval.value);
	}
	opool_release(env->bufpool,dname);
}

static void 
motoi_convertArgs(int argc, void **argv, char **at, char **vt,MotoVal** castvals){
	MotoEnv *env = moto_getEnv();
	int i;

	for (i = 0; i < argc; i++) 
		castvals[i] = NULL;

	for (i = 0; i < argc; i++) {
		char *atype = at[i];
		char *vtype = vt[i];
		
		if (strcmp("boolean", atype) == 0) {
			if (strcmp("boolean", vtype) == 0) {
				MotoVal *val = moto_createVal(env, "int",0);
				val->intval.value = *((char *)argv[i]);
				argv[i] = &val->intval.value;
				castvals[i] = val;
			}
		}
		else if (strcmp("byte", atype) == 0) {
			if (strcmp("byte", vtype) == 0) {
				MotoVal *val = moto_createVal(env, "int",0);
				val->intval.value = *((signed char *)argv[i]);
				argv[i] = &val->intval.value;
				castvals[i] = val;
			}
		}
		else if (strcmp("char", atype) == 0) {
			if (strcmp("char", vtype) == 0) {
				MotoVal *val = moto_createVal(env, "int",0);
				val->intval.value = *((char *)argv[i]);
				argv[i] = &val->intval.value;
				castvals[i] = val;
			}
		}
		else if (strcmp("long", atype) == 0) {
			if (strcmp("int", vtype) == 0) {
				MotoVal *val = moto_createVal(env, "long",0);
				val->longval.value = *((int32_t *)argv[i]);
				argv[i] = &val->longval.value;
				castvals[i] = val;
			}
		}
		else if (strcmp("float", atype) == 0) {
			if (strcmp("int", vtype) == 0) {
				MotoVal *val = moto_createVal(env, "float",0);
				val->floatval.value = *((int32_t *)argv[i]);
				argv[i] = &val->floatval.value;
				castvals[i] = val;
			}
			else if (strcmp("long", vtype) == 0) {
				MotoVal *val = moto_createVal(env, "float",0);
				val->floatval.value = *((int64_t *)argv[i]);
				argv[i] = &val->floatval.value;
				castvals[i] = val;
			}
		}
		else if (strcmp("double", atype) == 0) {
			if (strcmp("int", vtype) == 0) {
				MotoVal *val = moto_createVal(env, "double",0);
				val->doubleval.value = *((int32_t *)argv[i]);
				argv[i] = &val->doubleval.value;
				castvals[i] = val;
			}
			else if (strcmp("long", vtype) == 0) {
				MotoVal *val = moto_createVal(env, "double",0);
				val->doubleval.value = *((int64_t *)argv[i]);
				argv[i] = &val->doubleval.value;
				castvals[i] = val;
			}
			else if (strcmp("float", vtype) == 0) {
				MotoVal *val = moto_createVal(env, "double",0);
				val->doubleval.value = *((float *)argv[i]);
				argv[i] = &val->doubleval.value;
				castvals[i] = val;
			}
		}
	}
}

/*
 * HOF function support for extensions
 */
 
#define FUNC_CALL_COMMON \
	MotoEnv *env = moto_getEnv();  \
	MotoVal* rval,*self=NULL; \
	MotoVal* argVals[20]; \
	MotoFunction* mf = f->fn; \
	va_list args; \
	int i; \
	 \
	/* Wrap the arguments in moto vals and push them onto the stack */ \
	 \
	va_start(args, types); \
	for(i=0;i<strlen(types);i++){ \
		MotoVal* a; \
		switch (types[i]) { \
			case 'i':  \
				a=moto_createVal(env,"int",0);  \
				a->intval.value=(int32_t)va_arg(args, int32_t);  \
				break; \
			case 'l': \
				a=moto_createVal(env,"long",0);   \
				a->longval.value=(int64_t)va_arg(args, int64_t);  \
				break; \
			case 'f' :  \
				a=moto_createVal(env,"float",0);  \
				/* `float' is promoted to `double' when passed through `...' */ \
				a->floatval.value=(float)va_arg(args, double);  \
				break; \
			case 'd' :  \
				a=moto_createVal(env,"double",0);  \
				a->doubleval.value=(double)va_arg(args, double);  \
				break; \
			case 'b' :  \
				a=moto_createVal(env,"boolean",0);  \
				/* `unsigned char' is promoted to `int' when passed through `...' */ \
				a->booleanval.value=(unsigned char)va_arg(args, int);  \
				break; \
			case 'c' :  \
				a=moto_createVal(env,"char",0);  \
				/* `char' is promoted to `int' when passed through `...' */ \
				a->charval.value=(char)va_arg(args, int);  \
				break; \
			case 'y' :  \
				a=moto_createVal(env,"byte",0);  \
				/* `signed char' is promoted to `int' when passed through `...' */ \
				a->byteval.value=(signed char)va_arg(args, int);  \
				break; \
			case 'O' :  \
				a=moto_createVal(env,"Object",0);  \
				a->refval.value=(void*)va_arg(args, void*);  \
				break; \
			default: \
				a = NULL; \
				THROW_D("MotoException"); \
		} \
		argVals[i]=a; \
	} \
	 \
	va_end(args); \
	 \
	if(mfn_isMethod(mf)) { \
	 	self=moto_createVal(env,mf->classn,0); \
		self->refval.value = f->self; \
	} \
	/* Insert args already defined in the DDF into the args array */ \
	motoi_apply(f,mf,strlen(types),argVals); \
	 \
	if(mf->fptr != NULL) \
		rval= motoi_callEDF(self,argVals,mf); \
	else \
		rval= motoi_callMDF(self,argVals,mf); \
	 \
	if(mfn_isMethod(mf)) \
		moto_freeVal(env,self); \
	/* Free MotoVals used in call */  \
	 \
	for (i = 0; i < mf->argc; i++)   \
		moto_freeVal(env,argVals[i]); 

unsigned char mifunc_bcall(Function* f, char* types, ...) {
	unsigned char r; FUNC_CALL_COMMON r=rval->booleanval.value; moto_freeVal(env,rval); return r; 											
}	
char mifunc_ccall(Function* f, char* types, ...) {
	char r; FUNC_CALL_COMMON r=rval->charval.value; moto_freeVal(env,rval); return r; 											
}	
double mifunc_dcall(Function* f, char* types, ...) {
	double r; FUNC_CALL_COMMON r=rval->doubleval.value; moto_freeVal(env,rval); return r; 											
}
float mifunc_fcall(Function* f, char* types, ...) {
	float r; FUNC_CALL_COMMON r=rval->floatval.value; moto_freeVal(env,rval); return r; 											
}
int32_t mifunc_icall(Function* f, char* types, ...) {
	int32_t r; FUNC_CALL_COMMON r=rval->intval.value; moto_freeVal(env,rval); return r; 											
}
int64_t mifunc_lcall(Function* f, char* types, ...) {
	int64_t r; FUNC_CALL_COMMON r=rval->longval.value; moto_freeVal(env,rval); return r; 											
}
void* mifunc_rcall(Function* f, char* types, ...) {
	void* r; FUNC_CALL_COMMON r=rval->refval.value; moto_freeVal(env,rval); return r; 											
}
signed char mifunc_ycall(Function* f, char* types, ...) {
	signed char r; FUNC_CALL_COMMON r=rval->byteval.value; moto_freeVal(env,rval); return r; 											
}
void mifunc_vcall(Function* f, char* types, ...) {
	FUNC_CALL_COMMON moto_freeVal(env,rval); 											
}

void motoi_init(){
	ifunc_bcall = mifunc_bcall;
	ifunc_ccall = mifunc_ccall;
	ifunc_dcall = mifunc_dcall;
	ifunc_fcall = mifunc_fcall;
	ifunc_icall = mifunc_icall;
	ifunc_lcall = mifunc_lcall;
	ifunc_rcall = mifunc_rcall;
	ifunc_vcall = mifunc_vcall;	
	ifunc_ycall = mifunc_ycall;			
}

/*=============================================================================
// end of file: $RCSfile: motoi.c,v $
==============================================================================*/

