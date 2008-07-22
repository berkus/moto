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

   $RCSfile: err.h,v $   
   $Revision: 1.3 $
   $Author: kocienda $
   $Date: 2000/04/30 23:10:01 $
 
==============================================================================*/
#ifndef __ERR_H
#define __ERR_H

#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdarg.h>

#include "vector.h"

#define err_assert(expr, call) ((void) ((expr) ? 0 : (call))) 
#define err_d(code) (err_debug((code), (code), __FILE__, __LINE__)) 
#define err_i(code) (err_info((code), (code), __FILE__, __LINE__)) 
#define err_w(code) (err_warning((code), (code), __FILE__, __LINE__)) 
#define err_e(code) (err_error((code), (code), __FILE__, __LINE__)) 
#define err_f(code) (err_fatal((code), (code), __FILE__, __LINE__)) 
   
typedef void (*err_handler)(char *fmt, va_list ap);

typedef struct err {
	char *code;
	char *fmt;
   char *handler;
} Err;

void err_init();
void err_free();

void err_debug(char *code, ...);
void err_info(char *code, ...);
void err_warning(char *code, ...);
void err_error(char *code, ...);
void err_fatal(char *code, ...);

void err_register(Err *errv);
void err_registerHandler(char *name, void (*handler)(char *fmt, va_list ap));

#endif

/*=============================================================================
// end of file: $RCSfile: err.h,v $
==============================================================================*/

