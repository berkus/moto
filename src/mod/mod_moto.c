/*=============================================================================

	Copyright (C) 2000-2002 David Hakim.
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

	$RCSfile: mod_moto.c,v $	
	$Revision: 1.64 $
	$Author: dhakim $
	$Date: 2003/05/29 02:14:08 $
 
==============================================================================*/
#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/time.h>

#include <ftw.h>

#include "mman.h"
#include "runtime.h"
#include "mxstring.h"
#include "excpfn.h"
#include "err.h"

#include "module.h"
#include "timer.h"

#include "motolex.h"
#include "motohook.h"
#include "motofn.h"
#include "motoerr.h"
#include "motoi.h"
#include "motod.h"
#include "motov.h"
#include "dl.h"
#include "pp.h"
#include "env.h"
#include "exception.h"
#include "excpfn.h"
#include "memsw.h"

module moto_module;
MotoEnv *env;
MotoPP *ppenv;

extern int motoyyparse();

typedef struct modMotoConfig{
	table* options;
} mod_moto_config;

static const char* moto_setOption(cmd_parms *parms, void *mconfig,char* arg1, char* arg2);

static const command_rec moto_cmds[] = 
{ 
	{
		"MotoOption",				/* directive */
		moto_setOption,				/* configuration routine */
		NULL,					/* extra arguments */
		OR_ALL,					/* directive location flags */
		TAKE12,					/* argument handler flag */
		"MotoOption <Option Name> <Option Value>" /* usage */
	},
	{NULL}
};

int globalSharedHeapSize = 0;
time_t globalSessionTimeout = 0;
int globalStatesPerSession = 0;
char* globalMXPath = NULL;
char* globalLockDir = NULL;

char* 
moto_setMXPath(pool* p,char* mxpath_option){

	char* mxpath = (char*)ap_pcalloc(p,strlen(mxpath_option)+1);
	strcpy(mxpath,mxpath_option);
		
	return mxpath;
}

char* 
moto_setLockDir(pool* p,char* lockdir_option){

	char* lockdir = (char*)ap_pcalloc(p,strlen(lockdir_option)+1);
	strcpy(lockdir,lockdir_option);
		
	return lockdir;
}

static const char* 
moto_setOption(cmd_parms *parms, void *mconfig,char* arg1, char* arg2){
	mod_moto_config* cfg;
	
	cfg = (mod_moto_config*)mconfig;

	if(arg2 != NULL)
		ap_table_set(cfg->options, arg1, arg2);
	else
		ap_table_set(cfg->options, arg1, "TRUE");
	
	/* Wretched Wretched Apache Initializer ... I will force you
		to do my bidding I will... (There ain't no such thing as
		a global initializer in Apache) */
  
	if(!strcmp(arg1,"SharedHeapSize") && arg2 != NULL) {
		int sharedHeapSize = atoi(arg2);
		int multiplier =1;

		if (strlen(arg2) > 1){
			if(arg2[strlen(arg2) -1] == 'M')
				multiplier = 1024*1024;
			else if(arg2[strlen(arg2) -1] == 'K')
				multiplier = 1024;
		}

		if (globalSharedHeapSize < sharedHeapSize * multiplier)
			globalSharedHeapSize = sharedHeapSize * multiplier;
	}

	if(!strcmp(arg1,"Session.Timeout") && arg2 != NULL) {
		int sessionTimeout = atoi(arg2);
		
		if (globalSessionTimeout < sessionTimeout)
			globalSessionTimeout = sessionTimeout;
	}

	if(!strcmp(arg1,"Session.StatesPerSession") && arg2 != NULL) {
		int statesPerSession = atoi(arg2);

		if (globalStatesPerSession < statesPerSession)
			globalStatesPerSession = statesPerSession;
	}

	if(!strcmp(arg1,"MXPath") && arg2 != NULL){
		globalMXPath = moto_setMXPath(parms->pool,arg2);
	}

	if(!strcmp(arg1,"LockDir") && arg2 != NULL){
		globalLockDir = moto_setLockDir(parms->pool,arg2);
	}

	return NULL;
}

static void *
moto_create_directory_config(pool* p, char *dir){
	mod_moto_config* cfg =
		(mod_moto_config*) ap_pcalloc(p,sizeof(mod_moto_config));

	cfg->options = ap_make_table(p,10);

	return (void*)cfg;
}

static void *
moto_merge_directory_config(pool* p, void* base, void* new){
	mod_moto_config* merged =
		(mod_moto_config*) ap_pcalloc(p,sizeof(mod_moto_config));

	mod_moto_config* parent = (mod_moto_config*)base;
	mod_moto_config* child = (mod_moto_config*)new;

	merged->options = ap_overlay_tables(p,parent->options,child->options);
	
	return (void*)merged;
}

static void *
moto_create_server_config(pool* p, server_rec *s){
	mod_moto_config* cfg =
		(mod_moto_config*) ap_pcalloc(p,sizeof(mod_moto_config));

	cfg->options = ap_make_table(p,10);
	
	return (void*)cfg;
}

static void *
moto_merge_server_config(pool* p, void* base, void* new){
	mod_moto_config* merged =
		(mod_moto_config*) ap_pcalloc(p,sizeof(mod_moto_config));

	mod_moto_config* parent = (mod_moto_config*)base;
	mod_moto_config* child = (mod_moto_config*)new;

	merged->options = ap_overlay_tables(p, parent->options,child->options);

	return (void*)merged;
}


StringSet* 
loaded_extensions;
int 
preload(const char *file, const struct stat *sb,int flag){

	int i = mxstr_lastIndexOf(file,'/');
	char* filename;

	if(i!= -1) 
		filename = mxstr_substr(file, i+1);
	else
		filename = estrdup((char*)file);

	if(mxstr_endsWith(filename,".so") && mxstr_startsWith(filename,"libmx") &&
		!sset_contains(loaded_extensions,filename)){
			fprintf(stderr,"### Pre-Loading Extension '%s' from '%s'\n",filename,file);
			sset_add(loaded_extensions,filename);
			moto_os_dso_close(moto_os_dso_open(file));
	} else free(filename);

	return 0;
}

void 
moto_initializer(server_rec *s, pool *p) {
	char version[20];
	
	sprintf(version,"mod_moto/%d.%d.%d",
		MOTO_MAJOR_VERSION, 
		MOTO_MINOR_VERSION, 
		MOTO_MICRO_VERSION);
	ap_add_version_component(version);

	if(globalSharedHeapSize > 0)
		moto_init(globalSharedHeapSize,globalMXPath);
	else
		moto_init(32*1024*1024,globalMXPath);

	if(globalSessionTimeout > 0 )
		if(globalStatesPerSession > 0)
			ctxt_init(globalSessionTimeout,globalStatesPerSession);
		else
			ctxt_init(globalSessionTimeout,50); 
	else
		if(globalStatesPerSession > 0)
			ctxt_init(30*60,globalStatesPerSession);
		else
			ctxt_init(30*60,50);

	cook_init();

	loaded_extensions = sset_createDefault();
	{
		char *dupmxpath = estrdup(mxdl_getMXPath());
		char *dir = strtok(dupmxpath, ":");
		while(dir != NULL) {
			ftw(dir, preload , 5);
			dir = strtok(NULL, ":");
		}
		free(dupmxpath);
	}
	sset_free(loaded_extensions);
}

void 
moto_apout(int code) {

	if(!headers_already_sent)
		ap_send_http_header(current_request);

	if (code != MOTO_OK || buf_size(env->err) > 0) {
		StringBuffer *buf = buf_createDefault();
		char *err = buf_data(env->err);
		int len = strlen(err);
		int i;
		buf_puts(buf, "<HTML><HEAD><TITLE>*** Moto Error</TITLE></HEAD>\n");
		buf_puts(buf, "<BODY BGCOLOR=\"#ffffff\">\n");
		buf_puts(buf, "<H1>Moto Error</H1>\n");
		buf_puts(buf, "<PRE>\n");
		for (i = 0; i < len; i++) {
			char c = err[i];
			switch (c) {
				case '<':
					buf_puts(buf, "&lt;");
					break;
				case '>':
					buf_puts(buf, "&gt;");
					break;
				default:
					buf_putc(buf, c);
					break;
			}
		}
		buf_puts(buf, "</PRE>\n");
		buf_puts(buf, "<HR NOSHADE SIZE=1>\n");
		buf_puts(buf, "</BODY></HTML>\n");
		ap_rputs(buf_data(buf),current_request);
		buf_free(buf);
	} 
	else	{
		
		int bytesToWrite, totalBytesWritten = 0;
		char* data ;
		
		moto_freeFrame(env);
		data = buf_data(env->out);
		bytesToWrite = buf_size(env->out);
		
		while (totalBytesWritten < bytesToWrite) {
			int bytesWritten = 0;
			bytesWritten = 
				ap_rwrite(
					data + totalBytesWritten, 
					bytesToWrite-totalBytesWritten, 
					current_request
				);
			totalBytesWritten += bytesWritten;
		}
			
	}
}

void 
moto_apnull(int code){
	moto_freeFrame(env);
}

static void 
moto_pipepp(int code) {

	if (code == MOTO_FAIL || buf_size(ppenv->err) > 0) {
		StringBuffer *buf = buf_createDefault();
		char *err = buf_data(ppenv->err);
		int len = strlen(err);
		int i;

		buf_puts(buf, "<HTML><HEAD><TITLE>*** Moto Error</TITLE></HEAD>\n");
		buf_puts(buf, "<BODY BGCOLOR=\"#ffffff\">\n");
		buf_puts(buf, "<H1>Moto Error</H1>\n");
		buf_puts(buf, "<PRE>\n");

		for (i = 0; i < len; i++) {
			char c = err[i];
			switch (c) {
				case '<':
					buf_puts(buf, "&lt;");
					break;
				case '>':
					buf_puts(buf, "&gt;");
					break;
				default:
					buf_putc(buf, c);
					break;
			}
		}
		buf_puts(buf, "</PRE>\n");
		buf_puts(buf, "<HR NOSHADE SIZE=1>\n");
		buf_puts(buf, "</BODY></HTML>\n");

		ap_send_http_header(current_request);
		ap_rputs(buf_data(buf),current_request);

		buf_free(buf);

	}
}

StringBuffer *out; // FIXME!!!! Huge hack to get output for flush()

int 
executeTemplate(request_rec *r, char *filename) {
	MotoTree *tree = NULL;
	int result = MOTO_OK;	
	int redirect = 0;
	char *text = NULL;
	int size;
	int i;

	/* Install signal handlers for SignalsThrowException Option */
	installSignalHandlers();

	/* preprocess */
	TRY {
		motopp_start(filename, MACRO_DEBUG_FLAG);
		ppenv->outfn = moto_pipepp;
		motopp(ppenv);
		text = buf_toString(ppenv->out);
		motopp_done(MOTO_OK);				
	}
	CATCH_ALL {
		motopp_done(MOTO_FAIL);
		result = MOTO_FAIL;
		if (text) {
			free(text);
		}
	}
	END_TRY

	if (result == MOTO_OK) {

		/* initialize the runtime */
			rtime_init();

		TRY { 

			/* bootstap env */
			env = moto_createEnv(DEF_DL_FLAG,filename);	 
			moto_setEnv(env);
			moto_createFrame(env, MAIN_FRAME);	

			tree = env->tree;

			/* check syntax */
			moto_prepareMainBuffer(env, text);
			motoyyparse((void *)env);
			if (env->errflag || buf_size(env->err) > 0) {
				THROW_D("MotoException");
			}

			/* try "built-in" dynamic loads */
			if (moto_use(env, "moto") == -1) {
				THROW("DLException","Could not locate default extensions in '%s'",mxdl_getMXPath());
			}

			/* Get function definitions and load libraries */

			env->mode = VERIFIER_MODE;
			env->e = motod;
			env->outfn = NULL;
			size = vec_size(tree->cells);
			for (i = 0; i < size; i++) {
				(*env->e)(vec_get(tree->cells, i));
			}

			/* verify semantics */

			env->mode = VERIFIER_MODE;
			env->e = motov;
			env->outfn = NULL;
			size = vec_size(tree->cells);
			for (i = 0; i < size; i++) {
				(*env->e)(vec_get(tree->cells, i)); 
			}
			moto_freeFrame(env);
			if (env->errflag || buf_size(env->err) > 0) {
				THROW_D("MotoException");
			}
			else {
				moto_clearEnv(env);
			}

			/* interpret */

			moto_createFrame(env, MAIN_FRAME);	
						out = env->frame->out; // FIXME!!!! Huge hack to get output for flush()
			env->mode = INTERPRETER_MODE;
			env->e = motoi;
			env->outfn = moto_apout;
			size = vec_size(tree->cells);
			for (i = 0; i < size; i++) {
				(*env->e)(vec_get(tree->cells, i)); 
			}
			result = MOTO_OK;
		}
		CATCH("HTTPRedirectException") {
			env->outfn = moto_apnull; 
			result = MOTO_OK;
			redirect=1;
		}
		CATCH("MotoException"){
			env->outfn = moto_apout;
			result = MOTO_FAIL;
		}
		CATCH_ALL{

			/* Log the exception */
			
			ap_log_error(
				APLOG_MARK,
				APLOG_NOERRNO|APLOG_ERR,
				r->server,
				"Uncaught %s\n    message: %s\n\n%s",
				excp_getCurrentException()->t,
				excp_getCurrentException()->msg,
				excp_getCurrentException()->stacktrace
			);
			
			/* Output the exception */
			
			err_error(
				"MotoRuntimeError", 
				excp_getCurrentException()->t,
				excp_getCurrentException()->msg,
				excp_getCurrentException()->stacktrace
            );
			env->outfn = moto_apout;

			result = MOTO_FAIL;
		}
		END_TRY

		/* rtime_clearMPool(); */ /* Causes a memory leak when children are killed */

		rtime_cleanup();

		moto_done(result);

		moto_freeYYBuffer();				  
		if (text) {
			free(text);
		}
	}

	// debug();

	/* Return the signal handler for SIGSEGV to it's old state */
	unInstallSignalHandlers();

	return redirect;
}

static int 
moto_handler(request_rec *r)
{
	char* template;
	int redirect = 0;

	/* ap_log_error(
		APLOG_MARK,
		APLOG_NOERRNO|APLOG_INFO,
		r->server,
		">>> [%d]", getpid()
	); */
	
	/* Make sure this was a get or a post */

	r->allowed |= (1 << M_GET) | (1 << M_POST);

	if (r->method_number != M_GET && r->method_number != M_POST)
		return DECLINED;

	/* set the response type to text/html */

	r->content_type = "text/html";

	/* Ok, I think we can send the header now and stop if that was all 
		they wanted */

	if (r->header_only) {
		ap_send_http_header(r);
		return 0;
	}

	template = r->filename;

	if(template != NULL){
		int i;
		struct stat buf;
		i = stat(template, &buf);
		if (i == -1) {
			ap_send_http_header(r);
			ap_rprintf(r,"<html><body><b>File not found</b><pre>\n");
			ap_rprintf(r,"File: %s\n", r->filename);
			ap_rprintf(r,"URI:  %s\n", r->uri);
			ap_rprintf(r,"Path: %s\n", r->path_info);
			ap_rprintf(r,"</pre></html></body>");
		} else {
			mod_moto_config* cfg;

			cfg = (mod_moto_config*)ap_get_module_config(r->per_dir_config,&moto_module );

			startTimer();

			/* Initialize the current state, parts, cookies, request, and options */ 
			req_init(r,cfg->options);

			redirect = executeTemplate(r, template);

			/* Clean up the current state, parts, cookies, request, and options */
			req_cleanup();

			logExecutionTime(r,template,readTimer());

		}
	} 
	else {
		ap_send_http_header(r);
		ap_rprintf(r,"<html><body>No file specified</html></body>");	
	}

	/* ap_log_error(
		APLOG_MARK,
		APLOG_NOERRNO|APLOG_INFO,
		r->server,
		"<<< [%d]", getpid() 
	); */

	if (redirect)
		return REDIRECT;
	return OK;
}

void 
moto_child_init(server_rec* r, pool* s) {

	shared_initLocking(globalLockDir); /* Put the lock down on our major shared resources */ 
	mman_initLocking(globalLockDir);
	ctxt_initLocking(globalLockDir);

	moto_os_dso_init();
	stacktrace_init();
	motoi_init();
}

static const 
handler_rec moto_handlers[] =
{
	 {"moto", moto_handler},
	 {NULL}
};

module MODULE_VAR_EXPORT moto_module =
{
	 STANDARD_MODULE_STUFF,
	 moto_initializer,			/* initializer */
	 moto_create_directory_config,	/* create per-directory config structure */
	 moto_merge_directory_config, /* merge per-directory config structures */
	 moto_create_server_config,		/* create per-server config structure */
	 moto_merge_server_config,		/* merge per-server config structures */
	 moto_cmds,				/* command table */
	 moto_handlers,			/* handlers */
	 NULL,				/* translate_handler */
	 NULL,				/* check_user_id */
	 NULL,				/* check auth */
	 NULL,				/* check access */
	 NULL,				/* type_checker */
	 NULL,				/* pre-run fixups */
	 NULL,				/* logger */
	 NULL,				/* header parser */
	 moto_child_init,			/* child_init */
	 NULL,				/* child_exit */
	 NULL				/* post read-request */
};

/*=============================================================================
// end of file: $RCSfile: mod_moto.c,v $
==============================================================================*/

