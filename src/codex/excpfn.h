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

   $RCSfile: excpfn.h,v $   
   $Revision: 1.12 $
   $Author: shayh $
   $Date: 2002/12/31 12:36:50 $
 
==============================================================================*/
#ifndef __EXCPFN_H
#define __EXCPFN_H

#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <sys/ipc.h>

inline void *emalloc(size_t size);
inline void *ecalloc(size_t nmemb,size_t size);
inline void *erealloc(void* ptr, size_t size);

inline char *estrdup(char* src);
inline int estrcmp(const char *s1, const char *s2);
inline int estrcasecmp(const char *s1, const char *s2);
inline int estrncmp(const char *s1, const char *s2, size_t n);
inline int estrncasecmp(const char *s1, const char *s2, size_t n);

#endif

/*=============================================================================
// end of file: $RCSfile: excpfn.h,v $
==============================================================================*/

