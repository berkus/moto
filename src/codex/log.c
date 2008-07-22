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

   $RCSfile: log.c,v $   
   $Revision: 1.7 $
   $Author: dhakim $
   $Date: 2002/12/05 02:40:48 $
 
==============================================================================*/
#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#include "exception.h"
#include "log.h"

#define HEAD plog.head
#define TAIL plog.tail

static LogStruct plog;

void log_init(int mask) {
   if (plog.init != 1) {
      LogFile *p = malloc(sizeof(LogFile));
      char *name = "stderr";
      p->name = malloc(strlen(name) + 1);
      strcpy(p->name, name);
      p->file = stderr;
      HEAD = TAIL = p;
      TAIL->next = NULL;
      plog.mask = mask;
      strcpy(plog.strftimefmt, STRFTIME_DATE_FORMAT);
      strcpy(plog.printfdatefmt, PRINTF_DATE_FORMAT);
      strcpy(plog.loglevelfmt, LOG_LEVEL_FORMAT);
      strcpy(plog.loggerfmt, LOGGER_FORMAT);
      plog.init = 1;
   }
}

void log_excp(Exception e) {
   char timestamp[STRING_MAX_LENGTH];
   char *levelstring = LOG_ERROR_STR;
   time_t tp;
   LogFile *p;
   if (plog.init != 1) {
      log_init(LOG_ALL);   
   }
   tp = time(NULL);
   strftime(timestamp, STRING_MAX_LENGTH, plog.strftimefmt, localtime(&tp));         
   for(p = HEAD; p; p = p->next) {
      FILE *file = p->file;
      fprintf(file, plog.printfdatefmt, timestamp);   
      fprintf(file, plog.loglevelfmt, levelstring);
      fprintf(file, plog.loggerfmt, e.t);   
      fprintf(file, "%s:%d: %s", e.file, e.line, e.msg);   
      fflush(file);
   }
}

void log_debug(const char *logger, const char *format,...) {
   if (log_can(LOG_DEBUG)) {
      va_list ap;
      va_start(ap, format);
      log_write(LOG_DEBUG, logger, format, ap);   
      va_end(ap);
   }
}

void log_info(const char *logger, const char *format,...) {
   if (log_can(LOG_INFO)) {
      va_list ap;
      va_start(ap, format);
      log_write(LOG_INFO, logger, format, ap);   
      va_end(ap);
   }
}

void log_warning(const char *logger, const char *format,...) {
   if (log_can(LOG_WARNING)) {
      va_list ap;
      va_start(ap, format);
      log_write(LOG_WARNING, logger, format, ap);   
      va_end(ap);
   }
}

void log_error(const char *logger, const char *format,...) {
   if (log_can(LOG_ERROR)) {
      va_list ap;
      va_start(ap, format);
      log_write(LOG_ERROR, logger, format, ap);   
      va_end(ap);
   }
}

void log_fatal(const char *logger, const char *format,...) {
   if (log_can(LOG_FATAL)) {
      va_list ap;
      va_start(ap, format);
      log_write(LOG_FATAL, logger, format, ap);   
      va_end(ap);
   }
}

int log_can(int level) {
   return((level & plog.mask) != 0 || level > plog.mask);
}

void log_free() {
   if (plog.init == 1) {
      LogFile *p = HEAD;
      while (p) {
         if (p->file != stderr && p->file != stdout) {
            excp_assert(fclose(p->file) == 0, THROW_D("IOException"));
         }
         p = p->next;
      }
   }
}

int log_addFile(const char *filename) {
   LogFile *p;
   int result = 1;
   
   if (plog.init != 1) {
      log_init(LOG_ALL);   
   }
   for(p = HEAD; p; p = p->next) {
      if (strcmp(filename, p->name) == 0) {
         result = 0;
         break;
      }
   }
   if (result == 1) {
      LogFile *newp;
      FILE *file = fopen(filename, "a");
      excp_assert(file != NULL, THROW_D("IOException"));
      newp = malloc(sizeof(LogFile));
      newp->name = malloc(strlen(filename) + 1);
      strcpy(newp->name, filename);
      newp->file = file;
      if (TAIL) {
         TAIL->next = newp;
         TAIL = newp;      
         TAIL->next = NULL;
      }
      else {
         HEAD = TAIL = newp;
      }
   }
   return result;
}

int log_removeFile(const char *filename) {
   int result = 0;
   if (plog.init == 1 && 
       strcmp(filename, "stderr") != 0 &&
       strcmp(filename, "stdout") != 0) {
      LogFile *p;
      LogFile *prev;
      for(p = HEAD, prev = NULL; p; prev = p, p = p->next) {
         if (strcmp(filename, p->name) == 0) {
            excp_assert(fclose(p->file) == 0, THROW_D("IOException"));
            free(p->name);
            if (p == HEAD || p == TAIL) {
               if (p == HEAD && p == TAIL) {
                  HEAD = TAIL = NULL;
               }
               else if (p == TAIL) {
                  TAIL = prev;
                  if (TAIL) {
                     TAIL->next = NULL;
                  }
               }
               else {
                  HEAD = p->next;
               }
            }
            else {
               p->next = p->next->next;
            }
            result = 1;
            break;
         }
      }
   }
   return result;
}

void log_setMask(int mask) {
   plog.mask = mask;   
}

int log_getMask() {
   return plog.mask;
}

void log_setStrftimeFormat(const char *strftimefmt) {
   strcpy(plog.strftimefmt, strftimefmt);
}

char *log_getStrftimeFormat() {
   return plog.strftimefmt;   
}

void log_setLevelFormat(const char *loglevelfmt) {
   strcpy(plog.loglevelfmt, loglevelfmt);
}

char *log_getLevelFormat() {
   return plog.loglevelfmt;   
}

void log_setPrintfDateFormat(const char *printfdatefmt) {
   strcpy(plog.printfdatefmt, printfdatefmt);
}

char *log_getPrintfDateFormat() {
   return plog.printfdatefmt;   
}

void log_write(int level, const char *logger, const char *format, va_list ap) {
   char timestamp[STRING_MAX_LENGTH];
   char levelstring[STRING_MAX_LENGTH];
   time_t tp;
   LogFile *p;

   if (plog.init != 1) {
      log_init(LOG_ALL);   
   }

   switch (level) {
      case LOG_DEBUG:
         strcpy(levelstring, LOG_DEBUG_STR);
         break;
      case LOG_INFO:
         strcpy(levelstring, LOG_INFO_STR);
         break;
      case LOG_WARNING:
         strcpy(levelstring, LOG_WARNING_STR);
         break;
      case LOG_ERROR:
         strcpy(levelstring, LOG_ERROR_STR);
         break;
      case LOG_FATAL:
         strcpy(levelstring, LOG_FATAL_STR);
         break;
      default:
         strcpy(levelstring, LOG_UNK_STR);
         break;
   }

   tp = time(NULL);
   strftime(timestamp, STRING_MAX_LENGTH, plog.strftimefmt, localtime(&tp));         
   for(p = HEAD; p; p = p->next) {
      FILE *file = p->file;
      fprintf(file, plog.printfdatefmt, timestamp);   
      fprintf(file, plog.loglevelfmt, levelstring);
      fprintf(file, plog.loggerfmt, logger);   
      vfprintf(file, format, ap);   
      fflush(file);
   }
}

/*=============================================================================
// end of file: $RCSfile: log.c,v $
==============================================================================*/

