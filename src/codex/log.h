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

   $RCSfile: log.h,v $   
   $Revision: 1.4 $
   $Author: kocienda $
   $Date: 2000/04/30 23:10:01 $
 
==============================================================================*/
#ifndef __LOG_H
#define __LOG_H

#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdarg.h>
#include <stdio.h>

#include "exception.h"

enum {
   LOG_DEBUG   = 1,
   LOG_INFO    = 2,
   LOG_WARNING = 4,
   LOG_ERROR   = 8,
   LOG_FATAL   = 16,
   LOG_ALL     = 31,
};

#define LOG_DEBUG_STR   "DEBUG"
#define LOG_INFO_STR    "INFO"
#define LOG_WARNING_STR "WARNING"
#define LOG_ERROR_STR   "ERROR"
#define LOG_FATAL_STR   "FATAL"
#define LOG_UNK_STR     "UNK"

#define STRFTIME_DATE_FORMAT "%b %d %H:%M:%S "
#define LOG_LEVEL_FORMAT "%-8s "
#define PRINTF_DATE_FORMAT "%-18s "
#define LOGGER_FORMAT "%-16s "
#define STRING_MAX_LENGTH 32

typedef struct logfile {
   char *name;
   FILE *file;
   struct logfile *next;
} LogFile;

typedef struct logstruct {
	int init;
	int mask;
   LogFile *head;
   LogFile *tail;
   char strftimefmt[STRING_MAX_LENGTH];
   char loglevelfmt[STRING_MAX_LENGTH];
   char printfdatefmt[STRING_MAX_LENGTH];
   char loggerfmt[STRING_MAX_LENGTH];
} LogStruct;

void log_init(int mask);
void log_free();

int log_addFile(const char *filename);
int log_removeFile(const char *filename);

void log_excp(Exception e);
void log_debug(const char *logger, const char *format,...);
void log_info(const char *logger, const char *format,...);
void log_warning(const char *logger, const char *format,...);
void log_error(const char *logger, const char *format,...);
void log_fatal(const char *logger, const char *format,...);

void log_write(int level, const char *logger, const char *format, va_list ap);

int log_can(int level);

void log_setStrftimeFormat(const char *strftimefmt);
char *log_getStrftimeFormat();

void log_setPrintfDateFormat(const char *printfdatefmt);
char *log_getPrintfDateFormat();

void log_setLevelFormat(const char *loglevelfmt);
char *log_getLevelFormat();

void log_setMask(int mask);
int log_getMask();

#endif

/*=============================================================================
// end of file: $RCSfile: log.h,v $
==============================================================================*/

