/*=============================================================================

	Copyright (C) 2000-2003 David Hakim.
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

	$RCSfile: motod.c,v $	
	$Revision: 1.16 $
	$Author: dhakim $
	$Date: 2003/03/05 21:49:22 $
 
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

#include "motolex.h"
#include "moto.tab.h"
#include "motod.h"
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

static void motod_classdef(const UnionCell *p);
static void motod_declare(const UnionCell *p);
static void motod_define(const UnionCell *p);
static void motod_list(const UnionCell *p);
static void motod_use(const UnionCell *p);

/*-----------------------------------------------------------------------------
 * interpreter
 *---------------------------------------------------------------------------*/

void motod(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	MetaInfo curmeta = env->errmeta;
	env->errmeta = *uc_meta(p);

	switch(p->type) {
		case VALUE_TYPE:
			break;
		case OP_TYPE:
			switch(p->opcell.opcode) {
				case CLASSDEF: motod_classdef(p); break;
				case DECLARE: motod_declare(p); break;
				case DEFINE: motod_define(p); break;
				case LIST: motod_list(p); break;
				case USE: motod_use(p); break;
				default:
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

char* motod_nameForTypeCell(const UnionCell *p){
	MotoEnv* env = moto_getEnv();
	StringBuffer* sb = (StringBuffer*)opool_grab(env->bufpool);
	int i,dim=0;
	char* typen;
		
	if(uc_opcode(p) == TYPE) {
		typen = uc_str(p, 0);
		dim = uc_opcount(uc_operand(p, 1));
		
		buf_puts(sb,typen);
		for(i=0;i<dim;i++)
			buf_puts(sb,"[]");
	} else if(uc_opcode(p) == ARRAY_TYPE) {
		/* Extract the base type and dimension, then retrieve the MotoType */
		
		typen = motod_nameForTypeCell(uc_operand(p, 0));
		dim = uc_opcount(uc_operand(p, 1));

		buf_puts(sb,typen);
		for(i=0;i<dim;i++)
			buf_puts(sb,"[]");
		
		free(typen);

	} else /* if (uc_opcode(p) == DEF) */ {
		UnionCell* argListUC = uc_operand(p,1);
		int i,argc = argListUC->opcell.opcount;
		
		/* Add in the return type */
		typen = motod_nameForTypeCell(uc_operand(p,0));
		buf_puts(sb,typen);
		free(typen);
		
		buf_puts(sb,"(");
		for(i=0;i<argc;i++){
			if(i>0) buf_puts(sb,",");
			
			typen = motod_nameForTypeCell(uc_operand(argListUC,i));
			buf_puts(sb,typen);
			free(typen);
		}			
		buf_puts(sb,")");
	}
	typen = buf_toString(sb);
	opool_release(env->bufpool,sb);
	return typen;
		
}

void motod_classdef(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	char* classn;
	MotoClassDefinition* mcd;
	
	/* Grab the Class name (operand 1) */
	classn = moto_strdup(env, uc_str(p, 0));

	/* Check to see if this class was already defined */
	if(mttab_get(env->types,classn,0) != NULL){
		if((mcd = moto_getMotoClassDefinition(env,classn)) != NULL)
			moto_classAlreadyDefined(classn,&mcd->definition->opcell.meta);
		else
			// FIXME ! We need a way of discovering which extension defines a type */
			moto_classAlreadyDefinedInExtension(classn,"<unknown>"); 
	} else {
		/* Add a new, non-externally defined, MotoType for this MotoClassDefinition */
		mttab_add(env->types,classn,'\0');
	}
		
	/* Create an new MotoClassDefinition */
	mcd = mcd_create(classn,uc_operand(p,1));
	mman_trackf(env->mpool,mcd,(void(*)(void *))mcd_free);
	
	/* Set the Environment Variable for the ÔCurrent Class DefinitionÕ 
		to the MotoClassDefinition just created */
	env->ccdef = mcd;
	
	/* Call motod on the Statement List */
	motod(uc_operand(p,1));
	
	/* Un Set the ÔCurrent Class DefinitionÕ */
	env->ccdef = NULL;
	
	/* Compute storage requirements for Objects of this type */
	mcd_computeOffsetsAndSize(mcd);
	
	/* Add MotoClassDefinition to the Class Definitions Table */
	moto_setMotoClassDefinition(env,classn, mcd);
	
}

void motod_declare(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	int i;
	/* the type and name of the id being declared */
	char *ctypen, *varn;
	MotoClassDefinition* mcd = env->ccdef;
	
	UnionCell *type_uc,*type_qualifier_uc,*declarator_list_uc;
	
	/* Check that we are looking at a class member variable here, 
		if we are not then return */

	if(mcd == NULL)
		 return;
		 
	
	/* Extract the type_qualifier, type, and variable declarator list */

	type_qualifier_uc = uc_operand(p,0);
	type_uc = uc_operand(p,1);
	declarator_list_uc = uc_operand(p,2);

	/* Extract the canonical complete type name */
	
	ctypen = motod_nameForTypeCell(type_uc);
	
	/* Iterate over the declarators */
	
	for(i=0;i<declarator_list_uc->opcell.opcount;i++) {
		UnionCell* declarator_uc = uc_operand(declarator_list_uc,i);
		StringBuffer* sb = (StringBuffer*)opool_grab(env->bufpool);
		int i,vdim;
		char* mtypen;
		
		/* Get the variable name and dimension */
		varn = moto_strdup(env, uc_str(declarator_uc, 0));
		vdim = uc_opcount(uc_operand(declarator_uc, 1));

		buf_puts(sb,ctypen);
		for(i=0;i<vdim;i++)
			buf_puts(sb,"[]");
		
		mtypen = mman_track(env->mpool,buf_toString(sb));
		
		opool_release(env->bufpool,sb);
			
		/* Set the variable type and dimension in the MotoClassDefinition */
		mcd_addMember(mcd, varn,mtypen);
	}
	
	free(ctypen);
}

void motod_define(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	StringBuffer* sb = (StringBuffer*)opool_grab(env->bufpool);
	char *typen,*atypen,*aname,*fn;
	char **argtypes,**argnames;
	UnionCell *argListUC,*declaration_uc,*type_uc;
	MotoFunction *f=NULL, *g=NULL;
	int argc,i,j;
	char* motoname;
	
	/* Extract the declaration */
	
	declaration_uc = uc_operand(p,0);
	
	if(declaration_uc->type == OP_TYPE) {
	
		/* Extract the type operand and the function name */
		
		type_uc = uc_operand(declaration_uc, 1);
		fn = uc_str(declaration_uc, 2);

		/* Extract the type name and dimension */
		typen = motod_nameForTypeCell(type_uc);
		
	} else {
		fn = uc_str(p,0);
		typen = estrdup(uc_str(p,0));		
	}
	
	/* Extract the arguments */

	argListUC = uc_operand(p,1);
	argc = uc_opcount(argListUC);	 

	if(argc==0) {
		argtypes = NULL;
		argnames = NULL;
	} else {
		argtypes = emalloc(argc * sizeof(char *));
		argnames = emalloc(argc * sizeof(char *));
	}

	for(i=0;i<argc;i++){
		UnionCell* argdec = uc_operand(argListUC,i);
		UnionCell* atype_uc = uc_operand(argdec,0);

		int argdecDim;

		atypen = motod_nameForTypeCell(atype_uc);
		aname = uc_str(argdec,1);
		argdecDim = uc_opcount(uc_operand(argdec, 2));
		
		if(argdecDim > 0){
			buf_clear(sb);
			buf_puts(sb,atypen);
			for(j=0;j<argdecDim;j++)
				buf_puts(sb,"[]");
			free(atypen);
			atypen = buf_toString(sb);
		}
		
		argtypes[i] = atypen;
		argnames[i] = aname;
	}

	/* Generate the function / method / constructor */
	motoname = moto_createMotonameForFn((env->ccdef != NULL?env->ccdef->classn:NULL),fn);
	mman_track(env->mpool,motoname);
 
	f = mfn_create(
		motoname,
		typen,
		argc,
		argtypes,
		argnames,
		NULL, /* The symbol will get filled in lazily */
	
		'\0', /* Don't need to worry about tracked, any memory allocated is already tracked */

		p, /* opcell */
	
		/* not external so we don't need an fptr, handle, or libname */
		NULL, 
		NULL, 
		NULL 
	);
	
	/* Track the MotoFunction in the mpool */
	mman_trackf(env->mpool,f,(void(*)(void *))mfn_free);
	
	if(ftab_getExactMatch(env->ftable,&g, f->motoname, argc, f->argtypes) == FTAB_OK){
		if(env->ccdef == NULL){
			if(g->opcell != NULL)
				moto_functionAlreadyDefined(f->motoname,&g->opcell->opcell.meta);
			else
				moto_functionAlreadyDefinedInExtension(f->motoname,g->libname);
		} else { 
			if(g->opcell != NULL)
				moto_methodAlreadyDefined(f->motoname,&g->opcell->opcell.meta);
			else
				moto_methodAlreadyDefinedInExtension(f->motoname,g->libname);
		}
	}

	ftab_add(env->ftable, f->motoname, f);

	opool_release(env->bufpool,sb);
}

void motod_list(const UnionCell *p) {
	int i;
	for(i=0;i<p->opcell.opcount;i++)
		motod(p->opcell.operands[i]);
}

void motod_use(const UnionCell *p) {
	MotoEnv *env = moto_getEnv();
	char *usename = uc_str(p, 0);
	moto_use(env, usename) ; 
	/* moto_noSuchLibrary(usename); */

}
/*=============================================================================
// end of file: $RCSfile: motod.c,v $
==============================================================================*/

