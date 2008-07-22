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

   $RCSfile: mototree.c,v $   
   $Revision: 1.6 $
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

#include "memsw.h"

#include "exception.h"
#include "excpfn.h"

#include "mototree.h"

MotoTree *
moto_createTree() {
   MotoTree *tree = emalloc(sizeof(MotoTree));
   tree->cells = vec_createDefault();
   tree->mpool = mpool_create(8192);
   return tree;
}

void 
moto_freeTree(MotoTree *tree) {
   vec_free(tree->cells);
   mpool_free(tree->mpool);
   free(tree);
}

/*=============================================================================
// end of file: $RCSfile: mototree.c,v $
==============================================================================*/

