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

   $RCSfile: number.h,v $   
   $Revision: 1.3 $
   $Author: kocienda $
   $Date: 2000/04/30 23:10:01 $
 
==============================================================================*/
#ifndef __NUMBER_H
#define __NUMBER_H

#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "integer.h"
 
/*
 * Call in defines for the following types/macros for integer types
 */

#define ORANGE -1

typedef enum {
   I32 = 32,
   I64 = 64,
} IntType;

typedef enum {
   FP32 = 32,
   FP64 = 64,
} FloatType;

typedef struct parsedInt {
   IntType t;
   int32_t i;
   int64_t l;
} ParsedInt;

typedef struct parsedFloat {
   FloatType t;
   float f;
   double d;
} ParsedFloat;

int parse_int(char *s, ParsedInt *u);
void parse_float(char *s, ParsedFloat *u);

#endif

/*=============================================================================
// end of file: $RCSfile: number.h,v $
==============================================================================*/
