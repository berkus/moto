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

   $RCSfile: number.c,v $   
   $Revision: 1.6 $
   $Author: kocienda $
   $Date: 2000/04/30 23:10:01 $
 
==============================================================================*/
#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "number.h"

#define INT32_CHAR_LEN 10
#define INT64_CHAR_LEN 19

#define INT32_CHAR_POS_MAX "2147483647"
#define INT32_CHAR_NEG_MAX "2147483648"
#define INT64_CHAR_POS_MAX "9223372036854775807"
#define INT64_CHAR_NEG_MAX "9223372036854775808"

static int parse_int64(char *s, int64_t *n);
static int parse_int32(char *s, int32_t *n);

int parse_int(char *s, ParsedInt *u) {
   int out_of_range = parse_int32(s, &(u->i));
   
   if (!out_of_range) {
      u->t = I32;
   }
   else {
      /* parse a 64-bit integer */            
      out_of_range = parse_int64(s, &(u->l));
      u->t = I64;
   }
   
   return out_of_range;  
}

void parse_float(char *s, ParsedFloat *u) {
   double d;
	char c = s[strlen(s) - 1];

   errno = 0;
   d = strtod(s, (char **)NULL);
   
   if (c == 'f' || c == 'F') {
      u->t = FP32;
      u->f = (float)d;
   }
   else {
      u->t = FP64;
      u->d = d;
   }

}

static int parse_int32(char *s, int32_t *n) {
   int out_of_range = 0;
   int count = 0;
   int sign = 1;
   char *b;
   int32_t num = 0;

   while (*s == ' ' || *s == '\t') {
      s++;
   }

   if (*s == '-') {
      sign = -1;
      s++;
   }

   /* set begin index for out of range string compare */
   b = s;

   while (*s != '\0') {
      if (*s >= '0' && *s <= '9') {
         count++;
         if (count > INT32_CHAR_LEN) {
            out_of_range = ORANGE;
            break;
         }
         num = num * 10 + (*s - '0');
         s++;
      }
      else {
         break;
      }
   }

   /* check range and adjust for sign */
   if (out_of_range) {
      if (sign == 1) {
         num = INT32_MAX;
      }
      else {
         num = INT32_MIN;
      }
   }
   else if (count == INT32_CHAR_LEN) {
      /* do a string compare to see if out of range */
      if (sign == 1) {
         if (strncmp(b, INT32_CHAR_POS_MAX, count) > 0) {
            out_of_range = ORANGE;
            num = INT32_MAX;
         }
      }
      else {
         if (strncmp(b, INT32_CHAR_NEG_MAX, count) > 0) {
            out_of_range = ORANGE;
            num = INT32_MIN;
         }
         else {
            num *= (int32_t)sign;
         }
      }
   }
   else {
      /* everything OK, in range int */
      num *= (int32_t)sign;
   }

   *n = num;
   return out_of_range;
}

static int parse_int64(char *s, int64_t *n) {
   int out_of_range = 0;
   int count = 0;
   int sign = 1;
   char *b;
   int64_t num = 0;
   int64_t factor = 10;

   while (*s == ' ' || *s == '\t') {
      s++;
   }

   if (*s == '-') {
      sign = -1;
      s++;
   }

   /* set begin index for out of range string compare */
   b = s;

   while (*s != '\0') {
      if (*s >= '0' && *s <= '9') {
         count++;
         if (count > INT64_CHAR_LEN) {
            out_of_range = ORANGE;
            break;
         }
         num = num * factor + (*s - '0');
         s++;
      }
      else {
         break;
      }
   }

   /* check range and adjust for sign */
   if (out_of_range) {
      if (sign == 1) {
         num = INT64_MAX;
      }
      else {
         num = INT64_MIN;
      }
   }
   else if (count == INT64_CHAR_LEN) {
      /* do a string compare to see if out of range */
      if (sign == 1) {
         if (strncmp(b, INT64_CHAR_POS_MAX, count) > 0) {
            out_of_range = ORANGE;
            num = INT64_MAX;
         }
      }
      else {
         if (strncmp(b, INT64_CHAR_NEG_MAX, count) > 0) {
            out_of_range = ORANGE;
            num = INT64_MIN;
         }
         else {
            num *= (int64_t)sign;
         }
      }
   }
   else {
      /* everything OK, in range int */
      num *= (int64_t)sign;
   }

   *n = num;
   return out_of_range;
}

/*=============================================================================
// end of file: $RCSfile: number.c,v $
==============================================================================*/

