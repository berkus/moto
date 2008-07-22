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

   $RCSfile: stacktrace.c,v $   
   $Revision: 1.15 $
   $Author: dhakim $
   $Date: 2003/02/27 22:27:58 $
 
==============================================================================*/
#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "stacktrace.h"
#include "memsw.h"
#include "excpfn.h"
#include "stringbuffer.h"
#include "stack.h"

/************** PROCESS GLOBALS ***************/
StackTraceItem stacktrace[4096];
int strace_size = 0;
/*************************************/

void
stacktrace_init()
{
   strace_size = 0;
   stacktrace_push(NULL, 0, "<main>", ST_FUNCTION);
}

inline void
stacktrace_push(char *file, int line, char *fn, int flags)
{
   StackTraceItem entry;

   entry.file = file;
   entry.function = fn;
   entry.line = line;
   entry.flags = flags;

	stacktrace[strace_size] = entry;
   strace_size ++;
}

inline int32_t stacktrace_ipop(int32_t val){
   stacktrace_pop();
   return val;
}

inline unsigned char stacktrace_bpop(unsigned char val){
   stacktrace_pop();
   return val;
}

inline int64_t stacktrace_lpop(int64_t val){
   stacktrace_pop();
   return val;
}

inline float stacktrace_fpop(float val){
   stacktrace_pop();
   return val;
}

inline double stacktrace_dpop(double val){
   stacktrace_pop();
   return val;
}

inline char stacktrace_cpop(char val){
   stacktrace_pop();
   return val;
}

inline signed char stacktrace_ypop(signed char val){
   stacktrace_pop();
   return val;
}

inline void* stacktrace_rpop(void *val){
   stacktrace_pop();
   return val;
}

inline void stacktrace_vpop(){
   stacktrace_pop();
}

inline void
stacktrace_pop()
{
   if (strace_size > 0)
   	strace_size --;
}

char *
stacktrace_toString(Exception *e)
{
   StringBuffer *sb = buf_createDefault();
   StackTraceItem *entry;
   StackTraceItem *nextEntry;
   char *r = NULL;
   int i;

   entry = &stacktrace[strace_size-1];
   buf_printf(sb,"%s thrown in %s(%s:%d)\n", e->t, entry->function, e->file, e->line);

   for(i = strace_size-1; i > 0; i--){
      entry       = &stacktrace[i];
      nextEntry   = &stacktrace[i-1];

      buf_printf(sb, "   %s %s %s(%s:%d)\n", "called from",
         nextEntry->flags&(ST_METHOD|ST_CONSTRUCTOR)?"method":"function",
         nextEntry->function, entry->file, entry->line);
   }

   r = buf_toString(sb);
   buf_free(sb);

   return r;
}

int
stacktrace_size()
{
   return strace_size;
}

void
stacktrace_cleanup()
{
    strace_size = 0;
}


