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

   $RCSfile: motoppi.c,v $   
   $Revision: 1.15 $
   $Author: dhakim $
   $Date: 2003/06/09 18:35:49 $
 
==============================================================================*/
#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memsw.h"

#include "exception.h"
#include "excpfn.h"
#include "enumeration.h"
#include "log.h"
#include "stringbuffer.h"

#include "motocodes.h"
#include "motoerr.h"
#include "motohook.h"
#include "motopp.tab.h"
#include "motoppi.h"
#include "motopplex.h"
#include "pp.h"
#include "pputil.h"

extern int pplineno;
extern int ryylineno;
extern int ppparse(void *param);
extern struct yy_buffer_state *pp_switch_to_buffer(struct yy_buffer_state *buf);
extern int ryyparse();

static void motoppi_defineArgMacro(const PPUnionCell *p);
static void motoppi_defineNoArgMacro(const PPUnionCell *p);
static void motoppi_defined(const PPUnionCell *p);
static void motoppi_elseif(const PPUnionCell *p);
static void motoppi_elseiflist(const PPUnionCell *p);
static void motoppi_execArgMacro(const PPUnionCell *p);
static void motoppi_execNoArgMacro(const PPUnionCell *p);
static void motoppi_if(const PPUnionCell *p);
static void motoppi_ifdef(const PPUnionCell *p);
static void motoppi_ifndef(const PPUnionCell *p);
static void motoppi_include(const PPUnionCell *p);
static void motoppi_list(const PPUnionCell *p);
static void motoppi_logical(const PPUnionCell *p);
static void motoppi_macro(const PPUnionCell *p);
static void motoppi_not(const PPUnionCell *p);
static void motoppi_print(const PPUnionCell *p);
static void motoppi_readdefs(const PPUnionCell *p);
static void motoppi_rel(const PPUnionCell *p);
static void motoppi_undefine(const PPUnionCell *p);
static void motoppi_value(const PPUnionCell *p);

/*-----------------------------------------------------------------------------
 * interpreter
 *---------------------------------------------------------------------------*/

void motoppi(const PPUnionCell *p) {
   MotoPP *ppenv = motopp_getEnv();
   switch (p->type) {
      case VALUE_TYPE:
         ppenv->errlineno = p->valuecell.lineno;
         switch (p->valuecell.kind) {
            case INT32_VALUE:
            case STRING_VALUE:
               motoppi_value(p);
               break;
            default:
               THROW_D("MotoCellTypeException");
               break;
         }
         break;
      case OP_TYPE:
         ppenv->errlineno = p->opcell.lineno;
         switch (ppuc_opcode(p)) {
            case AND:               
					motoppi_logical(p);
					break;
            case DEFARG:               
					motoppi_defineArgMacro(p);
					break;
            case DEFNOARG:               
					motoppi_defineNoArgMacro(p);
					break;
            case DEFINED:               
					motoppi_defined(p);
					break;
            case ELSEIF:               
               motoppi_elseif(p);
               break;
            case ELSEIFLIST:               
               motoppi_elseiflist(p);
               break;
            case IF:
               motoppi_if(p);
               break;
            case IFDEF:
               motoppi_ifdef(p);
               break;
            case IFNDEF:
               motoppi_ifndef(p);
               break;
            case INCLUDE:
               motoppi_include(p);
               break;
            case LIST:
               motoppi_list(p);
               break;
            case MACROCALL:
               motoppi_macro(p);
               break;
            case NOT:               
					motoppi_not(p);
					break;
            case OR:               
					motoppi_logical(p);
					break;
            case PRINT:               
					motoppi_print(p);
					break;
            case READDEFS:               
					motoppi_readdefs(p);
					break;
            case REL_EQ:
            case REL_NE:
               motoppi_rel(p);
               break;
            case UNDEF:               
					motoppi_undefine(p);
					break;
            default:
               THROW_D("MotoOpcodeException");
               break;
         }
   }
}

/*-----------------------------------------------------------------------------
 * ops
 *---------------------------------------------------------------------------*/

static void motoppi_defineArgMacro(const PPUnionCell *p) {
   MotoPP *ppenv = motopp_getEnv();
   MotoMacro *m;
   char *mname;
   int argc;
   int osize;
   
   osize = ppopstack_size();
   motoppi(ppuc_operand(p, 1));
   argc = ppopstack_size() - osize;		

   /* set the macro name */
   mname = emalloc(strlen(ppuc_str(p, 0)) + 32);
   sprintf(mname, "%s(%d)", ppuc_str(p, 0), argc);
   
   /* free any previous macro with this same name in this frame */
   if ((m = stab_get(ppenv->frame->macros, mname)) != NULL) {
      stab_remove(ppenv->frame->macros, mname); /* NEED to do this because the key is stored with the macro
                                                   and will be freed when the macro gets freed */
      motopp_freeMacro(m);
   }
   m = (MotoMacro *)emalloc(sizeof(MotoMacro));

   m->name = mname;
   m->startline = pplineno;
   m->relative_root = estrdup(ppenv->frame->relative_root);
   m->filename = estrdup(ppenv->frame->filename);

   m->infile = 1;
   m->argc = argc;
   m->arguments = NULL;
   stab_put(ppenv->frame->macros, mname, m);   	
   
   /* set macro arguments */
   if (argc > 0) {
      int i;
      m->argv = emalloc(argc * sizeof(char *));
      for (i = argc - 1; i >= 0; i--) {
         MotoPPVal *val = ppopstack_pop();
         char *arg = val->sval;
         m->argv[i] = estrdup(arg);
      }
   }

   /* set macro replacement text */
   if (ppuc_opcount(p) == 2) {
      m->rtext = estrdup(" ");
   }
   else {
      char *s;
      int len;
      int i = 0;

      /* trim whitespace from front of rtext */
      s = ppuc_str(p, 2);
      len = strlen(s);
      while (i < len) {
         char c = *s;
         if (c != ' ' && c != '\t') {
            break;
         }
         s += 1;
         i++;
      }	
      m->rtext = estrdup(s);
   }

   buf_printf(ppenv->frame->out, "$*#define %s*$", mname);

}

static void motoppi_defineNoArgMacro(const PPUnionCell *p) {
   MotoPP *ppenv = motopp_getEnv();
   MotoMacro *m;
   char *mname;
   
   mname = estrdup(ppuc_str(p, 0));
   
   /* free any previous macro with this same name in this frame */
   if ((m = stab_get(ppenv->frame->macros, mname)) != NULL) {
      stab_remove(ppenv->frame->macros, mname); /* NEED to do this because the key is stored with the macro
                                                   and will be freed when the macro gets freed */
      motopp_freeMacro(m);
   }
   m = (MotoMacro *)emalloc(sizeof(MotoMacro));
   m->name = mname;
   m->argc = 0;
   m->argv = NULL;
   m->startline = pplineno;
   m->relative_root = estrdup(ppenv->frame->relative_root);
   m->filename = estrdup(ppenv->frame->filename);
   m->infile = 1;
   m->arguments = NULL;
   stab_put(ppenv->frame->macros, mname, m);

   if (ppuc_opcount(p) == 1) {
      m->rtext = estrdup(" ");
   }
   else {
      char *s;
      int len;
      int i = 0;

      /* trim whitespace from front of rtext */
      s = ppuc_str(p, 1);
      len = strlen(s);
      while (i < len) {
         char c = *s;
         if (c != ' ' && c != '\t') {
            break;
         }
         s += 1;
         i++;
      }	
      m->rtext = estrdup(s);
   }

   buf_printf(ppenv->frame->out, "$*#define %s*$", mname);
}

static void motoppi_defined(const PPUnionCell *p) {
   MotoPP *ppenv = motopp_getEnv();
   MotoPPVal *val;

   val = motopp_createVal(ppenv);
   if (motopp_getMacro(ppenv, ppuc_str(p, 0)) != NULL) {
      val->sval = "1";
   }
   else {
      val->sval = "0";
   }
   ppopstack_push(val);
}

static void motoppi_elseif(const PPUnionCell *p) {
   MotoPPVal *val;
   
   motoppi(ppuc_operand(p, 0));
   val = ppopstack_pop();
   if (atoi(val->sval)) {
      motoppi(ppuc_operand(p, 1));
   }
   ppopstack_push(val);

}

void motoppi_elseiflist(const PPUnionCell *p) {
   MotoPPVal *val;
   
   motoppi(ppuc_operand(p, 0));
   val = ppopstack_peek();
   if (!(atoi(val->sval))) {
      motoppi(ppuc_operand(p, 1));
   }

}

static void motoppi_execArgMacro(const PPUnionCell *p) {
   MotoPP *ppenv = motopp_getEnv();
   MotoMacro *m = NULL;
   char *name;
   char *mname;
   int argc;
   int size;
   int i;
   
   size = ppopstack_size();
   motoppi(ppuc_operand(p, 1));
   argc = ppopstack_size() - size;		
   
   name = ppuc_str(p, 0);
   mname = mman_malloc(ppenv->mpool, strlen(name) + 32);
   sprintf(mname, "%s(%d)", name, argc);   
   
   m = motopp_getMacro(ppenv, mname);
   if (m == NULL) {
      /* no such macro...
         replace with literal text of code that looked like macro call */
      MotoPPVal *val;
      StringBuffer *buf = buf_createDefault();
      char **args = emalloc(argc * sizeof(char *));
      buf_putc(buf, '$');
      buf_puts(buf, name);
      buf_putc(buf, '(');
      for (i = argc - 1; i >= 0; i--) {
         MotoPPVal *val = ppopstack_pop();
         args[i] = val->sval;
      }
      for (i = 0; i < argc; i++) {
         if (i > 0) {
            buf_putc(buf, ',');
         }
         buf_puts(buf, args[i]);
      }
      buf_putc(buf, ')');
      val = motopp_createVal(ppenv);
      val->sval = buf_toString(buf);
      ppopstack_push(val);
      buf_free(buf);
      free(args);
   }
   else {
      /* macro found....call it */
      char **args;
      MotoPPFrame *pframe = ppenv->frame;
      /* flip order of args on stack */
      args = emalloc(argc * sizeof(char *));
      for (i = argc - 1; i >= 0; i--) {
      /* for (i = 0; i < argc; i++) { */
         MotoPPVal *val = ppopstack_pop();
         args[i] = val->sval;
      }
      motopp_prepareFrameForParenthood(ppenv);
      motopp_createFrame(ppenv, MACRO_FRAME, m->filename);

      /* add arguments as macros in this new frame */
      /* m->argv[i] has macro name */
      /* args[i] has macro replacement text */

      for (i = 0; i < argc; i++) {
         MotoMacro *argm = (MotoMacro *)emalloc(sizeof(MotoMacro));
         argm->infile = 0;
         argm->name = estrdup(m->argv[i]);
         argm->argc = 0;
         argm->argv = NULL;

         argm->relative_root = estrdup(m->relative_root);
         argm->filename      = estrdup(m->filename);

         argm->startline = m->startline;
         argm->rtext = estrdup(args[i]);
         
         /* Set a pointer to the macroset this argument was defined in so the stack can 
         be popped down to here at expansion time */
         
         argm->arguments = ppenv->frame->macros; 
         stab_put(ppenv->frame->macros, argm->name, argm);
      }
      /* free array allocated to flip args */
      free(args);
      motopp_prepareFrameBuffer(ppenv, m->rtext);
      motopp_prepareMacroFrameForExec(ppenv, m);
      if (ppenv->flags & MACRO_DEBUG_FLAG) {
         buf_printf(ppenv->frame->out, LINE_FMT_1, pframe->filename, pframe->lineno, m->filename, m->startline, m->name);      
      }
      ppparse((void *)ppenv);
      
      motopp_freeFrame(ppenv);
		pp_switch_to_buffer(ppenv->frame->yybuf);
   }
}

static void motoppi_execNoArgMacro(const PPUnionCell *p) {
   MotoPP *ppenv = motopp_getEnv();
   MotoMacro *m = NULL;
   char *mname = ppuc_str(p, 0);
   
   m = motopp_getMacro(ppenv, mname);
   
   if (m == NULL) {
      MotoPPVal *val;
      StringBuffer *buf = buf_createDefault();
      buf_putc(buf, '$');
      buf_puts(buf, mname);
      val = motopp_createVal(ppenv);
      val->sval = buf_toString(buf);
      ppopstack_push(val);
      buf_free(buf);
   }
   else {
   	
   	Stack* parentMacros = NULL;
   	
      MotoPPFrame *pframe = ppenv->frame;
      motopp_prepareFrameForParenthood(ppenv);
 
       /* If this isn't really a macro but an argument to a macro being expanded
      	than we want to make sure it or its sibling arguments aren't available 
      	for substitution while its being expanded ... FIXME: why on earth the
      	macrostack is seperate from the frame stack is really beyond me :P */
      	
      if(m->arguments) {
      	SymbolTable* macros = NULL;
      	parentMacros = stack_createDefault();
      	do {
      		macros = (SymbolTable*)stack_pop(ppenv->macrostack); 
      		stack_push(parentMacros,macros);
      	} while (macros != m->arguments);
      }
          
      motopp_createFrame(ppenv, MACRO_FRAME, m->filename);
      motopp_prepareFrameBuffer(ppenv, m->rtext);
      motopp_prepareMacroFrameForExec(ppenv, m);      
      if (ppenv->flags & MACRO_DEBUG_FLAG) {
         buf_printf(ppenv->frame->out, LINE_FMT_1, pframe->filename, pframe->lineno, m->filename, m->startline, m->name);      
      }
      
      ppparse((void *)ppenv);
      
      motopp_freeFrame(ppenv);  
		pp_switch_to_buffer(ppenv->frame->yybuf);
		
		if(m->arguments) {
			while(stack_size(parentMacros) > 0)
				stack_push(ppenv->macrostack,stack_pop(parentMacros));
			stack_free(parentMacros);
		}
   }
}

void motoppi_if(const PPUnionCell *p) {
	MotoPPVal *val;

   motoppi(ppuc_operand(p, 0));
   val = ppopstack_pop();
   if (atoi(val->sval)) {
      motoppi(ppuc_operand(p, 1));   
   } 
   else if (ppuc_opcount(p) >= 3) {
      int count = ppuc_opcount(p); 
      if (count == 3) {
         motoppi(ppuc_operand(p, 2));
      } 
      else {
         motoppi(ppuc_operand(p, 2));
         val = ppopstack_pop();
         if (!(atoi(val->sval))) {
            motoppi(ppuc_operand(p, 3));   
         }
      }
   }
}

void motoppi_ifdef(const PPUnionCell *p) {
   MotoPP *ppenv = motopp_getEnv();
	MotoPPVal *val;

   if (motopp_getMacro(ppenv, ppuc_str(p, 0)) != NULL) {
      motoppi(ppuc_operand(p, 1));   
   } 
   else if (ppuc_opcount(p) >= 3) {
      int count = ppuc_opcount(p); 
      if (count == 3) {
         motoppi(ppuc_operand(p, 2));
      } 
      else {
         motoppi(ppuc_operand(p, 2));
         val = ppopstack_pop();
         if (atoi(val->sval)) {
            motoppi(ppuc_operand(p, 3));   
         }
      }
   }
}

void motoppi_ifndef(const PPUnionCell *p) {
   MotoPP *ppenv = motopp_getEnv();
	MotoPPVal *val;

   if (motopp_getMacro(ppenv, ppuc_str(p, 0)) == NULL) {
      motoppi(ppuc_operand(p, 1));   
   } 
   else if (ppuc_opcount(p) >= 3) {
      int count = ppuc_opcount(p); 
      if (count == 3) {
         motoppi(ppuc_operand(p, 2));
      } 
      else {
         motoppi(ppuc_operand(p, 2));
         val = ppopstack_pop();
         if (!(atoi(val->sval))) {
            motoppi(ppuc_operand(p, 3));   
         }
      }
   }
}

static void motoppi_include(const PPUnionCell *p) {	
   MotoPP *ppenv = motopp_getEnv();
   int found;	
   char *origFilename = ppuc_str(p, 0),*filename=NULL;

   if(origFilename[0] != '/'){
      StringBuffer* fullyQualifiedPath = buf_createDefault();

      buf_puts(fullyQualifiedPath, ppenv->frame->relative_root);
      buf_putc(fullyQualifiedPath, '/');
      buf_puts(fullyQualifiedPath, origFilename);

      filename = motopp_getRealPath(buf_data(fullyQualifiedPath));

      buf_free(fullyQualifiedPath);
   } else {
      filename = motopp_getRealPath(origFilename);
   }

   if (filename == NULL) {
      motopp_includeFileNotFound(ppenv, pplineno, origFilename);
   }

   if (!motopp_isInIncludeChain(ppenv, filename)) {
      char *lname = ppenv->frame->filename;
      int lline = pplineno;
      if (ppenv->frame->type == MACRO_FRAME) {
         lline += ppenv->frame->macro->startline - 1;
      }

      motopp_prepareFrameForParenthood(ppenv);
      motopp_createFrame(ppenv, INCLUDE_FRAME, filename);

      found = motopp_prepareFileFrameBuffer(ppenv, filename);

      if (found != 0) {
         motopp_includeFileNotFound(ppenv, pplineno, filename);
      }
      motopp_prepareFrameForExec(ppenv, filename);

      if (ppenv->flags & MACRO_DEBUG_FLAG) {
         buf_printf(ppenv->frame->out, LINE_FMT_1, lname, lline, filename, 1, "#");
      }
      ppparse((void *)ppenv);
      
      motopp_freeFrame(ppenv);
		pp_switch_to_buffer(ppenv->frame->yybuf);
   }
  
   if(filename!=NULL)
      free(filename);
}

static void motoppi_list(const PPUnionCell *p) {
   motoppi(ppuc_operand(p, 0));
   motoppi(ppuc_operand(p, 1));
}

static void motoppi_logical(const PPUnionCell *p) {
   MotoPP *ppenv = motopp_getEnv();
   MotoPPVal *op1;
   MotoPPVal *op2;
   MotoPPVal *r;
   int iop1;
   int iop2;
   
   motoppi(ppuc_operand(p, 0));   
   motoppi(ppuc_operand(p, 1));   

   op2 = ppopstack_pop();
   op1 = ppopstack_pop();

   iop1 = atoi(op1->sval);
   iop2 = atoi(op2->sval);

   r = motopp_createVal(ppenv);

   switch (ppuc_opcode(p)) {
      case AND:
         if (iop1 && iop2) {
            r->sval = "1";
         }
         else {
            r->sval = "0";
         }
         break;
      case OR:
         if (iop1 || iop2) {
            r->sval = "1";
         }
         else {
            r->sval = "0";
         }
         break;
   }

   ppopstack_push(r);
}

static void motoppi_macro(const PPUnionCell *p) {      
   if (ppuc_opcount(p) == 1) {
      motoppi_execNoArgMacro(p);
   }
   else {
      motoppi_execArgMacro(p);
   }
}

static void motoppi_not(const PPUnionCell *p) {
   MotoPP *ppenv = motopp_getEnv();
   MotoPPVal *val;
   MotoPPVal *r;

   motoppi(ppuc_operand(p, 0));
   val = ppopstack_pop();
   
   r = motopp_createVal(ppenv);

   if (atoi(val->sval)) {
      r->sval = "0";
   }
   else {
      r->sval = "1";
   }
   
   ppopstack_push(r);

}

static void motoppi_print(const PPUnionCell *p) {
   MotoPP *ppenv = motopp_getEnv();
   MotoPPVal *val;

   motoppi(ppuc_operand(p, 0));
   val = ppopstack_pop();
   if (val != NULL) {
      buf_puts(ppenv->frame->out, val->sval);
   } 
   else {
      buf_puts(ppenv->frame->out, "(null)");
   }
   
}

static void motoppi_readdefs(const PPUnionCell *p) {
   MotoPP *ppenv = motopp_getEnv();
   MotoMacroBuilder *mb;
   Enumeration *e;
   int found;	
   char *lname = ppenv->frame->filename;
   int lline = pplineno;
   char *origFilename = ppuc_str(p, 0),*filename=NULL;

   if(origFilename[0] != '/'){
      StringBuffer* fullyQualifiedPath = buf_createDefault();

      buf_puts(fullyQualifiedPath, ppenv->frame->relative_root);
      buf_putc(fullyQualifiedPath, '/');
      buf_puts(fullyQualifiedPath, origFilename);

      filename = motopp_getRealPath(buf_data(fullyQualifiedPath));

      buf_free(fullyQualifiedPath);
   } else {
      filename = motopp_getRealPath(origFilename);
   }

   if (filename == NULL) {
      motopp_readdefsFileNotFound(ppenv, pplineno, origFilename);
   }

   if (ppenv->frame->type == MACRO_FRAME) {
      lline += ppenv->frame->macro->startline - 1;
   }
   
   motopp_createFrame(ppenv, READDEFS_FRAME, filename);
   found = motopp_prepareReaddefsFrameBuffer(ppenv, filename);
   if (found != 0) {
      motopp_readdefsFileNotFound(ppenv, pplineno, filename);
   }
   motopp_prepareFrameForExec(ppenv, filename);
   ppenv->mb = mb = motopp_createMacroBuilder(filename);
   buf_printf(ppenv->frame->out, LINE_FMT_1, lname, lline, filename, 1, "*");
   free(filename);

   ryylineno = 1; /* yuck */
   ryyparse();

   buf_printf(ppenv->frame->out, LINE_FMT_2, lname, lline);

   e = stab_getKeys(mb->macros);
   while (enum_hasNext(e)) {
      char *n = (char *)enum_next(e);
      MotoMacro *m = stab_get(mb->macros, n);

      if(stab_get(ppenv->frame->macros, n) != NULL){
         MotoMacro *tmp = stab_get(ppenv->frame->macros, n);
         stab_remove(ppenv->frame->macros, n); /* NEED to do this because the key is stored with the macro
                                                  and will be freed when the macro gets freed */
         motopp_freeMacro(tmp); 
      }
      
      m->arguments = NULL;
      stab_put(ppenv->frame->macros, n, m);
   }
   enum_free(e);
   motopp_freeMacroBuilder(mb);
   ppenv->mb = NULL;
}

static void motoppi_rel(const PPUnionCell *p) {   
   MotoPP *ppenv = motopp_getEnv();
   MotoPPVal *op1;
   MotoPPVal *op2;
   MotoPPVal *r;
   char *cop1;
   char *cop2;
   
   motoppi(ppuc_operand(p, 0));   
   motoppi(ppuc_operand(p, 1));   

   op2 = ppopstack_pop();
   op1 = ppopstack_pop();

   r = motopp_createVal(ppenv);

   cop1 = motopp_uncomment(op1->sval);
   cop2 = motopp_uncomment(op2->sval);

   switch (ppuc_opcode(p)) {
      case REL_EQ:
         if (strcmp(cop1, cop2) == 0) {
            r->sval = "1";
         }
         else {
            r->sval = "0";
         }
         break;
      case REL_NE:
         if (strcmp(cop1, cop2) == 0) {
            r->sval = "0";
         }
         else {
            r->sval = "1";
         }
         break;
   }

   free(cop1);
   free(cop2);

   ppopstack_push(r);
}

static void motoppi_undefine(const PPUnionCell *p) {
   MotoPP *ppenv = motopp_getEnv();
   MotoMacro *m = NULL;
	char *name;
   name = ppuc_str(p, 0);
	m = stab_remove(ppenv->frame->macros, name);
	if (m != NULL) {
	   motopp_freeMacro(m);
   }
}

static void motoppi_value(const PPUnionCell *p) {
   MotoPP *ppenv = motopp_getEnv();
   MotoPPVal *val;
      
   val = motopp_createVal(ppenv);
   val->sval = p->valuecell.value;
   
   ppopstack_push(val);
}

/*=============================================================================
// end of file: $RCSfile: motoppi.c,v $
==============================================================================*/

