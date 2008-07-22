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

   $RCSfile: excpfn.c,v $   
   $Revision: 1.22 $
   $Author: shayh $
   $Date: 2002/12/31 12:36:50 $
 
==============================================================================*/
#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "excpfn.h"
#include "memsw.h"
#include "exception.h"


inline void *emalloc(size_t size) {
   void *p = malloc(size);
   if (p == NULL) {
      THROW_D("AllocationFailureException");
   }
   return p;
}

inline void *ecalloc(size_t nmemb,size_t size) {
   void *p = calloc(nmemb,size);
   if (p == NULL) {
      THROW_D("AllocationFailureException");
   }
   return p;
}

inline void *erealloc(void* ptr, size_t size) {
   void *p = realloc(ptr, size);
   if (p == NULL && size != 0) {
      THROW_D("AllocationFailureException");
   }
   return p;
}

inline char *estrdup(char* src){
   char* dest;
   if (src == NULL) {
      THROW_D("NullPointerException");
   }
   dest = (char*)emalloc(strlen(src)+1);
   strcpy (dest,src);
   return dest;
}

inline int estrcmp(const char *s1, const char *s2){
   if (s1 == NULL || s2 == NULL) {
      THROW_D("NullPointerException");
   }
   return strcmp (s1,s2);
}

inline int estrcasecmp(const char *s1, const char *s2){
   if (s1 == NULL || s2 == NULL) {
      THROW_D("NullPointerException");
   }
   return strcasecmp (s1,s2);
}

inline int estrncasecmp(const char *s1, const char *s2, size_t n){
   while (n > 0 && toupper(*s1) == toupper(*s2)){
     if (*s1 == '\0')
       return 0;

     s1++; 
     s2++;
     n--;
   }
   if (n == 0)
     return 0;

   return toupper(*s1) - toupper(*s2);
}

inline int estrncmp(const char *s1, const char *s2,size_t n){
   if (s1 == NULL || s2 == NULL) {
      THROW_D("NullPointerException");
   }
   return strncmp (s1,s2,n);
}

/*=============================================================================
// end of file: $RCSfile: excpfn.c,v $
==============================================================================*/

