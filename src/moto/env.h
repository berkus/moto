/*=============================================================================

	Copyright (C) 2000 Kenneth Kocienda and David Hakim.
	This file is part of the Moto Programming Language.

	Moto Programming Language is free software; you can redistribute it and/or
	modify it under the terms of the GNU Library General Public License as
	published by the Free Software Foundation; either version 2 of the
	License, or (at your option) any later version.

	Moto Programming Language is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the GNU
	Library General Public License for more details.

	You should have received a copy of the GNU Library General Public
	License along with the Codex C Library; see the file COPYING.	If not,
	write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
	Boston, MA 02111-1307, USA.

	$RCSfile: env.h,v $	 
	$Revision: 1.49 $
	$Author: dhakim $
	$Date: 2003/05/29 02:14:08 $
 
==============================================================================*/
#ifndef __MOTOENV_H
#define __MOTOENV_H

#include <stdio.h>
#include <setjmp.h>

#include "exception.h"
#include "hashtable.h"
#include "hashset.h"
#include "inthashtable.h"
#include "intstack.h"
#include "mman.h"
#include "stack.h"
#include "stringbuffer.h"
#include "stringset.h"
#include "vector.h"
#include "mototree.h"
#include "cell.h"
#include "mc.h"
#include "motofunction.h"
#include "ftable.h"
#include "motocodes.h"
#include "type.h"
#include "objectpool.h"

typedef enum {
	INTERPRETER_MODE = 1,
	COMPILER_MODE = 2,
	VERIFIER_MODE = 3,
	DEPENDENCY_MODE = 4
} MotoMode;

typedef struct motoParseScope {
	int type;
	MetaInfo meta;
} MotoParseScope;
 
typedef struct motoFrame {
	int type;
	Stack *opstack;
	SymbolTable *symtab;
	StringBuffer *out;
} MotoFrame;

typedef struct motoEnv {
	MotoMode mode;					/* the execution mode for this environment */
	int flags;						/* place to store flags...paying attention? */
	int mman_level;				/* the memory management level...what else? */
	void (*e)(const UnionCell *); /* pointer to interpreter or compiler */
	MPool *mpool;					 /* the memory pool...duh */

	HashSet *ptrs;					/* ptrs freed when env is freed */
	FILE *file;						/* the pointer to the main file being executed */
	MotoTree *tree;				/* the parse tree being executed */

	char *filename;				/* the name of the main file being executed */

	MetaInfo meta;					/* container for current meta information such as
											line number, filename, current macro while parsing */
	MetaInfo errmeta;				/* container for meta information such as
											line number, filename, current macro for the
											error being thrown */
 
	StringBuffer *out;			/* the top-level output buffer */
	StringBuffer *err;			/* the error buffer */
	void (*outfn)(int code);	/* pointer to output function */	  
	SymbolTable *types;			/* maintains a map of all defined types */
	SymbolTable *cdefs;			/* maps class names to class definitions */
	MotoClassDefinition* ccdef;/* the current class definition being parsed by motod/v/c */ 
	StringSet *uses;				/* maintains a set of all used extensions */
	Stack *scope;					/* tracks current scope (e.g. IF, FOR, etc.) */
	
	Vector *frames;				/* the list of all stack frames */
	int frameindex;				/* the stack frame depth, top-level is 0 */
	int frametype;					/* the type of frame */
	MotoFrame *frame;				/* pointer to current stack frame */
	
	Stack *callstack;				/* stack for macro and include motopp calls */
	SymbolTable *globals;		/* SymbolTable for storing global variables */
	FTable *ftable;				/* the function table */
	SymbolTable *rxcache;		/* the cache for regular expressions */

	/* Compiler Goodies */
	Hashtable *fdefs;				/* Map MXFunctions to moto function C definitions */
	Hashtable *adefs;				/* Map Anonymous functions to C definitions */
	
	StringSet *includes;			/* includes used; compiler only */
	StringBuffer *constantPool;/* large constant structure values (e.g. dfas)*/ 
	StringBuffer *fcodebuffer; /* Holds the code for the current function */
 
   int curScopeID;				/* Holds the current scope id for the compiler */
   IntStack* scopeIDStack;		/* Holds all ancestors of the current scope id for the compiler */
   
   int constantCount;         /* the number of entries in the contant pool */
   StringSet *errs;           /* all errors get added to this list */
   int errflag;               /* on an error, set to one */

   ObjectPool* valpool;       /* Object pool for re-use of MotoValues */
   ObjectPool* bufpool;       /* Object pool for re-use of StringBuffers */
   ObjectPool* stkpool;       /* Object pool for re-use of Stacks */
   ObjectPool* vecpool;       /* Object pool for re-use of Vectors 
                                 used in matching function prototypes */

   SymbolTable* fcache;       /* Cache for mapping function prototypes to 
                                 function pointers */

   UnionCell * rval_uc;		  /* Pointer to the last rval unioncell. */
} MotoEnv;

/*-----------------------------------------------------------------------------
		  allocation functions
-----------------------------------------------------------------------------*/

/* Create a new MotoEnv */
MotoEnv *moto_createEnv(int flags, char* filename);

/* Free a MotoEnv */
void moto_freeEnv(MotoEnv *env);
void moto_clearEnv(MotoEnv *env);

/* MotoFrame related functions */
void moto_createFrame(MotoEnv *env, int type);
void moto_freeFrame(MotoEnv *env);

void moto_freeTreeCells(MotoEnv *env);

void moto_delete(MotoEnv *env, MotoVal *val);

void moto_readdefs(MotoEnv *env, char *filename);

/* MotoVal related functions */
MotoVal *moto_createVal(MotoEnv *env, char *typen,int dim);

/* Initlialize a MotoVal */
void moto_initVal(MotoEnv *env,MotoVal *val, char* typen, int dim);

/* Clone a MotoVal */
MotoVal *moto_cloneVal(MotoEnv *env, MotoVal *orig);

/* Return a MotoVal to the ObjectPool */
void moto_freeVal(MotoEnv *env, MotoVal *val);

/* MotoVar related functions */
MotoVar *moto_createVar(MotoEnv *env, char* varn, char *typen, int dim,char isMemberVar,void *address);
/* Free a MotoVar */
void moto_freeVar(MotoEnv *env, MotoVar *var);

MotoVar *moto_declareVar(MotoEnv *env, char* varn, char *typen, int dimension, char isGlobal);
MotoVar *moto_getFrameVar(MotoEnv *env, char *name);
MotoVar *moto_getGlobalVar(MotoEnv *env, char *name);
MotoVar *moto_getVar(MotoEnv *env, char *name);
MotoVal *moto_getVal(MotoEnv *env, char *name);
char moto_isVarGlobal(MotoEnv *env, char *name);

/* Creates a new MotoVal and stores the value of the variable 'var' in it*/
MotoVal* moto_getVarVal(MotoEnv* env, MotoVar *var);
/* Sets the value of the variable 'var' to the value stored by motovalue 'val' 
	doing any appropriate implicit casts */
void moto_setVarVal(MotoEnv *env, MotoVar *var, MotoVal *val);

void moto_castVal(MotoEnv *env, MotoVal *to, MotoVal *from);

/* Code Generation Functions */

void moto_emitCHeader(MotoEnv *env, StringBuffer *out);
/* emit C prototypes for moto functions */
void moto_emitCPrototypes(MotoEnv *env, StringBuffer *out);
/* emit C declarations of global varables */
void moto_emitCGlobals(MotoEnv *env, StringBuffer *out);
/* emit C definitions for moto functions */
void moto_emitCFunctions(MotoEnv *env, StringBuffer *out);
/* emit C typedefs for moto defined classes */
void moto_emitCStructures(MotoEnv *env, StringBuffer *out);
/* emit C prototypes for implicit constructors for moto defined classes */
void moto_emitCImplicitConstructorPrototypes(MotoEnv *env, StringBuffer *out);
/* emit C functions for implicit constructors for moto defined classes */
void moto_emitCImplicitConstructors(MotoEnv *env, StringBuffer *out);

void moto_emitCFooter(MotoEnv *env, StringBuffer *out);
char* moto_valToCType(MotoVal* val);
char* moto_defaultValForCType(MotoVal* val);

int moto_use(MotoEnv *env, char *usename);

/* Retrieves the class definition for the class named <name> from the environment */
MotoClassDefinition* moto_getMotoClassDefinition(MotoEnv* env, char* name);
/* Stores a class definition for the class named <name> in the environment */
void moto_setMotoClassDefinition(MotoEnv* env, char* name, MotoClassDefinition* mcd);

/* Creates a motoname for a function suitable for lookups in the ftbale */
char* moto_createMotonameForFn(char* classn, char* fn);
char* moto_createMotonameForFnInBuffer(char* classn, char* fn, char* buffer);

#endif

/*=============================================================================
// end of file: $RCSfile: env.h,v $
==============================================================================*/

