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

   $RCSfile: cdate.c,v $
   $Revision: 1.8 $
   $Author: shayh $
   $Date: 2003/01/24 16:31:11 $

==============================================================================*/
#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <time.h>
#include <string.h>

#include "cdate.h"
#include "memsw.h"
#include "exception.h"
#include "excpfn.h"


#include <stdio.h>

int 
secondsSinceTheEpoch(){
   return (int)time(NULL);
}

Date* 
date_createDefault(){
   Date *date = (Date*)emalloc(sizeof(Date));
   time_t sec = time(NULL);
   localtime_r(&sec, (struct tm*)date);
   mktime((struct tm*)date);
   return date;
}
	
Date* 
date_create(char* format, char* date){
   Date *d;

   if (date == NULL)
      return NULL;

   d = (Date*)emalloc(sizeof(Date));
   strptime(date, format, (struct tm*)d);
   return d;
}

void 
date_free(Date* this){
   free(this);
}

char* 
date_format(Date* this, char* format){
   //strlen(format)*2 didn't work. If format = "%Y" then
   // len = 4, which is not big enough to hold the '\0' 
   // so strftime() returns 0
   int len = 257;
   char* s = (char*)ecalloc(sizeof(char), len);
   strftime(s, len, format, (struct tm*)this);
   return s;
}

int 
date_getSeconds(Date* this){
   return this->tm_sec;
}

int 
date_getMinutes(Date* this){
   return this->tm_min;
}

int 
date_getHour(Date* this){
   return this->tm_hour;
}

int 
date_getMday(Date* this){
   return this->tm_mday;
}

int 
date_getMon(Date* this){
   return this->tm_mon;
}

int 
date_getYear(Date* this){
   return this->tm_year;
}

int 
date_getWday(Date* this){
   return this->tm_wday;
}

int 
date_getYday(Date* this){
   return this->tm_yday;
}


