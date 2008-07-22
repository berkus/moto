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

   $RCSfile: integer.h,v $   
   $Revision: 1.8 $
   $Author: dhakim $
   $Date: 2002/02/27 05:33:52 $
 
==============================================================================*/
#ifndef __INTEGER_H
#define __INTEGER_H

#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_STDINT_H
#  include <stdint.h>
#   ifndef INT32_MAX
       /* Certain FreeBSD installations don't define these */
#      define INT32_MAX 2147483647
#      define INT64_MAX 9223372036854775807LL
#      define INT32_MIN (-INT32_MAX-1)
#      define INT64_MIN (-INT64_MAX-1)
#   endif
#else
#   ifdef SOLARIS_OS
#      include <sys/types.h>
#      include <sys/int_limits.h>
#   endif /* SOLARIS_OS */
#endif 

#ifdef HAVE_INTTYPES_H
#   include <inttypes.h>
#   ifndef INT32_MAX
       /* Certain FreeBSD installations don't define these */
#      define INT32_MAX 2147483647
#      define INT64_MAX 9223372036854775807LL
#      define INT32_MIN (-INT32_MAX-1)
#      define INT64_MIN (-INT64_MAX-1)
#   endif
#endif

#endif

/*=============================================================================
// end of file: $RCSfile: integer.h,v $
==============================================================================*/

