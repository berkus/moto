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

	$RCSfile: env.c,v $	 
	$Revision: 1.99 $
	$Author: dhakim $
	$Date: 2003/05/29 02:14:08 $
 
==============================================================================*/
#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "memsw.h"

#include "enumeration.h"
#include "exception.h"
#include "excpfn.h"
#include "fileio.h"
#include "log.h"	
#include "mdfa.h"
#include "path.h" 
#include "properties.h"
#include "stringset.h"
#include "sysmalloc.h"
#include "mxstring.h"

#include "cell.h"
#include "motolex.h"
#include "moto.tab.h"
#include "dl.h"	
#include "env.h"	
#include "meta.h" 
#include "motoi.h"
#include "motoc.h"
#include "motov.h"
#include "motoerr.h" 
#include "motoutil.h"	

#define DEFAULT_BUF_SIZE 1024
#define MAX_DATE_LEN 1024

extern int yylineno;

extern int myyparse();

void 
moto_clearEnv(MotoEnv *env) {
	Enumeration *e;
	int size;
	//hset_free(env->ptrs);
	//mpool_free(env->mpool); 

	/* Clear out the globals */
	e = stab_getKeys(env->globals);
	while (enum_hasNext(e)) {
		char* n = (char*)enum_next(e);
		MotoVar* var = stab_get(env->globals,n);

		if(env->mode != COMPILER_MODE) {
			moto_freeVal(env,var->vs);
		} else {
			opool_release(env->valpool,var->vs);
		}
		free(var);
	}
	enum_free(e);
	stab_clear(env->globals);	

	/* Clear out the frames */
	size = vec_size(env->frames);
	while (--size >= 0) {
		MotoFrame *frame = (MotoFrame *)vec_get(env->frames, size);
		stack_free(frame->opstack);
		stab_free(frame->symtab);
		buf_free(frame->out);
		free(frame);
	}
	vec_clear(env->frames);	  
		
	e = stack_elements(env->scope);
	while (enum_hasNext(e)) {
		free(enum_next(e));			 
	}
	enum_free(e);
	
	stack_clear(env->scope);	
	buf_clear(env->out);	  
	buf_clear(env->err);	  

	// stab_free(env->types);	 
	// ftab_free(env->ftable);	  
	// stab_free(env->rxcache);	
	// stack_clear(env->callstack);	 
	sset_clear(env->errs);	 
	// sset_clear(env->uses);

	if (env->mode == COMPILER_MODE) {
		sset_free(env->includes);
		htab_free(env->fdefs);
		htab_free(env->adefs);		
		buf_free(env->constantPool);	 
		buf_free(env->fcodebuffer); 
		istack_free(env->scopeIDStack);
	}	 
	env->frameindex = -1;

	/* error stuff */
	env->meta.filename = env->filename;
	env->meta.caller = NULL;
	env->meta.macroname = NULL;
	env->meta.lineno = 1;

	env->errflag = 0;
	//free(env);	
}

MotoVal *
moto_createEmptyVal(MotoEnv *env) {
	/* Allocate a new MotoVal and track it in  env->mpool . 
	   The reason we need track as well as pool motovals is
	   if an exception is thrown in the interpreter the motoval
	   my not be returned to the pool */
	return mman_track(env->mpool,emalloc(sizeof(MotoVal)));
}

static MotoType *
mttab_addBuiltInType(MotoEnv* env, char *name) {

	MotoType *type = (MotoType *)mman_malloc(env->mpool, sizeof(MotoType));
	if (strcmp(name, "boolean") == 0) { type->kind = BOOLEAN_TYPE; }
	else if (strcmp(name, "byte") == 0) { type->kind = BYTE_TYPE; }
	else if (strcmp(name, "char") == 0) { type->kind = CHAR_TYPE; }
	else if (strcmp(name, "int") == 0) { type->kind = INT32_TYPE; }
	else if (strcmp(name, "long") == 0) { type->kind = INT64_TYPE; }
	else if (strcmp(name, "float") == 0) { type->kind = FLOAT_TYPE; }
	else if (strcmp(name, "double") == 0) { type->kind = DOUBLE_TYPE; }
	else if (strcmp(name, "void") == 0) { type->kind = VOID_TYPE; }
	else {
		type->kind = REF_TYPE;
	}
	type->name = name;

	/* This is a base type so set its atype, argc, argt, and dimension to 0 */
	type->dim = 0;
	type->atype = NULL;
	type->argc = 0;
	type->argt = NULL;
	
	type->isExternallyDefined = '\1';
	stab_put(env->types, name, type);
	return type;
}

MotoEnv *
moto_createEnv(int flags,char* filename) {
	MotoEnv *env = (MotoEnv *)emalloc(sizeof(MotoEnv));	
	
	env->flags = flags;
	env->filename = filename;

	if (flags & MMAN_LEVEL1_FLAG) {
		env->mman_level = 1;
	}
	else if (flags & MMAN_LEVEL2_FLAG) {
		env->mman_level = 2;
	}
	else {
		env->mman_level = 0;
	}

	env->mpool = mpool_create(2048);
	
	env->valpool = opool_createWithExt(
							(void*(*)())moto_createEmptyVal,
							NULL,
							NULL,
							NULL,
							env,
							10
						);
	env->bufpool = opool_create(
							(void*(*)())buf_createDefault,
							(void(*)(void*))buf_free,
							(void(*)(void*))buf_clear,
							NULL,
							10
						);
	env->stkpool = opool_create(
							(void*(*)())stack_createDefault,
							(void(*)(void*))stack_free,
							NULL,
							NULL,
							10
						);
	env->fcache = stab_createDefault();

	env->tree = moto_createTree();
	env->ptrs = hset_createDefault();
	env->out = buf_create(DEFAULT_BUF_SIZE);
	env->err = buf_create(DEFAULT_BUF_SIZE);
	env->types = stab_createDefault();
	env->cdefs = stab_createDefault();
	env->ccdef = NULL;
	env->globals = stab_createDefault();
	env->frames = vec_createDefault();
	env->ftable = ftab_create();	
	env->scope = stack_createDefault(); 
	env->rxcache = stab_createDefault();
	env->uses = sset_createDefault();

	mttab_addBuiltInType(env, "boolean");
	mttab_addBuiltInType(env, "byte");
	mttab_addBuiltInType(env, "char");
	mttab_addBuiltInType(env, "int");
	mttab_addBuiltInType(env, "long");
	mttab_addBuiltInType(env, "float");
	mttab_addBuiltInType(env, "double");
	mttab_addBuiltInType(env, "void");			/* Special symbol for the void type */
	mttab_addBuiltInType(env, "null");			/* Special symbol for null type */
	mttab_addBuiltInType(env, "Object");
	mttab_addBuiltInType(env, "String");
	mttab_addBuiltInType(env, "Exception");
	mttab_addBuiltInType(env, "Regex"); 
	
	/* compiler extras */
	env->includes = sset_createDefault();
	sset_add(env->includes, "<stdio.h>");
	sset_add(env->includes, "\"mxarr.h\"");
	sset_add(env->includes, "\"stringbuffer.h\"");
	sset_add(env->includes, "\"excpfn.h\"");
	sset_add(env->includes, "\"mman.h\"");
	sset_add(env->includes, "\"runtime.h\"");
	sset_add(env->includes, "\"cdx_function.h\"");
	
	env->fdefs = htab_createDefault();
	env->adefs = htab_createDefault();	
	env->constantPool = buf_createDefault();
	env->constantCount = 0;
	env->fcodebuffer = buf_createDefault();


	env->curScopeID = 0;				
	env->scopeIDStack = istack_createDefault();

	env->frameindex = -1;

	/* error stuff */
	env->errs = sset_createDefault();

	env->meta.filename = env->filename;
	env->meta.caller = NULL;
	env->meta.macroname = NULL;
	env->meta.lineno = 1;

	env->errflag = 0;

	return env;
}

void 
moto_createFrame(MotoEnv *env, int type) {
	MotoFrame *frame;

	if (env->frameindex >= MAX_FRAMES) 
		THROW("RuntimeException","Stack Overflow");
	
	frame = emalloc(sizeof(MotoFrame));
	
	frame->type = type;
	frame->opstack = (Stack*)opool_grab(env->stkpool); 
	frame->symtab = stab_createDefault();	
	frame->out = (StringBuffer*)opool_grab(env->bufpool);
	
	vec_add(env->frames, frame);
	env->frameindex = env->frameindex + 1;

	env->frame = frame;

}


MotoVal *
moto_cloneVal(MotoEnv *env, MotoVal* orig) {
	MotoVal *val;
 
	val = opool_grab(env->valpool);
	*val = *orig;

	return val;
}

void
moto_initVal(MotoEnv *env, MotoVal* val, char* typen, int dim) {
  
  	val->type = mttab_get(env->types, typen, dim);	 
  	val->codeval.value = NULL;
  	
  	if (val->type == NULL)
  		THROW("IllegalArgumentException","Type '%s' is undefined",typen);
	
	switch (val->type->kind) {
		case BOOLEAN_TYPE: val->booleanval.value = '\0'; break;
		case BYTE_TYPE: val->byteval.value = '\0'; break;
		case CHAR_TYPE: val->charval.value = '\0'; break;
		case INT32_TYPE: val->intval.value = 0; break;
		case INT64_TYPE: val->longval.value = 0; break;
		case FLOAT_TYPE: val->floatval.value = 0.0f; break;
		case DOUBLE_TYPE: val->doubleval.value = 0.0f; break;
		case VOID_TYPE: val->refval.value = NULL; /*FIXME: This probably doesn't matter */
			break;
		case REF_TYPE: val->refval.value = NULL; break;
		default:
			THROW_D("IllegalArgumentException");
			break;
	}
}


MotoVal *
moto_createVal(MotoEnv *env, char *typen,int dim) {
 	/* Grab a new MotoVal */
	MotoVal *val = (MotoVal*)opool_grab(env->valpool);
	
	/* Initialize it */
	moto_initVal(env,val,typen,dim);
	
	/* Return it */
	return val;
}

MotoClassDefinition* 
moto_getMotoClassDefinition(MotoEnv* env, char* name){
	return (MotoClassDefinition*)stab_get(env->cdefs,name);
}

void 
moto_setMotoClassDefinition(MotoEnv* env, char* name, MotoClassDefinition* mcd){
	 stab_put(env->cdefs,name,mcd);
}

MotoVar *
moto_createVar(MotoEnv *env, char* varn, char *typen, int dim,char isMemberVar,void *address){
	MotoVar* var;
		 
	var = (MotoVar *)emalloc(sizeof(MotoVar));
	var->type = mttab_get(env->types,typen,dim);
	var->n = varn;
	var->isMemberVar = isMemberVar;
	var->address = address;
	
	var->vs = moto_createVal(env,typen,dim);	 

	return var;
}

MotoVar *
moto_declareVar(MotoEnv *env, char* varn, char *typen, int dim , char isGlobal) {
	MotoVar *var;
	
	var = stab_get(env->frame->symtab, varn);
	
	/* Blow up if the var has already been defined in this frame */
	excp_assert(var == NULL, THROW_D("IllegalArgumentException"));
	
	var = moto_createVar(env,varn,typen,dim,'\0',NULL);
	
	if(isGlobal) 
	  stab_put(env->globals, var->n, var);
	else
	  stab_put(env->frame->symtab, var->n, var);
	
	return var;
}

void 
moto_delete(MotoEnv *env, MotoVal *val) {
	char *classname;
	void *r;
	char *fname;
	int code;
	void **argv;
	MotoFunction *f;

	classname = val->type->name;
	
	/* allocate enough room for a destructor function name */
	/* 2 x classname plus two colons, one tilde, the string '(0)' and one '\0' */
	fname = emalloc((2 * strlen(classname)) + 7);
	sprintf(fname, "%s::~%s(0)", classname, classname);

	code = ftab_get(env->ftable, &f, fname, 0, NULL);
	switch (code) {
		case FTAB_NONE:
		case FTAB_MULTIPLE:
			free(val->refval.value);
			break;
		case FTAB_OK:
			/* call destructor */
			argv = emalloc(sizeof(void *));
			argv[0] = val->refval.value;
			(*(f->fptr))(env, 1, argv, &r);
			free(argv);
			break;
	}

	free(fname);

}

void 
moto_emitCFooter(MotoEnv *env, StringBuffer *out) {
	buf_puts(out, "\n");	  
	buf_puts(out, "	/* END GENERATED CODE */\n\n");	 
	buf_puts(out, "}\n\n");	  
	buf_puts(out, "#ifdef __MAIN__\n\n");
	buf_puts(out, "int __MAIN__(int argc,char** argv) {\n");	  
	buf_puts(out, "	StringBuffer *out;\n");
	buf_puts(out, "	shared_initDefault();\n");
	buf_puts(out, "	stacktrace_init();\n");	
	buf_puts(out, "	mman_init();\n");
	buf_puts(out, "	rtime_init();\n");
	buf_puts(out, "	TRY{\n");
	buf_puts(out, "	   out = buf_createDefault();\n");	 
	buf_puts(out, "	   __EXEC__");	  
	buf_puts(out, "    (out);\n");	  
	buf_puts(out, "	   {\n");
	buf_puts(out, "		  int totalBytesWritten = 0;\n");
	buf_puts(out, "	      char* data = buf_data(out);\n");				  
	buf_puts(out, "		  int bytesToWrite = buf_size(out);\n");
			  
	buf_puts(out, "		  while (totalBytesWritten < bytesToWrite) {\n");
	buf_puts(out, "			 int bytesWritten = 0; \n");
	buf_puts(out, "			 bytesWritten = \n");
	buf_puts(out, "				fwrite(data + totalBytesWritten, 1, bytesToWrite-totalBytesWritten, stdout);\n");
	buf_puts(out, "			 totalBytesWritten += bytesWritten;\n");
	buf_puts(out, "		  }\n");
	buf_puts(out, "	   }\n");
	buf_puts(out, "	}CATCH_ALL{\n");
	buf_puts(out, "	    printf(\"Uncaught %s\n   message:%s\n\n%s\",excp_getCurrentException()->t,excp_getCurrentException()->msg,excp_getCurrentException()->stacktrace);");
	buf_puts(out, "	}END_TRY\n");				
	buf_puts(out, "	buf_free(out);\n");	 
	buf_puts(out, "	rtime_cleanup();\n");
	buf_puts(out, "	mman_cleanup();\n");
	buf_puts(out, "	while (1) {\n");
	buf_puts(out, "		int c = getopt(argc, argv, \"dl\");\n");
	buf_puts(out, "		if(c==-1) break;\n");
	buf_puts(out, "		else if(c=='d') debug();\n");
	buf_puts(out, "		else if(c=='l') {\n");
	buf_puts(out, "			if (shared_nodesAllocated() == 1) {\n");
	buf_puts(out, "				printf(\"//mem-ok//\\n\");\n");
	buf_puts(out, "			} else {\n");
	buf_puts(out, "				printf(\"//mem-leak//\\n\");\n");
	buf_puts(out, "			}\n");
	buf_puts(out, "		}\n");
	buf_puts(out, "	}\n");
	buf_puts(out, "	stacktrace_cleanup();\n");
	buf_puts(out, "	shared_cleanup();\n");
	buf_puts(out, "	return 0;\n");	  
	buf_puts(out, "}\n");  
	buf_puts(out, "#endif\n") ;
	buf_puts(out, "/* END OF FILE */\n");	 
}

void 
moto_emitCPrototypes(MotoEnv *env, StringBuffer *out){
	Enumeration* e;

	buf_puts(out, "/* BEGIN GENERATED FUNCTION PROTOTYPES */\n\n");
	e = htab_getKeys(env->fdefs);
	while (enum_hasNext(e)) {
		MotoFunction *f = (MotoFunction*)enum_next(e);

		buf_puts(out, mfn_cprototype(f));
		buf_puts(out, ";\n");
	}
	buf_puts(out, "/* END GENERATED FUNCTION PROTOTYPES */\n\n");
	
	buf_puts(out, "/* BEGIN GENERATED ANONYMOUS FUNCTION PROTOTYPES */\n\n");
	e = htab_getKeys(env->adefs);
	while (enum_hasNext(e)) {
		MotoFunction *f = (MotoFunction*)enum_next(e);

		buf_puts(out, mfn_cprototype(f));
		buf_puts(out, ";\n");
	}
	buf_puts(out, "/* END GENERATED ANONYMOUS FUNCTION PROTOTYPES */\n\n");
}

void 
moto_emitCStructures(MotoEnv *env, StringBuffer *out){
	Enumeration* e;
	buf_puts(out, "/* BEGIN GENERATED STRUCTURES */\n\n");
	
	/* Foreach MotoClassDefinition */
	
	e = stab_getValues(env->cdefs);
	while(enum_hasNext(e)){
		MotoClassDefinition* mcd = (MotoClassDefinition*)enum_next(e);
		Enumeration* ve;
		
		/* Output Ôtypedef struct _<typename> {Ô */
		buf_printf(out, "typedef struct _%s {\n",mcd->classn);
		
		/* Foreach var in the MotoClassDefinition */
		ve = vec_elements(mcd->memberVarNames);
		while(enum_hasNext(ve)){
			char* varn = (char*)enum_next(ve);
			
			MotoVar* mv = moto_createVar(env,varn,mcd_getMemberType(mcd,varn),0,'\0',NULL); 
						
			/* if the var is another MotoClassDefinition */ 
				/* output Ôstruct _<var typename> *Õ */
			/* else */
				/* output <typename> */
			/* output varname */
			/* output Ô;\nÕ */
				
			buf_printf(out,"	 %s %s;\n", moto_valToCType(mv->vs), mv->n);
			
			moto_freeVar(env,mv);
		}
		enum_free(ve);
			
		/* output Ô} <typename>;Õ */
		buf_printf(out, "} %s;\n\n",mcd->classn);
	}
	enum_free(e);

	buf_puts(out, "/* END GENERATED STRUCTURES */\n\n");
}

void 
moto_emitCImplicitConstructorPrototypes(MotoEnv *env, StringBuffer *out){
	Enumeration* e;
	buf_puts(out, "/* BEGIN GENERATED IMPLICIT CONSTRUCTOR PROTOTYPES */\n\n");
	
	/* Foreach MotoClassDefinition */
	
	e = stab_getValues(env->cdefs);
	while(enum_hasNext(e)){
		MotoClassDefinition* mcd = (MotoClassDefinition*)enum_next(e);
		
		/* output an implicit constructor prototype */
		buf_printf(out, "static %s* _%s_%s___();\n",
			mcd->classn,mcd->classn,mcd->classn);
	}
	enum_free(e);

	buf_puts(out, "/* END GENERATED IMPLICIT CONSTRUCTOR PROTOTYPES */\n\n");	
}

void 
moto_emitCImplicitConstructors(MotoEnv *env, StringBuffer *out){
	Enumeration* e;
	buf_puts(out, "/* BEGIN GENERATED IMPLICIT CONSTRUCTORS */\n\n");
	
	/* Foreach MotoClassDefinition */
	
	e = stab_getValues(env->cdefs);
	while(enum_hasNext(e)){
		MotoClassDefinition* mcd = (MotoClassDefinition*)enum_next(e);
		Enumeration* ve;
		
		/* Output the implicit constructor prototype for this MotoClassDefinition */
		buf_printf(out, "static %s* _%s_%s___(){\n",mcd->classn,mcd->classn,mcd->classn);

		/* Allocate the space for the type being constructed and set ÔthisÕ */
		buf_printf(out, "	  %s* this = (%s*)mman_track(rtime_getMPool(),emalloc(sizeof(%s)));\n",
			mcd->classn,mcd->classn,mcd->classn);
		
		/* Foreach var in the MotoClassDefinition */
		ve = vec_elements(mcd->memberVarNames);
		while(enum_hasNext(ve)){
			char* varn = (char*)enum_next(ve);
			
			MotoVar* mv = moto_createVar(env,varn,mcd_getMemberType(mcd,varn),0,'\0',NULL); 
	
			buf_printf(out,"	 this->%s=%s;\n",	 
				mv->n,moto_defaultValForCType(mv->vs)
			);
			
			moto_freeVar(env,mv);
	
		}
		enum_free(ve);
		
		/* Output the mcd->code */
		buf_puts(out,mcd->code);
		
		/* Output 'return this\n}\n\nÕ */
		buf_puts(out,"	  return this;\n}\n\n");
	} 
	enum_free(e);

	buf_puts(out, "/* END GENERATED IMPLICIT CONSTRUCTORS */\n\n");
}

void 
moto_emitCGlobals(MotoEnv *env, StringBuffer *out){
	Enumeration* e;
	buf_puts(out, "/* BEGIN GENERATED GLOBAL DECLARATIONS */\n\n");

	e = stab_getKeys(env->globals);
	while (enum_hasNext(e)) {
		char* n = (char*)enum_next(e);
		MotoVar* mv = stab_get(env->globals,n);

		buf_printf(out,"static %s _G_%s=%s;\n", moto_valToCType(mv->vs), n,moto_defaultValForCType(mv->vs));

	}
	enum_free(e);

	buf_puts(out, "/* END GENERATED GLOBAL DECLARATIONS */\n\n");
}

void 
moto_emitCFunctions(MotoEnv *env, StringBuffer *out){
	Enumeration* e;

	buf_puts(out, "/* BEGIN GENERATED FUNCTION DEFINITIONS */\n\n");
	e = htab_getKeys(env->fdefs);
	while (enum_hasNext(e)) {
		MotoFunction *f = (MotoFunction *)enum_next(e);
		buf_puts(out, mfn_cprototype(f));
		buf_puts(out, "{\n");
		buf_puts(out,(char*)htab_get(env->fdefs,f));
		buf_puts(out, "}\n\n");
	}
	buf_puts(out, "/* END GENERATED FUNCTION DEFINITIONS */\n\n");
	
	buf_puts(out, "/* BEGIN GENERATED ANONYMOUS FUNCTION DEFINITIONS */\n\n");
	e = htab_getKeys(env->adefs);
	while (enum_hasNext(e)) {
		MotoFunction *f = (MotoFunction *)enum_next(e);
		buf_puts(out, mfn_cprototype(f));
		buf_puts(out, "{\n");
		buf_puts(out,(char*)htab_get(env->adefs,f));
		buf_puts(out, "}\n\n");
	}
	buf_puts(out, "/* END GENERATED ANONYMOUS FUNCTION DEFINITIONS */\n\n");
}

void 
moto_emitCHeader(MotoEnv *env, StringBuffer *out) {
	Enumeration *e;
	char bts[MAX_DATE_LEN];
	char gts[MAX_DATE_LEN];
	int maxlen;
	char *execname = path_alloc(&maxlen);
	char *filename = env->filename;
	int i;
	for (i = 0; i < maxlen; i++) {
		if (filename[i] == '\0' || filename + i == strrchr(filename,'.')) {
			execname[i] = '\0';
			break;
		} else if ( !isalpha(filename[i]) && !isdigit(filename[i]) ) {
			execname[i] = '_';
		} else {
			execname[i] = filename[i];
		}
	}
	moto_buildTimeStamp(bts);
	moto_curTimeStamp(gts);
	buf_puts(out, "/***************** DO NOT EDIT ****************\n");	 
	buf_puts(out, " * FILE GENERATED BY THE MOTO COMPILER\n");	 
	buf_puts(out, " * Moto Build: ");	
	buf_puts(out, bts);	 
	buf_puts(out, "\n");	  
	buf_puts(out, " * Generated:	");	
	buf_puts(out, gts);	 
	buf_puts(out, "\n");	  
	buf_puts(out, " * Input File: ");	
	buf_puts(out, env->filename);	  
	buf_puts(out, "\n");	  
	buf_puts(out, " **********************************************/\n\n");	 
	e = (sset_elements(env->includes));
	while (enum_hasNext(e)) {
		char *incl = (char *)enum_next(e);
		buf_puts(out, "#include ");	
		buf_puts(out, incl);	  
		buf_puts(out, "\n");	  
	}
	enum_free(e);
	buf_puts(out, "\n");
	buf_puts(out, "#ifdef SHARED_MALLOC\n");
	buf_puts(out, "#	define STD_FREE_FN shared_free\n");
	buf_puts(out, "#else\n");
	buf_puts(out, "#	define STD_FREE_FN free\n");
	buf_puts(out, "#endif\n");
	buf_puts(out, "\n");
	buf_puts(out, "#ifdef HAVE_FN_NAME_H\n");	  
	buf_puts(out, "#include \"mod_fn.h\"\n");	  
	buf_puts(out, "#endif\n");	  
	buf_puts(out, "#ifndef __MAIN__\n");	
	buf_puts(out, "#	 define __MAIN__ main_");	 
	buf_puts(out, execname);
	buf_puts(out, "\n");	  
	buf_puts(out, "#endif\n");	  
	buf_puts(out, "#define __EXEC__ exec_");	 
	buf_puts(out, execname);	
	buf_puts(out, "\n");
	buf_puts(out, buf_data(env->constantPool));
	buf_puts(out, "\n\n");	 

	buf_puts(out, "enum {\n");	  
	buf_puts(out, "NOOP_ACTION = 0,\n");	
	buf_puts(out, "BREAK_ACTION = 1,\n");	 
	buf_puts(out, "CONTINUE_ACTION = 2,\n");	 
	buf_puts(out, "RETURN_ACTION = 3\n");	 
	buf_puts(out, "};\n");	 
	buf_puts(out, "static int _MOTO_ACTION = 0;\n");	
	
	moto_emitCStructures(env,out);
	
	moto_emitCImplicitConstructorPrototypes(env,out);
	moto_emitCPrototypes(env,out);

	buf_puts(out, "StringBuffer *out;\n"); /* FIXME: big hack to get output buffer to functions */
	
	/* FIXME : The following hacks all exist because non-gcc compilers don't like inline array instantiation */
	buf_puts(out, "int _MDS_[20];\n\n"); /* FIXME: big hack to allow for array instantiation */
	buf_puts(out, "int _PARGI_[20];\n\n"); /* FIXME: big hack to allow for partially applied functions */
	buf_puts(out, "FuncTypeKind _PARGT_[20];\n\n"); /* FIXME: big hack to allow for partially applied functions */

	moto_emitCGlobals(env,out);
	moto_emitCImplicitConstructors(env,out);
	moto_emitCFunctions(env,out);

	buf_puts(out, "void __EXEC__");	 
	buf_puts(out, "(StringBuffer *outputBuffer) {\n\n");	 
	buf_puts(out, "	/* BEGIN GENERATED CODE */\n\n");	
	free(execname);
}

void 
moto_freeCell(MotoEnv *env, UnionCell *p) {
	int i;
	if (p->type == OP_TYPE) {
		int opcount = p->opcell.opcount;
		for (i = 0; i < opcount; i++) {
			moto_freeCell(env, p->opcell.operands[i]);
		}
	}
	mman_free(p);
}

void 
moto_freeEnv(MotoEnv *env) {
	Enumeration *e;

	/* Free any outstanding frames */
	while(vec_size(env->frames) > 0){
		moto_freeFrame(env);
	}
	
	/* Free all the globals */
	e = stab_getKeys(env->globals);
	while (enum_hasNext(e)) {
		char* n = (char*)enum_next(e);
		MotoVar* var = stab_get(env->globals,n);

		if(env->mode != COMPILER_MODE) {
			moto_freeVal(env,var->vs);
		} else {
			opool_release(env->valpool,var->vs);
		}
		free(var);
	}
	enum_free(e);
	stab_free(env->globals);

	/* free all cached regular expressions */
	e = stab_getKeys(env->rxcache);
	while (enum_hasNext(e)) {
		char *rx = (char *)enum_next(e);
		MDFA *mdfa = (MDFA *)stab_get(env->rxcache, rx);
		if (mdfa != NULL) {
			mdfa_free(mdfa);
		}
	}
	enum_free(e);
		
	/* free all remaining pointers */
	e = hset_elements(env->ptrs);
	while (enum_hasNext(e)) {
		void *ptr = enum_next(e);
		if (shared_check(ptr)) {
			free(ptr);			 
		}
	}
	enum_free(e);
		
	/* free all errors */
	e = sset_elements(env->errs);
	while (enum_hasNext(e)) {
		void *ptr = enum_next(e);
		if (shared_check(ptr)) {
			free(ptr);			 
		}
	}
	enum_free(e);

	/* free all scopes */
	e = stack_elements(env->scope);
	while (enum_hasNext(e)) {
		free(enum_next(e));			 
	}
	enum_free(e);

	/* free all cells */
	moto_freeTreeCells(env);

	/* free all class defs */
	stab_free(env->cdefs);
	
	/* free remainder of env struct */
	hset_free(env->ptrs);
	buf_free(env->out);	 
	buf_free(env->err);	 
	stab_free(env->types);	
	vec_free(env->frames);	
		 
	ftab_free(env->ftable); 
	
	/* Free all the stuff that got put in the mpool ... this includes
		MotoFunctions and MotoClassDefinitions */	  
	mpool_free(env->mpool);
	
	stack_free(env->scope);	  
	stab_free(env->rxcache);	 
	//stack_free(env->callstack);	  
	sset_free(env->errs);	
	sset_free(env->uses);
	sset_free(env->includes);
	buf_free(env->fcodebuffer);
	istack_free(env->scopeIDStack);

	htab_free(env->fdefs);
	htab_free(env->adefs);	
	buf_free(env->constantPool);	 
	moto_freeTree(env->tree);

	opool_free(env->valpool);
	opool_free(env->bufpool);
	opool_free(env->stkpool);

	e = stab_getKeys(env->fcache);
	while (enum_hasNext(e)) 
		free((char *)enum_next(e));
	enum_free(e);
	stab_free(env->fcache);

	free(env);	 
}

void 
moto_freeFrame(MotoEnv *env) {
	MotoFrame *frame = env->frame;
	MotoFrame *pframe = NULL;
	StringBuffer *pout = NULL;
	Enumeration *e;
  
	/* Append declarations if in compiler mode and not in a function frame*/
	if (env->mode == COMPILER_MODE && frame->type != FN_FRAME) { 

		if (frame->type == MAIN_FRAME) 
			pout = env->out;
		else if(frame->type == SUB_FRAME)
			pout = ((MotoFrame *)vec_get(env->frames, env->frameindex - 1))->out;

		e = stab_getKeys(frame->symtab);
		while (enum_hasNext(e)) {
			int indent = env->frameindex+1;
			char* n = (char*)enum_next(e);
			MotoVar* mv = (MotoVar*)stab_get(frame->symtab,n);

			while (--indent >= 0) 
				buf_puts(pout, "	 ");

			buf_printf(pout,"%s %s=%s;\n", moto_valToCType(mv->vs), n,moto_defaultValForCType(mv->vs));
			
		}
		enum_free(e);

		buf_puts(pout, "\n"); 

		if (frame->type == MAIN_FRAME)  /* FIXME : big hack to get out buffer set for functions */
			buf_printf(pout,"out = outputBuffer;\n");
	}
	
	switch (frame->type) {
		case MAIN_FRAME:
			/* append buffer from frame to env main buffer */
			pout = env->out;
			buf_cat(pout, frame->out);
			break;
		case FN_FRAME:
			if (env->mode != COMPILER_MODE) {
				/* append buffer from frame current function buffer */
				pframe = (MotoFrame *)vec_get(env->frames, env->frameindex - 1);
				buf_cat(pframe->out, frame->out);
			}else {
				/* append buffer from frame current function buffer */
				buf_cat(env->fcodebuffer, frame->out);
			}
			break;
		case SUB_FRAME:
		default:
			/* append buffer from frame to buffer in parent frame */
			pframe = (MotoFrame *)vec_get(env->frames, env->frameindex - 1);
			buf_cat(pframe->out, frame->out);
			break;
	}

	/* Release the frame's output buffer */
	opool_release(env->bufpool,frame->out);
			
	/* Free frame structure */
	opool_release(env->stkpool,frame->opstack);

	/* Free vars and associated vals */
	e = stab_getKeys(frame->symtab);
	while (enum_hasNext(e)) {
		char* n = (char*)enum_next(e);
		MotoVar* var = (MotoVar*)stab_get(frame->symtab,n);
  
		/* Do not free vars that are class member vars */
		
		
		  if(env->mode != COMPILER_MODE) {
			 moto_freeVal(env,var->vs);
		  } else {
			 opool_release(env->valpool,var->vs);
		  }
		  free(var);
		
	}
	enum_free(e);

	stab_free(frame->symtab);
	
	/* set new frame */

	vec_removeAt(env->frames, env->frameindex);	 
	env->frameindex = env->frameindex - 1;
	if(env->frameindex < 0)
		env->frame = NULL;
	else
		env->frame = vec_get(env->frames, env->frameindex);
	
	free(frame);
	
}

void 
moto_freeTreeCells(MotoEnv *env) {
	MotoTree *tree = env->tree;
	Vector *cells = tree->cells;
	int size = vec_size(cells);
	int i;
	for (i = 0; i < size; i++) {
		moto_freeCell(env, vec_get(cells, i));
	}
}

/* Free a MotoVar */
void
moto_freeVar(MotoEnv *env, MotoVar *var) {
	moto_freeVal(env,var->vs);
	free(var);
}

void 
moto_freeVal(MotoEnv *env, MotoVal *val) {
	
	if (env->mode == COMPILER_MODE) {
		if (val->codeval.value != NULL) {
			free(val->codeval.value);
		}
	} else if (val != NULL ) {
		switch (val->type->kind) {
			case REF_TYPE: 
				/*

				may want to do something here with strings or regexes in the interpreter
				but otherwise nothing

				*/
				break;
			default:
				break;
		}
	}
	
	opool_release(env->valpool,val);
}

MotoVar *
moto_getFrameVar(MotoEnv *env, char *name) {
	return stab_get(env->frame->symtab, name);
}

MotoVar *
moto_getGlobalVar(MotoEnv *env, char *name) {
	return stab_get(env->globals, name);
}

MotoVal *
moto_getVal(MotoEnv *env, char *name) {
	MotoVar *var = NULL;
	
	if((var = moto_getVar(env, name)) != NULL)
		return var->vs;

	return NULL;
}

MotoVar *
moto_getVar(MotoEnv *env, char *name) {
	MotoVar *var = NULL;
	
	if ((var = (MotoVar *)stab_get(env->frame->symtab, name)) == NULL) {		  
		int i = env->frameindex;
		while (--i >= 0) {
			MotoFrame *frame = (MotoFrame *)vec_get(env->frames, i);
			SymbolTable *symtab = frame->symtab;
			if ((var = (MotoVar *)stab_get(symtab, name)) != NULL) {
				break;
			}
			if (frame->type == FN_FRAME)
				break;
		}
	}

	/* If we still don't have it, check the globals */

	if(var==NULL){
		var=stab_get(env->globals, name);
	}
	
	return var;
}

char 
moto_isVarGlobal(MotoEnv *env, char *name){
	MotoVar* var;

	if ((var = (MotoVar *)stab_get(env->frame->symtab, name)) == NULL) {
		int i = env->frameindex;
		while (--i >= 0) {
			MotoFrame *frame = (MotoFrame *)vec_get(env->frames, i);
			SymbolTable *symtab = frame->symtab;
			if ((var = (MotoVar *)stab_get(symtab, name)) != NULL) {
				break;
			}
			if (frame->type == FN_FRAME)
				break;
		}
	}

	/* If we still don't have it, check the globals */

	if(var==NULL){
		var=stab_get(env->globals, name);
		if(var!=NULL) return '\1';
	}

	return '\0';
}


/*
 * Sets the value of the variable 'var' to the value stored by motovalue 'val'
 * doing any appropriate implicit casts
 */

void 
moto_setVarVal(MotoEnv *env, MotoVar *var, MotoVal *val) {
	moto_castVal(env,var->vs,val);
	
	/* If the MotoVar is a member var then we need to set the var in the MCI
		which this var is a member of */
	
	if(var->isMemberVar) {
		switch (var->type->kind) {
			case INT32_TYPE:	 *(int32_t*)var->address = iv(var->vs); break;
			case INT64_TYPE:	 *(int64_t*)var->address = lv(var->vs); break;
			case FLOAT_TYPE:	 *(float*)var->address = fv(var->vs); break;
			case DOUBLE_TYPE:	 *(double*)var->address = dv(var->vs); break;
			case BOOLEAN_TYPE: *(unsigned char*)var->address = bv(var->vs); break;
			case BYTE_TYPE:	 *(signed char*)var->address = yv(var->vs); break; 
			case CHAR_TYPE:	 *(char*)var->address = cv(var->vs); break;
			case REF_TYPE:		 *(void**)var->address = var->vs->refval.value; break;
			default: break;
		}
	}
}

/*
 * Creates a new MotoVal and stores the value of the variable 'var' in it
 */
 
MotoVal*
moto_getVarVal(MotoEnv* env, MotoVar *var){
	
	/* Make sure this vars value is current */
	
	if(var->isMemberVar) {
		switch (var->type->kind) {
			case INT32_TYPE:	 var->vs->intval.value = *(int32_t*)var->address ; break;
			case INT64_TYPE:	 var->vs->longval.value = *(int64_t*)var->address ; break;
			case FLOAT_TYPE:	 var->vs->floatval.value = *(float*)var->address ; break;
			case DOUBLE_TYPE:	 var->vs->doubleval.value = *(double*)var->address ; break;
			case BOOLEAN_TYPE: var->vs->booleanval.value = *(unsigned char*)var->address ; break;
			case BYTE_TYPE:	 var->vs->byteval.value = *(signed char*)var->address ; break; 
			case CHAR_TYPE:	 var->vs->charval.value = *(char*)var->address ; break;
			case REF_TYPE:		 var->vs->refval.value = *(void**)var->address ; break;
			default: break;
		}
	}
	
	return moto_cloneVal(env,var->vs);
}

void 
moto_castVal(MotoEnv *env, MotoVal *to, MotoVal *from) {

	switch (to->type->kind) {
		case BOOLEAN_TYPE: to->booleanval.value = from->booleanval.value; break;
		case BYTE_TYPE: 
			switch (from->type->kind) {
				case BYTE_TYPE: to->byteval.value = yv(from); break;
				case CHAR_TYPE: to->byteval.value = cv(from); break;
				case INT32_TYPE: to->byteval.value = iv(from); break;
				case INT64_TYPE: to->byteval.value = lv(from); break;
				case FLOAT_TYPE: to->byteval.value = fv(from); break;
				case DOUBLE_TYPE: to->byteval.value = dv(from); break;
				default: THROW_D("MotoException"); break;
			}
			break;
		case CHAR_TYPE: 
			switch (from->type->kind) {
				case BYTE_TYPE: to->charval.value = yv(from); break;
				case CHAR_TYPE: to->charval.value = cv(from); break;
				case INT32_TYPE: to->charval.value = iv(from); break;
				case INT64_TYPE: to->charval.value = lv(from); break;
				case FLOAT_TYPE: to->charval.value = fv(from); break;
				case DOUBLE_TYPE: to->charval.value = dv(from); break;
				default: THROW_D("MotoException"); break;
			}
			break;
		case INT32_TYPE:
			switch (from->type->kind) {
				case BYTE_TYPE: to->intval.value = yv(from); break;
				case CHAR_TYPE: to->intval.value = cv(from); break;
				case INT32_TYPE: to->intval.value = iv(from); break;
				case INT64_TYPE: to->intval.value = lv(from); break;
				case FLOAT_TYPE: to->intval.value = fv(from); break;
				case DOUBLE_TYPE: to->intval.value = dv(from); break;
				default: THROW_D("MotoException"); break;
			}
			break;
		case INT64_TYPE:
			switch (from->type->kind) {
				case BYTE_TYPE: to->longval.value = yv(from); break;
				case CHAR_TYPE: to->longval.value = cv(from); break;
				case INT32_TYPE: to->longval.value = iv(from); break;
				case INT64_TYPE: to->longval.value = lv(from); break;
				case FLOAT_TYPE: to->longval.value = fv(from); break;
				case DOUBLE_TYPE: to->longval.value = dv(from); break;
				default: THROW_D("MotoException"); break;
			}	  
			break;
		case FLOAT_TYPE:
			switch (from->type->kind) {
				case BYTE_TYPE: to->floatval.value = yv(from); break;
				case CHAR_TYPE: to->floatval.value = cv(from); break;
				case INT32_TYPE: to->floatval.value = iv(from); break;
#ifdef CAST_INT64_TO_INT32_FIRST
				case INT64_TYPE: to->floatval.value = (int32_t)lv(from); break;
#else
				case INT64_TYPE: to->floatval.value = lv(from); break;
#endif
				case FLOAT_TYPE: to->floatval.value = fv(from); break;
				case DOUBLE_TYPE: to->floatval.value = dv(from); break;
				default: THROW_D("MotoException"); break;
			}	  
			break;
		case DOUBLE_TYPE:
			switch (from->type->kind) {
				case BYTE_TYPE: to->doubleval.value = yv(from); break;
				case CHAR_TYPE: to->doubleval.value = cv(from); break;
				case INT32_TYPE: to->doubleval.value = iv(from); break;
				case INT64_TYPE: to->doubleval.value = lv(from); break;
				case FLOAT_TYPE: to->doubleval.value = fv(from); break;
				case DOUBLE_TYPE: to->doubleval.value = dv(from); break;
				default: THROW_D("MotoException"); break;
			}	  
			break;
		case REF_TYPE:
			switch (from->type->kind) {
				case REF_TYPE:
					to->refval.value = from->refval.value;
					/* This causes big problems when assigning to a variable */
					/* to->refval.subtype = from->refval.subtype;
					to->refval.dim = from->refval.dim; */
					break;
				default: THROW_D("MotoException"); break;

			}
			break;
		default: THROW_D("MotoException"); break;
	}

}

int 
moto_use(MotoEnv *env, char *usename) {
	char *libpath;

	libpath = mxdl_find(usename);
	if (libpath == NULL) {
		mman_track(env->mpool, libpath);
		return -1;
	} 

	if(!sset_contains(env->uses,usename) ){ 
		int i;
		Enumeration* e;
		MotoExtension* mx = mxdl_load(usename);
		sset_add(env->uses,moto_strdup(env,usename)) ;
		
		/* Track the MotoExtension in the mpool */
		mman_trackf(env->mpool,mx,(void(*)(void *))mext_free);
			
		/* Collect all the new includes required by this extension */
		for (i = 0; i < mx->includeCount; i++)	  
			sset_add(env->includes,mx->includes[i]);
			
		e = mext_getFunctions(mx);
		while(enum_hasNext(e)){
			MotoFunction* mfn = enum_next(e);
			
			/* Track the MotoFunction in the mpool */
			mman_trackf(env->mpool,mfn,(void(*)(void *))mfn_free);
			
			/* Add the function to the ftable */
			ftab_add(env->ftable, mfn->motoname, mfn);
			
			/* Define a new type if need be */
			if (mfn->deftype != NULL) {
				if (mttab_get(env->types, mfn->deftype,0) == NULL) {
					mttab_add(env->types, mfn->deftype,'\1');
				}
			}
		}
		enum_free(e);
			
	}

	free(libpath);

	return 0;
}

char*
moto_valToCType(MotoVal* val){
	return mtype_toCType(val->type);
}

char*
moto_defaultValForCType(MotoVal* val) {

	switch (val->type->kind) {
		case INT32_TYPE:
			return "0";
		case INT64_TYPE:
			return "0";
		case FLOAT_TYPE:
			return "0.0f";
		case DOUBLE_TYPE:
			return "0.0f";
		case BOOLEAN_TYPE:
			return "'\\0'";
		case CHAR_TYPE:
			return "'\\0'";
		case BYTE_TYPE:
			return "'\\0'";
		case REF_TYPE:
			return "NULL";
		default: /* FIXME: I sure hope we don't get here */
			return "NULL";
	}
}

char* 
moto_createMotonameForFn(char* classn, char* fn){
	StringBuffer* buf = buf_createDefault();
	char *r;
	
	/* If we are dealing with a method and not a function we must
		prepend the class name and '::' to the generated motoname */
		  
	if(classn != NULL) {
		buf_puts(buf,classn);
		buf_puts(buf,"::");
	}
	
	/* Print the function name */
	buf_puts(buf,fn);
	
	r = buf_toString(buf);
	buf_free(buf);
	
	return r;
}

char* 
moto_createMotonameForFnInBuffer(char* classn, char* fn, char* buffer){
	buffer[0]='\0';
	
	/* If we are dealing with a method and not a function we must
		prepend the class name and '::' to the generated motoname */
		  
	if(classn != NULL) {
		strcpy(buffer,classn);
		strcat(buffer,"::");
	}
	
	/* Print the function name */
	strcat(buffer,fn);
	
	return buffer;
}

/*=============================================================================
// end of file: $RCSfile: env.c,v $
==============================================================================*/

