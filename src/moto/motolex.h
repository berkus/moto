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

   $RCSfile: motolex.h,v $   
   $Revision: 1.7 $
   $Author: dhakim $
   $Date: 2002/03/07 01:25:30 $
 
==============================================================================*/
#ifndef __LEXEX_H
#define __LEXEX_H

#include <stdio.h>

#include "env.h"

void moto_initYY(FILE *file);
void moto_freeYYBuffer();
void moto_updateYYFrame();

int moto_prepareMainBuffer(MotoEnv *env, char *text);

#endif

/*=============================================================================
// end of file: $RCSfile: motolex.h,v $
==============================================================================*/

