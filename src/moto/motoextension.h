/*=============================================================================

   Copyright (C) 2002 David Hakim.
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

   $RCSfile: motoextension.h,v $   
   $Revision: 1.3 $
   $Author: dhakim $
   $Date: 2002/10/31 01:17:31 $
 
==============================================================================*/

#ifndef __MOTOEXTENSION_H
#define __MOTOEXTENSION_H

#include "enumeration.h"
#include "vector.h"

typedef struct motoExtension {
   int symbolCount;
   int archiveCount;
   int includeCount;
   int includePathCount;
   int libraryCount;
   int libraryPathCount;
   char **symbols;   
   char **includes;
   char **libraries;
   char **libraryPaths;
   char **includePaths;
   char **archives;
   char *usename;
   char *path;
   Vector* functions;
} MotoExtension;

/* Frees the memory for a MotoExtension */
void mext_free(MotoExtension* mx);

/* Returns an Enumeration of String types defined by this extension */
/* Enumeration* mext_getTypes(MotoExtension* mx); */

/* Returns an Enumeration of MotoFunction *s this extension defines */
Enumeration* mext_getFunctions(MotoExtension* mx); 

/* Returns an Enumeration of String includes that this extension defines */
/* Enumeration* mext_getIncludes(MotoExtension* mx); */

/* Reurns an Enumeration of IncludePaths that are required to compile a generated C file that requires this extension */
/* Enumeration* mext_getIncludePaths(MotoExtension* mx); */

/* Returns an Enumeration of LibraryPaths that are required to link with this extension */
/* Enumeration* mext_getLibraryPaths(MotoExtension* mx); */

/* Returns an Enumeration of .a files that are requires to link with this extension */
/* Enumeration* mext_getArchives(MotoExtension* mx); */

/* Returns an Enumeration of shared libraries that this Extension depends upon */
/* Enumeration* mext_getLibraries(MotoExtension* mx); */

/* Returns the path on disk to this Extension */
/* char* mext_getPath(MotoExtension* mx); */

/* Returns the path on disk to this Extension */
/* char* mext_getUseName(MotoExtension* mx); */

#endif

/*=============================================================================
// end of file: $RCSfile: motoextension.h,v $
==============================================================================*/

