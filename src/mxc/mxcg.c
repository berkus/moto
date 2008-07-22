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

	$RCSfile: mxcg.c,v $	  
	$Revision: 1.32 $
	$Author: scorsi $
	$Date: 2003/07/04 22:53:41 $
 
==============================================================================*/
#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>

#include "memsw.h"

#include "mxstring.h"
#include "log.h"
#include "stringbuffer.h"
#include "fileio.h"
#include "stringset.h"
#include "err.h"
#include "path.h"
#include "kmpstring.h"
#include "tokenizer.h"

#include "cell.h"
#include "mx.tab.h"
#include "mxext.h"
#include "mxmapping.h"
#include "cfn.h"
#include "mfn.h"
#include "mxcg.h"

extern int yyerror(char *s);

static char *vs(Vector *v);
static char *replace(char* string, char* find, char* replace);
static char *opName(char * op);
static char *isOvop(char * fname);

#define OVOPSNUM 15
static char * ovops[OVOPSNUM][2] = {
	{ "_plus", "+" },
	{ "_minus", "-" },
	{ "_pluseq", "+=" },
	{ "_minuseq", "-=" },
	{ "_times", "*" },
	{ "_timeseq", "*=" }, 
	{ "_lt", "lt" },
	{ "_gt", "gt" }, 
	{ "_lte", "lte" },
	{ "_gte", "gte" },
	{ "_inc", "++" },
	{ "_dec", "--" },
	{ "_equal", "eq" },
	{ "_notequal", "ne" },
	{ "_arrindex", "[]" },
};

char* mxc_nameForTypeCell(const UnionCell *p){
	StringBuffer* sb = buf_createDefault();
	char* typen;
		
	if(p->opcell.opcode == TYPE) {
		typen = p->opcell.operands[0]->strcell.value;
		
		buf_puts(sb,typen);

	} else if(p->opcell.opcode == ARRAY_TYPE) {
		
		typen = mxc_nameForTypeCell(p->opcell.operands[0]);

		buf_puts(sb,typen);
		buf_puts(sb,"[]");
		
		free(typen);

	} else /* if (uc_opcode(p) == DEF) */ {
		UnionCell* argListUC = p->opcell.operands[1];
		int i,argc = argListUC->opcell.opcount;
		
		/* Add in the return type */
		typen = mxc_nameForTypeCell(p->opcell.operands[0]);
		buf_puts(sb,typen);
		free(typen);
		
		buf_puts(sb,"(");
		for(i=0;i<argc;i++){
			if(i>0) buf_puts(sb,",");
			
			typen = mxc_nameForTypeCell(argListUC->opcell.operands[i]);
			buf_puts(sb,typen);
			free(typen);
		}			
		buf_puts(sb,")");
	}
	typen = buf_toString(sb);
	buf_free(sb);
	return typen;
}

MXC *mxc_create() {
	MXC *p;
	p = (MXC *)malloc(sizeof(MXC));
	p->props = prop_create();
	p->exts = vec_createDefault();
	p->mpool = mpool_create(10);
	p->strlist = vec_createDefault();
	return p;
}

void mxc_free(MXC *p) {
	Enumeration *e;
	/* free libs memory */
	e = vec_elements(p->exts);
	while (enum_hasNext(e)) {
		MXExt *p = (MXExt *)enum_next(e);
		mxext_free(p);
	}
	enum_free(e);
	
	prop_free(p->props);
	vec_free(p->exts);
	vec_free(p->strlist);
	mpool_free(p->mpool);
	free(p);
}

char *mxc_c(MXC *c, UnionCell *p) {
	char *result = NULL;
	if (!p) {
		return "";
	}
	switch(p->celltype) {
		case VALUE_TYPE:
			result = p->strcell.value;
			vec_add(c->strlist, result);
			break;
		case OP_TYPE:
			switch(p->opcell.opcode) {
				case EXTENSION:
					log_debug(__FILE__, "EXTENSION\n");
					mxc_c(c, p->opcell.operands[0]);
					mxc_c(c, p->opcell.operands[1]);
					break;
				case EXTDECL:
					c->curlib = mxext_create();
					vec_add(c->exts, c->curlib);
					result = mxc_c(c, p->opcell.operands[0]);
					mxext_setName(c->curlib, result);
					log_debug(__FILE__, "EXTDECL: %s\n", result);
					break;
				case INCLUDE:
					result = mxc_c(c, p->opcell.operands[0]);
					log_debug(__FILE__, "INCLUDE: %s\n", result);
					mxext_addInclude(c->curlib, result);
					break;
				case LIBRARY:
					result = mxc_c(c, p->opcell.operands[0]);
					log_debug(__FILE__, "LIBRARY: %s\n", result);
					mxext_addLibrary(c->curlib, result);
					break;
				case ARCHIVE:
					result = mxc_c(c, p->opcell.operands[0]);
					log_debug(__FILE__, "ARCHIVE: %s\n", result);
					mxext_addArchive(c->curlib, result);
					break;
				case LIBRARYPATH:
					result = mxc_c(c, p->opcell.operands[0]);
					log_debug(__FILE__, "LIBRARYPATH: %s\n", result);
					mxext_addLibraryPath(c->curlib, result);
					break;
				case INCLUDEPATH:
					result = mxc_c(c, p->opcell.operands[0]);
					log_debug(__FILE__, "INCLUDEPATH: %s\n", result);
					mxext_addIncludePath(c->curlib, result);
					break;
					
				case XFILE:
					result = mxc_c(c, p->opcell.operands[0]);
					log_debug(__FILE__, "FILE: %s\n", result);
					mxext_addFile(c->curlib, result);
					break;
				case LIST:
					log_debug(__FILE__, "LIST\n");
					mxc_c(c, p->opcell.operands[0]);
					result = mxc_c(c, p->opcell.operands[1]);
					break;
				case MAP:
					{
						MXMapping *mapping;
						log_debug(__FILE__, "MAP\n");
						mxc_c(c, p->opcell.operands[0]);
						mxc_c(c, p->opcell.operands[1]);
						mapping = (MXMapping *)malloc(sizeof(MXMapping));
						mman_track(c->mpool, mapping);
						mapping->mfn = c->curmfn;
						mapping->cfn = c->curcfn;
						mxext_addMapping(c->curlib, mfn_symbolName(c->curmfn), mapping);
					}
					break;
				case C_FUNCTION_DECLARATION:
					/* create new function */
					c->curcfn = cfn_create();
					vec_clear(c->strlist);
					mman_track(c->mpool, c->curcfn);
					log_debug(__FILE__, "&&& C_FUNCTION_DECLARATION\n");
					/* set function return type */
					mxc_c(c, p->opcell.operands[0]);
					c->curcfn->rtype = vs(c->strlist);
					vec_clear(c->strlist);
					mxc_c(c, p->opcell.operands[1]);
					/*
					cfn_dump(c->curcfn);
					*/
					result = "";
					break;
				case C_FUNCTION_DIRECT_DECLARATOR:
					result = mxc_c(c, p->opcell.operands[0]);
					if (strcmp(vec_get(c->strlist,0), "*") == 0) {
						strcat(c->curcfn->rtype, " *");	 
					}
					vec_clear(c->strlist);
					c->curcfn->fname = result;
					log_debug(__FILE__, "C_FUNCTION_DIRECT_DECLARATOR\n");
					if(p->opcell.opcount == 2) {
						/* function with arguments */
						mxc_c(c, p->opcell.operands[1]);
					}
					else {
						log_debug(__FILE__, "no-arg function\n");
						/* no-arg function */
					}
					break;
				case C_PARAMETER_DECLARATION:
					/* create new function arg */
					{
						char *t;
						char *n;
						mxc_c(c, p->opcell.operands[0]);
						t = vs(c->strlist);
						vec_clear(c->strlist);
						mxc_c(c, p->opcell.operands[1]);
						n = vs(c->strlist);
						if (n[0] == '*') {
							n = n + 1;
							strcat(t , "*");
						}
						vec_clear(c->strlist);
						if (n != NULL) {
							c->curarg = (FNArg *)malloc(sizeof(FNArg));
							c->curarg->type = t;
							c->curarg->name = n;
							cfn_addArg(c->curcfn, c->curarg);
							log_debug(__FILE__, "C_PARAMETER_DECLARATION: %s %s\n", c->curarg->type, c->curarg->name);
						}
					}
					break;
				case C_PARAMETER_LIST:
					if(cfn_argc(c->curcfn) == 0) {
						mxc_c(c, p->opcell.operands[0]);	  
					}
					mxc_c(c, p->opcell.operands[1]);
					/* handle ellipsis */
					if (p->opcell.operands[1]->celltype == VALUE_TYPE) {
						c->curarg = (FNArg *)malloc(sizeof(FNArg));
						c->curarg->type = "...";
						c->curarg->name = "";
						cfn_addArg(c->curcfn, c->curarg);
					}
					break;
				case M_OPERATOR_DECLARATION:
				case M_FUNCTION_DECLARATION:
					{
						UnionCell* type = p->opcell.operands[0];
						char *t = mxc_nameForTypeCell(type);
							
						/* create new function */
						c->curmfn = mfn_create();
						mman_track(c->mpool, c->curmfn);
						log_debug(__FILE__, "&&& M_FUNCTION_DECLARATION\n");
						/* set function return type */
						c->curmfn->rtype = t;
						/*c->curmfn->tracked = *mxc_c(c, p->opcell.operands[2]);*/
						c->curmfn->tracked = p->opcell.operands[2]->strcell.value[0];
						c->curmfn->comment = 
							mxstr_substring(
								p->opcell.operands[3]->strcell.value,
								3,
								strlen(p->opcell.operands[3]->strcell.value)-3
							);
						mxc_c(c, p->opcell.operands[1]);
						/*
						mfn_dump(c->curmfn);
						*/
						result = "";
						break;
					}
				case M_OPERATOR_DECLARATOR:
				case M_FUNCTION_DECLARATOR:
					result = mxc_c(c, p->opcell.operands[0]);

					/* Method */
					if(mxstr_indexOf(result,':') != -1){
						c->curmfn->classn = mxstr_substring(result,0,mxstr_indexOf(result,':'));
						
						/* If it is an overloadded operator ... */
						if (p->opcell.opcode == M_OPERATOR_DECLARATOR) {
							c->curmfn->fname = opName(mxstr_substr(result,mxstr_indexOf(result,':')+2));
						} else {
							c->curmfn->fname = mxstr_substr(result,mxstr_indexOf(result,':')+2);
						}
					/* Function */
					} else {
						c->curmfn->classn = NULL;
						
						/* If it is an overloadded operator ... */
						if (p->opcell.opcode == M_OPERATOR_DECLARATOR) {
							c->curmfn->fname = opName(result);
						} else {
							c->curmfn->fname = result;
						}
					}

					log_debug(__FILE__, "M_FUNCTION_DECLARATOR\n");
					if(p->opcell.opcount == 2) {
						/* function with arguments */
						mxc_c(c, p->opcell.operands[1]);
					}
					else {
						log_debug(__FILE__, "no-arg function\n");
						/* no-arg function */
					}
					break;
				case M_PARAMETER_DECLARATION:
					/* create new function arg */
					{
						UnionCell* type = p->opcell.operands[0];
						char *t = mxc_nameForTypeCell(type);
						char *n = mxc_c(c, p->opcell.operands[1]);
						if (n != NULL) {
							c->curarg = (FNArg *)malloc(sizeof(FNArg));
							c->curarg->type = t;
							c->curarg->name = n;
							mfn_addArg(c->curmfn, c->curarg);
							log_debug(__FILE__, "M_PARAMETER_DECLARATION: %s %s\n", c->curarg->type, c->curarg->name);
						}
					}
					break;
				case M_PARAMETER_LIST:
					if(mfn_argc(c->curmfn) == 0) {
						mxc_c(c, p->opcell.operands[0]);	  
					}
					mxc_c(c, p->opcell.operands[1]);
					/* handle ellipsis */
					if (p->opcell.operands[1]->celltype == VALUE_TYPE) {
						c->curarg = (FNArg *)malloc(sizeof(FNArg));
						c->curarg->type = "...";
						c->curarg->name = "";
						mfn_addArg(c->curmfn, c->curarg);
					}
					break;
				default:
					{
						StringBuffer *d = buf_createDefault();
						buf_printf(d, "Unrecognized op: %d", p->opcell.opcode);
						yyerror(buf_data(d));
						buf_free(d);
					}
					break;
			}
	}
	return result;
}

void mxc_emitIncludes(MXC *c) {
	StringBuffer *buf = buf_createDefault();
	Enumeration *e = vec_elements(c->exts);
	
	/* add standard includes */
	buf_puts(buf, replace(prop_get(c->props,"STANDARD_INCLUDES"),"\n\t","\n"));	 
	
	/* add includes specific to each .i file */
	buf_puts(buf, "\n");
	while (enum_hasNext(e)) {
		MXExt *ext = enum_next(e);
		Enumeration *exte = mxext_getIncludes(ext);
		buf_printf(buf, "/* includes for extension %s */\n", ext->name);
		buf_printf(buf, "#ifdef %s\n", ext->ppname);
		while (enum_hasNext(exte)) {
			buf_printf(buf, "#	include %s\n", enum_next(exte));
		}
		buf_puts(buf, "#endif\n\n");
		enum_free(exte);	 
	}
	enum_free(e);	 
	
	/* add standard defines */
	buf_puts(buf, replace(prop_get(c->props,"STANDARD_DEFINES"),"\n\t","\n"));		

	/* output header file */
	fio_put("headers.mx.h", buf_data(buf));
	buf_free(buf);
}

void mxc_emitExports(MXC *c) {
	StringBuffer *buf = buf_createDefault();
	Enumeration *e;
	Enumeration *mape;
	StringBuffer *typ = buf_createDefault();
	StringSet *includes,*includePaths,*libraryPaths,*archives,*libraries; /* stringset used to guarentee one of each include */
	char *out;
	char *sint;
	int count = 0;

	/* get shell from properties file */
	out = prop_get(c->props,"EXPORT_FUNCTIONS");
	out = replace(out,"\n\t","\n");
	out = replace(out,"<TARGET>",c->target);

	/* build symbol information */
	e = vec_elements(c->exts);
	while(enum_hasNext(e)) {
		MXExt *ext = (MXExt *)enum_next(e);
		mape = stab_getKeys(ext->mappings);
		while(enum_hasNext(mape)) {
			int i;
			char *key = (char *)enum_next(mape);
			MXMapping *mapping = (MXMapping *)stab_get(ext->mappings, key);
			int argc = vec_size(mapping->mfn->argv);
			count++;
			/* Output qualified function or method name */
			buf_puts(buf, "	\"");
			if(mapping->mfn->classn != NULL)
				buf_printf(buf, "%s::%s",mapping->mfn->classn, mapping->mfn->fname);
			else 
				buf_printf(buf, "%s", mapping->mfn->fname);	  
			
			/* Output method return type */
			buf_printf(buf, "|%s", mapping->mfn->rtype);	
			
			/* Output if this function defines a type i.e. is a constructor or destructor */

			if (mapping->mfn->classn != NULL && 
					( 
						strcmp(mapping->mfn->fname, mapping->mfn->classn) == 0 ||
						( mapping->mfn->fname[0]=='~' && strcmp(mapping->mfn->fname+1, mapping->mfn->classn) == 0 )
					) 
				) {
				buf_puts(buf, "|");	 
				buf_puts(buf, mapping->mfn->classn);	
			} else { /* Destructor */
				buf_puts(buf, "|-");
			}
			/* output argument count and types */
			buf_printf(buf, "|%d", argc);	  
			if (argc == 0) {
				buf_puts(buf, "|-");	  
			}
			else {
				for (i = 0; i < argc; i++) {
					FNArg *arg = (FNArg *)vec_get(mapping->mfn->argv, i);
					buf_printf(buf, "|%s", arg->type);	 
				}
			}
			/* output C name for MOTO function */
			buf_printf(buf, "|%s", mfn_symbolName(mapping->mfn));	  

			/* output C name for C function */
			buf_printf(buf, "|%s", mapping->cfn->fname);

			/* output tracking information */
			buf_printf(buf, "|%c", mapping->mfn->tracked);

			buf_puts(buf, "\",\n");
		}
		enum_free(mape);
		buf_clear(typ);	
	}
	enum_free(e);	 
		
	/* set symbol function */
	sint = malloc(32);
	sprintf(sint, "%d", count);
	out = replace(out,"<SYMBOLCOUNT>",sint);
	out = replace(out,"<SYMBOLS>",buf_data(buf));
	free(sint);

	/* Collect library wide include information */
	count = 0;
	buf_clear(buf);	
	
	archives = sset_createDefault();	  
	includes = sset_createDefault();
	includePaths = sset_createDefault();
	libraries = sset_createDefault();
	libraryPaths = sset_createDefault();
	
	e = vec_elements(c->exts);
	while(enum_hasNext(e)) {
		MXExt *ext = (MXExt *)enum_next(e);

		/* Collect archive information */
		Enumeration* exte = mxext_getArchives(ext);
		while (enum_hasNext(exte))	 sset_add(archives, enum_next(exte));
		
		/* Collect include information */
		exte = mxext_getIncludes(ext);
		while (enum_hasNext(exte)) sset_add(includes, enum_next(exte));
		
		/* Collect include path information */
		exte = mxext_getIncludePaths(ext);
		while (enum_hasNext(exte)) sset_add(includePaths, enum_next(exte));
		
		/* Collect archive information */
		exte = mxext_getLibraries(ext);
		while (enum_hasNext(exte))	 sset_add(libraries, enum_next(exte));
		
		/* Collect library path information */
		exte = mxext_getLibraryPaths(ext);
		while (enum_hasNext(exte)) sset_add(libraryPaths, enum_next(exte));
		
	}
	enum_free(e);

	/* Build includes array */
	e = sset_elements(includes);
	while(enum_hasNext(e)) {
		char* inc = (char*)enum_next(e);
		count++;
		buf_puts(buf, "	\"");
		if(inc[0] == '<') {
			buf_put(buf, inc, strlen(inc));
		}
		else {
			buf_puts(buf,"\\\"");
			buf_put(buf, inc + 1, strlen(inc)-2);
			buf_puts(buf,"\\\"");
		}
		buf_puts(buf, "\"");
		if(enum_hasNext(e)) {
			buf_puts(buf, ",\n");
		}
	}
	
	/* substitute includes array */
	sint = malloc(32);
	sprintf(sint, "%d", count);
	out = replace(out,"<INCLUDECOUNT>",sint);
	out = replace(out,"<INCLUDES>",buf_data(buf));
	free(sint);

	/* Build includePaths array */
	buf_clear(buf);
	e = sset_elements(includePaths);
	while(enum_hasNext(e)) {
		char* s = (char*)enum_next(e);
		buf_printf(buf, "	  %s%s",s,enum_hasNext(e)?",\n":"");
	}
	
	/* Substitute includePaths array */
	sint = malloc(32);
	sprintf(sint, "%d", sset_size(includePaths));
	out = replace(out,"<INCLUDEPATHCOUNT>",sint);
	out = replace(out,"<INCLUDEPATHS>",buf_data(buf));
	free(sint);

	/* Build libraries array */
	buf_clear(buf);
	e = sset_elements(libraries);
	while(enum_hasNext(e)) {
		char* s = (char*)enum_next(e);
		buf_printf(buf, "	  \"%s\"%s",s,enum_hasNext(e)?",\n":"");
	}
	
	/* Substitute libraries array */
	sint = malloc(32);
	sprintf(sint, "%d", sset_size(libraries));
	out = replace(out,"<LIBRARYCOUNT>",sint);
	out = replace(out,"<LIBRARIES>",buf_data(buf));
	free(sint);
		
	/* Build libraryPaths array */
	buf_clear(buf);
	e = sset_elements(libraryPaths);
	while(enum_hasNext(e)) {
		char* s = (char*)enum_next(e);
		buf_printf(buf, "	  %s%s",s,enum_hasNext(e)?",\n":"");
	}
	
	/* Substitute libraryPaths array */
	sint = malloc(32);
	sprintf(sint, "%d", sset_size(libraryPaths));
	out = replace(out,"<LIBRARYPATHCOUNT>",sint);
	out = replace(out,"<LIBRARYPATHS>",buf_data(buf));
	free(sint);
	
	/* Build archives array */
	buf_clear(buf);
	e = sset_elements(archives);
	while(enum_hasNext(e)) {
		char* s = (char*)enum_next(e);
		buf_printf(buf, "	  %s%s",s,enum_hasNext(e)?",\n":"");
	}
	
	/* Substitute archives array */
	sint = malloc(32);
	sprintf(sint, "%d", sset_size(archives));
	out = replace(out,"<ARCHIVECOUNT>",sint);
	out = replace(out,"<ARCHIVES>",buf_data(buf));
	free(sint);
	
	/* output header file */
	fio_put("exports.mx.c", out);
	buf_free(buf);
	buf_free(typ);
}

void mxc_emitInterface(MXC *c) {
	StringBuffer *buf = buf_createDefault();
	Enumeration *e;
	Enumeration *mape;
	StringBuffer *tmp = buf_createDefault();
	char *out;

	/* get shell from properties file */
	out = prop_get(c->props,"USAGE_DOCS");
	out = replace(out,"\n\t","\n");

	/* build symbol information */
	e = vec_elements(c->exts);
	while(enum_hasNext(e)) {
		MXExt *ext = (MXExt *)enum_next(e);
		char *estart = prop_get(c->props,"EXTENSION_START");
		char *eend	 = prop_get(c->props,"EXTENSION_END");
		int stage = 0;

		buf_puts(buf,replace(estart,"<EXTNAME>",ext->name));

		for(stage=0;stage<5;stage++){
			switch(stage){
				case 0: buf_puts(buf,prop_get(c->props,"CONSTRUCTORS")); break;
				case 1: buf_puts(buf,prop_get(c->props,"METHODS")); break;
				case 2: buf_puts(buf,prop_get(c->props,"METHODOPERATORS")); break;
				case 3: buf_puts(buf,prop_get(c->props,"RELATEDFUNCTIONS")); break;
				case 4: buf_puts(buf,prop_get(c->props,"FUNCTIONOPERATORS")); break;
			}
			mape = stab_getKeys(ext->mappings);

			while(enum_hasNext(mape)) {
				int i;
				char *key = (char *)enum_next(mape);
				MXMapping *mapping = (MXMapping *)stab_get(ext->mappings, key);
				int argc = vec_size(mapping->mfn->argv);
				char* prototype = prop_get(c->props,"PROTOTYPE");
				char * ovop = NULL;

				/* test if overloaded operator */
				ovop = isOvop(mapping->mfn->fname);

				/* skip everything but constructors... */
				if (
					stage == 0 && 
					(mapping->mfn->classn == NULL 
					 	|| strcmp(mapping->mfn->fname, 
						mapping->mfn->classn) != 0
						|| ovop != NULL)) continue;

				/* skip everything but methods... */
				else if (stage == 1 && 
					(mapping->mfn->classn == NULL 
					 	|| strcmp(mapping->mfn->fname, 
						mapping->mfn->classn) == 0
						|| ovop != NULL)) continue;
				
				/* skip everything but method operators */
				else if (stage == 2 && 
					(mapping->mfn->classn == NULL 
					 	|| strcmp(mapping->mfn->fname, 
						mapping->mfn->classn) == 0
						|| ovop == NULL)) continue;

				/* skip everything but functions */
				else if (stage == 3 && 
					(mapping->mfn->classn != NULL 
					|| ovop != NULL)) continue;

				/* skip everything but function operators */
				else if (stage == 4 && 
					(mapping->mfn->classn != NULL 
					|| ovop == NULL)) continue;

				buf_clear(tmp);

				/* output method "signature" */
				if(mapping->mfn->classn != NULL)
					buf_printf(tmp, "%s <B>%s::%s</B>(", mapping->mfn->rtype, mapping->mfn->classn, (ovop  != NULL) ? ovop : mapping->mfn->fname);
				else
					buf_printf(tmp, "%s <B>%s</B>(", mapping->mfn->rtype, (ovop != NULL) ? ovop : mapping->mfn->fname);

				/* output argument count and types */
				for (i = 0; i < argc; i++) {
					FNArg *arg = (FNArg *)vec_get(mapping->mfn->argv, i);
					if(i>0) 
						buf_putc(tmp,',');
					buf_printf(tmp, "%s <i>%s</i>", arg->type, arg->name);
				}

				buf_puts(tmp,")<br>\n");

				prototype = replace(prototype,"<PROTO>",buf_data(tmp));
				prototype = replace(prototype,"<DESC>",mapping->mfn->comment);			  

				buf_puts(buf,prototype);
			}
		}

		buf_puts(buf,eend);

		enum_free(mape);
	}
	enum_free(e);

	/* set symbol function */
	out = replace(out,"<EXTENSIONS>",buf_data(buf));
	
	/* output documentation file */
	fio_put("interface.mx.html", out);
	buf_free(buf);
	buf_free(tmp);
}

void mxc_emitCode(MXC *c) {
	Enumeration *exte;
	Enumeration *mape;
	StringBuffer *buf = buf_createDefault();
	char *filename;
	char *out;

	exte = vec_elements(c->exts);
	while(enum_hasNext(exte)) {
		MXExt *ext = (MXExt *)enum_next(exte);
		
		/* get shell from properties file */
		out = prop_get(c->props,"CFILE");
		out = replace(out,"\n\t","\n");

		/* sub in symbol */
		out = replace(out,"<NAME>",ext->name);
		out = replace(out,"<SYMBOL>",ext->ppname);

		/* emit function prototypes */
		mape = stab_getKeys(ext->mappings);
		while(enum_hasNext(mape)) {
			char *key = (char *)enum_next(mape);
			char *proto;
			MXMapping *mapping = (MXMapping *)stab_get(ext->mappings, key);
			char *u = mfn_symbolName(mapping->mfn);
			mman_track(c->mpool, u);
			proto =	prop_get(c->props,"FUNCTION_PROTOTYPE");
			proto = replace(proto,"\n\t","\n");
			buf_puts(buf, replace(proto,"<FN>",u));	 

			/* This is a necessary (?) hack to provide a single arg function for a destructor */
			if(mapping->mfn->fname[0]=='~'){ /* Destructor */
				proto =	prop_get(c->props,"DESTRUCTOR_PROTOTYPE");
				proto = replace(proto,"\n\t","\n");
				buf_puts(buf, replace(proto,"<EN>",ext->name));
			}
		}
		enum_free(mape);	 
		out = replace(out,"<PROTOTYPES>",buf_data(buf));

		/* emit function bodies */
		buf_clear(buf);
		mape = stab_getKeys(ext->mappings);
		while(enum_hasNext(mape)) {
			int i;
			char *key = (char *)enum_next(mape);
			char *body;
			char *rtype;
			MXMapping *mapping = (MXMapping *)stab_get(ext->mappings, key);
			int argc = vec_size(mapping->cfn->argv);
			char *u = mfn_symbolName(mapping->mfn);
			StringBuffer *callbuf = buf_createDefault();

			mman_track(c->mpool, u);
			body =  prop_get(c->props,"FUNCTION_BODY");
			body = replace(body,"\n\t","\n");
			body = replace(body,"<FN>",u);	 
			rtype = mapping->mfn->rtype;
			
			if (strcmp("void", rtype) == 0) {
				buf_printf(callbuf, "	%s(", mapping->cfn->fname);	
			}
			else if (strcmp("boolean", rtype) == 0) {
				buf_printf(callbuf, "	moto_return_boolean(%s(", mapping->cfn->fname);	  
			}
			else if (strcmp("byte", rtype) == 0) {
				buf_printf(callbuf, "	moto_return_byte(%s(", mapping->cfn->fname);	  
			}
			else if (strcmp("char", rtype) == 0) {
				buf_printf(callbuf, "	moto_return_char(%s(", mapping->cfn->fname);	  
			}
			else if (strcmp("int", rtype) == 0) {
				buf_printf(callbuf, "	moto_return_int(%s(", mapping->cfn->fname);	 
			}
			else if (strcmp("long", rtype) == 0) {
				buf_printf(callbuf, "	moto_return_long(%s(", mapping->cfn->fname);	  
			}
			else if (strcmp("float", rtype) == 0) {
				buf_printf(callbuf, "	moto_return_float(%s(", mapping->cfn->fname);	
			}
			else if (strcmp("double", rtype) == 0) {
				buf_printf(callbuf, "	moto_return_double(%s(", mapping->cfn->fname);	 
			}
			else {
				/* this function returns a pointer type */
				buf_printf(callbuf, "	moto_return_ptr(%s(", mapping->cfn->fname);	 
			}
			
			for (i = 0; i < argc; i++) {
				FNArg *arg;
				char *type;
				int ispointer = 0;
				arg = (FNArg *)vec_get(mapping->cfn->argv, i);
				ispointer = arg->type[strlen(arg->type) - 1] == '*';
				type = arg->type;
				
				if (i > 0) {
					buf_puts(callbuf, ", ");	
				}
							
				if (strcmp(type, "boolean") == 0) {
					buf_printf(callbuf, "*((char *)argv[%d])", i);	 
				}
				else if (strcmp(type, "char") == 0) {
					buf_printf(callbuf, "(char)*((int *)argv[%d])", i);	
				}
				else if (strcmp(type, "signed char") == 0) {
					buf_printf(callbuf, "(signed char)*((int *)argv[%d])", i);	 
				}
				else if (strcmp(type, "int") == 0 || strcmp(type, "int32_t") == 0) {
					if (ispointer) {
						buf_printf(callbuf, "(int32_t *)argv[%d]", i);	 
					}
					else {
						buf_printf(callbuf, "*((int32_t *)argv[%d])", i);	 
					}
				}
				else if (strcmp(type, "int64_t") == 0) {
					if (ispointer) {
						buf_printf(callbuf, "(int64_t *)argv[%d]", i);	 
					}
					else {
						buf_printf(callbuf, "*((int64_t *)argv[%d])", i);	 
					}
				}
				else if (strcmp(type, "float") == 0) {
					if (ispointer) {
						buf_printf(callbuf, "(float *)argv[%d]", i);	  
					}
					else {
						buf_printf(callbuf, "*((float *)argv[%d])", i);	  
					}
				}
				else if (strcmp(type, "double") == 0) {
					if (ispointer) {
						buf_printf(callbuf, "(double *)argv[%d]", i);	
					}
					else {
						buf_printf(callbuf, "*((double *)argv[%d])", i);	
					}
				}
				else {
					buf_printf(callbuf, "argv[%d]", i);	  
				}			 
			}
			
			if (strcmp("void", rtype) == 0) {
				/* this function does not return anything */
				buf_puts(callbuf, ");\n");	  
				/* buf_puts(callbuf, "	 moto_return_ptr(NULL);"); */
			}
			else {
				buf_puts(callbuf, "));");	 
			}
			
			body = replace(body,"<CALL>",buf_data(callbuf));
			buf_free(callbuf);

			buf_puts(buf, body);
		 
			if(mapping->mfn->fname[0]=='~'){ /* Destructor */
				body =  prop_get(c->props,"DESTRUCTOR_BODY");
				body = replace(body,"\n\t","\n");
				body = replace(body,"<DN>",mapping->cfn->fname);
				buf_puts(buf, replace(body,"<EN>",ext->name));
			}

		}
		enum_free(mape);
		out = replace(out,"<FUNCTIONS>",buf_data(buf));

		/* output code file */
		filename = (char *)malloc(strlen(ext->name) + 16);
		sprintf(filename, "%s.mx.c", ext->name);
		fio_put(filename, out);
		free(filename);
		buf_clear(buf);
	
	}
	enum_free(exte);

	buf_free(buf);

}

void mxc_emitBuildScript(MXC *c) {
	StringBuffer *buf = buf_createDefault();
	StringSet *set;
	Enumeration *e,*exte;
	char *out;

	/* get shell from properties file */
	out = prop_get(c->props,"MAKEFILE");
	out = replace(out,"\n\t","\n");

#ifdef SHARED_MALLOC
	out = replace(out,"<SHARED_MALLOC>","-DSHARED_MALLOC");
#elif
	out = replace(out,"<SHARED_MALLOC>","");
#endif

	/* sub in moto root dir */
	out = replace(out,"<ROOT_DIR>",ROOT_DIR);
	
	/* sub in target */
	out = replace(out,"<TARGET>",c->target);

	/* sub in libs */
	buf_clear(buf);	
	set = sset_createDefault();
	for(e = vec_elements(c->exts);enum_hasNext(e);) {
		for (exte = mxext_getLibraries((MXExt *)enum_next(e));enum_hasNext(exte);)
			sset_add(set, enum_next(exte));
	}
	enum_free(e);
	
	for(e = sset_elements(set);enum_hasNext(e);) 
		buf_printf(buf,"%s ", enum_next(e));
	enum_free(e);
	out = replace(out,"<LIBS>",buf_data(buf));

	/* sub in include paths */
	
	buf_clear(buf);	
	set = sset_createDefault();
	for(e = vec_elements(c->exts);enum_hasNext(e);) {
		for (exte = mxext_getIncludePaths((MXExt *)enum_next(e));enum_hasNext(exte);)
			sset_add(set, enum_next(exte));
	}
	enum_free(e);
	
	for(e = sset_elements(set);enum_hasNext(e);) 
		buf_printf(buf,"-I%s ", (char*)enum_next(e));
	enum_free(e);
	out = replace(out,"<INCLUDEPATHS>",buf_data(buf));

	/* sub in library paths */
	
	buf_clear(buf);	
	set = sset_createDefault();
	for(e = vec_elements(c->exts);enum_hasNext(e);) {
		for (exte = mxext_getLibraryPaths((MXExt *)enum_next(e));enum_hasNext(exte);)
			sset_add(set, enum_next(exte));
	}
	enum_free(e);
	
	for(e = sset_elements(set);enum_hasNext(e);) 
		buf_printf(buf,"-L%s ", (char*)enum_next(e));
	enum_free(e);
	out = replace(out,"<LIBRARYPATHS>",buf_data(buf));

	/* sub in archives */
	
	buf_clear(buf);	
	set = sset_createDefault();
	for(e = vec_elements(c->exts);enum_hasNext(e);) {
		for (exte = mxext_getArchives((MXExt *)enum_next(e));enum_hasNext(exte);)
			sset_add(set, enum_next(exte));
	}
	enum_free(e);
	
	for(e = sset_elements(set);enum_hasNext(e);) 
		buf_printf(buf,"%s ", (char*)enum_next(e));
	enum_free(e);
	out = replace(out,"<ARCHIVES>",buf_data(buf));
			
	/* sub in files */
	buf_clear(buf);	
	set = sset_createDefault();
	e = vec_elements(c->exts);
	while(enum_hasNext(e)) {
		for (exte = mxext_getFiles((MXExt *)enum_next(e));enum_hasNext(exte);)
			sset_add(set, enum_next(exte));
	}
	enum_free(e);
	
	for(e = sset_elements(set);enum_hasNext(e);) 
		buf_printf(buf,"%s ", (char*)enum_next(e));
	enum_free(e);
	
	out = replace(out,"<FILES>",buf_data(buf));

	/* output script file */
	buf_clear(buf);
	buf_puts(buf, out);
	fio_put("mx.mk", buf_data(buf));
	buf_free(buf);

}

static char *vs(Vector *v) {
	char *result;
	StringBuffer *buf = buf_create(32);
	int i;
	int size = vec_size(v);
	if (size == 0) {
		buf_puts(buf, " ");	 
	}
	else {
		buf_puts(buf, (char *)vec_get(v, 0));
		for (i = 1; i < size; i++) {
			char *s = (char *)vec_get(v, i);
			if (buf_lastChar(buf) != '*') {
				buf_puts(buf, " ");	 
			}
			buf_puts(buf, s);	  
		}
	}
	result = buf_data(buf);
	return result;
}

static char *replace(char* string, char* find, char* replace) {
	StringBuffer* out;
	KMPString* kmp;
	Tokenizer* tok;
	char* substr,*result;

	out = buf_createDefault();
	kmp = kmp_create(find);
	tok = tok_createKMPTok(string,kmp);

	substr=tok_next(tok);

	while (substr){
		buf_puts(out,substr);
		if((substr=tok_next(tok)) != NULL)
			buf_puts(out,replace);
	}

	kmp_free(kmp);
	tok_free(tok);

	result = buf_toString(out);
	buf_free(out);

	return result;
}

static char *opName(char * op) {
	
	int i = 0;
	
	for (i = 0; i < OVOPSNUM; i++)
	if (!strcmp(op, ovops[i][1])) {
		return ovops[i][0];
	}

	return NULL;

}

static char * isOvop(char * fname) {

	int i = 0;

	/* Must begin with _ */
	if (fname[0] != '_') return NULL;

	for (i = 0; i < OVOPSNUM; i++)
	if (!strcmp(fname, ovops[i][0])) {
		return ovops[i][1];
	}
	return NULL;
		
}

/*=============================================================================
// end of file: $RCSfile: mxcg.c,v $
==============================================================================*/

