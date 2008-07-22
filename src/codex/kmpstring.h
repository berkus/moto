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

   $RCSfile: kmpstring.h,v $   
   $Revision: 1.5 $
   $Author: dhakim $
   $Date: 2001/04/21 20:07:45 $
 
==============================================================================*/
#ifndef __KMPSTRING_H
#define __KMPSTRING_H

#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

typedef struct kmpstring {
   size_t length;
   char* string;
   int* next;
} KMPString;

KMPString* kmp_create(char* string);
void kmp_free(KMPString* kmp);
int kmp_indexOf(KMPString* kmp, char* string);
char* kmp_strstr(KMPString* kmp, char* string);
char* kmp_memstr(KMPString* kmp, char* string,size_t length);
void kmp_generateCode(KMPString* kmp);

#endif

/*=============================================================================
// end of file: $RCSfile: kmpstring.h,v $
==============================================================================*/

