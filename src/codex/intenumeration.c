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

   $RCSfile: intenumeration.c,v $   
   $Revision: 1.7 $
   $Author: kocienda $
   $Date: 2000/04/30 23:10:01 $
 
==============================================================================*/
#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>

#include "memsw.h"
#include "intenumeration.h"
#include "exception.h"
#include "excpfn.h"

IntEnumeration* ienum_create(
   void *iterator, 
   int (*next_fn)(void *),
   char (*has_more_fn)(void *)
)
{
   IntEnumeration* e = (IntEnumeration*)emalloc(sizeof(IntEnumeration));
  
   e->iterator = iterator;
   e->next_fn = next_fn;
   e->has_more_fn = has_more_fn;

   return e;
}

void ienum_free(IntEnumeration *e) {
   free(e->iterator);
   free(e);
}

int ienum_next(IntEnumeration *e){
   return (*e->next_fn)(e->iterator);
}

int ienum_hasNext(IntEnumeration *e){
   return (*e->has_more_fn)(e->iterator);
}

/*=============================================================================
// end of file: $RCSfile: intenumeration.c,v $
==============================================================================*/

