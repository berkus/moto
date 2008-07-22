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

   $RCSfile: mxext.h,v $   
   $Revision: 1.4 $
   $Author: dhakim $
   $Date: 2003/05/29 02:14:08 $
 
==============================================================================*/
#ifndef __MXEXT_H
#define __MXEXT_H

#include "hashset.h"
#include "enumeration.h"
#include "mxmapping.h"
#include "symboltable.h"

typedef struct mxExt {
	char *name;
	char *ppname;
	HashSet *includes;
	HashSet *files;
	HashSet *libraries;
	HashSet *includePaths;
	HashSet *libraryPaths;
	HashSet *archives;
	SymbolTable *mappings; /* maps an MFN to a CFN */
} MXExt;

MXExt *mxext_create();
void mxext_free(MXExt *p);

void mxext_setName(MXExt *p, char *name);
void mxext_addFile(MXExt *p, char *file);
void mxext_addIncludePath(MXExt *p, char *includePath);
void mxext_addLibraryPath(MXExt *p, char *libraryPath);
void mxext_addInclude(MXExt *p, char *include);
void mxext_addLibrary(MXExt *p, char *library);
void mxext_addArchive(MXExt *p, char *archive);
void mxext_addMapping(MXExt *p, char *key, MXMapping *mapping);

Enumeration *mxext_getFiles(MXExt *p);
Enumeration *mxext_getIncludes(MXExt *p);
Enumeration *mxext_getLibraries(MXExt *p);
Enumeration *mxext_getIncludePaths(MXExt *p);
Enumeration *mxext_getLibraryPaths(MXExt *p);
Enumeration *mxext_getArchives(MXExt *p);
Enumeration *mxext_getMapKeys(MXExt *p);

#endif

/*=============================================================================
// end of file: $RCSfile: mxext.h,v $
==============================================================================*/

