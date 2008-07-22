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

   $RCSfile: motoextension.c,v $   
   $Revision: 1.3 $
   $Author: dhakim $
   $Date: 2002/10/31 01:17:31 $
 
==============================================================================*/
#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "memsw.h"

#include "motoextension.h"

/* Frees the memory for a MotoExtension */
void 
mext_free(MotoExtension* mx){
	int i;
	for(i=0;i<mx->includeCount;i++) free(mx->includes[i]);
	free(mx->includes);
	for(i=0;i<mx->symbolCount;i++) free(mx->symbols[i]);
	free(mx->symbols);	
	for(i=0;i<mx->includePathCount;i++) free(mx->includePaths[i]);
	free(mx->includePaths);	
	for(i=0;i<mx->libraryCount;i++) free(mx->libraries[i]);
	free(mx->libraries);
	for(i=0;i<mx->libraryPathCount;i++) free(mx->libraryPaths[i]);
	free(mx->libraryPaths);
	for(i=0;i<mx->archiveCount;i++) free(mx->archives[i]);
	free(mx->archives);
	vec_free(mx->functions);
	free(mx);
}

/* Returns an Enumeration of String types defined by this extension */
/* Enumeration* 
mext_getTypes(MotoExtension* mx){
} */

/* Returns an Enumeration of MotoFunction *s this extension defines */
Enumeration* 
mext_getFunctions(MotoExtension* mx){
	return vec_elements(mx->functions);
} 

/* Returns an Enumeration of String includes that this extension defines */
/* Enumeration* 
mext_getIncludes(MotoExtension* mx){
} */

/* Returns an Enumeration of IncludePaths that are required to compile a generated C file that requires this extension */
/* Enumeration* 
mext_getIncludePaths(MotoExtension* mx){
} */

/* Returns an Enumeration of LibraryPaths that are required to link with this extension */
/* Enumeration* 
mext_getLibraryPaths(MotoExtension* mx){
} */

/* Returns an Enumeration of .a files that are requires to link with this extension */
/* Enumeration* 
mext_getArchives(MotoExtension* mx){
} */

/* Returns an Enumeration of shared libraries that this Extension depends upon */
/* Enumeration* 
mext_getLibraries(MotoExtension* mx){
} */

/* Returns the path on disk to this Extension */
/* char* 
mext_getPath(MotoExtension* mx){
} */

/* Returns the path on disk to this Extension */
/* char* 
mext_getUseName(MotoExtension* mx){
} */

/*=============================================================================
// end of file: $RCSfile: motoextension.c,v $
==============================================================================*/

