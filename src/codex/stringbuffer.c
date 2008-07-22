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

   $RCSfile: stringbuffer.c,v $   
   $Revision: 1.16 $
   $Author: dhakim $
   $Date: 2003/02/01 04:58:59 $
 
==============================================================================*/
#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "memsw.h"

#include "log.h"
#include "exception.h"
#include "excpfn.h"

#include "stringbuffer.h"

/*-----------------------------------------------------------------------------
   memory management
-----------------------------------------------------------------------------*/

/* dynamically allocates a StringBuffer */
StringBuffer *
buf_create(int cap) {
   StringBuffer *d;
   d = (StringBuffer *)emalloc(sizeof(StringBuffer));
   if (cap <= 0) {
      cap = BUF_DEFAULT_CAP;
   }
   d->cap = cap;
   d->data = (char *)emalloc(d->cap + 1);
   d->size = 0;
   return d;
}

/* dynamically allocates a StringBuffer, using the default size */
StringBuffer *
buf_createDefault() {
   return buf_create(BUF_DEFAULT_CAP);
}

/* makes a copy of a StringBuffer */
StringBuffer *
buf_dup(StringBuffer *d) {
   StringBuffer *d1 = buf_create(d->cap);
   d1->size = d->size;
   memcpy(d1->data, d->data, d->size);
   excp_assert(d1->data != NULL, THROW_D("AllocationFailureException"));
   return d1;
}

/* frees a StringBuffer allocated */
void 
buf_free(StringBuffer *d) {
   free(d->data);
   free(d);
}

/* grows a buffer to a new size */
inline void 
buf_grow(StringBuffer *d, int cap) {
   if (cap >= d->cap) {
      int capt2 = d->cap * 2 + 1;
      int newcap = cap > capt2 ? cap : capt2;
      void *newmem = realloc(d->data, newcap + 1);
      if (newmem == NULL) {
         buf_free(d);
         THROW_D("AllocationFailureException");
      }
      
      d->data = newmem;
      d->cap = newcap;
   }
}

/*-----------------------------------------------------------------------------
   StringBuffer data manipulation
-----------------------------------------------------------------------------*/

/* clears a StringBuffer */
void 
buf_clear(StringBuffer *d) {
   d->size = 0;
}

/* puts an arbitrary data block into a StringBuffer */
inline void 
buf_put(StringBuffer *d, const void *data, int size) {
   buf_grow(d, (d->size) + size);
   memcpy(d->data + d->size, data, size);
   d->size += size;
}

/* appends a string into a StringBuffer */
inline void 
buf_puts(StringBuffer *d, const char *str) {
   if(str == NULL)
      buf_put(d,"null",4);
   else
      buf_put(d, str, strlen(str));
}

/* appends an int into a StringBuffer */
void 
buf_puti(StringBuffer *d, int n) {
   char s[32],rs[32];
   int i=n,j = 0;

   if(n<0) {
   	do{
      	s[j++]='0'-i%10;
      	i/=10;
   	} while (i!=0);
   	s[j++] = '-';
   } else {
      do{
         s[j++]='0'+i%10;
         i/=10;
      } while (i!=0);
   }
   
   for(i=0;i<=j;i++)
      rs[i]=s[j-i-1];
   rs[i]='\0';

   buf_put(d,rs,j);
}

/* appends a 64-bit integer into a StringBuffer */
void 
buf_putl(StringBuffer *d, int64_t n) {
   char s[32];
   sprintf(s, "%lld", n);
   buf_puts(d, s);
}

/* appends a float into a StringBuffer */
void 
buf_putf(StringBuffer *d, float n) {
   char s[64];
   sprintf(s, "%f", n);
   buf_puts(d, s);
}

/* appends a double into a StringBuffer */
void 
buf_putd(StringBuffer *d, double n) {
   char s[128];
   sprintf(s, "%e", n);
   buf_puts(d, s);
}

/* appends a char into a StringBuffer */
void 
buf_putc(StringBuffer *d, char c) {
   char s[1];
   s[0] = c;
   buf_put(d, s, 1);
}

/* appends a "true" into a StringBuffer if c>0 "false" otherwise */
void 
buf_putb(StringBuffer *d,unsigned char c) {
   if(c)
      buf_puts(d,"true");
   else
      buf_puts(d,"false");
}

/* appends a byte into a StringBuffer */
void 
buf_puty(StringBuffer *d, signed char c) {
   buf_puti(d, (int)c);
}

/* appends a pointer address into a StringBuffer */
void 
buf_putp(StringBuffer *d, void *p) {
   char s[32];
   sprintf(s, "%p", p);
   buf_puts(d, s);
}

/* appends a character array into a StringBuffer */
void 
buf_putca(StringBuffer *d, UnArray *x) {
   if(x == NULL)
      buf_put(d,"null",4);
   else
      buf_put(d,x->ca.data,x->meta.length);
}

/* appends a byte array into a StringBuffer */
void 
buf_putya(StringBuffer *d, UnArray *x) {
   if(x == NULL)
      buf_put(d,"null",4);
   else
      buf_put(d,x->ca.data,x->meta.length);
}

/* concats two StringBuffers together */
void 
buf_cat(StringBuffer *dest, StringBuffer *src) {
   buf_put(dest, src->data, src->size);
}

/* formatted output to a StringBuffer */
#ifdef __GNUC__ 
#ifdef NO_LEN_VSPRINTF
int 
buf_printf(StringBuffer *d, const char *format, ...) {
   int r;
   va_list args;
   buf_grow(d, (d)->size + BUF_PRINTF_MAXLEN);
   fflush(stdout);
   va_start(args, format);
   r = vsnprintf((d)->data + (d)->size, (BUF_PRINTF_MAXLEN - 1), format, args);
   va_end(args);
   (d)->size += strlen((d)->data);
   return r;
}
#else
int 
buf_printf(StringBuffer *d, const char *format, ...) {
   int r;
   va_list args;
   buf_grow(d, (d)->size + BUF_PRINTF_MAXLEN);
   fflush(stdout);
   va_start(args, format);
   r = (d)->size += 
      vsnprintf((d)->data + (d)->size, (BUF_PRINTF_MAXLEN - 1), format, args);
   va_end(args);
   return r;
} 
#endif
#endif

/* formatted output to a StringBuffer */
#ifdef __GNUC__ 
#ifdef NO_LEN_VSPRINTF
int 
buf_vprintf(StringBuffer *d, const char *format, va_list ap) {
   int r;
   buf_grow(d, (d)->size + BUF_PRINTF_MAXLEN);
   fflush(stdout);
   r = vsnprintf((d)->data + (d)->size, (BUF_PRINTF_MAXLEN - 1), format, ap);
   (d)->size += strlen((d)->data);
   return r;
}
#else
int 
buf_vprintf(StringBuffer *d, const char *format, va_list ap) {
   int r;
   buf_grow(d, (d)->size + BUF_PRINTF_MAXLEN);
   fflush(stdout);
   r = (d)->size += 
      vsnprintf((d)->data + (d)->size, (BUF_PRINTF_MAXLEN - 1), format, ap);
   return r;
} 
#endif
#endif

/* gets the size of the buffer */
int 
buf_size(StringBuffer *d) {
   return d->size;
}

/* gets the character at the specified index */
char 
buf_charAt(StringBuffer *d, int index) {
   char c = '\0';
   if (index > 0 && index < d->size) {
      c = d->data[index];
   }
   return c;
}

/* gets the char character in the buffer */
char 
buf_firstChar(StringBuffer *d) {
   char c = '\0';
   if (d->size > 0) {
      c = d->data[0];
   }
   return c;
}

/* gets the last character in the buffer */
char 
buf_lastChar(StringBuffer *d) {
   char c = '\0';
   int size = d->size;
   if (size > 0) {
      c = d->data[size - 1];
   }
   return c;
}

/* replaces every instance of oldc with newc */
void 
buf_replaceChar(StringBuffer *d, char oldc, char newc) {
   int i;
   int size = d->size;
   for (i = 0; i < size; i++) {
      if (d->data[i] == oldc) {
         d->data[i] = newc;   
      }
   }
}

/* trims the text in the buffer */
void 
buf_trim(StringBuffer *d) {
   if (d->size > 0) {
      int size = d->size;
      char *s = d->data;
      int b; /* index of first non-white space char */ 
      int e; /* index of last non-white space char */
      int i; /* a humble counter */
      int run;
      run = 0;
      i = size;
      while (run == 0 && i > 0) {
         char c = s[--i];
         switch (c) {
            case '\n':
            case '\t':
            case '\r':
            case '\f':
            case ' ':
               break;
            default:
               run = 1;
               break;
         }
      }
      e = i + 1;
      run = 0;
      i = 0;
      while (run == 0 && i < e) {
         char c = s[i++];
         switch (c) {
            case '\n':
            case '\t':
            case '\r':
            case '\f':
            case ' ':
               break;
            default:
               run = 1;
               break;
         }
      }
      b = i - 1;
      memmove(d->data, d->data + b, e-b);
      d->size = e-b;
   }
}

/* trims tabs and spaces off the text in the buffer */
void 
buf_trimLine(StringBuffer *d) {
   if (d->size > 0) {
      int size = d->size;
      char *s = d->data;
      int b; /* index of first non-white space char */ 
      int e; /* index of last non-white space char */
      int i; /* a humble counter */
      int run;
      run = 0;
      i = size;
      while (run == 0 && i > 0) {
         char c = s[--i];
         switch (c) {
            case '\t':
            case ' ':
               break;
            default:
               run = 1;
               break;
         }
      }
      e = i + 1;
      run = 0;
      i = 0;
      while (run == 0 && i < e) {
         char c = s[i++];
         switch (c) {
            case '\t':
            case ' ':
               break;
            default:
               run = 1;
               break;
         }
      }
      b = i - 1;
      memmove(d->data, d->data + b, e-b);
      d->size = e-b;
      printf("the trim indices are %d and %d\n", b, e);
   }
}


/*----------------------------------------------------------------------------
   StringBuffer comparisons
-----------------------------------------------------------------------------*/

int buf_comp(StringBuffer *d, StringBuffer *d1) {
   size_t size = d->size > d1->size ? d->size : d1->size;
   return memcmp(d->data, d1->data, size);
}

/*----------------------------------------------------------------------------
   StringBuffer output
-----------------------------------------------------------------------------*/

/* returns a StringBuffer as a string, without allocating new memory */
char *buf_data(StringBuffer *d) {
   int size = d->size;
   /*
   buf_grow(d, size + 1);
   */
   d->data[size] = '\0';
   return d->data;
}

/* returns a StringBuffer as a string, allocating new memory for the returned 
 * string 
 */
char *buf_toString(StringBuffer *d) {
   char *s = (char *)emalloc(d->size + 1);
   memcpy(s, d->data, d->size);
   s[d->size] = '\0';
   return s;
}

/* writes a StringBuffer to stdout...helpful for debugging */
int buf_dump(StringBuffer *d) {
   return write(1, d->data, d->size);
}

/*=============================================================================
// end of file: $RCSfile: stringbuffer.c,v $
==============================================================================*/

