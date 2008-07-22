/*=============================================================================

   Copyright (C) 2002 David Hakim.
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

   $RCSfile: stacktrace.h,v $   
   $Revision: 1.10 $
   $Author: dhakim $
   $Date: 2003/02/27 22:27:58 $
 
==============================================================================*/

#ifndef __STACKTRACE_H
#define __STACKTRACE_H

#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "exception.h"
#include <sys/types.h>

#define ST_STACK_SIZE   25
#define ST_ENTRY_LEN    100

/* Flags for StackTrace. Cheaper than using multiple stacks */
#define ST_FUNCTION     0x01
#define ST_METHOD       0x02
#define ST_CONSTRUCTOR  0x04
#define ST_EXTENSION    0x08
#define ST_HASARGS      0x16
#define ST_POPPED       0x32

#define ST_PUSH(FUNCN,FLAGS) stacktrace_push(__FILE__,__LINE__,FUNCN,FLAGS)

typedef struct stackTraceItem {
   char* file;
   char* function;
   int line;
   int flags;
} StackTraceItem;

void stacktrace_init();
char* stacktrace_toString(Exception *e);
int stacktrace_size();
void stacktrace_cleanup();

inline void stacktrace_push(char *file, int line, char *fn, int flags);
inline void stacktrace_pop();

inline int32_t stacktrace_ipop(int32_t val);
inline unsigned char stacktrace_bpop(unsigned char val);
inline int64_t stacktrace_lpop(int64_t val);
inline float stacktrace_fpop(float val);
inline double stacktrace_dpop(double val);
inline char stacktrace_cpop(char val);
inline signed char stacktrace_ypop(signed char val);
inline void* stacktrace_rpop(void *val);
inline void stacktrace_vpop();
 
#endif

