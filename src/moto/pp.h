/*=============================================================================

   Copyright (C) 2000 Kenneth Kocienda and David Hakim.
   This file is part of the Moto Programming Language.

   Moto Programming Language is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   Moto Programming Language is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the Codex C Library; see the file COPYING.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   $RCSfile: pp.h,v $   
   $Revision: 1.7 $
   $Author: dhakim $
   $Date: 2003/06/09 18:35:49 $
 
==============================================================================*/
#ifndef __MOTOPP_H
#define __MOTOPP_H

#include <setjmp.h>

#include "hashset.h"
#include "intstack.h"
#include "mman.h"
#include "stack.h"
#include "stringbuffer.h"
#include "symboltable.h"
#include "vector.h"

#define ARG_LIST_SIZE 6
#define ARG_LIST_FLAGS 0

#define LINE_FMT_1 "$>%s:%d:%s:%d:%s>$"
#define LINE_FMT_2 "$<%s:%d<$"

typedef struct motoPP {
   MPool *mpool;
   HashSet *sysptrs;
   HashSet *ptrs;
   void (*outfn)(int code);
   StringBuffer *out;
   StringBuffer *err;
   HashSet *vallist;
   IntStack *dirstack;
   Stack *macrostack;
   Stack *frames;
   struct motoPPFrame *frame;
   struct motoMacroBuilder *mb;
   StringBuffer *argbuf;
	jmp_buf entrypoint;        /* on an error, longjmp back to this spot */
   int flags;
	int errlineno;             /* on an error, set to the error line number */
	int errflag;               /* on an error, set to one */
} MotoPP;

typedef struct motoPPFrame {
   int type;
   char *relative_root;
   char *filename;
   int lineno;
   StringBuffer *out;
   struct yy_buffer_state *yybuf;
   void *yybufmem;
   Stack *opstack;
   SymbolTable *macros;
   struct motoMacro *macro;
} MotoPPFrame;

typedef struct motoMacro {
   char *relative_root;
   char *filename;
   char *name;
   int argc;
   char **argv;
   char *rtext;
   int startline;
   int infile;
   SymbolTable * arguments; 	/* when substituting a macro argument be sure to pop the macrostack
   								   down to 'arguments' and push it back on after substitution */ 
} MotoMacro;

typedef struct motoMacroBuilder {
   char *relative_root;
   char *filename;
   int startline;
   StringBuffer *buf;
   Vector *args;
   SymbolTable *macros;
} MotoMacroBuilder;

typedef struct motoppval {
   char *sval;
} MotoPPVal;

char* motopp_getRealPath(char* filename);
char* motopp_getRelativeRoot(char* filename);

MotoPP *motopp_createEnv(char *filename, int flags);
void motopp_freeEnv(MotoPP *ppenv);

MotoPPFrame *motopp_createFrame(MotoPP *ppenv, int type,char* filename);
void motopp_freeFrame(MotoPP *ppenv);

MotoPPVal *motopp_createVal(MotoPP *ppenv);

void motopp_freeMacro(MotoMacro *m);
MotoMacro *motopp_getMacro(MotoPP *ppenv, char *name);

MotoMacroBuilder *motopp_createMacroBuilder(char *filename);
void motopp_freeMacroBuilder(MotoMacroBuilder *mb);

int ppparse();

#endif

/*=============================================================================
// end of file: $RCSfile: pp.h,v $
==============================================================================*/


