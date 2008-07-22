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

   $RCSfile: strptime.c,v $
   $Revision: 1.3 $
   $Author: shayh $
   $Date: 2003/01/24 16:30:37 $
   
==============================================================================*/

#include <ctype.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "excpfn.h"

static const int tm_year_base = 1900;

static const char *abb_weekdays[] = {
  "Sun", "Mon", "Tues", "Wed",
  "Thu", "Fri", "Sat", NULL
};

static const char *full_weekdays[] = {
  "Sunday", "Monday", "Tuesday",
  "Wednesday", "Thursday", "Friday",
  "Saturday", NULL
};

static const char *abb_month[] = {
  "Jan", "Feb", "Mar", "Apr", "May",
  "Jun", "Jul", "Aug", "Sept", "Oct",
  "Nov", "Dec", NULL
};

static const char *full_month[] = {
  "January", "February", "March",
  "April", "May", "June", "July",
  "August", "September", "October",
  "November", "December", NULL
};

static const char *ampm[] = {
  "am", "pm", NULL
};

static const char *tzoffsets[] = {
  "-00", "-01", "-02", "-03", "-04", "-05",
  "-06", "-07", "-08", "-09", "-10", "-11",
  "-12", "-13", "-14", "-15", "-16", "-17",
  "-18", "-19", "-20", "-21", "-22",

  "+00", "+01", "+02", "+03", "+04", "+05",
  "+06", "+07", "+08", "+09", "+10", "+11",
  "+12", "+13", "+14", "+15", "+16", "+17",
  "+18", "+19", "+20", "+21", "+22", NULL
};


static int
match_string(const char **buf, const char **strs)
{
  int i;

  for (i = 0; strs[i] != NULL; i++){
    int len = strlen(strs[i]);

    if (estrncasecmp(*buf, strs[i], len) == 0){
      *buf += len;
      return i;
    }
  }

  return -1;
}


static int
is_leap_year(int year)
{
  return (year % 4) == 0 && ((year % 100) != 0 || (year % 400) == 0);
}


static int
first_day(int year)
{
  int ret = 4;

  for (; year > 1970; year--)
    ret = (ret + 365 + is_leap_year(year) ? 1 : 0) % 7;

  return ret;
}


static void
set_week_number_sun(struct tm *timeptr, int wnum)
{
  int fday = first_day(timeptr->tm_year + tm_year_base);
  timeptr->tm_yday = wnum * 7 + timeptr->tm_wday - fday;

  if (timeptr->tm_yday < 0){
    timeptr->tm_wday = fday;
    timeptr->tm_yday = 0;
  }
}


static void
set_week_number_mon(struct tm *timeptr, int wnum)
{
  int fday = (first_day(timeptr->tm_year + tm_year_base) + 6) % 7;
  timeptr->tm_yday = wnum * 7 + (timeptr->tm_yday + 6) % 7 - fday;

  if (timeptr->tm_yday < 0){
    timeptr->tm_wday = (fday + 1) % 7;
    timeptr->tm_yday = 0;
  }
}


static void
set_week_number_mon4(struct tm *timeptr, int wnum)
{
  int fday = (first_day(timeptr->tm_year + tm_year_base) + 6) % 7;
  int offset = 0;

  if (fday < 4)
    offset += 7;

  timeptr->tm_yday = offset + (wnum - 1) * 7 + timeptr->tm_wday - fday;

  if (timeptr->tm_yday < 0){
    timeptr->tm_wday = fday;
    timeptr->tm_yday = 0;
  }
}


char *
xstrptime(const char *buf, const char *format, struct tm *timeptr)
{
  char c;
  char t[33];

  memset(timeptr, 0, sizeof(struct tm));

  for (; (c = *format) != '\0'; format++){
    char *s;
    int ret;

    if (isspace(c)){
      while (isspace(*buf))
        buf++;
    }
    else if (c == '%' && format[1] != '\0'){
      c = *++format;

      if (c == 'E' || c == 'O')
        c = *++format;

RETRY:
      memset(t, '\0', 33);

      switch (c){
        case 'A':
          ret = match_string(&buf, full_weekdays);
          if (ret < 0)
            return NULL;
          timeptr->tm_wday = ret;
          break;

        case 'a':
          ret = match_string(&buf, abb_weekdays);
          if (ret < 0)
            return NULL;
          timeptr->tm_mon = ret;
          break;

        case 'B':
          ret = match_string(&buf, full_month);
          if (ret < 0)
            return NULL;
          timeptr->tm_mon = ret;
          break;

        case 'b':
        case 'h':
          ret = match_string(&buf, abb_month);
          if (ret < 0){
              c = 'H';
              goto RETRY;
          }
          else{
            timeptr->tm_mon = ret;
            break;
          }

        case 'C':
          ret = strtol(buf, &s, 10);
          if (s == buf)
            return NULL;
          timeptr->tm_year = (ret * 100) - tm_year_base;
          buf = s;
          break;

        // This also designates MySQL ordinal date
        case 'D':
          s = xstrptime(buf, "%m/%d/%y", timeptr);
          if (s == NULL){
            ret = strtol(buf, &s, 10);
            if (s == buf)
              return NULL;

            timeptr->tm_mday = ret;
            buf = s+2;
          }
          else{
            buf = s;
          }

          break;

        case 'd':
        case 'e':
          strncpy(t, buf, 2);
          ret = strtol(t, &s, 10);
          if (s == t)
            return NULL;
          timeptr->tm_mday = ret;
          buf += 2;
          break;

        case 'H':
        case 'k':
          strncpy(t, buf, 2);
          ret = strtol(t, &s, 10);
          if (s == t)
            return NULL;
          timeptr->tm_hour = ret;
          buf += 2;
          break;

        case 'I':
        case 'l':
          ret = strtol(buf, &s, 10);
          if (s == buf)
            return NULL;
          if (ret == 12)
            timeptr->tm_hour = 0;
          else
            timeptr->tm_hour = ret;
          buf = s;
          break;

        case 'j':
          ret = strtol(buf, &s, 10);
          if (s == buf)
            return NULL;
          timeptr->tm_yday = ret - 1;
          buf = s;
          break;

        case 'm':
        case 'c':
          strncpy(t, buf, 2);
          ret = strtol(t, &s, 10);
          if (s == t)
            return NULL;

          if ((ret < 1) || (ret > 12))
            return NULL;

          timeptr->tm_mon = ret - 1;
          buf += 2;
          break;

        case 'M':
        case 'i':
          strncpy(t, buf, 2);
          ret = strtol(t, &s, 10);
          if ((s == t) && (c == 'M')){
            c = 'B';
            goto RETRY;
          }
          timeptr->tm_min = ret;
          buf += 2;
          break;

        case 'n':
          if (*buf == '\n')
            ++buf;
          else
            return NULL;
          break;

        case 'p':
          ret = match_string(&buf, ampm);
          if (ret < 0)
            return NULL;
          if (timeptr->tm_hour == 0){
            if (ret == 1)
              timeptr->tm_hour = 12;
          }
          else{
            timeptr->tm_hour += 12;
          }
          break;

        case 'r':
          s = xstrptime(buf, "%I:%M:%S %p", timeptr);
          if (s == NULL)
            return NULL;
          buf = s;
          break;

        case 'R':
          s = xstrptime(buf, "%H:%M", timeptr);
          if (s == NULL)
            return NULL;
          buf = s;
          break;

        case 'S':
        case 's':
          strncpy(t, buf, 2);
          ret = strtol(t, &s, 10);
          if (s == t)
            return NULL;
          timeptr->tm_sec = ret;
          buf += 2;
          break;

        case 't':
          if (*buf == '\t')
            ++buf;
          else
            return NULL;
          break;

        case 'T':
        case 'X':
          s = xstrptime(buf, "%H:%M:%S", timeptr);
          if (s == NULL)
            return NULL;
          buf = s;
          break;

        case 'u':
          ret = strtol(buf, &s, 10);
          if (s == buf)
            return NULL;
          timeptr->tm_wday = ret - 1;
          buf = s;
          break;

        case 'w':
          ret = strtol(buf, &s, 10);
          if (s == buf)
            return NULL;
          timeptr->tm_wday = ret;
          buf = s;
          break;

        case 'U':
          ret = strtol(buf, &s, 10);
          if (s == buf)
            return NULL;
          set_week_number_sun(timeptr, ret);
          buf = s;
          break;

        case 'V':
          ret = strtol(buf, &s, 10);
          if (s == buf)
            return NULL;
          set_week_number_mon4(timeptr, ret);
          buf = s;
          break;

        case 'W':
          ret = strtol(buf, &s, 10);
          if (s == buf){
            c = 'B';
            goto RETRY;
          }
          set_week_number_mon(timeptr, ret);
          buf = s;
          break;

        case 'x':
          s = xstrptime(buf, "%Y:%m:%d", timeptr);
          if (s == NULL)
            return NULL;
          buf = s;
          break;

        case 'y':
          strncpy(t, buf, 2);
          ret = strtol(t, &s, 10);
          if (s == t)
            return NULL;
          if (ret < 70)
            timeptr->tm_year = 100 + ret;
          else
            timeptr->tm_year = ret;
          buf += 2;
          break;

        case 'Y':
          strncpy(t, buf, 4);
          ret = strtol(t, &s, 10);
          if (s == t)
            return NULL;
          timeptr->tm_year = ret - tm_year_base;
          buf += 4;
          break;

        case 'Z':
#if defined (__GLIBC__)
          ret = match_string(&buf, tzoffsets);

          if (ret < 0)
            return NULL;

          timeptr->tm_zone = tzoffsets[ret];
#endif
          buf += 3;
          break;

        case '\0':
          --format;

        case '%':
          if (*buf == '%')
            ++buf;
          else
            return NULL;
          break;
      }
    }
    else{
      if (*buf == c)
        ++buf;
      else
        return NULL;
    }

	}

  return (char*)buf;
}

