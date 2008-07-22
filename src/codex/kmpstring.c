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

   $RCSfile: kmpstring.c,v $   
   $Revision: 1.10 $
   $Author: dhakim $
   $Date: 2001/04/21 20:07:45 $
 
==============================================================================*/
#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <string.h>

#include "memsw.h"
#include "kmpstring.h"
#include "excpfn.h"

int kmp_indexOf(KMPString* kmp, char* string ){
   int i,j, length = strlen(string);

   for(i=0,j=0;j<kmp->length && i<length;i++,j++)
      while ((j>=0) && (string[i] != kmp->string[j])) j = kmp->next[j];
   if (j==kmp->length) 
      return i-kmp->length;
   return -1;

}

char* kmp_strstr(KMPString* kmp, char* string ){
   int i,j, length = strlen(string);

   for(i=0,j=0;j<kmp->length && i<length;i++,j++)
      while ((j>=0) && (string[i] != kmp->string[j])) j = kmp->next[j];
   if (j==kmp->length)
      return &string[i-kmp->length];
   return 0;
}

char* kmp_memstr(KMPString* kmp, char* string,size_t length){
   int i,j;

   for(i=0,j=0;j<kmp->length && i<length;i++,j++)
      while ((j>=0) && (string[i] != kmp->string[j])) j = kmp->next[j];
   if (j==kmp->length)
      return &string[i-kmp->length];
   return 0;
}

KMPString* kmp_create(char* string){
   int i,j, length = strlen (string);
   KMPString * kmp;

   kmp = (KMPString*)emalloc (sizeof(KMPString));

   kmp->length = length;
   kmp->string = string;
   kmp->next = (int*)emalloc(sizeof(int) * (length + 1));

   kmp->next[0] = -1;

   for(i=0,j=-1;i<length;i++,j++,kmp->next[i]=j)
      while((j>=0) && (string[i] !=string[j])) j = kmp->next[j];

   return kmp;
}

void kmp_free(KMPString* kmp){
   free(kmp->next);
   free(kmp);
}

void kmp_generateCode(KMPString* kmp){
   int i;

   printf ("\ninline char* KMP_indexOf(char* string){");
   printf ("\n   char* s = string - 1 ;");
   printf ("\n   KMP_sm: s++;");

   for (i=0;i<kmp->length;i++){
      printf("\n   KMP_s%d: if (*s='\\0')return 0;if(*s!='%c')goto KMP_s",
         i,kmp->string[i]);
      if(kmp->next[i] == -1)
         printf("m");
      else
         printf("%d", kmp->next[i]);
      printf(";s++;");
   }
   printf ("\n   return s;");
   printf("\n}\n\n");
}

/*=============================================================================
// end of file: $RCSfile: kmpstring.c,v $
==============================================================================*/

