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

   $RCSfile: sysmalloc.c,v $   
   $Revision: 1.2 $
   $Author: kocienda $
   $Date: 2000/04/30 23:10:02 $
 
==============================================================================*/

#include "sysmalloc.h"

void *sys_calloc(size_t nmemb, size_t size) {
   return calloc(nmemb, size);
}

void *sys_malloc(size_t size) {
   return malloc(size);
}

void sys_free(void *ptr) {
   free(ptr);
}

void *sys_realloc(void *ptr, size_t size) {
   return realloc(ptr, size);
}

/*=============================================================================
// end of file: $RCSfile: sysmalloc.c,v $
==============================================================================*/



