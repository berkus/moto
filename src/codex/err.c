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

   $RCSfile: err.c,v $   
   $Revision: 1.9 $
   $Author: dhakim $
   $Date: 2001/03/04 18:02:54 $
 
==============================================================================*/
#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <signal.h>
#include <string.h>

#include "err.h"
#include "log.h"

#include "symboltable.h"

static Vector *errlist;
static SymbolTable *handlerlist;

static char unkfmt[] = "Uncaught error: %s\n";
static char errfmt[] = "%s at %s:%d\n";
static Err errs[] = {
	{"FatalError", errfmt, NULL},
	{"NullPointer", errfmt, NULL},
	{"AllocationFailure", errfmt, NULL},
	{"ArrayBoundsError", errfmt, NULL},
	{"EmptyStack", errfmt, NULL},
	{"ForkFailed", errfmt, NULL},
	{"ExecFailed", errfmt, NULL},
	{"IllegalArgument", errfmt, NULL},
	{"NoSuchElement", errfmt, NULL},
	{"LibraryError", errfmt, NULL},
	{"IOError", errfmt, NULL},
	{"FileNotFound", errfmt, NULL},
	{"UnexpectedEOF", errfmt, NULL},
	{"NetworkError", errfmt, NULL},
	{"ConnectionRefused", errfmt, NULL},
	{"DynamicLoadingError", errfmt, NULL},
	{"DynamicLoadingOpenError", errfmt, NULL},
	{"DynamicLoadingSymbolError", errfmt, NULL},
	{"DynamicLoadingCloseError", errfmt, NULL},
   { NULL },
};

static int err_handle(int type, char *code, va_list ap);

void err_init() {
   errlist = vec_createDefault();
   handlerlist = stab_createDefault();
   err_register((Err *)errs);
}

void err_free() {
   vec_free(errlist);
   stab_free(handlerlist);
}


void err_register(Err *errv) {
   vec_add(errlist, errv);
}

void err_registerHandler(char *name, void (*handler)(char *fmt, va_list ap)) {
   stab_put(handlerlist, name, handler);
}

void err_debug(char *code, ...) {
   int handled = 0;
   va_list ap;
   va_start(ap, code);
   handled = err_handle(LOG_DEBUG, code, ap);      
   va_end(ap);
   if (!handled) {
      log_error(__FILE__, unkfmt, code);      
   }
}

void err_info(char *code, ...) {
   int handled = 0;
   va_list ap;
   va_start(ap, code);
   handled = err_handle(LOG_INFO, code, ap);      
   va_end(ap);
   if (!handled) {
      log_error(__FILE__, unkfmt, code);      
   }
}

void err_warning(char *code, ...) {
   int handled = 0;
   va_list ap;
   va_start(ap, code);
   handled = err_handle(LOG_WARNING, code, ap);      
   va_end(ap);
   if (!handled) {
      log_error(__FILE__, unkfmt, code);      
   }
}

void err_error(char *code, ...) {
   int handled = 0;
   va_list ap;
   va_start(ap, code);
   handled = err_handle(LOG_ERROR, code, ap);      
   va_end(ap);
   if (!handled) {
      log_error(__FILE__, unkfmt, code);      
   }
}

void err_fatal(char *code, ...) {
   int handled = 0;
   va_list ap;
   va_start(ap, code);
   handled = err_handle(LOG_FATAL, code, ap);      
   va_end(ap);
   if (!handled) {
      log_error(__FILE__, unkfmt, code);      
   }
   raise(SIGABRT);
}

static int err_handle(int type, char *code, va_list ap) {
   int handled = 0;
   int size = vec_size(errlist);
   int i;
   for (i = 0; i < size; i++) {
      Err *e = (Err *)vec_get(errlist, i);
      int j = 0;
      for (; e[j].code != NULL; j++) {
         if (strcmp(code, e[j].code) == 0) {
            if (e[j].handler != NULL) {
               void (*handler)(char *fmt, va_list ap) = 
                  (void (*)(char *fmt, va_list ap))stab_get(handlerlist, e[j].handler);
               if (handler != NULL) {
                  log_debug(__FILE__, "Calling custom error handler: %s\n", e[j].handler);
                  log_debug(__FILE__, "Custom error format: %s\n", e[j].fmt);
                  handler(e[j].fmt, ap);
               }
               else {
                  log_write(type, __FILE__, e[j].fmt, ap);      
               }
            }
            else {
               log_write(type, __FILE__, e[j].fmt, ap);      
            }
            handled = 1;
            break;
         }
      }
   }
   return handled;
}

/*=============================================================================
// end of file: $RCSfile: err.c,v $
==============================================================================*/

