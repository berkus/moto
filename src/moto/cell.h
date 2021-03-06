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
   $Revision: 1.8 $
   $Author: dhakim $
   $Date: 2003/01/26 22:20:19 $
 
==============================================================================*/
#ifndef __CELL_H
#define __CELL_H

#include "number.h"

typedef enum { 
   VALUE_TYPE  = 0, 
   OP_TYPE     = 1, 
} CellType;

typedef enum { 
   BOOLEAN_VALUE = 1, 
   CHAR_VALUE    = 2, 
   INT32_VALUE   = 3, 
   INT64_VALUE   = 4, 
   FLOAT_VALUE   = 5, 
   DOUBLE_VALUE  = 6, 
   NULL_VALUE    = 7, 
   STRING_VALUE  = 8, 
   REGEX_VALUE   = 9, 
   ID_VALUE      = 10, 
} CellKind;

typedef struct metaInfo {
   int lineno;
   char* filename;
   char* macroname;
   struct metaInfo* caller;
} MetaInfo;

typedef struct {
   MetaInfo meta;
   char* sval;
} SVS;

typedef struct {
   MetaInfo meta;
} IVS;

typedef struct valueCell {
   CellType type;
   CellKind kind;
   void *value;
   char *code;
   MetaInfo meta;
} ValueCell;

typedef struct OpCell {
   CellType type;
   unsigned short opcode;
   unsigned short opcount;
   MetaInfo meta;
   union unionCell *operands[1];
} OpCell;

typedef union unionCell {
   CellType type;
   ValueCell valuecell;
   OpCell opcell;
} UnionCell;

#endif

/*=============================================================================
// end of file: $RCSfile: cell.h,v $
==============================================================================*/

