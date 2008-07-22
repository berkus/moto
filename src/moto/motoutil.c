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

   $RCSfile: motoutil.c,v $   
   $Revision: 1.18 $
   $Author: dhakim $
   $Date: 2003/02/11 23:14:53 $
 
==============================================================================*/
#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>

#include "memsw.h"
#include "excpfn.h"
#include "mman.h"

#include "motolex.h"
#include "moto.tab.h"
#include "motoutil.h"
#include "motohook.h"

inline char *
uc_str(const UnionCell *p, int index) {
   return (char *)p->opcell.operands[index]->valuecell.value;
}

inline char *
uc_code(const UnionCell *p, int index) {
   return (char *)p->opcell.operands[index]->valuecell.code;
}

inline int 
uc_opcode(const UnionCell *p) {
   return p->opcell.opcode;
}

inline int 
uc_opcount(const UnionCell *p) {
   return p->opcell.opcount;
}

inline MetaInfo* 
uc_meta(const UnionCell *p) {
   if(p->type == OP_TYPE)
      return (MetaInfo*)&p->opcell.meta;
   return (MetaInfo*)&p->valuecell.meta;
}

inline UnionCell *
uc_operand(const UnionCell *p, int index) {
   return p->opcell.operands[index];
}

inline MotoVal *
opstack_pop(MotoEnv *env) {
   return (MotoVal *)stack_pop(env->frame->opstack);
}

inline MotoVal *
opstack_peek(MotoEnv *env) {
   return (MotoVal *)stack_peek(env->frame->opstack);
}

inline MotoVal *
opstack_peekAt(MotoEnv *env, int index) {
   return (MotoVal *)stack_peekAt(env->frame->opstack, index);
}

inline void 
opstack_push(MotoEnv *env, MotoVal *val) {
   stack_push(env->frame->opstack, val);
}

inline int 
opstack_size(MotoEnv *env) {
   return stack_size(env->frame->opstack);
}

inline char 
bv(MotoVal *val) {
   return val->booleanval.value;
}

inline char 
cv(MotoVal *val) {
   return val->charval.value;
}

inline signed char
yv(MotoVal *val) {
   return val->byteval.value;
}

inline int32_t 
iv(MotoVal *val) {
   return val->intval.value;
}

inline int64_t 
lv(MotoVal *val) {
   return val->longval.value;
}

inline float 
fv(MotoVal *val) {
   return val->floatval.value;
}

inline double 
dv(MotoVal *val) {
   return val->doubleval.value;
}

inline char *
sv(MotoVal *val) {
   return (char *)val->refval.value;
}

inline int 
isType(char *type, MotoVal *val) {
   return (strcmp(type, val->type->name) == 0);
}

inline int 
isVarType(char *type, MotoVar *var) {
   return (strcmp(type, var->type->name) == 0);
}

inline int 
isObject(MotoVal *val) {
   return (strcmp("Object", val->type->name) == 0);
}

inline int 
isString(MotoVal *val) {
   return (strcmp("String", val->type->name) == 0);
}

inline int 
isRegex(MotoVal *val) {
   return (strcmp("Regex", val->type->name) == 0);
}

inline int
isArray(MotoVal *val) {
   return (val->type->dim > 0 && val->type->atype != NULL);
}

char *
moto_strdup(MotoEnv *env, char *s) {
   char *dup = estrdup(s);
   hset_add(env->ptrs, dup);
   return dup;
}

void * 
moto_malloc(MotoEnv *env, size_t size) {
   void *r = emalloc(size);
   hset_add(env->ptrs, r);
   return r; 
}

char *
moto_nameForOp(int opcode) {
   switch (opcode) {
      case AND:
         return "and";
         break;
      case MATH_ADD:
         return "add";
         break;
      case MATH_SUB:
         return "subtract";
         break;
      case MATH_MULT:
         return "multiplication";
         break;
      case MATH_DIV:
         return "division";
         break;
      case MATH_MOD:
         return "modulo";
         break;
      case MINUS:
         return "minus";
         break;
      case NOT:
         return "not";
         break;
      case OR:
         return "or";
         break;
      case REL_EQ_M:
         return "equals";
         break;
      case REL_NE_M:
         return "not equal";
         break;
      case REL_GT_M:
         return "greater than";
         break;
      case REL_GTE_M:
         return "greater than or equal";
         break;
      case REL_LT_M:
         return "less than";
         break;
      case REL_LTE_M:
         return "less than or equal";
         break;
      case RX_MATCH:
         return "regex match";
         break;
      case RX_NOT_MATCH:
         return "regex not match";
         break;
      case UMINUS:
         return "minus";
         break;
      default:
         return "???";
         break;
   }
}

char *
moto_symbolForOp(int opcode) {
   switch (opcode) {
      case AND: return "&&"; break;
      case BITWISE_AND: return "&"; break;
      case BITWISE_OR: return "|"; break;
      case BITWISE_XOR: return "^"; break;
      case BITWISE_NOT: return "~"; break;
      case MATH_ADD: return "+"; break;
      case MATH_SUB: return "-"; break;
      case MATH_MULT: return "*"; break;
      case MATH_DIV: return "/"; break;
      case MATH_MOD: return "%"; break;
      case MINUS: return "-"; break;
      case NOT: return "!"; break;
      case OR: return "||"; break;
      case POSTFIX_DEC: case PREFIX_DEC: return "--"; break; 
      case POSTFIX_INC: case PREFIX_INC: return "++"; break; 
      case REL_EQ_M: return "=="; break;
      case REL_NE_M: return "!="; break;
      case REL_GT_M: return ">"; break;
      case REL_GTE_M: return ">="; break;
      case REL_LT_M: return "<"; break;
      case REL_LTE_M: return "<="; break;
      case REL_EQ_S: return "eq"; break;
      case REL_NE_S: return "ne"; break;
      case REL_GT_S: return "gt"; break;
      case REL_GTE_S: return "gte"; break;
      case REL_LT_S: return "lt"; break;
      case REL_LTE_S: return "lte"; break;
      case SHIFT_LEFT: return "<<"; break;
      case SHIFT_RIGHT: return ">>"; break;
      case UMINUS: return "-"; break;
      case RX_MATCH: return "=~"; break;
      case RX_NOT_MATCH: return "!~"; break;

      case ASSIGN: return "="; break;

      case LIST : return "list"; break;
      case NOOP : return "noop"; break;
      case DO: return "do"; break;
      case FOR: return "for"; break;
      case ENDFOR: return "endfor"; break; 
      case DECLARE: return "declare"; break;
      case BREAK: return "break"; break;
      case CONTINUE: return "continue"; break;
      case IF: return "if"; break;
      case ENDIF: return "endif"; break;
      case ELSEIF: return "elseif"; break;
      case ELSE: return "else"; break;
      case PRINT: return "print"; break;
      case USE: return "use"; break;
      case SWITCH: return "switch"; break;
      case ENDSWITCH: return "endswitch"; break;
      case CASE: return "case"; break;
      case WHILE: return "while"; break;
      case ENDWHILE: return "endwhile"; break;

      default:
         return "???";
         break;
   }
}

char* 
escapeStrConst(char* s){
   StringBuffer *tmp = buf_createDefault();
   char* r;
   int i, size = strlen(s);

   for (i = 0; i < size; i++) {
      char c = s[i];
      switch (c) {
         case '\t':
            buf_puts(tmp, "\\t");
            break;
         case '\n':
            buf_puts(tmp, "\\n");
            break;
         case '\r':
            buf_puts(tmp, "\\r");
            break;
         case '\f':
            buf_puts(tmp, "\\f");
            break;
         case '"':
            buf_putc(tmp, '\\');
            buf_putc(tmp, c);
            break;
         case '\\':
            buf_puts(tmp, "\\\\");
            break;
         default:
            buf_putc(tmp, c);
            break;
      }
   }
  
   r = buf_toString(tmp);
   buf_free(tmp);

   return r;
}

char* 
unescapeStrConst(char* s){
   StringBuffer *tmp = buf_createDefault();
   char* r;
   int i, size = strlen(s);

   for (i = 1; i < size - 1; i++) {
      char c = s[i];
      if(s[i]=='\\') {
         c = s[++i];
         switch (c) {
            case 't':
               buf_puts(tmp, "\t");
               break;
            case 'n':
               buf_puts(tmp, "\n");
               break;
            case 'r':
               buf_puts(tmp, "\r");
               break;
            case 'f':
               buf_puts(tmp, "\f");
               break;
            case '"':
               buf_puts(tmp, "\"");
               break;
            case '\\':
               buf_puts(tmp, "\\");
               break;
            default:
               buf_putc(tmp, c);
               break;
         }
      } else {
         buf_putc(tmp, c);
      }
   }

   r = buf_toString(tmp);
   buf_free (tmp);

   return r;
}

/*=============================================================================
// end of file: $RCSfile: motoutil.c,v $
==============================================================================*/

