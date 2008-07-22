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

   $RCSfile: meta.c,v $   
   $Revision: 1.7 $
   $Author: dhakim $
   $Date: 2002/02/23 03:14:58 $
 
==============================================================================*/
#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#include "meta.h"

#define STRFTIME_MAX 128

void moto_buildTimeStamp(char *s) {
   strcpy(s, __TIME__);
   strcat(s, " ");
   strcat(s, __DATE__);
}

void moto_curTimeStamp(char *s) {
   time_t t;
   struct tm *tm;
   time(&t);
   tm = localtime(&t);
   strftime(s, STRFTIME_MAX, "%H:%M:%S %b %d %Y", tm);
}

/*=============================================================================
// end of file: $RCSfile: meta.c,v $
==============================================================================*/

