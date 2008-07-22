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

   $RCSfile: timer.c,v $   
   $Revision: 1.4 $
   $Author: dhakim $
   $Date: 2001/03/04 18:02:58 $
 
==============================================================================*/
#include <config.h>

#include "timer.h"

static struct timeval tv;
static long msecs = 0;
static long secs = 0;

void startTimer(){
   gettimeofday(&tv,0);
   msecs= tv.tv_usec;
   secs = tv.tv_sec;
}

float readTimer(){
   struct timeval rotv;

   gettimeofday(&rotv,0);
   return (float)(rotv.tv_sec - secs) +
          (((float)rotv.tv_usec-msecs)/(float)1000000);
}

/*=============================================================================
// end of file: $RCSfile: timer.c,v $
==============================================================================*/

