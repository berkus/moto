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

   $RCSfile: pp.c,v $   
   $Revision: 1.21 $
   $Author: dhakim $
   $Date: 2003/06/09 18:35:49 $
 
==============================================================================*/
#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <limits.h>

#include "memsw.h"

#include "exception.h"
#include "excpfn.h"
#include "enumeration.h"
#include "log.h"
#include "sysmalloc.h"

#include "motocodes.h"
#include "motopplex.h"
#include "pp.h"
#include "pputil.h"

#define M_SIZE 1025

extern int pplineno;

char* motopp_getRealPath(char* filename) {
   int path_max;
   char *path;

#ifdef PATH_MAX
   path_max = PATH_MAX;
#else
   path_max = pathconf(path, _PC_PATH_MAX);
   if (path_max <= 0) {
      path_max = DEF_PATH_MAX;
   }
#endif

   path = realpath(filename, (char*)emalloc(path_max));

   return path;
}

char* motopp_getRelativeRoot(char* filename) {
   char* path = motopp_getRealPath(filename);

   if (path == NULL) return NULL;
   if(rindex(path,'/') != NULL)
      *(rindex(path,'/')) = '\0';

   return path;
}

MotoPP *motopp_createEnv(char *filename, int flags) {
   MotoPP *ppenv;

   ppenv = emalloc(sizeof(MotoPP));
  
   ppenv->flags = flags;
   ppenv->mpool = mpool_create(10);
   ppenv->sysptrs = hset_create(M_SIZE, NULL, NULL, 0);
   ppenv->ptrs = hset_create(M_SIZE, NULL, NULL, 0);
   ppenv->out = buf_createDefault();
   ppenv->err = buf_createDefault();
   ppenv->argbuf = buf_createDefault();
   ppenv->dirstack = istack_createDefault();
   ppenv->macrostack = stack_createDefault();
   ppenv->vallist = hset_create(M_SIZE, NULL, NULL, 0);
   ppenv->frames = stack_createDefault();
   ppenv->frame = motopp_createFrame(ppenv, MAIN_FRAME,filename);
   ppenv->mb = NULL;

   return ppenv;
}

void motopp_freeEnv(MotoPP *ppenv) {
   Enumeration *e;
   
   log_debug(__FILE__, ">>> motopp_freeEnv\n");
  
   buf_free(ppenv->out);
   buf_free(ppenv->err);
   buf_free(ppenv->argbuf);
   istack_free(ppenv->dirstack);
   stack_free(ppenv->frames);   

   /* free vals */
   e = hset_elements(ppenv->vallist);
   while (enum_hasNext(e)) {
      MotoPPVal *val = enum_next(e);
      if (shared_check(val->sval)) {
         free(val->sval);
         hset_remove(ppenv->ptrs, val->sval);
      }
      free(val);
   }
   enum_free(e);
   hset_free(ppenv->vallist);
   
   /* free macros */
   while (stack_size(ppenv->macrostack) > 0) {
      SymbolTable *macros = stack_pop(ppenv->macrostack);
      e = stab_getKeys(macros);
      while (enum_hasNext(e)) {
         char *name = (char *)enum_next(e);
         MotoMacro *m = (MotoMacro *)stab_get(macros, name);
         motopp_freeMacro(m);
      }
      stab_free(macros);
      enum_free(e);
   }
   
   /* free all remaining sys pointers */
   e = hset_elements(ppenv->sysptrs);
   while (enum_hasNext(e)) {
      void *ptr = enum_next(e);
      if (ptr) {
         sys_free(ptr);         
      }
   }
   enum_free(e);
      
   /* free all remaining pointers */
   e = hset_elements(ppenv->ptrs);
   while (enum_hasNext(e)) {
      void *ptr = enum_next(e);
      if (shared_check(ptr)) {
         free(ptr);         
      }
   }
   enum_free(e);
      
   /* free remaining pooled memory */
   mpool_free(ppenv->mpool);

   /* free remainder of env struct */
   hset_free(ppenv->sysptrs);
   hset_free(ppenv->ptrs);
   stack_free(ppenv->macrostack);   
   
   free(ppenv);

   log_debug(__FILE__, "<<< motopp_freeEnv\n");

}

MotoPPFrame *motopp_createFrame(MotoPP *ppenv, int type,char* filename) {
   MotoPPFrame *frame;
	
   frame = (MotoPPFrame *)emalloc(sizeof(MotoPPFrame));

   frame->relative_root = motopp_getRelativeRoot(filename);
   frame->filename      = motopp_getRealPath(filename); 

   /*
   if (stack_size(ppenv->frames) > 0) {
      MotoPPFrame *pframe  = stack_peek(ppenv->frames);

      frame->relative_root = pframe->relative_root;
      frame->filename      = pframe->filename;
   } else {
      frame->relative_root = ppenv->relative_root;
      frame->filename      = ppenv->filename;
   } 
   frame->relative_root = ppenv->relative_root;
   frame->filename 	= ppenv->filename;
   */

   frame->type = type;
   frame->out = buf_createDefault();
   frame->macro = NULL;
   frame->opstack = stack_createDefault();

   frame->yybuf = NULL;
   frame->yybufmem = NULL;
   
   switch (frame->type) {
      case MAIN_FRAME:
      case MACRO_FRAME:
         frame->macros = stab_createDefault();
         stack_push(ppenv->macrostack, frame->macros);
         break;
      default:
         frame->macros = stack_peek(ppenv->macrostack);
         break;
   }

   stack_push(ppenv->frames, frame);
   ppenv->frame = frame;
   
   return frame;
}

void motopp_freeFrame(MotoPP *ppenv) {
   MotoPPFrame *frame;
   Enumeration *e;
   
   if (stack_size(ppenv->frames) <= 1) {
      frame = stack_pop(ppenv->frames);
      buf_cat(ppenv->out, frame->out);
      ppenv->frame = NULL;
   }
   else {
      MotoPPVal *val;
      MotoPPFrame *pframe;
      SymbolTable *macros;
      MotoMacro *m;
      char *catstr;
      int catstrlen;
      frame = stack_pop(ppenv->frames);
      pframe = stack_peek(ppenv->frames);
      
      switch (frame->type) {
         case MACRO_FRAME:
            m = frame->macro;
            val = motopp_createVal(ppenv);
            if (ppenv->flags & MACRO_DEBUG_FLAG) {
               buf_printf(frame->out, LINE_FMT_2, pframe->filename, pframe->lineno);
            }
            val->sval = buf_toString(frame->out);
            stack_push(pframe->opstack, val);
                                                                           
            macros = stack_pop(ppenv->macrostack);
            e = stab_getKeys(macros);
            while (enum_hasNext(e)) {
               char *name = (char *)enum_next(e);
               MotoMacro *m = (MotoMacro *)stab_get(macros, name);
               motopp_freeMacro(m);
            }
            stab_free(macros);
            enum_free(e);
            break;
         case INCLUDE_FRAME:
            catstr = buf_data(frame->out);
            catstrlen = strlen(catstr);            
            if (catstr[catstrlen - 1] == '\n') {
               catstr[catstrlen - 1] = '\0';
            }
            buf_puts(pframe->out, catstr);
            if (ppenv->flags & MACRO_DEBUG_FLAG) {
               buf_printf(pframe->out, LINE_FMT_2, pframe->filename, pframe->lineno);
            }
            break;
         default:
            buf_cat(pframe->out, frame->out);
            break;
      }
      
      /* reset pplineno */
      pplineno = pframe->lineno;

      /* reset frame */
      ppenv->frame = pframe;   
   } 

   if (frame->yybuf != NULL) {
      if (shared_check(frame->yybuf)) {
         motopp_freeYYBuffer(frame->yybuf);
      }
   }
   if (frame->yybufmem != NULL) {
      free(frame->yybufmem);
   }
   buf_free(frame->out);
   stack_free(frame->opstack);
   free(frame->filename);
   free(frame->relative_root);
   free(frame);
   
}

MotoPPVal *motopp_createVal(MotoPP *ppenv) {
   MotoPPVal *val;
   val = ecalloc(1,sizeof(MotoPPVal));
   hset_add(ppenv->vallist, val);
   return val;
}

void motopp_freeMacro(MotoMacro *m) {
   if (m != NULL) {
      free(m->name);
      if (m->argc > 0) {
         int i;
         for (i = 0; i < m->argc; i++) {
            free(m->argv[i]);
         }
         free(m->argv);
      }
      if(m->relative_root != NULL)
         free(m->relative_root);
      if(m->filename != NULL)
         free(m->filename);
      free(m->rtext);
      free(m);
   }
   
}

MotoMacro *motopp_getMacro(MotoPP *ppenv, char *name) {
   MotoMacro *m = NULL;
   int size = stack_size(ppenv->macrostack);
   int i;
      
   for (i = 1; i <= size; i++) {
      SymbolTable *macros = stack_peekAt(ppenv->macrostack, i);
      if ((m = stab_get(macros, name)) != NULL) {
         break;
      }
   }
   
   return m;
}

MotoMacroBuilder *motopp_createMacroBuilder(char *filename) {
   MotoMacroBuilder *mb = emalloc(sizeof(MotoMacroBuilder));   
   
   mb->relative_root 	= motopp_getRelativeRoot(filename);
   mb->filename 	= motopp_getRealPath(filename);

   mb->startline = 1;
   mb->buf = buf_createDefault();
   mb->macros = stab_createDefault();
   mb->args = vec_create(5, (int(*)(const void *, const void *))strcmp);
   
   return mb;
}

void motopp_freeMacroBuilder(MotoMacroBuilder *mb) {

   free(mb->relative_root);
   free(mb->filename);

   buf_free(mb->buf);
   stab_free(mb->macros);
   vec_free(mb->args);
   free(mb);
}

/*=============================================================================
// end of file: $RCSfile: pp.c,v $
==============================================================================*/

