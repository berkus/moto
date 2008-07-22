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

   $RCSfile: mxext.c,v $   
   $Revision: 1.7 $
   $Author: dhakim $
   $Date: 2002/10/23 04:44:23 $
 
==============================================================================*/
#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "memsw.h"

#include "mxext.h"

MXExt *
mxext_create() {
   MXExt *p;

   p = (MXExt *)malloc(sizeof(MXExt));

   p->files = hset_createDefault();
   p->includePaths = hset_createDefault();
   p->libraryPaths = hset_createDefault();
   p->archives = hset_createDefault();
   p->includes = hset_createDefault();
   p->libraries = hset_createDefault();
   p->mappings = stab_createDefault();

   return p;
}

void 
mxext_free(MXExt *p) {
   free(p->ppname);   
   hset_free(p->libraries);
   hset_free(p->includes);   
   hset_free(p->libraryPaths);
   hset_free(p->includePaths);
   hset_free(p->archives);
   hset_free(p->files);   
   stab_free(p->mappings);
   free(p);
}

void 
mxext_setName(MXExt *p, char *name) {
   p->name = name;
   /* make include symbol name of the form __MX_<ext_name>_H */
   p->ppname = malloc(strlen(name) + 8);
   sprintf(p->ppname, "__MX_%s_H", name);
}

void 
mxext_addFile(MXExt *p, char *file) {
   hset_add(p->files, strdup(file));   
}
void 
mxext_addInclude(MXExt *p, char *include) {
   hset_add(p->includes, strdup(include));   
}
void 
mxext_addLibrary(MXExt *p, char *file) {
   hset_add(p->libraries, strdup(file));   
}
void 
mxext_addArchive(MXExt *p, char *file) {
   hset_add(p->archives, strdup(file));   
}
void 
mxext_addIncludePath(MXExt *p, char *path) {
   hset_add(p->includePaths, strdup(path));   
}
void 
mxext_addLibraryPath(MXExt *p, char *path) {
   hset_add(p->libraryPaths, strdup(path));   
}

void 
mxext_addMapping(MXExt *p, char *key, MXMapping *mapping) {
   stab_put(p->mappings, strdup(key), mapping);   
}

Enumeration *
mxext_getFiles(MXExt *p) {
   return hset_elements(p->files);
}
Enumeration *
mxext_getIncludes(MXExt *p) {
   return hset_elements(p->includes);
}
Enumeration *
mxext_getLibraries(MXExt *p) {
   return hset_elements(p->libraries);
}
Enumeration *
mxext_getArchives(MXExt *p) {
   return hset_elements(p->archives);
}
Enumeration *
mxext_getLibraryPaths(MXExt *p) {
   return hset_elements(p->libraryPaths);
}
Enumeration *
mxext_getIncludePaths(MXExt *p) {
   return hset_elements(p->includePaths);
}
Enumeration *
mxext_getMapKeys(MXExt *p) {
   return stab_getKeys(p->mappings);
}

/*=============================================================================
// end of file: $RCSfile: mxext.c,v $
==============================================================================*/

