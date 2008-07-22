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

   $RCSfile: main.c,v $   
   $Revision: 1.12 $
   $Author: dhakim $
   $Date: 2001/12/19 19:29:48 $
 
==============================================================================*/
#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <unistd.h>

#include "memsw.h"

#include "mman.h"
#include "log.h"
#include "err.h"

#include "vector.h"
#include "enumeration.h"

#include "mxcg.h"
#include "mxext.h"

MXC *c;

extern FILE *yyin;
extern int yylineno;

#ifndef LOG_LEVEL
#  define LOG_LEVEL LOG_ERROR
#endif

void usage() {
   printf("mxcg <files>");
}

int main (int argc, char **argv) {
   int i;
   char *target = NULL;
   char *mxdir = NULL;
   char *propfile = NULL; 
   FILE *file;
   shared_init(32*1024*1024);
   err_init();	
   log_init(LOG_LEVEL);
   mman_init();

   while (1) {
      char c = getopt(argc, argv, "n:d:p:");
      if (c == -1) {
         break;
      }
      switch (c) {
         case 'd':
            mxdir = optarg;
            break;
         case 'n':
            target = optarg;
            break;
         case 'p':
            propfile = optarg;
            break;
      }
   }

   if (argc < 2) {
      usage();
   }
   else {
      c = mxc_create();
      c->mxdir = mxdir;
      c->target = target;
      prop_addFromFile(c->props,  propfile);
      for (i = optind; i < argc; i++) {      
         c->filename = argv[i];
         file = fopen(argv[i], "r");
         err_assert(file != NULL, err_f("IOError"));
         yyin = file;
         yylineno = 1;
         yyparse();	
      }

      mxc_emitIncludes(c);
      mxc_emitExports(c);
      mxc_emitInterface(c);
      mxc_emitCode(c);
      mxc_emitBuildScript(c);
      //mxc_free(c);
   }
   
   shared_cleanup();

   return 0;
}

/*=============================================================================
// end of file: $RCSfile: main.c,v $
==============================================================================*/

