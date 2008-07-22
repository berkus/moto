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

   $RCSfile: intenumeration.h,v $   
   $Revision: 1.3 $
   $Author: kocienda $
   $Date: 2000/04/30 23:10:01 $
 
==============================================================================*/
#ifndef __INTENUMERATION_H
#define __INTENUMERATION_H

#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

typedef struct intEnumeration {
   void *iterator;
   int (*next_fn)(void *);
   char (*has_more_fn)(void *);
} IntEnumeration;

IntEnumeration *ienum_create(
   void *iterator, 
   int (*next_fn)(void *),
   char (*has_more_fn)(void *)
);

void ienum_free(IntEnumeration *e);

int ienum_next(IntEnumeration *e);
int ienum_hasNext(IntEnumeration *e);

#endif

/*=============================================================================
// end of file: $RCSfile: intenumeration.h,v $
==============================================================================*/

