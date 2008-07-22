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

   $RCSfile: magic.c,v $   
   $Revision: 1.6 $
   $Author: kocienda $
   $Date: 2000/04/30 23:10:01 $
 
==============================================================================*/
#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>

#include "memsw.h"

#include "sharedmem.h"
#include "magic.h"
#include "stringbuffer.h"
#include "log.h"

inline void *magic_check(void *p) {
   register char *pc = (char *)p;   
   if (pc[0] == 0x63 && pc[1] == 0x1a && pc[2] == 0x1a && pc[3] == 0x63) {
      return p;
   }
   return NULL;
}

void magic_free(void *p) {
   MagicalStruct *ms = (MagicalStruct *)p;
   if(magic_check(p) && ms->magic->free_fn != NULL) {
      ms->magic->free_fn(p);
   }
   else {
      /* just call "regular" free */
      free(p);
   }
}

unsigned int magic_hash(void *p) {
   unsigned int result = 0;
   MagicalStruct *ms = (MagicalStruct *)p;
   if(magic_check(p) && ms->magic->hash_fn != NULL) {
      result = ms->magic->hash_fn(p);
   }
   return result;
}

char *magic_name(void *p) {
   char *result;
   MagicalStruct *ms = (MagicalStruct *)p;
   if(magic_check(p) && ms->magic->name_fn != NULL) {
      result = ms->magic->name_fn();
   }
   else {
      result = "[unnamed]";
   }
   return result;
}

char *magic_toString(void *p) {
   char *result;
   MagicalStruct *ms = (MagicalStruct *)p;
   if(magic_check(p) && ms->magic->string_fn != NULL) {
      result = ms->magic->string_fn(p);
   }
   else {
      StringBuffer *buf = buf_createDefault();
      buf_puts(buf, "magic");
      result = buf_toString(buf);
      buf_free(buf);
   }
   return result;
}

char *magic_info(void *p) {
   char *result;
   MagicalStruct *ms = (MagicalStruct *)p;
   if(magic_check(p) && ms->magic->info_fn != NULL) {
      result = ms->magic->info_fn(p);
   }
   else {
      StringBuffer *buf = buf_createDefault();
      buf_printf(buf, "%s: %p", magic_name(p), p);
      result = buf_toString(buf);
      buf_free(buf);
   }
   return result;
}

/*=============================================================================
// end of file: $RCSfile: magic.c,v $
==============================================================================*/

