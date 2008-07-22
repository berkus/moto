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

   $RCSfile: mfn.c,v $   
   $Revision: 1.8 $
   $Author: dhakim $
   $Date: 2003/02/27 19:47:00 $
 
==============================================================================*/
#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <string.h>

#include "memsw.h"

#include "err.h"
#include "log.h"
#include "stringbuffer.h"

#include "fnarg.h"
#include "mfn.h"

MFN *mfn_create() {
   MFN *p;

   /* allocate function and arg vector */
   p = (MFN *)malloc(sizeof(MFN));
   p->argv = vec_createDefault();

   /* set other fields */
   p->cname = NULL;

   return p;
}

void mfn_free(MFN *p) {
   /* free FNArg structs */
   Enumeration *e = vec_elements(p->argv);
   while(enum_hasNext(e)) {
      FNArg *arg = (FNArg *)enum_next(e);
      free(arg);
   }
   enum_free(e);
   /* free internally allocated memory */
   vec_free(p->argv);

   free(p->cname);
   free(p);
   /*
   log_debug(__FILE__, "Freed motofunction: 0x%x\n", p);
   */
}


/*
 * Generates a symbol name from a moto function or method name and a list of args
 */
 
char *mfn_symbolName(MFN *p) {
   if (p->cname == NULL) {
		int i;
		int argc = vec_size(p->argv);
		StringBuffer *buf = buf_create(32);
	
		/* If the classname is not null then this is a method and the classname 
			should prefix the generated cname */
			
		if (p->classn != NULL)
			 buf_printf(buf, "_%s",p->classn);
		
		/* If the fname starts with '~' (i.e. this is a destructor) convert it 
			into an '_' */
		
		if (p->fname[0] == '~')
			 buf_printf(buf,"__%s__", p->fname+1);
		else     
			 buf_printf(buf, "_%s__", p->fname);
       
      for (i = 0; i < argc; i++) {
      	FNArg *arg = (FNArg *)vec_get(p->argv, i);
      	int l = strlen(arg->type);
      
			/* Output one P for each dimension of an array argument */
			while(arg->type[l-1] == ']'){ buf_putc(buf,'P'); l -= 2; }
				
			/* For built in types output the first letter of the 
				type name for the argument */
			
			if(l == 3 && !strncmp(arg->type,"int",l)) buf_putc(buf,'i');
			else if(l == 5 && !strncmp(arg->type,"float",l)) buf_putc(buf,'f');
			else if(l == 6 && !strncmp(arg->type,"double",l)) buf_putc(buf,'d');
			else if(l == 4 && !strncmp(arg->type,"char",l)) buf_putc(buf,'c');
			else if(l == 4 && !strncmp(arg->type,"long",l)) buf_putc(buf,'l');
			else if(l == 7 && !strncmp(arg->type,"boolean",l)) buf_putc(buf,'b');
			else if(l == 4 && !strncmp(arg->type,"byte",l)) buf_putc(buf,'y');
			else if(l == 6 && !strncmp(arg->type,"String",l)) buf_putc(buf,'S');
			else if(l == 6 && !strncmp(arg->type,"Object",l)) buf_putc(buf,'O');
			else if(arg->type[l-1] == ')') buf_putc(buf,'F');
			else { buf_puti(buf, l); buf_put(buf,arg->type,l); }
			  
      }   

      p->cname = buf_toString(buf);
      buf_free(buf);
   }
   return p->cname;
}

void mfn_addArg(MFN *p, FNArg *arg) {
   vec_add(p->argv, arg);   
}

int mfn_argc(MFN *p) {
   return vec_size(p->argv);
}

Enumeration *mfn_args(MFN *p) {
   return vec_elements(p->argv);
}

void mfn_dump(MFN *p) {
   Enumeration *e;
   printf("MFN--------------------\n");   
   printf("rtype: %s\n", p->rtype);   
   printf("fname: %s\n", p->fname);   
   printf("argc: %d\n", vec_size(p->argv));   
   printf("argv: \n");   
   e = vec_elements(p->argv);
   while(enum_hasNext(e)) {
      FNArg *arg = (FNArg *)enum_next(e);
      printf("\t%s %s\n", arg->type, arg->name);   
   }
   enum_free(e);
   printf("--------------------------------\n");   
}

/*=============================================================================
// end of file: $RCSfile: mfn.c,v $
==============================================================================*/

