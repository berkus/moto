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
   $Revision: 1.28 $
   $Author: dhakim $
   $Date: 2003/02/03 14:30:07 $
 
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

#include "sharedmem.h"
#include "runtime.h"

#include "env.h"
#include "meta.h"
#include "motoi.h"
#include "motoc.h"
#include "motofn.h"
#include "dl.h"
#include "pp.h"

Runtime *rt;
MotoEnv *env;
MotoPP *ppenv;

extern int optind;

static void usage(int code);

int main (int argc, char **argv) {
   int result = 0;
   int c;
   int nfiles;
   int flags;
   int mman_level;
   char* mxpath=NULL;

   size_t shared_heap = 32*1024*1024;

   flags = MOTOPP_FLAG | MOTOV_FLAG | MOTOI_FLAG;
   flags |= MACRO_DEBUG_FLAG | DEF_DL_FLAG;

   while (1) {
      c = getopt(argc, argv, "cdhlmM:X:Opvux");
      if (c == -1) {
         break;
      }
      switch (c) {
         
         /* execution mode flags */
         case 'c':
            flags |= MOTOPP_FLAG;
            flags |= MOTOV_FLAG;
            flags &= ~MOTOI_FLAG;
            flags |= MOTOC_FLAG;
            flags &= ~MOTOD_FLAG;
            break;
   	   case 'u':
            flags |= MOTOPP_FLAG;
            flags |= MOTOV_FLAG;
            flags &= ~MOTOI_FLAG;
            flags &= ~MOTOC_FLAG;
            flags |= MOTOD_FLAG;
            break;
         case 'p':
            flags |= MOTOPP_FLAG;
            flags &= ~MOTOV_FLAG;
            flags &= ~MOTOI_FLAG;
            flags &= ~MOTOC_FLAG;
            flags &= ~MOTOD_FLAG;
            break;
         case 'v':
            flags |= MOTOPP_FLAG;
            flags |= MOTOV_FLAG;
            flags &= ~MOTOI_FLAG;
            flags &= ~MOTOC_FLAG;
            flags &= ~MOTOD_FLAG;
            break;
         
         /* other option flags */

         case 'h':
            usage(0);
            break;
         case 'd':
            flags |= DEBUG_FLAG;
            break;
         case 'l':
            flags |= MEM_LEAK_FLAG;
            break;
         case 'm':
            mman_level = atoi(optarg);
            /* mman_level must be 0, 1, or 2 */
            if ((mman_level < 0 || mman_level > 2) ||
                (mman_level == 0 && strcmp(optarg, "0") != 0)) {
               usage(-1);
            }
            switch (mman_level) {
               case 0:
                  flags &= ~MMAN_LEVEL1_FLAG;
                  flags &= ~MMAN_LEVEL2_FLAG;
                  break;
               case 1:
                  flags |= MMAN_LEVEL1_FLAG;
                  flags &= ~MMAN_LEVEL2_FLAG;
                  break;
               case 2:
                  flags &= ~MMAN_LEVEL1_FLAG;
                  flags |= MMAN_LEVEL2_FLAG;
                  break;
            }
            break;
         case 'M': 
            {
               int multiplier = 1;
               if(optarg[strlen(optarg) -1] == 'M')
                  multiplier = 1024*1024;
               else if(optarg[strlen(optarg) -1] == 'K')
                  multiplier = 1024;
               shared_heap=atoi(optarg) * multiplier;
            }
            break;
         case 'O':
            flags &= ~MACRO_DEBUG_FLAG;
            break;
         case 'x':
            flags &= ~DEF_DL_FLAG;
            break;      
         case 'X':
            mxpath = strdup(optarg);
            break;
      }
   }

   nfiles = argc - optind;
   if (nfiles <= 0) {
      //result = moto_stdin(flags);
   }
   else {
      result = moto_files(nfiles, &(argv[optind]), flags, shared_heap,mxpath);
   }

   if (flags & MEM_LEAK_FLAG) {
      if (shared_getANodes() == 0) {
         printf("//mem-ok//\n");
      }
      else {
         printf("//mem-leak//\n");
      }
   }
	if (flags & DEBUG_FLAG) {
   	printf("---------------------------------------------------------\n");
   	debug();
   } 
  
   return result;
}

static void usage(int code) {
   char build[64];
   moto_buildTimeStamp(build);
   printf("\nThis is moto, version %d.%d.%d\n\n", 
		MOTO_MAJOR_VERSION, 
		MOTO_MINOR_VERSION, 
		MOTO_MICRO_VERSION);
   printf("Build date: %s\n\n", build);
   printf("Copyright 2000-2003, David Hakim and Ken Kocienda\n\n");
   printf("Usage: moto [options] <files...>\n\n");
   printf("Options:\n\n");
   printf("   -c:        Compiles the moto files to C code\n");
   printf("   -d:        Prints memory debugging information\n");
   printf("   -h:        Prints this help message\n");
   printf("   -l:        Print simple memory leak diagnostic\n");
   printf("   -m<level>: Sets the level of automatic memory management\n");
   printf("              Level must be one of: \n");
   printf("                 0: No automatic memory management\n");
   printf("                 1: Pooled memory management (default) \n");
   printf("                 2: Garbage collection (not yet implemented)\n");
   printf("   -M<size>:  Sets the heap size in bytes,kilobytes or megabytes\n");
   printf("                 #: Sets the heap size in bytes\n");
   printf("                 #K: Sets the heap size in kilobytes\n");
   printf("                 #M: Sets the heap size in megabytes\n");
   printf("   -X<path>:  Prepends the specified path to the list\n");
   printf("              of paths in which to look for moto extensions\n");
   printf("   -O:        Omits debugging information\n");
   printf("   -p:        Runs the moto preprocessor only\n");
   printf("   -v:        Runs the moto type verifier only\n");
   printf("   -x:        Runs moto without loading default extensions\n");
   printf("\n");
   exit(code);
}

/*=============================================================================
// end of file: $RCSfile: main.c,v $
==============================================================================*/

