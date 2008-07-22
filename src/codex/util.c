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

   $RCSfile: util.c,v $   
   $Revision: 1.9 $
   $Author: dhakim $
   $Date: 2002/12/05 02:40:48 $
 
==============================================================================*/
#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>

#include "memsw.h"

#include "util.h"
#include "exception.h"

inline unsigned int string_hash(void *p){
   char *s = (char *)p;
   unsigned int h = 0;
   int i = 0;
   if(p==NULL){
      THROW_D("NullPointerException");
   }
   while (s[i] != '\0') {
      h = h * 31 + s[i];
      i++;
   }
   return h;
}

inline unsigned int void_hash(void *p){
   unsigned int n = (int)p;
   return n;
} 

inline int void_comp(const void *number1, const void *number2){
   int n1 = (int)number1;
   int n2 = (int)number2;
   return n1 - n2;
}

inline unsigned int int_hash(void *p){
   unsigned int n = *((int *)p);
   return n;
}

inline int int_comp(const void *p1, const void *p2){
   int n1 = *((int *)p1);
   int n2 = *((int *)p2);
   return n1 - n2;
} 

/*=============================================================================
// end of file: $RCSfile: util.c,v $
==============================================================================*/

