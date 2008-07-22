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

   $RCSfile: mx.h,v $   
   $Revision: 1.5 $
   $Author: dhakim $
   $Date: 2002/10/29 00:21:53 $
 
==============================================================================*/
#ifndef __MX_H
#define __MX_H

#include <stdlib.h>
#include <string.h>

#if defined LINUX_OS
   #include <sys/types.h>
   #include <stdint.h>
#elif defined SOLARIS_OS
   #include <sys/types.h>
   #include <sys/int_limits.h>
#endif

#include "memsw.h"
#include "err.h"
#include "exception.h"
#include "excpfn.h"
#include "mman.h"

#include "motofunction.h"
#include "motoextension.h"
#include "motoerr.h"
#include "motohook.h"
#include "motocodes.h"

#endif

/*=============================================================================
// end of file: $RCSfile: mx.h,v $
==============================================================================*/

