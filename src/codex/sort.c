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

   $RCSfile: sort.c,v $   
   $Revision: 1.7 $
   $Author: dhakim $
   $Date: 2003/03/08 02:32:22 $
 
==============================================================================*/
#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "sort.h"

static void quick_sort_fn(void **a, int l, int r, Function* cmp) {
   int lt = l;
   int rt = r;
   int pt;
   void *pivot;

   if (lt >= rt) {
      return;
   }
   
   pt = lt;
   pivot = *(a + pt); 
   
   while (lt < rt) {
      while(lt < rt && 
      	 ( cmp->flags & F_INTERPRETED ? 
			ifunc_icall(cmp,"OO", *(a + lt), pivot) : 
			((int(*)(Function*,void*,void*))cmp->fn)(cmp, *(a + lt), pivot)  )
      	 <= 0) {
         lt++;
      }
      while(
      	 ( cmp->flags & F_INTERPRETED ? 
			ifunc_icall(cmp,"OO", *(a + rt), pivot) : 
			((int(*)(Function*,void*,void*))cmp->fn)(cmp, *(a + rt), pivot)  )
      	 > 0) {
         rt--;
      }
      if (lt < rt) {
         void *tmp = *(a + lt); 
         *(a + lt) = *(a + rt);
         *(a + rt) = tmp;
      }
   }

	*(a+l) = *(a+rt);
	*(a+rt) = pivot;
	
   quick_sort_fn(a, l, rt - 1, cmp);
   quick_sort_fn(a,  rt + 1 , r, cmp);

} 
   
void quick_sort(void **a, int size, Function* cmp) {
   quick_sort_fn(a, 0, size - 1, cmp);
}

void bubble_sort(void **a, int size, int (*cmp)(const void *, const void *)) {
   int i;
   int j;
   for (i = 0; i < size - 1; i++) {
      for (j = i + 1; j < size; j++) {
         if (cmp(*(a + j), *(a + i)) < 0) {
            void *swap = *(a + i);
            *(a + i) = *(a + j);
            *(a + j) = swap;
         }
      }
   }
}  

/*=============================================================================
// end of file: $RCSfile: sort.c,v $
==============================================================================*/

