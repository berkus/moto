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

   $RCSfile: sort.h,v $   
   $Revision: 1.4 $
   $Author: dhakim $
   $Date: 2003/02/27 19:46:59 $
 
==============================================================================*/
#ifndef __SORT_H
#define __SORT_H

#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "cdx_function.h"

void quick_sort(void **a, int size, Function* cmp); 	
void bubble_sort(void **a, int size, int (*cmp)(const void *, const void *)); 	

#endif

/*=============================================================================
// end of file: $RCSfile: sort.h,v $
==============================================================================*/

