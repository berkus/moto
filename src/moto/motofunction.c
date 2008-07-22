/*=============================================================================

   Copyright (C) 2002-2003 David Hakim.
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

   $RCSfile: motofunction.c,v $   
   $Revision: 1.7 $
   $Author: dhakim $
   $Date: 2003/03/09 07:15:30 $
 
==============================================================================*/
#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <stdlib.h>

#include "excpfn.h"
#include "mxstring.h"
#include "log.h"
#include "memsw.h"

#include "stringbuffer.h"
#include "dl.h"
#include "motohook.h"
#include "motofunction.h"

MotoFunction*
mfn_create(
	
   char* motoname,
   
   char* rtype,
   int argc,
   char** argtypes,
   char** argnames,
   
   char* csymbol,
   char tracked,
   
   const UnionCell* opcell,
   
   char* libname,
   void* handle,
   void (*fptr)()
){
   MotoFunction *mfn = (MotoFunction *)emalloc(sizeof(MotoFunction));
   
   
   mfn->motoname = motoname;

   mfn->rtype = rtype;
   mfn->deftype = NULL; // FIXME: there should be no such thing as deftype
   
   mfn->argc = argc;
   mfn->argtypes = argtypes; 
   mfn->argnames = argnames;

   mfn->csymbol = csymbol;
   
   mfn->tracked = tracked; 

   mfn->opcell = opcell;
      
   mfn->libname = libname;
   mfn->handle = handle;   
   mfn->fptr = fptr;
   
   if (mxstr_indexOf(motoname,':')>=0)
		mfn->classn = mxstr_substring(motoname,0,mxstr_indexOf(motoname,':'));
	else
		mfn->classn = NULL;
		
	if (mxstr_indexOf(motoname,':')>=0) {
		mfn->name = mxstr_substr(motoname,mxstr_indexOf(motoname,':')+2);
	} else {
		mfn->name = estrdup(motoname);
	}
		
   return mfn; 
}

static char *SEP = "|";

MotoFunction*
mfn_createFromSymbol(char* sym, char* libname, void* handle){
	StringBuffer* sb = buf_createDefault();
	char *error;
	MotoFunction* mfn = (MotoFunction *)emalloc(sizeof(MotoFunction));
	char* typen, *isymbol;
	
	mfn->libname = libname;	

	/* Get the symbol signature */
	mfn->motoname = strtok(sym, SEP);
	
	/* Get the symbol return type */
	typen = strtok(NULL, SEP);		 
	
	buf_puts(sb,typen);
	
	mfn->rtype = buf_toString(sb);
	
	/* FIXME: Does symbol return type define a type? */
	mfn->deftype = strtok(NULL, SEP);
	if (mfn->deftype[0] == '-') {
		mfn->deftype = NULL;
	}
		
	/* Get the symbol argument count */
	mfn->argc = atoi(strtok(NULL, SEP));
	
	/* Get the symbol argument types */
	if (mfn->argc == 0) {
		mfn->argtypes = NULL;
		strtok(NULL, SEP);
	} else {
		int j;
		char **v = emalloc(mfn->argc * sizeof(char *));

		for (j = 0; j < mfn->argc; j++) {
			typen = strtok(NULL, SEP); 
			
			buf_clear(sb);
			buf_puts(sb,typen);
	
			v[j] = buf_toString(sb);
		}
		mfn->argtypes = v;
	}
	
	/* Get the symbol argument names */
	mfn->argnames = NULL;
	
	/* Get the moto symbol fname */
	isymbol = strtok(NULL, SEP);
	
	/* Get the c symbol fname */
	mfn->csymbol = strtok(NULL, SEP);
	
	/* Get tracking info */
	mfn->tracked =	 strtok(NULL, SEP)[0] == '0' ? '\0' : '\1';
	
	/* load function */
	mfn->fptr = moto_os_dso_sym(handle, isymbol);
	if ((error = moto_os_dso_error()) != NULL) {
		log_error(__FILE__, "%s\n",  error);
	}
	
	mfn->handle = handle;
	mfn->opcell = NULL;
	
	if (mxstr_indexOf(mfn->motoname,':')>=0)
		mfn->classn = mxstr_substring(mfn->motoname,0,mxstr_indexOf(mfn->motoname,':'));
	else
		mfn->classn = NULL;
		
	if (mxstr_indexOf(mfn->motoname,':')>=0) {
		mfn->name = mxstr_substr(mfn->motoname,mxstr_indexOf(mfn->motoname,':')+2);
	} else {
		mfn->name = estrdup(mfn->motoname);
	}
	
	buf_free(sb);
	return mfn;
}

void
mfn_free(MotoFunction* mfn){
	int i;
	
	free(mfn->rtype);
	free(mfn->name);
	
   if (mfn->argtypes != NULL) {
      for (i=0;i<mfn->argc;i++)
   		free(mfn->argtypes[i]);
   		
      free(mfn->argtypes);
   }
   if (mfn->argnames != NULL) {
      free(mfn->argnames);
   }
   if (mfn->classn != NULL) {
   	free(mfn->classn);
   }
   
   free(mfn);
}

/* Returns true if this MotoFunction is a method. Returns false otherwise */
int
mfn_isMethod(MotoFunction* mfn){
	return mfn->classn != NULL;
}

/* Returns true if this MotoFunction is a constructor. Returns false otherwise */
int
mfn_isConstructor(MotoFunction* mfn){
	return (mfn->classn != NULL && estrcmp(mfn->classn,mfn->name) == 0);
}

MotoType* 
mfn_type(MotoFunction* mfn){
	MotoEnv* env = moto_getEnv();
	MotoType* type;
	StringBuffer* sb = buf_createDefault();
	int i;
		
	buf_puts(sb, mfn->rtype);
	buf_putc(sb, '(');
	for (i = 0; i < mfn->argc; i++) {
		if (i > 0) buf_puts(sb, ",");
		buf_printf(sb,"%s",mfn->argtypes[i]);
	}
	buf_putc(sb, ')');
	
	type = mttab_get(env->types, buf_data(sb),0);
	buf_free(sb);
	return type;
}

static char*
mfn_casymbol(char* argtn){
	MotoEnv* env = moto_getEnv();
	MotoType* argt = mttab_get(env->types,argtn,0);
	
	StringBuffer* sb = buf_createDefault();
	char* argsym;
	int i;
	
	if(argt->atype == NULL) {
		/* Basic type */

		if(!strcmp(argt->name,"int")) buf_putc(sb,'i');
		else if(!strcmp(argt->name,"float")) buf_putc(sb,'f');
		else if(!strcmp(argt->name,"double")) buf_putc(sb,'d');
		else if(!strcmp(argt->name,"char")) buf_putc(sb,'c');
		else if(!strcmp(argt->name,"long")) buf_putc(sb,'l');
		else if(!strcmp(argt->name,"boolean")) buf_putc(sb,'b');
		else if(!strcmp(argt->name,"byte")) buf_putc(sb,'y');
		else if(!strcmp(argt->name,"String")) buf_putc(sb,'S');
		else if(!strcmp(argt->name,"Object")) buf_putc(sb,'O');
		else { buf_puti(sb, strlen(argt->name)); buf_puts(sb,argt->name); }
	} else if(argt->dim != 0) {
		/* Array Type */
		char* btsym;
		
		/* Output one P for each dimension of an array argument */
		for(i=0;i<argt->dim;i++) buf_putc(sb,'P');
		
		btsym = mfn_casymbol(argt->atype->name);
		buf_puts (sb,btsym);
		free(btsym);
	} else {
		/* Functional Type */
		char* rtsym, *atsym;
		int i;
		
		/* Output 'F' to begin the functional type argument */
		buf_putc(sb,'F');

		/* Output the functional type argument's return type */
		rtsym = mfn_casymbol(argt->atype->name);
		buf_puts(sb,rtsym);
		free(rtsym);
		
		/* Output 'o' to begin the functional type argument's arguments :) */
		buf_putc(sb,'o');
		
		/* For each type argument */
		for(i=0;i<argt->argc;i++){
			atsym = mfn_casymbol(argt->argt[i]->name);
			buf_puts(sb,atsym);
			free(atsym);
		}
				
		/* Output 'e' to end the functional type argument */
		buf_putc(sb,'e');
	}
	
	argsym = buf_toString(sb);
	buf_free(sb);
	
	return argsym;
}
 
char*
mfn_csymbol(MotoFunction* mfn){
	MotoEnv* env = moto_getEnv();

	/* See if the symbol name for this function is cached */
	if (mfn->csymbol == NULL) { 
		StringBuffer *sb = buf_create(32);
		int i;
		
		/* If the classname is not null then this is a method and the classname 
			should prefix the generated cname */
			
		if (mfn->classn != NULL)
			buf_printf(sb, "_%s",mfn->classn);
		
		/* If the fname starts with '~' (i.e. this is a destructor) convert it 
			into an '_' */
		if (mfn->name[0] == '~')
			buf_printf(sb,"__%s__", mfn->name+1);
		else		
			buf_printf(sb, "_%s__", mfn->name);
			 
		for (i = 0; i < mfn->argc; i++) {
			char* asym = mfn_casymbol(mfn->argtypes[i]);
			buf_puts(sb, asym);
			free(asym);
		}
	
		/* Make sure this gets cleaned up eventually */
		mfn->csymbol = mman_track(env->mpool,buf_toString(sb) );
		buf_free(sb);
	}
	
	return mfn->csymbol;
}

char*
mfn_cprototype(MotoFunction* mfn){
	MotoEnv* env = moto_getEnv();
	int i;
	StringBuffer *sb = buf_create(32);
	char* cprototype;

	/* Output the C symbol return type and name */
	buf_printf(sb,"static %s %s", 
		mtype_toCType(mttab_get(env->types, mfn->rtype,0)), mfn_csymbol(mfn));

	/* Output the C arguments */
	buf_putc(sb,'(');
	
	/* If the classname is not null then this is a method and 'this' 
		should be the first argument */
	if (mfn->classn != NULL)
		 buf_printf(sb, "%s* this",mfn->classn);
		 
	for (i = 0; i < mfn->argc; i++) {
		char *atypen = mfn->argtypes[i];
		char *aname = mfn->argnames[i];

		if (i > 0 || mfn->classn != NULL) buf_puts(sb, ",");
	 
		buf_puts(sb, mtype_toCType(mttab_get(env->types, atypen,0)));

		buf_putc(sb, ' ');
		buf_puts(sb, aname);
	}
	buf_putc(sb,')');

	cprototype = buf_toString(sb);
	buf_free(sb);
	
	return cprototype;
}


/*=============================================================================
// end of file: $RCSfile: motofunction.c,v $
==============================================================================*/

