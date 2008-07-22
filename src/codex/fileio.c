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

   $RCSfile: fileio.c,v $   
   $Revision: 1.10 $
   $Author: dhakim $
   $Date: 2002/12/05 02:40:48 $
 
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

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include "memsw.h"

#include "exception.h"
#include "excpfn.h"
#include "log.h"

#include "fileio.h"

#define BUFSIZE 1024

static int fio_lo_put
(const char *filename, const char *text, const char *mode);

static int fio_lo_put_bytes
(const char *filename, void *data, int length, const char* mode);

int fio_exists(const char *filename) {
   struct stat buf;
   return (stat(filename, &buf) == 0);
}

int fio_get(const char *filename, char **text) {
   FILE *file = NULL;
   int total_read = 0;
   struct stat buf;
   int r;

   r = stat(filename, &buf);
   switch (r) {
      case ENOENT:
      case ENOTDIR:
      case ELOOP:
      case EFAULT:
      case ENAMETOOLONG:
         THROW("FileNotFoundException", "%s", filename);
         break;
      case EACCES:
         THROW("SecurityException", "Cannot access %s", filename);
         break;
      case ENOMEM:
         THROW("AllocationFailureException", "%s", filename);
         break;
   }

   file = fopen(filename, "r");
   if (file == NULL) {
      THROW_D("IOException");
   }
   else {
      int char_size = sizeof(char);
      int index = 0;
      int cap = BUFSIZE;
      char *buf;

      *text = (char *)emalloc(cap);
      buf = (char *)emalloc(BUFSIZE);
   
      do {
         int read;
         read = fread(buf, char_size, BUFSIZE, file);
         if (ferror(file) != 0) {
            free(buf);
            free(*text);
            fclose(file);
            THROW_D("IOException");
         }
         total_read += read;
         if (total_read > cap) {
            char *newmem = realloc(*text, (cap * 2));
            if(newmem == NULL) {
               free(buf);
               free(*text);
               THROW_D("AllocationFailureException");
            }
            *text = newmem;   
            cap = total_read;
         }
         memcpy((*text) + index, buf, read);
         index = total_read;
      }
      while (feof(file) == 0);
      free(buf);
      (*text)[total_read] = '\0';
      
      /* close file */
      excp_assert(fclose(file) == 0, THROW_D("IOException"));
   
   }
   return total_read;
}

int fio_put(const char *filename, const char *text) {
   return fio_lo_put(filename, text, "w");
}

int fio_append(const char *filename, const char *text) {
   return fio_lo_put(filename, text, "a");
}

int fio_get_bytes(const char *filename, void **data) {
   FILE *file = NULL;
   int total_read = 0;

   file = fopen(filename, "r");
   if (file == NULL) {
      THROW_D("IOException");
   }
   else {
      int index = 0;
      int cap = BUFSIZE;
      void *buf;

      *data = (void *)emalloc(cap);
      buf = (void *)emalloc(BUFSIZE);
   
      do {   
         int read;
         read = fread(buf, 1, BUFSIZE, file);
         if (ferror(file) != 0) {
            free(buf);
            free(*data);
            fclose(file);
            THROW_D("IOException");
         }
         if (read > 0) {
            total_read += read;
            if (total_read > cap) {
               void *newmem = (void *)realloc((*data), (cap * 2));
               if(newmem == NULL) {
                  free(buf);
                  free(data);
                  THROW_D("AllocationFailureException");
               }
               *data = newmem;   
               cap = total_read;
            }
            memcpy((*data) + index, buf, read);
            index = total_read;
         }
      } 
      while (feof(file) == 0);
      free(buf);
      
      /* close file */
      excp_assert(fclose(file) == 0, THROW_D("IOException"));
   
   }
      
   return total_read;
}

int fio_put_bytes(const char *filename, void *data, int length) {
   return fio_lo_put_bytes(filename, data, length, "w");
}

int fio_append_bytes(const char *filename, void *data, int length) {
   return fio_lo_put_bytes(filename, data, length, "a");
}

static int fio_lo_put
(const char *filename, const char *text, const char *mode) {
   FILE *file = NULL;
   int length = strlen(text);
   int total_written = 0;

   file = fopen(filename, mode);
   if (file == NULL) {
      THROW_D("IOException");
   }
   else {
      int char_size = sizeof(char);
      int write_length = BUFSIZE;
      char *buf = (char *)emalloc(BUFSIZE);
      setvbuf(file, buf, _IOFBF, BUFSIZE);
      while (total_written < length) {
         int written = 0;
         if (length - total_written < BUFSIZE) {
            write_length = length - total_written;   
         }
         written = fwrite(text + total_written, char_size, write_length, file);
         excp_assert(ferror(file) == 0 && feof(file) == 0, 
                     THROW_D("IOException"));
         total_written += written;
      }

      /* close file */
      excp_assert(fclose(file) == 0, THROW_D("IOException"));
      free(buf);
   }
   
   return total_written;
}

static int fio_lo_put_bytes
(const char *filename, void *data, int length, const char* mode) {
   FILE *file = NULL;
   int total_written = 0;

   file = fopen(filename, mode);
   if (file == NULL) {
      THROW_D("IOException");
   }
   else {
      int write_length = BUFSIZE;
      char *buf = (char *)emalloc(BUFSIZE);
      setvbuf(file, buf, _IOFBF, BUFSIZE);
      while (total_written < length) {
         int written = 0;
         if (length - total_written < BUFSIZE) {
            write_length = length - total_written;   
         }
         written = fwrite(data + total_written, 1, write_length, file);
         excp_assert(ferror(file) == 0, THROW_D("IOException"));
         total_written += written;
      }

      /* close file */
      excp_assert(fclose(file) == 0, THROW_D("IOException"));
      
      free(buf);
   }
   
   return total_written;
}

/*=============================================================================
// end of file: $RCSfile: fileio.c,v $
==============================================================================*/

