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

   $RCSfile: mgen.c,v $   
   $Revision: 1.10 $
   $Author: dhakim $
   $Date: 2001/12/19 21:39:15 $
 
==============================================================================*/
#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

#include "memsw.h"
#include "excpfn.h"
#include "log.h"
#include "jumptable.h"
#include "hashtable.h"
#include "properties.h"
#include "kmpstring.h"
#include "tokenizer.h"
#include "fileio.h"

char* JTAB_PREFIX = "template";

char* replace(char* string, char* find, char* replace){
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

char* replaceChar(char* string, char find, char replace){
   StringBuffer* out;
   Tokenizer* tok;
   char* substr,*result;

   out = buf_createDefault();
   tok = tok_createCTok(string,find);

   substr=tok_next(tok);

   while (substr){
      buf_puts(out,substr);
      if((substr=tok_next(tok)) != NULL)
         buf_putc(out,replace);
   }

   tok_free(tok);

   result = buf_toString(out);
   buf_free(out);

   return result;
}

int main(int argc, char** argv){
   JumpTable* jtab;
   MJTable* mjtab;
   int i, tpathscnt;
   char** tpathsarr;
   char* modname;
   char* capmodname;
   StringBuffer* out,*jtab_code;
   char* result;
   char* propfile = NULL;
   Properties* props;
   char c;

   shared_init(32*1024*1024) ;
   log_init(LOG_ERROR);

   props= prop_create();

   /* Grab the properties file */

   while (1) {
      c = getopt(argc, argv, "p:");
      if (c == -1) {
         break;
      }
      switch (c) {
         case 'p':
            propfile = estrdup(optarg);
            break;
      }
   }

   if(propfile == NULL || !fio_exists((const char*)propfile)){
      fprintf(stderr,"mgen: No such properties file '%s'\n",propfile);
      exit(-1);
   }
      
   prop_addFromFile(props, propfile);

   out = buf_createDefault();

   /* Output module includes */

   buf_puts(out,replace(prop_get(props,"MODULE_INCLUDES"),"\n   ","\n")); 

   /* Output an array of function pointers, one for each template */

   tpathscnt = argc - optind - 1; /* Set the number of template paths */
   tpathsarr = argv + optind + 1; /* Set the array of template paths */
   modname   = argv[optind];      /* Set the module name */
  
   capmodname = estrdup(modname);
   capmodname[0] = toupper(capmodname[0]);

   for(i=0;i<tpathscnt;i++)
      buf_printf(out,"\nextern void exec_%s_%d(StringBuffer* out);",modname,i);

   buf_printf(out,
      "\n\ntypedef void(*template_function)(StringBuffer*);");

   buf_printf(out,"\n\nstatic template_function template_functions[%d] = {",tpathscnt);

   for(i=0;i<tpathscnt;i++)
      buf_printf(out,"%s   exec_%s_%d",(i==0?"\n":",\n"),modname,i);
   

   buf_printf(out,"\n};\n\n");

   /* Create and insert the template jumptable; */

   jtab_code = buf_createDefault();

   jtab = jtab_create(tpathscnt,tpathsarr);
   mjtab = mjtab_create(jtab);
   mjtab_generateCode(mjtab,jtab_code,JTAB_PREFIX);

   buf_puts(out,buf_data(jtab_code));

   buf_free(jtab_code); 

   /* Output the module body */

   buf_puts(out,replace(prop_get(props,"MODULE_INIT"),"\n   ","\n"));
   buf_puts(out,replace(prop_get(props,"MODULE_HANDLER"),"\n   ","\n"));
   buf_puts(out,replace(prop_get(props,"MODULE_END"),"\n   ","\n"));

   result = replace(buf_data(out),"${MOD_NAME}",modname);
   result = replace(result,"${CAPITALIZED_MOD_NAME}",capmodname);

   printf("%s",result);

   return 0;
}

/*=============================================================================
// end of file: $RCSfile: mgen.c,v $
==============================================================================*/

