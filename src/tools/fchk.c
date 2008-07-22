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

   $RCSfile: fchk.c,v $   
   $Revision: 1.2 $
   $Author: kocienda $
   $Date: 2000/04/30 23:10:04 $
 
==============================================================================*/
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>

extern int errno;

static void usage(int code) {
   printf("Usage: fchk <file>\n");
   exit(code);
}

int main(int argc, char **argv) {
   int result = 0;
   struct stat sb; 
   char *arg;

   if (argc < 2) {
      usage(1);
   }

   arg = argv[1];
   result = stat(arg, &sb);
   if (result == 0) {
      if (S_ISDIR(sb.st_mode)) {
         printf("d");
      }
      else if (S_ISREG(sb.st_mode)) {
         printf("f");
      }
      else if (S_ISCHR(sb.st_mode)) {
         printf("c");
      }
      else if (S_ISBLK(sb.st_mode)) {
         printf("b");
      }
      else if (S_ISFIFO(sb.st_mode)) {
         printf("p");
      }
      else if (S_ISSOCK(sb.st_mode)) {
         printf("s");
      }
      printf("\n");
   }
   else {
      switch (result) {
         case EACCES:
            printf("fchk: Read or search permission was denied "
                   "for a component of the path prefix: %s\n", arg);
            break;
         case EIO:
            printf("fchk: An I/O error occurred while reading "
                   "from the file system: %s\n", arg);
            break;
         case ELOOP:
            printf("fchk: Too many symbolic links were encountered "
                   "in translating the pathname: %s\n", arg);
            break;                                                            
         case ENAMETOOLONG:
            printf("fchk: Name or path is too long: %s\n", arg);
            break;
         case ENOENT:
            printf("fchk: The named file does not exist: %s\n", arg);
            break;
         case ENOTDIR:
            printf("fchk: A component of the path prefix "
                   "is not a directory: %s\n", arg);
            break;
         default:
            printf("fchk: Unknown error while "
                   "processing path %s\n", arg);
            break;
      }
   }

   return result;
}

/*=============================================================================
// end of file: $RCSfile: fchk.c,v $
==============================================================================*/

