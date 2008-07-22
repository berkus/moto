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

   $RCSfile: exception.h,v $   
   $Revision: 1.20 $
   $Author: dhakim $
   $Date: 2003/01/29 04:58:42 $
 
==============================================================================*/
#ifndef __EXCEPTION_H
#define __EXCEPTION_H

#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <setjmp.h>
#include <stdarg.h>

/*-----------------------------------------------------------------------------
   enums, typedefs, and externs
-----------------------------------------------------------------------------*/


enum {
   TRY_STATE = 0,
   CATCH_STATE = 1,
   FINALLY_STATE = 2,
   END_STATE = 3,
   EXTB_STACK_SIZE = 2048,
   MAX_ETYPE_SIZE = 256,
   MAX_EMSG_SIZE = 4096
};


typedef struct M_exception {
   char *t;
   char *msg; 
   char *file;
   int line;
   char *stacktrace;
} Exception;

typedef struct M_tryBlock {
   jmp_buf jmp;
   Exception *excp;
   int code;  /* Set to 0 if this try block doesn't contain an exception that needs further
   				 handling */
   int state; /* Starts off set to TRY_STATE, gets set to CATCH state if an exception is
   				 caught, transitions to FINALLY state when the finally block is entered */
   int tracesize; /* Records the size of current stack trace when the try block is openned */
} TryBlock;

/* ifndef __EXCEPTION_C */

extern TryBlock excp_try_b[EXTB_STACK_SIZE];
extern int excp_try_i;
extern Exception excp_e;
extern char *excp_file;
extern int excp_line;
extern char excp_type[MAX_ETYPE_SIZE];
extern char excp_message[MAX_EMSG_SIZE];
extern char excp_stacktrace[MAX_EMSG_SIZE];

/*
 * Macro for externing exception definitions 
 */
 
#define excp_extern(e_type) \
	typedef Exception e_type; \
	e_type* e_type ## _createDefault(); \
	e_type* e_type ## _create(char* msg);
	
/* "Built-in" exceptions */
excp_extern(AllocationFailureException)
excp_extern(ArrayBoundsException)
excp_extern(ConnectionRefusedException)
excp_extern(DLCloseException)
excp_extern(DLException)
excp_extern(DLOpenException)
excp_extern(DLSymbolException)
excp_extern(DefaultException)
excp_extern(EmptyQueueException)
excp_extern(EmptyStackException)
excp_extern(ExecFailedException)
excp_extern(FileNotFoundException)
excp_extern(ForkFailedException)
excp_extern(IOException)
excp_extern(IllegalArgumentException)
excp_extern(MathException)
excp_extern(NetException)
excp_extern(NoSuchElementException)
excp_extern(NullPointerException)
excp_extern(NumberFormatException)
excp_extern(SecurityException)
excp_extern(SemaphoreException)
excp_extern(UnexpectedEOFException)
excp_extern(UnsupportedOperationException)
excp_extern(SignalException)
excp_extern(RegexParseException)
excp_extern(RuntimeException)
excp_extern(SQLException)
excp_extern(IllegalConversionException)

/* endif *//* __EXCEPTION_C */

/*-----------------------------------------------------------------------------
   macros
-----------------------------------------------------------------------------*/
/* 
 * macro for conditionally throwing an exception based on an expression's value
 */
#define excp_assert(expr, call) do {if (!(expr)) {call;}}while(0); 

/* 
 * macro for defining exception types 
 */
#define excp_declare(e_type) \
	\
	e_type * e_type ## _createDefault(){ \
		e_type * e = (e_type * )malloc(sizeof(e_type)); \
		e->t = #e_type; \
		e->msg = NULL; \
      e->stacktrace = NULL; \
		return e; \
	} \
	\
	e_type * e_type ## _create(char* msg){ \
		e_type * e = (e_type * )malloc(sizeof(e_type)); \
		e->t = #e_type; \
		e->msg = msg; \
      e->stacktrace = NULL; \
		return e; \
	} 

/* 
 * Convenience macro for referring to current try block 
 */
#define TRYBLK excp_try_b[excp_try_i]

/*
 * macro for throwing exceptions
 */
#define THROW \
   excp_file=__FILE__,\
   excp_line=__LINE__,\
   excp_getFillAndThrow

/*
 * macro for constructing and throwing exceptions without messages
 */
#define THROW_D \
   excp_file=__FILE__,\
   excp_line=__LINE__,\
   excp_getFillAndThrowDefault

/* Macro for starting a try-catch block */
#define TRY { \
     extb_beginTry();\
     if((TRYBLK.code = setjmp(TRYBLK.jmp)) == 0)
          
/* Macro for catching specific exceptions */
#define CATCH(etype) else if (extb_catch(etype))

/* Macro for catching any and all exceptions */
#define CATCH_ALL else if (extb_catchAll())

/* Macro for catching all exceptions but those that start with a specific prefix (FIXME) */		
#define CATCH_ALL_BUT(etype) else if (extb_catchAllBut(etype))
			
/* Macro for executing code both on success and failure */
#define FINALLY if (extb_finally())

/* Macro for ending a try-catch block*/
#define END_TRY extb_endTry(); }

/*-----------------------------------------------------------------------------
   function prototypes
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
   Begin exception try block method prototypes
-----------------------------------------------------------------------------*/

/* 
 * Fuction for throwing an exception object. This method
 *
 * 1) Sets the exception for the current try block
 * 2) longjumps back to the most recent try if there is one
 *    otherwise calls excp_uncaught
 */
 
void extb_throw(Exception *excp);

/*
 * Function called by the TRY macro
 * 
 * 1) Pushes a new TryBlock onto the try block stack
 * 2) Initializes this to try block to the TRY_STATE setting its code to 0
 *		and its exception to NULL.
 */
void extb_beginTry();

/* 
 * Function called by the CATCH macro
 *
 * Returns true iff
 * 1) The current TryBlock contains an exception that matches etype 
 * 2) The current TryBlock's state is set to TRY_STATE
 *
 * In this case the following effects will also occur
 * 1) The current TryBlock's code will be set to 0
 * 2) The current TryBlock's state will be set to CATCH_STATE
 */
 
int extb_catch(char* etype);

/* 
 * Function called by the CATCH_ALL macro
 *
 * Returns true iff
 * 1) The current TryBlock's state is set to TRY_STATE 
 *
 * In this case the following effects will also occur
 * 1) The current TryBlock's code will be set to 0
 * 2) The current TryBlock's state will be set to CATCH_STATE
 */
 
int extb_catchAll();
int extb_catchAllBut(char* etype);

/*
 * Function called by the FINALLY macro
 *
 * Returns true iff
 * 1) The current TryBlock's state is set to TRY_STATE or CATCH_STATE
 *
 * In this case the following effect will also occur
 * 1) The current TryBlock's state is set to FINALLY_STATE
 */
 
int extb_finally();

/* 
 * Function called by the END_TRY macro
 *
 * Function called at the end of a try-catch block 
 * 1) Pops off the current TryBlock from the try block stack
 * 2) If that try block contains an exception which has not been handled
 *    2.1) Then it gets re-thrown if there was a parent try block
 *    2.2) Otherwise excp_uncaught gets called
 */
 
void extb_endTry();

/*-----------------------------------------------------------------------------
   End exception try block method prototypes
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
   Begin exception method prototypes
-----------------------------------------------------------------------------*/


/* 
 * Function to prepare for throwing an exception object. This method
 * 1) Sets the filename and line# in the exception being thrown
 * 2) Calls extb_throw
 */
 
void excp_marshal( Exception *excp, char* file, int line);

/* Fuction called when an exception is not caught */
void excp_uncaught(Exception* excp);

/* Creates an exception using the given information */
Exception *excp_fill(Exception* e, char* etype, char *s, ...) ;

void excp_getFillAndThrow( char* etype , char*s, ...) ;

void excp_getFillAndThrowDefault( char* etype ) ;

/* Accessor for the static process exception */
Exception *excp_getProcessException();

/* Returns the caught exception */
Exception *excp_getCurrentException();

/* Returns a String representation of this exception */
char* excp_toString(Exception* e);

/* Returns the name of the file this exception was thrown from */
char* excp_getFile(Exception* e);

/* Returns the line number this exception was thrown on */
int excp_getLine(Exception* e);

/* Returns a String representation of this exception */
char* excp_getMessage(Exception* e);

/* Returns the String exception type */
char* excp_getType(Exception* e);

/* Returns the String representation of the stack trace */
char* excp_getStackTrace(Exception* e);

/* Allocates storage for a new exception */
Exception* excp_createDefault();

/* Allocates storage for a new exception and sets its message */
Exception* excp_create(char* msg);

/* Free's an exception object (don't try this on the process exception) */
void excp_free(Exception* e);

/*-----------------------------------------------------------------------------
   End exception method prototypes
-----------------------------------------------------------------------------*/

#endif

/*=============================================================================
   end of file: $RCSfile: exception.h,v $
==============================================================================*/

