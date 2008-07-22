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

   $RCSfile: mxcg.h,v $   
   $Revision: 1.5 $
   $Author: dhakim $
   $Date: 2003/03/13 15:59:43 $
 
==============================================================================*/
#ifndef __MXC_H
#define __MXC_H

#include "mman.h"
#include "symboltable.h"
#include "properties.h"

#include "cell.h"
#include "fnarg.h"

/*-----------------------------------------------------------------------------
 *  MXC struct
 *---------------------------------------------------------------------------*/

typedef struct mxc {
   MPool *mpool;
   char *mxdir;
   char *target;
   char *filename;
   Properties *props;
   Vector *exts;
   Vector *strlist;
   struct mxExt *curlib;
   struct mFunction *curmfn;
   struct cFunction *curcfn;
   struct fnArg *curarg;
} MXC;

/*-----------------------------------------------------------------------------
 *  MXC functions
 *---------------------------------------------------------------------------*/

MXC *mxc_create();
void mxc_free(MXC *p);

char *mxc_c(MXC *c, UnionCell *p);

void mxc_emitIncludes(MXC *c);
void mxc_emitExports(MXC *c);
void mxc_emitCode(MXC *c);
void mxc_emitInterface(MXC *c);
void mxc_emitBuildScript(MXC *c);

#endif

/*=============================================================================
// end of file: $RCSfile: mxcg.h,v $
==============================================================================*/

