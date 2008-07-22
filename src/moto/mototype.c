/*=============================================================================

   Copyright (C) 2002 David Hakim.
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

   $RCSfile: mototype.c,v $   
   $Revision: 1.5 $
   $Author: dhakim $
   $Date: 2003/03/09 07:15:30 $
 
==============================================================================*/
#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <string.h>

#include "memsw.h"

#include "mxstring.h"
#include "excpfn.h"

#include "env.h"
#include "motohook.h"

MotoType *
mttab_add(MotoTypeTable *table, char *name, char isExternallyDefined) {
	MotoEnv *env = moto_getEnv();
	MotoType *type;
	if ((MotoType *)stab_get(table, name) != NULL) {
		THROW("MotoException","Attempt to add duplicate type %s to the type system",name ) ; 
	}
	type = (MotoType *)mman_malloc(env->mpool, sizeof(MotoType));
	type->kind = REF_TYPE;
	type->name = name;
	type->isExternallyDefined = isExternallyDefined;
	
	/* This is a base type so set its atype, argc, argt, and dimension to 0 */
	type->dim = 0;
	type->atype = NULL;
	type->argc = 0;
	type->argt = NULL;
	
	stab_put(table, name, type);
	return type;
}

/* This is a private method which should only be called by moto_getType */
static MotoType *
mttab_addArrayType(MotoTypeTable* table, char *name, int dim) {
	MotoEnv *env = moto_getEnv();
	MotoType *type = NULL, *atype = NULL;
	int i;
		
	/* Retrieve the base type for the new array type, if it doesn't exist we have a problem */
	atype = (MotoType *)stab_get(table, name);
	if(atype == NULL)
		return NULL;
	
	/* If the type name passed is the name of another array then get the true
	   base type and add the dimensions of this type to those passed in */
	if(atype->dim > 0) {
		dim += atype->dim;
		atype = atype->atype;
	}
	
	/* Allocate the storage for our new type */		
	type = (MotoType *)mman_malloc(env->mpool, sizeof(MotoType));
	type->kind = REF_TYPE;
	
	/* Construct the canonical name for this new type */
	type->name = mman_malloc(env->mpool,strlen(atype->name)+2*dim+1);
	strcpy(type->name,atype->name);
	for(i=0;i<dim;i++)
		strcat(type->name,"[]");
	
	/* Set the ancestor type to the base type this array type stores. Set the dimension
		to the dimension of this array type */
	type->atype = atype;
	type->dim = dim;
	
	/* This is not a HOF type so zero out argc and argt */
	type->argc = 0;
	type->argt = NULL;
	
	type->isExternallyDefined = '\1';
	stab_put(table, type->name, type);
	return type;
}

MotoType *
mttab_getHOFType(MotoTypeTable* table, MotoType* rtype, int argc, MotoType** argt){
	MotoEnv *env = moto_getEnv();
	MotoType *type = NULL;
	char* name;
	int i,l;
	
	/* Construct the canonical name for this new type */
	l = strlen(rtype->name)+2;
	for(i=0;i<argc;i++)
		l += strlen(argt[i]->name)+ 1;
		
	name = mman_malloc(env->mpool,l+1);
	strcpy(name,rtype->name);
	strcat(name,"(");
	for(i=0;i<argc;i++){
		if(i>0) strcat(name,",");
		strcat(name,argt[i]->name);
	}
	strcat(name,")");
	
	/* See if this type has already exists */
	type = (MotoType *)stab_get(table, name);
	if(type != NULL) 
		return type;
		
	/* Allocate the storage for our new type */		
	type = (MotoType *)mman_malloc(env->mpool, sizeof(MotoType));
	type->name = name;
	type->kind = REF_TYPE;
	type->atype = rtype;
	type->argc = argc;
	type->argt = (MotoType **)mman_malloc(env->mpool, argc*sizeof(MotoType*));
	memmove(type->argt,argt, argc*sizeof(MotoType*));
	type->dim = 0;
	type->isExternallyDefined = '\1';
	stab_put(table, type->name, type);
	
	// printf("### Defined %s -> %s\n",type->atype->name,type->name);
	return type;
}

static MotoType *
mttab_addHOFType(MotoTypeTable* table, MotoType* rtype, int argc, MotoType** argt){
	MotoEnv *env = moto_getEnv();
	MotoType *type = NULL;
	char* name;
	int i,l;
	
	/* Construct the canonical name for this new type */
	l = strlen(rtype->name)+2;
	for(i=0;i<argc;i++)
		l += strlen(argt[i]->name)+ 1;
		
	name = mman_malloc(env->mpool,l+1);
	strcpy(name,rtype->name);
	strcat(name,"(");
	for(i=0;i<argc;i++){
		if(i>0) strcat(name,",");
		strcat(name,argt[i]->name);
	}
	strcat(name,")");
		
	/* Allocate the storage for our new type */		
	type = (MotoType *)mman_malloc(env->mpool, sizeof(MotoType));
	type->name = name;
	type->kind = REF_TYPE;
	type->atype = rtype;
	type->argc = argc;
	type->argt = (MotoType **)mman_malloc(env->mpool, argc*sizeof(MotoType*));
	memmove(type->argt,argt, argc*sizeof(MotoType*));
	type->dim = 0;
	type->isExternallyDefined = '\1';
	stab_put(table, type->name, type);
	
	// printf("### Defined %s -> %s\n",type->atype->name,type->name);
	return type;
}

MotoType *
mttab_typeForName(MotoTypeTable *table, char *typen) {
	MotoEnv *env = moto_getEnv();
	MotoType *type = NULL;
	int i;
	
	// printf("### Looking Up '%s'\n",typen);
	
	type = (MotoType *)stab_get(table, typen);
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
				argt[argc - i - 1] = mttab_typeForName(table,atypen[i]);
				if(argt[argc - i - 1] == NULL) return NULL;
			}
			
			/* Get the actual return type */ 
			rtypen = mman_track(env->mpool,mxstr_substring(typen,0,sindex));
			rtype = mttab_typeForName(table,rtypen);
			if(rtype == NULL) return NULL;
			
			type = mttab_addHOFType(table,rtype,argc,argt);
			
		} else if(typen[strlen(typen)-1] == ']') {
		
			/* This is an Array Type */
		
			char* atypen;
			MotoType* atype;
			
			int dim = 0;
			while(typen[strlen(typen) - 2*dim -1] == ']' && typen[strlen(typen) - 2*dim - 2] == '[')
				dim ++;
			
			atypen = mman_track(env->mpool,mxstr_substring(typen,0,strlen(typen) - 2*dim));
			atype = mttab_typeForName(table,atypen);
			
			if(atype == NULL) return NULL;
			
			type = mttab_addArrayType(table,atypen,dim);
		}
	}	
	
	return type;
}

MotoType *
mttab_get(MotoTypeTable *table, char *name, int dim) {
	MotoType *type;
	int i;
	
	type = (MotoType *)mttab_typeForName(table, name);
	if(type == NULL) 
		return NULL;
	
	if(dim > 0) {
		char typen[256];

		/* Construct the canonical name for the array type */
		strcpy(typen,name);
		for(i=0;i<dim;i++)
			strcat(typen,"[]");		
			
		/* Retrieve the array type */	
		type = (MotoType *)stab_get(table, typen);
		
		if(type==NULL)
			type = mttab_addArrayType(table,name,dim);
	}	
	
	return type;
}

static int 
moto_checkArrayCast( MotoType  *from, MotoType *to){
	/* Allow implicit casts of arrays to null for assignment */
	if(strcmp(from->name, "null") ==0 )
		return 1;
		
	/* Allow implicit casts of arrays to objects */
	if(from->dim > 0 && to->dim == 0 && strcmp("Object", to->name) == 0) {
		return 1;
	
	/* Allow implicit casts of reference type arrays to Object[] when dimensions are equal */
	} else if ( from->dim == to->dim && from->atype->kind == REF_TYPE && 
				strcmp("Object", to->atype->name) == 0) {
	
		return 1;
		
	/* Allow implicit casts of any multidimensional arrays to Object[] of lesser dimension */
	} else if ( from->dim > to->dim && to->dim > 0 && strcmp("Object", to->atype->name) == 0) {
	
		return 1;
	}
	return 0;
}

static int
moto_checkFunctionalCast( MotoType  *from, MotoType *to){
	/* Allow implicit casts of functions to objects */
	if(from->atype != NULL && to->atype == NULL && strcmp("Object", to->name) == 0) 
		return 1;
		
	/* Allow implicit casts of functions to compatable functions */
	if(from->atype != NULL && to->atype != NULL && from->argc == to->argc){
		int i = 0;
		
		for(i=0;i<to->argc;i++){
		
			/* If a function takes an argument of the form X(Y) we should be able to 
				pass it an argument of the form X(Z) where Z is of type Y or Z is an 
				ancestor of type Y */
				
			if(! (
				to->argt[i] == from->argt[i] || 
				(strcmp("Object",from->argt[i]->name) == 0 && to->argt[i]->kind == REF_TYPE)
			) )
				return 0; /* Arguments not castable */
		}
		
		/*
			If a function takes an argument of type void(Y) we should be able to pass it 
			an argument of the form X(Y) since wherever it is used the return value is 
			ignored

			If a function takes an argument of the form X(Y) we should be able to pass it 
			an argument of the form Z(Y) where is of type X or a descendant of type X
		*/
		
		if(to->atype == from->atype || strcmp("void",to->atype->name) == 0 || 
			(strcmp("Object",to->atype->name) == 0 && from->atype->kind == REF_TYPE) )
			return 1;

	}

	return 0;
}

/* Returns true if a variable / value of type stype can be assigned to a
	variable or value of type dtype */
	
int
mtype_checkAssign(MotoType *dtype, MotoType *stype) {
	int result = 0;

	/* If the destination type and the source type are the same, the assignment succeeds */
	if (dtype == stype)
		return 1;

	/* Special casting rules for Arrays */	
	if(stype->dim > 0 || dtype->dim > 0)
		return moto_checkArrayCast(stype,dtype);

	/* Special casting rules for Functional Types */	
	if(stype->atype != NULL || dtype->atype != NULL ) 
		return moto_checkFunctionalCast(stype,dtype);
			
	/* Try some implicit casting */	
	switch (dtype->kind) {
		case BOOLEAN_TYPE:
			if(stype->kind == BOOLEAN_TYPE)
				result = 1;
			break;
		case BYTE_TYPE:
		case CHAR_TYPE:
		case INT32_TYPE:
		case INT64_TYPE:
		case FLOAT_TYPE:
		case DOUBLE_TYPE:
			switch (stype->kind) {
				case BYTE_TYPE:
				case CHAR_TYPE:
				case INT32_TYPE:
				case INT64_TYPE:
				case FLOAT_TYPE:
				case DOUBLE_TYPE:
					result = 1;
					break;
				default:
					break;
			}
		break;
		case REF_TYPE:
			if(stype->kind == REF_TYPE) {
				if (strcmp(dtype->name,"Object") == 0) {
					result = 1; /* Assignment to Object subtype: Success */
				} else if (strcmp(stype->name, "null") ==0 ){
					result = 1; /* Assignment of null: Success */
				} else if (dtype->dim > 0 && strcmp(dtype->atype->name,"Object") == 0) {
					if(stype->dim > dtype->dim)
						result = 1;
					else if (stype->dim == dtype->dim && stype->atype->kind == REF_TYPE)
						result = 1;
				} 
			}
			break;
		default:
			break;
	}

	return result;
}

/* 	Explicit casting is a symmetric operation. If a is castable to b, then b is castable to a
	Returns true if a variable / value of type stype can be explicitly cast to a
	variable or value of type dtype */
	
int
mtype_checkCast(MotoType *dtype, MotoType *stype) {
	int result = 0;

	/* If the destination type and the source type are the same, the cast succeeds */
	if (dtype == stype)
		return 1;

	/* Special casting rules for Arrays */	
	if(stype->dim > 0 || dtype->dim > 0) 
		return (moto_checkArrayCast(stype,dtype) || moto_checkArrayCast(dtype,stype) ); 

	/* Special casting rules for Functional Types */	
	if(stype->atype != NULL || dtype->atype != NULL ) 
		return (moto_checkFunctionalCast(stype,dtype) || moto_checkFunctionalCast(dtype,stype) );
				
	switch (dtype->kind) {
		case BOOLEAN_TYPE:
			if(stype->kind == BOOLEAN_TYPE)
				result = 1;
			break;
		case BYTE_TYPE:
		case CHAR_TYPE:
		case INT32_TYPE:
		case INT64_TYPE:
		case FLOAT_TYPE:
		case DOUBLE_TYPE:
			switch (stype->kind) {
				case BYTE_TYPE:
				case CHAR_TYPE:
				case INT32_TYPE:
				case INT64_TYPE:
				case FLOAT_TYPE:
				case DOUBLE_TYPE:
					result = 1;
					break;
				default:
					break;
			}
		break;
		case REF_TYPE:
			if(stype->kind == REF_TYPE) {
				if (strcmp(dtype->name,"Object") == 0 || strcmp(stype->name,"Object") == 0) {
					result = 1; /* Cast to or from Object subtype: Success */
				} else if (strcmp(stype->name, "null") == 0 ){
					result = 1; /* Cast of null: Success */
				} else if (dtype->dim > 0 && stype->dim > 0) {
					if(dtype->dim < stype->dim && strcmp(dtype->atype->name,"Object") == 0)
						result = 1;
					else if(stype->dim < dtype->dim && strcmp(stype->atype->name,"Object") == 0)
						result = 1;
					else if(dtype->dim == stype->dim && (
								(strcmp(stype->atype->name,"Object") == 0 && dtype->atype->kind == REF_TYPE)
							|| 
								(strcmp(dtype->atype->name,"Object") == 0 && stype->atype->kind == REF_TYPE)
							)
						) 
						result = 1;
				} else if (mxstr_endsWith(dtype->name,"Exception") && mxstr_endsWith(stype->name,"Exception") ) {
					result = 1; /* FIXME: Cast between Exception subtypes: Success ... but implementation is all wrong */
				} 
			}
			break;
		default:
			break;
	}

	return result;
}

int 
moto_checkImplicitCast( MotoType  *from, MotoType *to) {
	int result = 0;
	
	if(from == NULL || to == NULL)
		THROW_D("MotoException");
			
	if(from == to) 
		return 1;
	
	/* Special casting rules for Arrays */	
	if(from->dim > 0 || to->dim > 0)
		return moto_checkArrayCast(from,to);

	/* Special casting rules for Functional Types */	
	if(from->atype != NULL || to->atype != NULL ) 
		return moto_checkFunctionalCast(from,to);
			
	/* do more complicated checks */

	switch (from->kind) {
		case INT32_TYPE:
			switch (to->kind) {
				case INT32_TYPE:
				case INT64_TYPE:
				case FLOAT_TYPE:
				case DOUBLE_TYPE:
					result = 1;
					break;
				default:
					break;
			}
			break;
		case INT64_TYPE:
			switch (to->kind) {
				case INT64_TYPE:
				case FLOAT_TYPE:
				case DOUBLE_TYPE:
					result = 1;
					break;
				default:
					break;
			}
			break;
		case FLOAT_TYPE:
			switch (to->kind) {
				case FLOAT_TYPE:
				case DOUBLE_TYPE:
					result = 1;
					break;
				default:
					break;
			}
			break;
		case DOUBLE_TYPE:
			switch (to->kind) {
				case DOUBLE_TYPE:
					result = 1;
					break;
				default:
					break;
			}
			break;
		case REF_TYPE:
	/* REF_TYPE can only be cast to other REF_TYPE */
			if (to->kind != REF_TYPE) {
				break;
			} else if (strcmp("null", from->name) == 0) {
				result = 1;
			} else if (strcmp("Object", to->name) == 0) {
				result = 1;
			}
	break;
		default:
	break;
	} 
	
	return result;
}

/* Returns the type that should be used in generated C code corresponding to
	this moto type */
	
char* 
mtype_toCType(MotoType* type) {
	char* rval;

	switch (type->kind) {
		case INT32_TYPE:
			return "int32_t";
		case INT64_TYPE:
			return "int64_t";
		case FLOAT_TYPE:
			return "float";
		case DOUBLE_TYPE:
			return "double";
		case BOOLEAN_TYPE:
			return "unsigned char";
		case CHAR_TYPE:
			return "char";
		case BYTE_TYPE:
			return "signed char";
		case VOID_TYPE:
			return "void";
		case REF_TYPE:
			if (type->dim > 0 && type->atype != NULL)
				return "UnArray*";
			else if (!strcmp(type->name,"String"))
				return "char*";
			else if (!strcmp(type->name,"Regex"))
				return "Regex*";
			else if (!strcmp(type->name,"Object"))
				return "void*";
			else if(type->atype != NULL)
				return "Function*";
	}

	if(type->isExternallyDefined) {
		rval = emalloc(strlen(type->name)+2);
		strcpy(rval ,type->name);
		rval[strlen(type->name)] = '*'; 
		rval[strlen(type->name)+1] = '\0';
		return rval;
	} else {
		rval = emalloc(strlen(type->name)+10);
		strcpy(rval, "struct _");
		strcat(rval, type->name);
		rval[strlen(type->name)+8] = '*'; 
		rval[strlen(type->name)+9] = '\0';
		return rval;
	}
}

/*
 * Takes the functional type orig and removes pargc arguments from it. The
 * specific arguments to be removed are list in the index array pargi
 */
 
MotoType *
mttab_apply(MotoTypeTable *table, MotoType* orig,int pargc, int* pargi){
	int i,j;
	MotoType* applied;
	
	if(pargc == 0) 
		applied = orig;
	else {
		StringBuffer* sb = buf_createDefault();
		
		buf_puts(sb,orig->atype->name);
		buf_putc(sb,'(');
		
		for(i=0,j=0;i+j<orig->argc;i++){
			while (j<pargc && pargi[j] == i+j) j++;
			if(i+j == orig->argc) break;
			if(i>0) buf_putc(sb,',');
			buf_puts(sb,orig->argt[i+j]->name);
		}
		
		buf_putc(sb,')');
		
		applied = mttab_typeForName(table,buf_data(sb));
		buf_free(sb);
	}	
	
	return applied;
}

/*=============================================================================
// end of file: $RCSfile: mototype.c,v $
==============================================================================*/
