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

   $RCSfile: cell.h,v $   
   $Revision: 1.2 $
   $Author: kocienda $
   $Date: 2000/04/30 23:10:04 $
 
==============================================================================*/
#ifndef __CELL_H
#define __CELL_H

/*-----------------------------------------------------------------------------
 *  parser cell structs
 *---------------------------------------------------------------------------*/

typedef enum { 
   VALUE_TYPE  = 0, 
   OP_TYPE     = 1, 
} CellType;

typedef struct strCell {
   CellType celltype;
   char *value;
} StrCell;

typedef struct OpCell {
   CellType celltype;
   short opcode;
   short opcount;
   union unionCell *operands[1];
} OpCell;

typedef union unionCell {
   CellType celltype;
   StrCell strcell;
   OpCell opcell;
} UnionCell;

#endif

/*=============================================================================
// end of file: $RCSfile: cell.h,v $
==============================================================================*/

