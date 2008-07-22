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

   $RCSfile: pputil.c,v $   
   $Revision: 1.7 $
   $Author: dhakim $
   $Date: 2001/03/04 18:02:59 $
 
==============================================================================*/
#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>

#include "memsw.h"

#include "exception.h"
#include "excpfn.h"
#include "log.h"

#include "motocodes.h"
#include "pp.h"
#include "pputil.h"
#include "motohook.h"

extern int pplineno;

inline char *ppuc_str(const PPUnionCell *p, int index) {
   return (char *)p->opcell.operands[index]->valuecell.value;
}

inline int ppuc_opcode(const PPUnionCell *p) {
   return p->opcell.opcode;
}

inline int ppuc_opcount(const PPUnionCell *p) {
   return p->opcell.opcount;
}

inline PPUnionCell *ppuc_operand(const PPUnionCell *p, int index) {
   return p->opcell.operands[index];
}

inline MotoPPVal *ppopstack_pop() {
   MotoPP *ppenv = motopp_getEnv();
   MotoPPVal *val;
   val = stack_pop(ppenv->frame->opstack);
   return val;
}

inline MotoPPVal *ppopstack_peek() {
   MotoPP *ppenv = motopp_getEnv();
   return (MotoPPVal *)stack_peek(ppenv->frame->opstack);
}

inline MotoPPVal *ppopstack_peekAt(int index) {
   MotoPP *ppenv = motopp_getEnv();
   return (MotoPPVal *)stack_peekAt(ppenv->frame->opstack, index);
}

inline void ppopstack_push(MotoPPVal *val) {
   MotoPP *ppenv = motopp_getEnv();
   stack_push(ppenv->frame->opstack, val);
}

inline int ppopstack_size() {
   MotoPP *ppenv = motopp_getEnv();
   return stack_size(ppenv->frame->opstack);
}

inline char *motopp_strdup(MotoPP *ppenv, const char *s) {
   char *dup = estrdup((char*)s);
   hset_add(ppenv->ptrs, dup);
   return dup;
}

/* checks if an include has been included by a parent */
int motopp_isInIncludeChain(MotoPP *ppenv, char *name) {
	int result = 0;
	int depth = stack_size(ppenv->frames);
	int i;
	
   log_debug(__FILE__, ">>> motopp_isInIncludeChain\n");				
   
   for (i = 1; i <= depth; i++) {
		MotoPPFrame *frame = stack_peekAt(ppenv->frames, i);
		if (frame->filename != NULL) {
			if (strcmp(frame->filename, name) == 0) {					
				result = 1;
				break;
			}
		}
	}
	
   log_debug(__FILE__, "<<< motopp_isInIncludeChain\n");				
   
   return result;
}

char *motopp_uncomment(const char *s) {
   StringBuffer *buf = buf_createDefault();
	char *result;
   int len;
   char c1;
   char c2;

   log_debug(__FILE__, ">>> motopp_uncomment: %s\n", s);				

   len = strlen(s);
   if (len <= 2) {
      buf_puts(buf, s);
   }
   else {
      int state = 0;
      int addchar = 1;
      int i = 0;
      c1 = c2 = 0;
      while (i < len) {
         c1 = c2;
         c2 = s[i++];
         if (c1 == '$') {
            state = '$';
         }
         else if (c1 == '*') {
            state = '*';
         }
         else if (c1 == '>') {
            state = '>';
         }
         else if (c1 == '<') {
            state = '<';
         }
         else {
            state = 0;
         }
         switch (c2) {
            case '*':
            case '>':
            case '<':
               if (state == '$') {
                  addchar = 0;
               }
               break;
            case '$':
               if (state == '*' || state == '>' || state == '<') {
                  addchar = 1;
                  if (i < len) {
                     c2 = s[i];
                  }
               }
               break;
            default:
               if (addchar) {
                  if (state == '$') {
                     buf_putc(buf, '$');
                  }
                  else if (state == '*') {
                     buf_putc(buf, '*');
                  }
                  else if (state == '>') {
                     buf_putc(buf, '>');
                  }
                  else if (state == '<') {
                     buf_putc(buf, '<');
                  }
                  buf_putc(buf, c2);
               }
               break;
         }
      }	
   }

   log_debug(__FILE__, "<<< motopp_uncomment: %s\n", buf_data(buf));				
   
   result = buf_toString(buf);
   buf_free(buf);

   return result;
}

/*=============================================================================
// end of file: $RCSfile: pputil.c,v $
==============================================================================*/


