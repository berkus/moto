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

   $RCSfile: motoc.h,v $   
   $Revision: 1.3 $
   $Author: dhakim $
   $Date: 2003/05/29 02:14:08 $
 
==============================================================================*/
#ifndef __MOTOC_H
#define __MOTOC_H

#include "cell.h"

void motoc(const UnionCell *p);

/* Prototypes for functions used in motox.c */
MotoVal* motoc_callADF(MotoVal* self, void** argv,MotoFunction* f, int freevals);

#endif

/*=============================================================================
// end of file: $RCSfile: motoc.h,v $
==============================================================================*/

