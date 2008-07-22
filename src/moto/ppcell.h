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

   $RCSfile: ppcell.h,v $   
   $Revision: 1.2 $
   $Author: kocienda $
   $Date: 2000/04/30 23:10:03 $
 
==============================================================================*/
#ifndef __PPCELL_H
#define __PPCELL_H

#include "cell.h"
#include "number.h"

/*-----------------------------------------------------------------------------
 *  parser cell structs
 *---------------------------------------------------------------------------*/

typedef struct ppValueCell {
   CellType type;
   CellKind kind;
   void *value;
   short lineno;
} PPValueCell;

typedef struct ppOpCell {
   CellType type;
   short opcode;
   short opcount;
   short lineno;
   union ppUnionCell *operands[1];
} PPOpCell;

typedef union ppUnionCell {
   CellType type;
   PPValueCell valuecell;
   PPOpCell opcell;
} PPUnionCell;

#endif

/*=============================================================================
// end of file: $RCSfile: ppcell.h,v $
==============================================================================*/


