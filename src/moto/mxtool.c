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

   $RCSfile: mxtool.c,v $   
   $Revision: 1.3 $
   $Author: dhakim $
   $Date: 2003/05/29 02:14:08 $
 
==============================================================================*/
#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "memsw.h"
#include "mxstring.h"
#include "meta.h"
#include "dl.h"
#include "motoextension.h"
#include "env.h"
#include "pp.h"

#include "stringset.h"

MotoEnv *env;
MotoPP *ppenv;

#define ARCHIVE_FLAG 0x1
#define INCLUDE_FLAG 0x2
#define INCLUDEPATH_FLAG 0x4
#define LIBRARY_FLAG 0x8
#define LIBRARYPATH_FLAG 0x10
#define EXTENSION_FLAG 0x20
#define DEBUG_FLAG 0x40

static void usage(int code) {
   char build[64];
   moto_buildTimeStamp(build);
   printf("\nThis is mxdeps, version %d.%d.%d\n\n", 
		MOTO_MAJOR_VERSION, 
		MOTO_MINOR_VERSION, 
		MOTO_MICRO_VERSION);
   printf("Build date: %s\n\n", build);
   printf("Copyright 2002, David Hakim\n\n");
   printf("Usage: mxdeps [options] <files...>\n\n");
   printf("Options:\n\n");
   printf("   -h:        Prints this help message\n");
   printf("   -a:        Output archives needed to resolve external symbols\n");
   printf("              in these archives not including libcodex or libc\n");
   printf("   -d:        Prints out the current MXPath\n"); 
   printf("   -i:        Output header files these extensions depend upon\n");
   printf("   -I:        Output include paths these extensions depend upon\n");
   printf("   -l:        Output shared libraries these extensions depend upon\n");
   printf("   -L:        Output library paths these extensions depend upon\n");
   printf("   -e:        Include the path to the extension folder when used in\n");
   printf("              combination with -i. Include the path to the extension\n");
   printf("              archive when used in combination with -a\n");
   printf("   -X<path>:  Prepends the specified path to the list\n");
   printf("              of paths in which to look for moto extensions\n");
   printf("\n");
   exit(code);
}

int main (int argc, char **argv) {
   int flags=0;
   int i,j;
   int nfiles;

	StringSet* extensions, *dependencies;
	Enumeration* e;
	
   shared_init(32*1024*1024);
	mxdl_init();
	
   while (1) {
      int c = getopt(argc, argv, "adehiIlLX:");
      if (c == -1) break;
      switch (c) {
         
         /* execution mode flags */
         case 'd': flags |= DEBUG_FLAG; break;
         case 'e': flags |= EXTENSION_FLAG; break;
         case 'a': flags |= ARCHIVE_FLAG; break;
         case 'i': flags |= INCLUDE_FLAG; break;
         case 'I': flags |= INCLUDEPATH_FLAG; break;
         case 'l': flags |= LIBRARY_FLAG; break;
         case 'L': flags |= LIBRARYPATH_FLAG; break;
         case 'X': mxdl_prependMXPath(optarg); break;
         case 'h': usage(0); return 0;
      }
   }
    
   nfiles = argc - optind;
   
   dependencies = sset_createDefault();
   extensions = sset_createDefault();
   
   for(i=optind;i<argc;i++){
   	char* libpath = mxdl_find(argv[i]);
      switch (flags & (ARCHIVE_FLAG|INCLUDE_FLAG|INCLUDEPATH_FLAG|LIBRARY_FLAG|LIBRARYPATH_FLAG)){
      case INCLUDEPATH_FLAG :
      	sset_add(extensions,mxstr_substring(libpath,0,mxstr_lastIndexOf(libpath,'/')));
      	break;
      case ARCHIVE_FLAG : {
				char * apath = mxstr_substring(libpath,0,mxstr_lastIndexOf(libpath,'o'));
				apath[strlen(apath)-1] = 'a';
				sset_add(extensions,apath);
         }
      	break;
      }
   }
   	   
   for(i=optind;i<argc;i++){
   	MotoExtension* mx = mxdl_load(argv[i]);   	
		switch (flags & (ARCHIVE_FLAG|INCLUDE_FLAG|INCLUDEPATH_FLAG|LIBRARY_FLAG|LIBRARYPATH_FLAG)){
			case ARCHIVE_FLAG : 
				for(j=0;j<mx->archiveCount;j++) 
					sset_add(dependencies,mx->archives[j]); 
				break;
			case INCLUDE_FLAG : 
				for(j=0;j<mx->includeCount;j++) 
					sset_add(dependencies,mx->includes[j]); 
				break;
			case INCLUDEPATH_FLAG : 
				for(j=0;j<mx->includePathCount;j++) 
					sset_add(dependencies,mx->includePaths[j]); 
				break;
			case LIBRARY_FLAG : 
				for(j=0;j<mx->libraryCount;j++) 
					sset_add(dependencies,mx->libraries[j]); 
				break; 
			case LIBRARYPATH_FLAG : 
				for(j=0;j<mx->libraryPathCount;j++) 
					sset_add(dependencies,mx->libraryPaths[j]); 
				break;
		}
   }

   if(flags & DEBUG_FLAG)
      printf("MXPath is set to '%s'\n",mxdl_getMXPath());
      
   if(flags & EXTENSION_FLAG) {   
		e = sset_elements(extensions);
		while(enum_hasNext(e))
			printf("%s\n",(char*)enum_next(e));
   }
   
   e = sset_elements(dependencies);
   while(enum_hasNext(e))
   	printf("%s\n",(char*)enum_next(e));
   	   
   return 0;
}

