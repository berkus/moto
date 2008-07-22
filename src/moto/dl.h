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

   $RCSfile: dl.h,v $   
   $Revision: 1.10 $
   $Author: dhakim $
   $Date: 2003/02/28 20:50:20 $
 
==============================================================================*/
#ifndef __DL_H
#define __DL_H

#include "motofunction.h"
#include "motoextension.h"

/* Return the full path to the extension library referred to by usename */
char *mxdl_find(char *usename);

/* Return a MotoExtension object for the specified extension library */
MotoExtension* mxdl_load(char *usename);

/* Initialize the mxpath */
void mxdl_init();

/* Free the mxpath */
void mxdl_cleanup();

/* Prepend a new path component to the mxpath */
void mxdl_prependMXPath(char* xpath);

/* Return the current MXPath */
char* mxdl_getMXPath();

void moto_loadDestructor(void *handle, char *name, void(**free_fn)(void *));

void moto_os_dso_init(void);
void* moto_os_dso_open(const char* path);
void moto_os_dso_close(void* handle);
void* moto_os_dso_sym(void *handle, const char* symname);
char* moto_os_dso_error();

#endif

/*=============================================================================
// end of file: $RCSfile: dl.h,v $
==============================================================================*/

