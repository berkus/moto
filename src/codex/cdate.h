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

   $RCSfile: cdate.h,v $
   $Revision: 1.4 $
   $Author: shayh $
   $Date: 2002/12/19 22:20:00 $

==============================================================================*/
#ifndef __CDATE_H
#define __CDATE_H

#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <ctype.h>
#include <time.h>

typedef struct tm Date;

#define strptime(b,f,t) xstrptime(b,f,t)
extern char* xstrptime(const char *buf, const char *format, struct tm *timeptr);

Date* date_createDefault();
	
Date* date_create(char* format,char* date);
void date_free(Date* this);

char* date_format(Date* this,char* format);

int date_getSeconds(Date* this);
int date_getMinutes(Date* this);
int date_getHour(Date* this);
int date_getMday(Date* this);
int date_getMon(Date* this);
int date_getYear(Date* this);
int date_getWday(Date* this);
int date_getYday(Date* this);

int secondsSinceTheEpoch();

#endif

