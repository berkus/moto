/*=============================================================================

   Copyright (C) 2000 Kenneth Kocienda and David Hakim.
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

   $RCSfile: motofn.c,v $   
   $Revision: 1.59 $
   $Author: dhakim $
   $Date: 2003/03/20 22:33:08 $
 
==============================================================================*/
#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <sys/types.h>
#include <sys/file.h>
#include <stdio.h>
#include <fcntl.h>

#include "memsw.h"

#include "mman.h"
#include "log.h"
#include "fileio.h"
#include "runtime.h"
#include "exception.h"
#include "stacktrace.h"
#include "excpfn.h"
#include "err.h"
#include "env.h"

#include "dl.h"
#include "motolex.h"
#include "moto.tab.h"
#include "motoi.h"
#include "motoc.h"
#include "motod.h"
#include "motov.h"
#include "motoerr.h"
#include "motofn.h"
#include "motocodes.h"
#include "motohook.h"
#include "mototree.h"
#include "motopplex.h"
#include "pp.h"

#ifndef LOG_LEVEL
#  define LOG_LEVEL LOG_ERROR
#endif

/* Calls shared_init, mxdl_init, mman_init, err_init, log_init, and moto_errInit in
   the proper order */
void 
moto_init(size_t shared_heap,char* xpath) {
   shared_init(shared_heap);
   stacktrace_init();
   mxdl_init();
   if(xpath !=NULL)
      mxdl_prependMXPath(xpath);
   mman_init();
   err_init();   
   log_init(LOG_LEVEL);
   moto_errInit();
   motoi_init();
}

void 
moto_cleanup() {
   log_debug(__FILE__, ">>> moto_cleanup\n");
   log_free();
   err_free();
   motopp_freeYYStack();
   mman_cleanup();
   mxdl_cleanup();
   stacktrace_cleanup();
   shared_cleanup();
   log_debug(__FILE__, "<<< moto_cleanup\n");
}

void 
motopp_start(char *filename, int flags) {	
   MotoPP *ppenv;
   
   log_debug(__FILE__, ">>> motopp_start\n");
   
   ppenv = motopp_createEnv(filename, flags);
   motopp_setEnv(ppenv);
	
   if (motopp_prepareMainBuffer(ppenv, filename) != 0) {
      motopp_inputFileNotFound(filename);
   }

   log_debug(__FILE__, "<<< motopp_start\n");
}

void 
motopp(MotoPP *ppenv) {
   log_debug(__FILE__, ">>> motopp\n");
   ppparse((void *)ppenv);            
   log_debug(__FILE__, "<<< motopp\n");
}

void 
motopp_out(int code) {
   MotoPP *ppenv = motopp_getEnv();
   log_debug(__FILE__, ">>> motopp_out\n");
   if (code == MOTO_FAIL || buf_size(ppenv->err) > 0) {
      printf("%s", buf_data(ppenv->err));
   }
   else {
      printf("%s", buf_data(ppenv->out));
   }
   log_debug(__FILE__, "<<< motopp_out\n");
}

void 
motopp_done(int code) {
   MotoPP *ppenv = motopp_getEnv();
	
   log_debug(__FILE__, ">>> motopp_done\n");
   
   if (ppenv->outfn != NULL) {
      (*ppenv->outfn)(code);   
   }
   if (code == MOTO_FAIL) {
      while (stack_size(ppenv->frames) > 0) {
         /* free frames */
         MotoPPFrame *frame = stack_pop(ppenv->frames);
         if (frame->yybuf != NULL) {
            motopp_freeYYBuffer(frame->yybuf);
         }
         if (frame->yybufmem != NULL) {
            free(frame->yybufmem);
         }
         buf_free(frame->out);
         stack_free(frame->opstack);
         free(frame);
      }
   
      /* free macro builder macros */
      if (ppenv->mb != NULL) {
         Enumeration *e;
         SymbolTable *macros = ppenv->mb->macros;         
         e = stab_getKeys(macros);            
         while (enum_hasNext(e)) {
            char *name = (char *)enum_next(e);
            MotoMacro *m = (MotoMacro *)stab_get(macros, name);
            motopp_freeMacro( m);
         }
         /* don't free macro symbol table...
            it's done in motopp_freeMacroBuilder() */ 
         /* stab_free(macros); */
         enum_free(e);
         motopp_freeMacroBuilder(ppenv->mb);
      }

      /* free macros */
      while (stack_size(ppenv->macrostack) > 0) {
         Enumeration *e;
         SymbolTable *macros = stack_pop(ppenv->macrostack);         
         e = stab_getKeys(macros);            
         while (enum_hasNext(e)) {
            char *name = (char *)enum_next(e);
            MotoMacro *m = (MotoMacro *)stab_get(macros, name);
            motopp_freeMacro( m);
         }
         stab_free(macros);
         enum_free(e);
      }
   }
   motopp_freeEnv(ppenv);
   
   log_debug(__FILE__, "<<< motopp_done\n");
}
      
void 
moto_out(int code) {
   MotoEnv *env = moto_getEnv();
   if (code != MOTO_OK || buf_size(env->err) > 0) {
      printf("%s", buf_data(env->err));
   }
   else {
      moto_freeFrame(env);
      if (env->mode == INTERPRETER_MODE) {

         int totalBytesWritten = 0;
         char* data = buf_data(env->out);            
			int bytesToWrite = buf_size(env->out);
			  
			while (totalBytesWritten < bytesToWrite) {
				int bytesWritten = 0;
				bytesWritten = 
				   fwrite(data + totalBytesWritten, 1, bytesToWrite-totalBytesWritten, stdout);
				totalBytesWritten += bytesWritten;
			}
      
         // printf("%s", buf_data(env->out));
      }
      else if (env->mode == COMPILER_MODE) {
         StringBuffer *out = buf_createDefault();
         moto_emitCHeader(env, out);
         buf_puts(out,buf_data(env->out));
         moto_emitCFooter(env, out);
         printf("%s",buf_data(out));
         buf_free(out);
      }
   }
}

void 
moto_done(int code) {
   MotoEnv *env = moto_getEnv();
   if (env->outfn == NULL) {
      moto_out(code);
   }
   else {
      (*env->outfn)(code);
   }
   moto_freeEnv(env);   
}
      
int 
moto_files(int argc, char **argv, int flags,size_t shared_heap,char* mxpath) {
   int result = 0;
   int i = 0;

   moto_init(shared_heap,mxpath);
   
   for (i = 0; i < argc; i++) {
      result += moto_run(argv[i], flags);   
   }
   moto_cleanup();
   /*
   if (pid > 0) {
      waitpid(pid, NULL, NULL);
      moto_cleanup();
   }
   */
   return result;
}

static void 
moto_pipepp(int code) {
   MotoPP *ppenv = motopp_getEnv();
   if (code == MOTO_FAIL || buf_size(ppenv->err) > 0) {
      printf("%s\n", buf_data(ppenv->err));
   }
}

extern int motoyyparse (void *);

int 
moto_run(char *filename, int flags) {
   MotoPP *ppenv = NULL;
   int result = MOTO_OK;
   
   rtime_init();

   if (flags & MOTOPP_FLAG) {
      if ((flags & (MOTOV_FLAG | MOTOI_FLAG | MOTOC_FLAG | MOTOD_FLAG)) == 0) {
         /* run preprocessor only */
         motopp_start(filename, flags);
         ppenv = motopp_getEnv();
         ppenv->outfn = motopp_out;
         TRY {
            motopp(ppenv);
            motopp_done(MOTO_OK);
            result = MOTO_OK;
         }
         CATCH_ALL {
            motopp_done(MOTO_FAIL);
            result = MOTO_FAIL;
         }
         END_TRY
      }
      else {
         MotoEnv *env = NULL;
         MotoTree *tree;
         char *text = NULL;
         int size;
         int i;
         
         /* preprocess */
         TRY {
            motopp_start(filename, flags);
            ppenv = motopp_getEnv();
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
            TRY {               
               /* bootstrap env */
               env = moto_createEnv(flags,filename);   
               moto_setEnv(env);
               moto_createFrame(env, MAIN_FRAME);	
               /* env->frame->filename = filename;	 FIXME: this isn't necessary */
               tree = env->tree;

               /* check syntax */
               moto_prepareMainBuffer(env, text);
               motoyyparse((void *)env);
               if (env->errflag || buf_size(env->err) > 0) {
                  THROW_D("MotoException");
               }
           
               /* try "built-in" dynamic loads */
               if (flags & DEF_DL_FLAG) {
                  if (moto_use(env, "moto") == -1) {
                     THROW("DLException","Could not locate default extensions in '%s'",mxdl_getMXPath());
                  }
               }

               /* Get function definitions and load libraries */

               if (flags & MOTOV_FLAG) {
                  env->mode = VERIFIER_MODE;
                  env->e = motod;
               }
               env->outfn = NULL;
               size = vec_size(tree->cells);
               for (i = 0; i < size; i++) {
                  (*env->e)(vec_get(tree->cells, i));
               }
 
               /* verify semantics */

               if (flags & MOTOV_FLAG) {
                  env->mode = VERIFIER_MODE;
                  env->e = motov;
               }               
               env->outfn = NULL;
               size = vec_size(tree->cells);
               for (i = 0; i < size; i++) {
                  (*env->e)(vec_get(tree->cells, i)); 
               }
               moto_freeFrame(env);
               if (env->errflag || buf_size(env->err) > 0) {
                  THROW_D("MotoException");
               }
               
               if (flags & MOTOD_FLAG) {
                  /* print out dependency list */
                  Enumeration *e = (sset_elements(env->uses));
                  while (enum_hasNext(e)) {
                     char *used = (char *)enum_next(e);
                     printf("%s\n", used);
                  }
                  printf("\n");
                  enum_free(e);
               }
               else {
                  /* interpret/compile */
                  moto_clearEnv(env);
                  moto_createFrame(env, MAIN_FRAME);	
                  if (flags & MOTOC_FLAG) {
                     env->mode = COMPILER_MODE;
                     env->e = motoc;
                  }
                  else if (flags & MOTOI_FLAG) {
                     env->mode = INTERPRETER_MODE;
                     env->e = motoi;
                  }
                  env->outfn = moto_out;
                  size = vec_size(tree->cells);
                  for (i = 0; i < size; i++) {
                     (*env->e)(vec_get(tree->cells, i)); 
                  }
                  moto_done(MOTO_OK);
                  rtime_cleanup();
               }
           
            }
            CATCH("MotoException"){
               env->outfn = moto_out;
               moto_done(MOTO_FAIL);
               rtime_cleanup(); 
               result = MOTO_FAIL;
            }
            CATCH_ALL{
         		printf("Uncaught %s\n   message:%s\n\n%s",
         			excp_getCurrentException()->t,
         			excp_getCurrentException()->msg,
         			excp_getCurrentException()->stacktrace
         		);
               env->outfn = moto_out;
               moto_done(MOTO_FAIL);
               rtime_cleanup();
               result = MOTO_FAIL;
            }
            END_TRY
            moto_freeYYBuffer();            
            if (text) {
               free(text);
            }
         }
      }
   }

   return result;
}

/*=============================================================================
// end of file: $RCSfile: motofn.c,v $
==============================================================================*/

