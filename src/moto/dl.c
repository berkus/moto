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

	$RCSfile: dl.c,v $	
	$Revision: 1.41 $
	$Author: dhakim $
	$Date: 2003/02/28 20:50:20 $
 
==============================================================================*/
#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef USE_DYLD					 /* NeXT/Apple dynamic linker */
#include <mach-o/dyld.h>
#else
#include <dlfcn.h>
#endif

#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include "memsw.h"

#include "exception.h"
#include "excpfn.h"
#include "stringbuffer.h"
#include "symboltable.h"
#include "platform.h"
#include "log.h"
#include "runtime.h"

#include "dl.h"
#include "cell.h"
#include "meta.h"

extern int errno;												

static int 
stat_file(char *path) {
	int result = 0;
	struct stat buf;
	if (stat(path, &buf) != 0) {
		result = errno;
	}
	return result;
}

char *
mxdl_find(char *usename) {
	char *result = NULL;
	char *libname = NULL;
	int len;
	int trlen;
	char *tr_usename;
	char *tr_pathname;
	char *mxpath = mxdl_getMXPath();
	StringBuffer *d;

	/* translate the usename */
	trlen = len = strlen(usename);
	tr_usename = emalloc(len + 1);
	tr_usename[len] = '\0';
	while (--len >= 0) {
		if (*(usename + len) == '.') {
			*(tr_usename + len) = '_';	  
		}
		else {
			*(tr_usename + len) = *(usename + len);	
		}
	}
	
	if (index(tr_usename, '_') == NULL) {
		/* leave name alone */
	}

	/* translate the tr_usename to a path */
	len = strlen(tr_usename);
	tr_pathname = emalloc(len + 1);
	tr_pathname[len] = '\0';
	while (--len >= 0) {
		if (*(tr_usename + len) == '_') {
			*(tr_pathname + len) = '/';	
		}
		else {
			*(tr_pathname + len) = *(tr_usename + len);	 
		}
	}

	/* look in all path directories for the library */
	d = buf_createDefault();
	if (mxpath != NULL) {
		char *dupmxpath = estrdup(mxpath);
		char *dir = strtok(dupmxpath, ":");
		while (dir != NULL) {
			buf_clear(d);
			buf_puts(d, dir);
			if (dir[strlen(dir) - 1] != '/') {
				buf_putc(d, '/');
			}
			buf_puts(d, tr_pathname);
			buf_putc(d, '/');
			buf_puts(d, "libmx_");
			buf_puts(d, tr_usename);
			buf_puts(d, ".so");
			libname = buf_data(d);						 
			if (stat_file(libname) == 0) {
				result = buf_toString(d);
				//mman_track(rtime_getRuntime()->mpool, result);
				buf_free(d);
				d = NULL;
				break;
			}
			dir = strtok(NULL, ":");
		}
		free(dupmxpath);
	}
	else {
		buf_printf(d, "libmx_%s.so", tr_usename);
		libname = buf_toString(d);
		buf_free(d);
		d = NULL;
		if (stat_file(libname) == 0) {
			result = libname;
		}
	}
	
	if (d != NULL) {
		buf_free(d);
	}

	free(tr_usename);
	free(tr_pathname);

	return result;
}

#if USE_DYLD
void 
undefined_symbol_handler(const char *symbolName)
{
	printf("Undefined symbol %s, could not load extension\n",symbolName);
//	  exit(1);
}

NSModule 
multiple_symbol_handler (NSSymbol s, NSModule old, NSModule new)
{
	/*
	 * Since we can't unload symbols, we're going to run into this
	 * every time we reload a module. Workaround here is to just
	 * rebind to the new symbol, and forget about the old one.
	 * This is crummy, because it's basically a memory leak.
	 */

	return(old);
}

void 
linkEdit_symbol_handler (NSLinkEditErrors c, int errorNumber,
										const char *fileName, const char *errorString)
{
	 
	abort();
}
#endif


static SymbolTable* extension_cache;

void 
moto_os_dso_init(void)
{
#if USE_DYLD 
	 NSLinkEditErrorHandlers handlers;

	 handlers.undefined = undefined_symbol_handler;
	 handlers.multiple  = multiple_symbol_handler;
	 handlers.linkEdit  = linkEdit_symbol_handler;

	 NSInstallLinkEditErrorHandlers(&handlers);
#endif
	/* FIXME: Argh ... this sucks ... on MacOS X every process needs to 'load' its
		own extensions even though they are already 'loaded' ... garbage */
	extension_cache = stab_createDefault();
}

char* 
moto_os_dso_error(){
#if USE_DYLD
	return NULL;
#else
	return dlerror();
#endif
}

void* 
moto_os_dso_open(const char* path){
	void* handle = stab_get(extension_cache,(char*)path);
	if (handle != NULL) return handle;
	
	{
#if USE_DYLD
		NSObjectFileImage image;
		if (NSCreateObjectFileImageFromFile(path, &image) !=
			NSObjectFileImageSuccess)
			return NULL;
		
		handle = NSLinkModule(image, path, NSLINKMODULE_OPTION_PRIVATE);
#else
		handle = dlopen(path, RTLD_LAZY);
#endif

		stab_put(extension_cache,estrdup((char*)path),handle);
		return handle;
	}
}

void 
moto_os_dso_close(void* handle){
#if USE_DYLD
	NSUnLinkModule((NSModule*)handle,NSUNLINKMODULE_OPTION_KEEP_MEMORY_MAPPED);
#endif
}

void* 
moto_os_dso_sym(void *handle, const char* symname){
#if USE_DYLD
	NSSymbol symbol;
	char *symname2 = (char*)malloc(sizeof(char)*(strlen(symname)+2));
	sprintf(symname2, "_%s", symname);
	symbol = NSLookupSymbolInModule(handle,symname2);
	free(symname2);
	return NSAddressOfSymbol(symbol);
#else
	return dlsym(handle, symname);
#endif
}

static char* mxpath;

/* Return the current MXPath */
char*
mxdl_getMXPath(){
	return mxpath;
}

/* Initialize the mxpath */
void
mxdl_init(){
	int mxpathlen;
	
	char* inst_mx_dir = INST_MX_DIR; /* The default path to look for moto extensions */

	moto_os_dso_init();
		
	mxpathlen = 0;

	if(inst_mx_dir != NULL)
		mxpathlen = strlen(inst_mx_dir) + 1;
	if(getenv("MXPATH") != NULL)
		mxpathlen += strlen(getenv("MXPATH")) + 1;

	mxpath = (char*)calloc(mxpathlen + 1,1);

	/* The order in which extension paths are searched should be
		first the -x option, next the environment variable, 
		last the instdir */

	if(getenv("MXPATH") != NULL){
		strcpy(mxpath,getenv("MXPATH"));
	}
	if(inst_mx_dir != NULL){
		strcat(mxpath,":");
		strcat(mxpath,inst_mx_dir);
	}
}

void 
mxdl_cleanup(){
	Enumeration* e = stab_getKeys(extension_cache);
	while(enum_hasNext(e)){
		free(enum_next(e));
	}
	enum_free(e);
	stab_free(extension_cache);
	free(mxpath);
}

/* Prepend a new path component to the mxpath */
void
mxdl_prependMXPath(char* xpath){
	char* newmxpath = (char*)malloc(strlen(mxpath) + strlen(xpath) + 2);
	strcpy(newmxpath,xpath);
	strcat(newmxpath,":");
	strcat(newmxpath,mxpath);
	free(mxpath);
	mxpath=newmxpath; 
}


MotoExtension* 
mxdl_load(char *libname) {
	void *handle;
	void (*expsyms)();
	MotoExtension *mx;	  
	char *error;
	int i,len;
	StringBuffer* buf;
	char* tr_usename;
	char* libpath;
	
	/* make sure we have a fully-qualified library path */
	libpath = mxdl_find(libname);
	if(libpath == NULL) 
		return NULL;
	
	/* get a handle to the library */

	tr_usename = estrdup(libname);
	len = strlen(tr_usename);
	for(i=0;i<len;i++)
		if(tr_usename[i] == '.')
			tr_usename[i] = '_';	 

	handle = moto_os_dso_open(libpath);

	if (!handle && (error = moto_os_dso_error()) != NULL) {
		log_error(__FILE__, "%s\n", error);
	}

	/* get all the exported symbols from the library */
	buf = buf_createDefault();
	buf_printf(buf,"__MOTO_%s_get_record",tr_usename);
	expsyms = moto_os_dso_sym(handle, buf_data(buf));
	if ((error = moto_os_dso_error()) != NULL) {
		log_error(__FILE__, "%s\n", error);
	}
	
	buf_free(buf);
	free(tr_usename);
	/* create a MotoExtension struct -- 
		put in one MotoFunction struct for each exported symbol */
	mx = (MotoExtension *)emalloc( sizeof(MotoExtension));
	
	(*expsyms)(mx);
	
	mx->functions = vec_createDefault();
	
	for (i = 0; i < mx->symbolCount; i++) {
		MotoFunction* mfn = mfn_createFromSymbol(mx->symbols[i],libname,handle);
		vec_add(mx->functions,mfn);	
	}
	
	free(libpath);
	
	return mx;
}

void 
moto_loadDestructor(void *handle, char *name, void(**free_fn)(void *)) {
	char *error;
	char dname[128];

	moto_os_dso_error();
	sprintf(dname,"__MOTO_D_%s",name);
	*free_fn = moto_os_dso_sym(handle, dname);
	if ((error = moto_os_dso_error()) != NULL) {
		log_error(__FILE__, "%s\n", error);
		*free_fn = NULL;
	}
}

/*=============================================================================
// end of file: $RCSfile: dl.c,v $
==============================================================================*/

