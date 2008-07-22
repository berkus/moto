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

   $RCSfile: stringbuffer.h,v $   
   $Revision: 1.8 $
   $Author: dhakim $
   $Date: 2003/07/06 17:58:43 $
 
==============================================================================*/
#ifndef __STRINGBUFFER_H
#define __STRINGBUFFER_H

#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdarg.h>

#include "integer.h"
#include "mxarr.h"

/*-----------------------------------------------------------------------------
	constants
-----------------------------------------------------------------------------*/

enum {
	/* the default size for a StringBuffer */
   BUF_DEFAULT_CAP = 1024,
   /* the maximum limit for formatted output to a StringBuffer */
   BUF_PRINTF_MAXLEN = 8192,
};

/*-----------------------------------------------------------------------------
	typedefs
-----------------------------------------------------------------------------*/

/* the StringBuffer struct */
typedef struct {
   char *data;		/* the pointer to the string buffer */
   int size;      /* # of bytes in buffer */
   int cap;     	/* current allocated size of buffer */
} StringBuffer;

/*-----------------------------------------------------------------------------
	memory management
-----------------------------------------------------------------------------*/

/* dynamically allocates a StringBuffer */
StringBuffer *buf_create(int size);

/* dynamically allocates a StringBuffer, using the default size */
StringBuffer *buf_createDefault();

/* frees a StringBuffer */
void buf_free(StringBuffer *d);

/* grows a buffer to a new size */
void buf_grow(StringBuffer *d, int cap);

/* makes a copy of a StringBuffer */
StringBuffer *buf_dup(StringBuffer *d);

/*-----------------------------------------------------------------------------
	StringBufferfer data manipulation
-----------------------------------------------------------------------------*/

/* clears a StringBuffer */
void buf_clear(StringBuffer *d);

/* puts an arbitrary data block into a StringBuffer */
void buf_put(StringBuffer *d, const void *data, int size);

/* appends a string into a StringBuffer */
void buf_puts(StringBuffer *d, const char *str);

/* appends an int into a StringBuffer */
void buf_puti(StringBuffer *d, int n);

/* appends a 64-bit integer into a StringBuffer */
void buf_putl(StringBuffer *d, int64_t n);

/* appends a float into a StringBuffer */
void buf_putf(StringBuffer *d, float n);

/* appends a double into a StringBuffer */
void buf_putd(StringBuffer *d, double n);

/* appends a pointer address into a StringBuffer */
void buf_putp(StringBuffer *d, void *p);

/* appends a char into a StringBuffer */
void buf_putc(StringBuffer *d, char c);

/* appends a "true" into a StringBuffer if c>0 "false" otherwise */
void buf_putb(StringBuffer *d, unsigned char c);

/* appends a byte into a StringBuffer */
void buf_puty(StringBuffer *d, signed char c);

/* appends a character array to a StringBuffer */
void buf_putca(StringBuffer *d, UnArray *x);

/* appends a byte array to a StringBuffer */
void buf_putya(StringBuffer *d, UnArray *x);

/* formatted output to a StringBuffer */
int buf_printf(StringBuffer *d, const char *format, ...);

/* formatted output to a StringBuffer */
int buf_vprintf(StringBuffer *d, const char *format, va_list ap);

/* concats two StringBuffers together */
void buf_cat(StringBuffer *dest, StringBuffer *src);

/* gets the size of the buffer */
int buf_size(StringBuffer *d);

/* gets the character at the specified index */
char buf_charAt(StringBuffer *d, int index);

/* gets the char character in the buffer */
char buf_firstChar(StringBuffer *d);

/* gets the last character in the buffer */
char buf_lastChar(StringBuffer *d);

/* replaces every instance of oldc with newc */
void buf_replaceChar(StringBuffer *d, char oldc, char newc);

/* trims the text in the buffer */
void buf_trim(StringBuffer *d);

/* trims tabs and spaces off the text in the buffer */
void buf_trimLine(StringBuffer *d);

/*----------------------------------------------------------------------------
   StringBuffer comparisons
-----------------------------------------------------------------------------*/

int buf_comp(StringBuffer *d, StringBuffer *d1);

/*----------------------------------------------------------------------------
	StringBufferfer output
-----------------------------------------------------------------------------*/

/* returns a StringBuffer as a string, without allocating new memory */
char *buf_data(StringBuffer *d);

/* returns a StringBuffer as a string, allocating new memory for the returned 
 * string 
 */
char *buf_toString(StringBuffer *d);

/* writes a StringBuffer to stdout...helpful for debugging */
int buf_dump(StringBuffer *d);

#endif

/*=============================================================================
// end of file: $RCSfile: stringbuffer.h,v $
==============================================================================*/

