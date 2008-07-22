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

	$RCSfile: ftable.c,v $	 
	$Revision: 1.22 $
	$Author: dhakim $
	$Date: 2003/05/29 02:14:08 $
 
==============================================================================*/
#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "memsw.h"

#include "excpfn.h"
#include "exception.h"
#include "log.h"
#include "enumeration.h"
#include "hashset.h"

#include "ftable.h"
#include "env.h"
#include "motohook.h"

FTable *
ftab_create() {
	FTable *table = (FTable*)emalloc(sizeof(FTable));
	table->t = stab_createDefault();
	return table;
}

void 
ftab_free(FTable *table) {
	Enumeration *e = stab_getValues(table->t);
	while (enum_hasNext(e)) 
		hset_free(enum_next(e));
	enum_free(e);	 
	
	stab_free(table->t);
	free(table);
}

void 
ftab_add(FTable *table, char *name, MotoFunction *f) {
	HashSet *set = stab_get(table->t, name);
	if (set == NULL) {
		set = hset_createDefault();
		stab_put(table->t, name, set);
	}
	hset_add(set, f);
}

int 
ftab_cacheGet(FTable *table,MotoFunction **f,char *name,int argc, char **types) {
	int i,r;
	MotoEnv *env = moto_getEnv();
	StringBuffer *key = (StringBuffer*)opool_grab(env->bufpool);

	buf_puts(key,name);
	buf_putc(key,'(');
	for(i=0;i<argc;i++){
		if(i!=0) buf_putc(key,',');
		buf_puts(key,types[i]);
	}
	buf_putc(key,')');

	if ((*f = (stab_get(env->fcache,buf_data(key))))!= NULL) {
		opool_release(env->bufpool,key);
		return FTAB_OK;
	}

	if ((r =ftab_get(table,f,name,argc,types)) == FTAB_OK){
		stab_put(env->fcache,buf_toString(key),*f);
	}

	opool_release(env->bufpool,key);
	return r;
}

char 
ftab_argsMatchExactly(int argc, char **candidateTypes, char **argTypes){
	int i;
	for (i = 0; i < argc; i++) {
		/* exact match */	  
		if (candidateTypes[i] == NULL) 
			return '\0';
		if (estrcmp(candidateTypes[i], argTypes[i]) != 0 ) 
			return '\0';		
	}
	return '\1';
}

char 
ftab_argsCanBeCast(int argc, char **candidateTypes, char **argTypes){
	MotoEnv *env = moto_getEnv();
	int i;
	for (i = 0; i < argc; i++) {
		/* exact match */	
		if (candidateTypes[i] == NULL) 
			continue;  
		if (!moto_checkImplicitCast(
				mttab_typeForName(env->types,candidateTypes[i]), 
				mttab_typeForName(env->types,argTypes[i])
			))
			return '\0';		
	}
	return '\1';
}

/*
 * Given an ftable, a motoname, an arg count, and an array of canonical types, the MotoFunction
 * f is set to the function that matches the inputs exactly
 */
 
int 
ftab_getExactMatch(FTable *table, MotoFunction **f, char *name, int argc, char **types) {

	/* See if there are any methods / functions with the same motoname */
	HashSet *set = stab_get(table->t, name);	
	Enumeration *e;
	
	*f = NULL;
	
	/* If not then return FTAAB_NONE */
	if (set == NULL) 
		return FTAB_NONE;
		 
	/* Foreach function / method with this motoname */
	e = hset_elements(set);
	while (enum_hasNext(e)) {
		MotoFunction* mxf = (MotoFunction *)enum_next(e);
		
		/* Check if the Arguments match exactly */
		if (argc == mxf->argc && ftab_argsMatchExactly(argc,types,mxf->argtypes)) {
			/* If they do then we've found our function */		  
			*f = mxf;
			enum_free(e);
			return FTAB_OK;
		}
	}
	enum_free(e);
	
	return FTAB_NONE;
}

/*
 * Given an ftable, a motoname, an arg count, and an array of canonical types, the MotoFunction
 * f is set to the function that can be called with input of the specified types. First the 
 * algorithm attempts to find a function which matches the input types exactly. If such a function
 * is not found an attempt is made to find a function that can be called (by implicitly casting
 * the argument types)
 */
 
int 
ftab_get(FTable *table, MotoFunction **f, char *name, int argc, char **types) {
	MotoEnv *env = moto_getEnv();
	Vector *accept;
	Enumeration *e;
	
	/* See if there are any methods / functions with the same motoname */
	HashSet *set = stab_get(table->t, name);	

	*f = NULL;
	
	/* If not then return FTAB_NONE */
	if (set == NULL) 
		return FTAB_NONE;
		 
	accept = vec_createDefault();
	
	/* Foreach function / method with this motoname */
	e = hset_elements(set);
	while (enum_hasNext(e)) {
		MotoFunction* mxf = (MotoFunction *)enum_next(e);
		
		/* Check if the Arguments match exactly */
		if (argc == mxf->argc && ftab_argsMatchExactly(argc,types,mxf->argtypes)) {
			/* If they do then we've found our function */		  
			*f = mxf;
			break;
		}

		/* If not, Check if the Arguments can be implicitly cast to this function */		
		if (argc == mxf->argc && ftab_argsCanBeCast(argc,types,mxf->argtypes)) {
			vec_add(accept, mxf);
		}
	}
	enum_free(e);
	
	if (*f != NULL) {
		vec_free(accept);
		return FTAB_OK;	
	} else {
		int i;
		int size = vec_size(accept);
		if (size == 0) {
			vec_free(accept);
			return FTAB_NONE;	  
		}
		else if (size == 1) {
			*f = vec_get(accept, 0);
			vec_free(accept);
			return FTAB_OK;	
		}
		else {
			for (i = 0; i < vec_size(accept); i++) {
				/* check all possible candidates */
				MotoFunction *candidate = vec_get(accept, i);
				int j;
				int k;
				for (j = 0; j < vec_size(accept); j++) {
					MotoFunction *check;
					int match = 1;
					if (j == i) {
						/* don't check a function against itself */
						continue;
					}
					check = vec_get(accept, j);
					/* now check function[i] against function[j] */
					for (k = 0; k < argc; k++) {
						/* check cast */
						if (!moto_checkImplicitCast(
								mttab_typeForName(env->types,candidate->argtypes[k]),
								mttab_typeForName(env->types,check->argtypes[k]))
							) {
							match = 0;
							break;
						}
					}
					if (match) {
						vec_remove(accept, check);
						break;
					}
				}
			}
			if (vec_size(accept) > 1) {
				vec_free(accept);
				return FTAB_MULTIPLE;	
			}
			*f = vec_get(accept, 0);
		}
	}
	vec_free(accept);
	return FTAB_OK;	

}

int 
ftab_fnForName(FTable *table, MotoFunction **f,char *name){
	/* See if there are any methods / functions with the same motoname */
	HashSet *set = stab_get(table->t, name);	
	Enumeration* e;
	
	if (set == NULL) return FTAB_NONE;
	
	if (hset_size(set) == 0) return FTAB_NONE;
	if (hset_size(set) > 1) return FTAB_MULTIPLE;
	
	e = hset_elements(set);
	while (enum_hasNext(e)) {
		MotoFunction* mxf = (MotoFunction *)enum_next(e);
		*f = mxf;
	}
	enum_free(e);
	
	return FTAB_OK;
}

/*=============================================================================
// end of file: $RCSfile: ftable.c,v $
==============================================================================*/

