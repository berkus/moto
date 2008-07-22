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

   $RCSfile: motoi.h,v $   
   $Revision: 1.4 $
   $Author: dhakim $
   $Date: 2003/05/29 02:14:08 $
 
==============================================================================*/
#ifndef __MOTOI_H
#define __MOTOI_H

#include "cell.h"

void motoi_init();
void motoi(const UnionCell *p);

/* Prototypes for functions used in motox.c */
MotoVal* motoi_callEDF(MotoVal* self, MotoVal** args,MotoFunction* f);
MotoVal* motoi_callMDF(MotoVal* self,MotoVal** args, MotoFunction* f);

#endif

/*=============================================================================
// end of file: $RCSfile: motoi.h,v $
==============================================================================*/

