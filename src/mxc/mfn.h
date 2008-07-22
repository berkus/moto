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

   $RCSfile: mfn.h,v $   
   $Revision: 1.5 $
   $Author: dhakim $
   $Date: 2003/02/27 19:47:00 $
 
==============================================================================*/
#ifndef __MFN_H
#define __MFN_H

#include "enumeration.h"
#include "vector.h"

#include "fnarg.h"

typedef struct mFunction {
	char *rtype;
	char *classn;
	char *fname;
	// char *signature;
	char *cname;
	Vector *argv;
    char tracked;
	char *comment; 
} MFN;

MFN *mfn_create();
void mfn_free(MFN *p);

char *mfn_symbolName(MFN *p);
void mfn_addArg(MFN *p, FNArg *arg);
int mfn_argc(MFN *p);
Enumeration *mfn_args(MFN *p);
void mfn_dump(MFN *p);

#endif

/*=============================================================================
// end of file: $RCSfile: mfn.h,v $
==============================================================================*/

