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

   $RCSfile: mxcerr.c,v $   
   $Revision: 1.7 $
   $Author: dhakim $
   $Date: 2001/03/04 18:03:01 $
 
==============================================================================*/
#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "mxcg.h"
#include "mxcerr.h"

void mxc_syntaxError(FILE *file, char *filename, int lineno, int colno) {   
   char linenos[22]; /* maximum length an unsigned 64-bit int can be...plus 2 */
   char colnos[22]; /* maximum length an unsigned 64-bit int can be...plus 2 */
   int line = 1;
   int col;
   char c;
      
   /* tell 'em what happened */
   fprintf(stderr, "%s:%d:%d: *** syntax error\n", filename, lineno, colno);
   
   /* get error line and print it */
   rewind(file);
   while (line < lineno) {
      c = getc(file);
      if (c == '\n') {
         line++;
      }
      else if (c == EOF) {
         exit(1);
      }
   }
   fprintf(stderr, "%s:%d:%d: >>> ", filename, lineno, colno);
   c = getc(file);
   while (c != '\n' && c != EOF) {
      fprintf(stderr, "%c", c);
      c = getc(file);
   }
   fprintf(stderr, "\n");
   
   /* print error "carat" under offending character */
   /* munge carat position by adding file name length, 
    * line no length, col no length, three >'s, three colons, and two spaces 
    */
   sprintf(linenos, "%d", lineno);
   sprintf(colnos, "%d", colno);
   col = colno + strlen(filename) + strlen(linenos) + strlen(colnos) + 3 + 3 + 2;

   while (--col > 0) {
      fprintf(stderr, " ");
   }
   fprintf(stderr, "^\n");

   /* bye bye */
   exit(1);
}

/*=============================================================================
// end of file: $RCSfile: mxcerr.c,v $
==============================================================================*/

