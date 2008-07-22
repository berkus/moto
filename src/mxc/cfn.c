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

   $RCSfile: cfn.c,v $   
   $Revision: 1.5 $
   $Author: kocienda $
   $Date: 2000/04/30 23:10:04 $
 
==============================================================================*/
#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>

#include "memsw.h"

#include "err.h"
#include "log.h"

#include "cfn.h"
#include "fnarg.h"

CFN *cfn_create() {
   CFN *p;

   /* allocate function and arg vector */
   p = (CFN *)malloc(sizeof(CFN));
   p->argv = vec_createDefault();

   p->rtype = "";
   p->fname = "";

   return p;
}

void cfn_free(CFN *p) {
   /* free FNArg structs */
   Enumeration *e = vec_elements(p->argv);
   while(enum_hasNext(e)) {
      FNArg *arg = (FNArg *)enum_next(e);
      free(arg);
   }
   enum_free(e);
   /* free internally allocated memory */
   /*
   vec_free(p->argv);
   */
   free(p);
   /*
   log_debug(__FILE__, "Freed c function: 0x%x\n", p);
   */
}


void cfn_addArg(CFN *p, FNArg *arg) {
   vec_add(p->argv, arg);   
}

int cfn_argc(CFN *p) {
   return vec_size(p->argv);
}

Enumeration *cfn_args(CFN *p) {
   return vec_elements(p->argv);
}

void cfn_dump(CFN *p) {
   Enumeration *e;
   printf("CFN--------------------\n");   
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
// end of file: $RCSfile: cfn.c,v $
==============================================================================*/

