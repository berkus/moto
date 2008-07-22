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

   $RCSfile: magic.h,v $   
   $Revision: 1.3 $
   $Author: kocienda $
   $Date: 2000/04/30 23:10:01 $
 
==============================================================================*/
#ifndef __MAGIC_H
#define __MAGIC_H

#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

enum {
   U_MAGIC = 0x631a1a63, /* a byte-order palindrome */
};

/* the magic struct */
typedef struct magic {
   void (*free_fn)(void*);         /* The free function */
   unsigned int (*hash_fn)(void*); /* The hash code function */
   char *(*name_fn)(void);         /* The type name function */
   char *(*string_fn)(void*);      /* The toString function */
   char *(*info_fn)(void*);        /* The info function */
} Magic;

/* the dummy struct for casting void pointers before dereferencing them */
typedef struct magicalStruct {
   int u_magic;      /* The Magic number */
   Magic *magic;     /* The Magic struct */
} MagicalStruct;

inline void *magic_check(void *p);

unsigned int magic_type(void *p);
void magic_free(void *p);
char *magic_name(void *p);
unsigned int magic_hash(void *p);
char *magic_toString(void *p);
char *magic_info(void *p);

#endif

/*=============================================================================
// end of file: $RCSfile: magic.h,v $
==============================================================================*/

