/*=============================================================================

	Copyright (C) 2000 Kenneth Kocienda and David Hakim.
	This file is part of the Codex C Library.

	The Codex C Library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Library General Public License as
	published by the Free Software Foundation; either version 2 of the
	License, or (at your option) any later version.

	The Codex C Library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the GNU
	Library General Public License for more details.

	You should have received a copy of the GNU Library General Public
	License along with the Codex C Library; see the file COPYING.	If not,
	write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
	Boston, MA 02111-1307, USA.

	$RCSfile: exception.c,v $	 
	$Revision: 1.24 $
	$Author: shayh $
	$Date: 2003/01/29 22:38:57 $
 
==============================================================================*/
#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <signal.h>
#include <stdio.h>
#include <string.h>

#define __EXCEPTION_C

#include "exception.h"
#include "stacktrace.h"
#include "memsw.h"
#include "stringbuffer.h"
#include "excpfn.h"

/*-----------------------------------------------------------------------------
	Global variables
-----------------------------------------------------------------------------*/

TryBlock excp_try_b[EXTB_STACK_SIZE];
int excp_try_i = 0;
Exception excp_e;
char *excp_file;
int excp_line;
char excp_type[MAX_ETYPE_SIZE];
char excp_message[MAX_EMSG_SIZE];
char excp_stacktrace[MAX_EMSG_SIZE];

/*-----------------------------------------------------------------------------
	"Built-in" exceptions
-----------------------------------------------------------------------------*/

excp_declare(AllocationFailureException);
excp_declare(ArrayBoundsException);
excp_declare(ConnectionRefusedException);
excp_declare(DLCloseException);
excp_declare(DLException);
excp_declare(DLOpenException);
excp_declare(DLSymbolException);
excp_declare(DefaultException);
excp_declare(EmptyStackException);
excp_declare(EmptyQueueException);
excp_declare(ExecFailedException);
excp_declare(FileNotFoundException);
excp_declare(ForkFailedException);
excp_declare(IOException);
excp_declare(IllegalArgumentException);
excp_declare(MathException);
excp_declare(NetException);
excp_declare(NoSuchElementException);
excp_declare(NullPointerException);
excp_declare(NumberFormatException);
excp_declare(RegexParseException);
excp_declare(SecurityException);
excp_declare(SemaphoreException);
excp_declare(UnexpectedEOFException);
excp_declare(UnsupportedOperationException);
excp_declare(RuntimeException);
excp_declare(SignalException);
excp_declare(SQLException);
excp_declare(IllegalConversionException);

/*-----------------------------------------------------------------------------
	function implementations
-----------------------------------------------------------------------------*/

/* Default Constructor for exception ebjects */
Exception* 
excp_createDefault(){ 
	Exception *e = (Exception *)malloc(sizeof(Exception)); 
	e->t = "Exception"; 
	e->msg = NULL;
	e->stacktrace = NULL;
	return e; 
} 

/* Constructor for exception objects that takes a message */	
Exception* 
excp_create(char* msg){ 
	Exception *e = (Exception *)malloc(sizeof(Exception)); 
	e->t = "Exception"; 
	e->msg = msg;
	e->stacktrace = NULL;
	return e; 
} 

/* Destructor for exception objects */
void 
excp_free(Exception* e){
	if(e->stacktrace != NULL)
		free(e->stacktrace);
	free(e);
}

/* 
 * Fuction to prepare for throwing an exception object. This method
 * 1) Sets the filename and line# in the exception being thrown
 * 2) Calls excp_throw
 */
void 
excp_marshal( Exception *excp, char* file, int line) {
	excp->file = file;
	excp->line = line;
	excp->stacktrace = stacktrace_toString(excp);
	
	extb_throw(excp);
}

/* 
 * Accessor for the current exception (for use in a catch block) 
 */
Exception *
excp_getProcessException() {
	excp_e.msg = excp_message;
	excp_e.stacktrace = excp_stacktrace;
	excp_e.t = excp_type;
	return &excp_e;
}

Exception *
excp_getCurrentException() {
	return TRYBLK.excp;
}

/* Return a String representation of this exception */
char* 
excp_toString(Exception* e) {
	StringBuffer* sb = buf_createDefault();
	char* r ;
	buf_puts(sb,e->t);
	if(e->msg) {
		buf_puts(sb," : ");
		buf_puts(sb,e->msg);
	}
	r = buf_toString(sb);
	buf_free(sb);
	return r;
}

/* Return the name of the file this exception was thrown from */
char* 
excp_getFile(Exception* e){
	return estrdup(e->file);
}

/* Return the message thrown up with this exception */
char* 
excp_getMessage(Exception* e){
	return estrdup(e->msg);
}

/* Return a new String containing the type of the exception */
char* 
excp_getType(Exception* e){
	return estrdup(e->t);
}

/* Return the line number the exception was thrown on */
int 
excp_getLine(Exception* e){
	return e->line;
}

char*
excp_getStackTrace(Exception* e){
	return estrdup(e->stacktrace);
}

/*-----------------------------------------------------------------------------
	End exception methods
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
	Begin exception try block methods
-----------------------------------------------------------------------------*/

/* 
 * Fuction for throwing an exception object. This method
 *
 * 1) Sets the exception for the current try block
 * 2) longjumps back to the most recent try if there is one
 *		otherwise calls excp_uncaught
 */
 
void 
extb_throw(Exception *excp) {
	TryBlock *tb = &TRYBLK;

	if (excp_try_i > 0) {
		/* If we are actually in a try block than throw the exception */
		tb->excp = excp;
		longjmp(tb->jmp, 1);

	} else {
		/* Otherwise call exception uncaught */
		excp_uncaught(excp);
	}
}

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
 
int 
extb_catch(char* etype) {
	TryBlock *tb = &TRYBLK;

	/* Pop the stack trace down to where it belongs */
	while(stacktrace_size() > tb->tracesize)
		stacktrace_pop();
		
	/* Check to see if the current exception is of the type we are looking for 
		and that we are not trying to re-catch something thrown from inside of
		a catch block */
		
	if (tb->state == TRY_STATE && !strcmp(tb->excp->t ,etype)) {
		/* Set the tryblock code to matched */
		tb->code = 0;
		/* Set the tryblock state to caught */
		tb->state = CATCH_STATE;
		return 1;
	} else
		return 0;
}

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
 
int 
extb_catchAll() {
	TryBlock *tb = &TRYBLK;

	/* Pop the stack trace down to where it belongs */
	while(stacktrace_size() > tb->tracesize)
		stacktrace_pop();
		
	if (tb->state == TRY_STATE ) {
		tb->state = CATCH_STATE;
		tb->code = 0;
		return 1;
	} else
		return 0;
}

int 
extb_catchAllBut(char* etype) {
	TryBlock *tb = &TRYBLK;

	/* Pop the stack trace down to where it belongs */	
	while(stacktrace_size() > tb->tracesize)
		stacktrace_pop();
		
	if (strncmp(etype,tb->excp->t,strlen(etype)) && tb->state == TRY_STATE ) {
		tb->state = CATCH_STATE;
		tb->code = 0;
		return 1;
	} else
		return 0;
}

/*
 * Function called by the FINALLY macro
 *
 * Returns true iff
 * 1) The current TryBlock's state is set to TRY_STATE or CATCH_STATE
 *
 * In this case the following effect will also occur
 * 1) The current TryBlock's state is set to FINALLY_STATE
 */
 
int
extb_finally() {
	TryBlock *tb = &TRYBLK;

	/* Pop the stack trace down to where it belongs */	
	while(stacktrace_size() > tb->tracesize)
		stacktrace_pop();
		
	if (tb->state == TRY_STATE || tb->state == CATCH_STATE) { 
		tb->state = FINALLY_STATE;
		return 1;
	} else
		return 0;
}

/*
 * Function called by the TRY macro
 * 
 * 1) Pushes a new TryBlock onto the try block stack
 * 2) Initializes this to try block to the TRY_STATE setting its code to 0
 *		and its exception to NULL.
 */
 
void
extb_beginTry() {
	TryBlock* tryblk;
	excp_try_i++;
	tryblk = &TRYBLK;
	tryblk->excp = NULL;
	tryblk->state = TRY_STATE;
	tryblk->code = 0;
	tryblk->tracesize = stacktrace_size();
}

/* 
 * Function called by the END_TRY macro
 *
 * Function called at the end of a try-catch block 
 * 1) Pops off the current TryBlock from the try block stack
 * 2) If that try block contains an exception which has not been handled
 *		2.1) Then it gets re-thrown if there was a parent try block
 *		2.2) Otherwise excp_uncaught gets called
 */
 
void 
extb_endTry() {
	TryBlock *tb = &TRYBLK;
	excp_try_i--;

	/* Pop the stack trace down to where it belongs */
	while(stacktrace_size() > tb->tracesize)
		stacktrace_pop();
		
	if(tb->code) {
		Exception *excp = tb->excp; 
		if (excp_try_i > 0) 
			extb_throw(excp);
		else 
			excp_uncaught(excp);
	} 
}

/*-----------------------------------------------------------------------------
	End exception try block methods
-----------------------------------------------------------------------------*/

/* Fuction called when an exception is not caught */
void 
excp_uncaught(Exception* excp) {
	fprintf(stderr, "%s:%d: *** uncaught exception: %s\n", 
			  excp->file, excp->line, excp->t);
	if (strlen(excp->msg) > 0) {
		fprintf(stderr, "%s:%d: *** message: %s", 
				  excp->file, excp->line, excp->msg);
	}
	fprintf(stderr, "\n");
	raise(SIGABRT);
}

/* 
 * Fills in the processes static Exception object using the provided information 
 */
Exception *
excp_fill(Exception* e, char* etype, char *s, ...) {
	int maxchars = MAX_EMSG_SIZE - 6;
	int nchars;
	
	va_list ap;
	va_start(ap, s);
	
	strcpy(e->t,etype);
	
	nchars = vsnprintf(excp_e.msg, maxchars, s, ap);
	if (nchars < 0 || nchars >= maxchars) {
		strcat(e->msg, " ...\n");
	}

	e->stacktrace = stacktrace_toString(e);
	
	return e;
}

void 
excp_getFillAndThrow( char* etype , char *s, ...){
	Exception* e = excp_getProcessException();
	int maxchars = MAX_EMSG_SIZE - 6;
	int nchars;
	char* tmp;
	
	va_list ap;
	va_start(ap, s);
	
	e->file = excp_file;
	e->line = excp_line;
	e->stacktrace = excp_stacktrace;
	
	strcpy(e->t,etype);
	
	nchars = vsnprintf(excp_e.msg, maxchars, s, ap);
	if (nchars < 0 || nchars >= maxchars) {
		strcat(e->msg, " ...\n");
	}

	tmp = stacktrace_toString(e);
	strncpy(excp_e.stacktrace,	 tmp,maxchars);
	if (strlen(excp_e.stacktrace) >= maxchars) {
		strcat(e->stacktrace, " ...\n");
	}	
	free (tmp);
	
	extb_throw(e);
}

void 
excp_getFillAndThrowDefault( char* etype ){
	excp_getFillAndThrow( etype , "", "");
}


/*=============================================================================
	end of file: $RCSfile: exception.c,v $
==============================================================================*/

