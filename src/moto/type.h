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

   $RCSfile: type.h,v $   
   $Revision: 1.11 $
   $Author: dhakim $
   $Date: 2003/02/11 23:14:53 $
 
==============================================================================*/
#ifndef __TYPE_H
#define __TYPE_H

#include "mototype.h"

typedef struct booleanVal {
   MotoType *type;
   char value;
} BooleanVal;

typedef struct byteVal {
   MotoType *type;
   signed char value;
} ByteVal;

typedef struct charVal {
   MotoType *type;
   char value;
} CharVal;    

typedef struct intVal {
   MotoType *type;
   int32_t value;
} IntVal;
    
typedef struct longVal {
   MotoType *type;
   int64_t value;
} LongVal;

typedef struct floatVal {
   MotoType *type;
   float value;
} FloatVal;
    
typedef struct doubleVal {
   MotoType *type;
   double value;
} DoubleVal;

typedef struct refVal {
   MotoType *type;
   void* value;
} RefVal;

typedef struct codeVal {
   MotoType *type;
   char* value;
} CodeVal;

/* a moto value */
typedef union motoVal {
   MotoType *type;
   BooleanVal booleanval;
   CharVal charval;
   ByteVal byteval;
   IntVal intval;
   LongVal longval;
   FloatVal floatval;
   DoubleVal doubleval;
   RefVal refval;
   CodeVal codeval;
} MotoVal;

/* a moto variable */
typedef struct motoVar {
   struct motoType *type;
   char *n;
   MotoVal *vs;         /* Private storage used for simple vars */
   char isMemberVar;  	/* Specifies that this variable is a class member variable */
   void* address; 		/* Address of the data associated with this MotoVar */
} MotoVar;

#endif

/*=============================================================================
// end of file: $RCSfile: type.h,v $
==============================================================================*/

