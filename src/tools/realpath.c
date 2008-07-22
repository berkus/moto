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

   $RCSfile: realpath.c,v $   
   $Revision: 1.2 $
   $Author: kocienda $
   $Date: 2000/04/30 23:10:04 $
 
==============================================================================*/
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <errno.h>

#define DEF_PATH_MAX 2048

extern int errno;

static void usage(int code) {
   printf("Usage: realpath <file>\n");
   exit(code);
}

int main(int argc, char **argv) {
   int result = 0;

   int path_max;
   char *path;
   char *arg;

   if (argc < 2) {
      usage(1);
   }

#ifdef PATH_MAX
   path_max = PATH_MAX;
#else
   path_max = pathconf(path, _PC_PATH_MAX);
   if (path_max <= 0) {
      path_max = DEF_PATH_MAX;
   }
#endif

   arg = argv[1];
   path = realpath(arg, (char*)malloc(path_max));
   if (path != NULL) {
      printf("%s\n", path);
   }
   else {
      result = errno;
      switch (result) {
         case EACCES:
            printf("realpath: Read or search permission was denied "
                   "for a component of the path prefix: %s\n", arg);
            break;
         case EIO:
            printf("realpath: An I/O error occurred while reading "
                   "from the file system: %s\n", arg);
            break;
         case ELOOP:
            printf("realpath: Too many symbolic links were encountered "
                   "in translating the pathname: %s\n", arg);
            break;                                                            
         case ENAMETOOLONG:
            printf("realpath: Name or path is too long: %s\n", arg);
            break;
         case ENOENT:
            printf("realpath: The named file does not exist: %s\n", arg);
            break;
         case ENOTDIR:
            printf("realpath: A component of the path prefix "
                   "is not a directory: %s\n", arg);
            break;
         default:
            printf("realpath: Unknown error while "
                   "processing path %s\n", arg);
            break;
      }
   }

   return result;
}

/*=============================================================================
// end of file: $RCSfile: realpath.c,v $
==============================================================================*/

