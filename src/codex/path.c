/*=============================================================================

   Copyright (C) 2000 Kenneth Kocienda and David Hakim.
   This file is part of the Codex C Library.

   The Codex C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The Codex C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the Codex C Library; see the file COPYING.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   $RCSfile: path.c,v $   
   $Revision: 1.8 $
   $Author: dhakim $
   $Date: 2002/04/05 02:03:14 $
 
==============================================================================*/
#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>

#include "memsw.h"
#include "excpfn.h"
#include "err.h"

#include "path.h"

#ifdef PATH_MAX
static int pathmax = PATH_MAX;
#else
static int pathmax = 0;
#endif

#define PATH_MAX_GUESS 1024

char *path_alloc(int *maxlen) {
   char *s;

   if (pathmax == 0) {
      errno = 0;
      if ((pathmax = pathconf("/", _PC_PATH_MAX) < 0)) {
         if (errno == 0) {
            pathmax = PATH_MAX_GUESS;
         }
         else {
            err_w("LibraryError");
         }
      }
      else {
         pathmax++;
      }
   }

   s = emalloc(pathmax + 1);

   if (maxlen != NULL) {
      *maxlen = pathmax + 1;   
   }

   return s;
}

/*=============================================================================
// end of file: $RCSfile: path.c,v $
==============================================================================*/

