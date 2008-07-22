%{

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

   $RCSfile: readdefs.y,v $   
   $Revision: 1.8 $
   $Author: dhakim $
   $Date: 2002/02/12 23:13:17 $
 
==============================================================================*/

#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "fileio.h"
#include "log.h"

#include "motopplex.h"
#include "pp.h"
#include "pputil.h"
#include "excpfn.h"

#include "memsw.h"

#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_ALL
#endif

extern int ryylineno;

extern MotoPP *ppenv;

extern struct yy_buffer_state *pp_switch_to_buffer(struct yy_buffer_state *buf);
extern int ryyerror(char *s);
extern int ryylex();

static void createMacro(char *name) {
   int i;
   int argc;
   MotoMacroBuilder *mb = ppenv->mb;
   MotoMacro *m;
   StringBuffer *n = buf_createDefault();
	
   argc = vec_size(mb->args);

   if (argc > 0) {
      buf_puts(n, name);
      buf_putc(n, '(');
      buf_puti(n, argc);
      buf_putc(n, ')');
   }
   else {
      buf_puts(n, name);
   }

   m = (MotoMacro *)emalloc(sizeof(MotoMacro));

   m->name = buf_toString(n);
   m->startline = mb->startline;
   m->relative_root = estrdup(mb->relative_root);
   m->filename = estrdup(mb->filename);
   m->infile = 0;
   m->argc = argc;

   if (argc > 0) {
      m->argv = emalloc(argc * sizeof(char *));
      for (i = argc - 1; i >= 0; i--) {
         char *arg = (char *)vec_get(mb->args, i);
         m->argv[i] = estrdup(arg);
      }	
   }

   /* 
   * process the macro text
   *    1. strip of all leading white space
   *    2. strip of all trailing white space 
   *    3. append ASCII character 4 as the macro end mark 
   */

   /*buf_trim(mb->buf);*/
   m->rtext = buf_toString(mb->buf);

   if(stab_get(mb->macros,m->name) != NULL){
      MotoMacro* tmp = stab_get(mb->macros,m->name);
      stab_remove(mb->macros, m->name); /* NEED to do this because the key is stored with the macro
                                           and will be freed when the macro gets freed */
      motopp_freeMacro(tmp); 
   }

   stab_put(mb->macros, m->name, m);

   buf_free(n);
	
   /* clear out the builder */
   vec_clear(mb->args);
   buf_clear(mb->buf);

}

%}

%union {
	char *sval;
}

%token <sval> ANYCHARS 
%token <sval> ARG 
%token <sval> NAME 

%token DEF
%token EOB

%type <sval> macro 
%type <sval> no_arg_macro 
%type <sval> arg_macro 
%type <sval> arg_list 
%type <sval> arg 
%type <sval> body 
%type <sval> body_element 

%%
program
	: translation_unit 
;

translation_unit
	: translation_unit macro  
	|
;

macro
	: no_arg_macro { createMacro($1); }
	| arg_macro { createMacro($1); }
	| EOB 
		{ 
			motopp_freeRYYBuffer();
			motopp_freeFrame(ppenv);
			pp_switch_to_buffer(ppenv->frame->yybuf);
			YYACCEPT; 
		}
;

no_arg_macro
	: NAME body
;

arg_macro
	: NAME '(' arg_list ')' body
;

arg_list
	: arg_list ',' arg
	| arg
;

arg
	: ARG
		{
			MotoMacroBuilder *mb = ppenv->mb;
			if (vec_contains(mb->args, $1)) {
				ryyerror($1);
			}
			else {
				vec_add(mb->args, $1);
			}
		}
;

body
	: body body_element
	| body_element 
;

body_element
	: ANYCHARS
		{
			buf_puts(ppenv->mb->buf, $1);
		}
;

%%
