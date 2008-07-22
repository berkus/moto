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

   $RCSfile: fileio.h,v $   
   $Revision: 1.3 $
   $Author: kocienda $
   $Date: 2000/04/30 23:10:01 $
 
==============================================================================*/
#ifndef __FILEIO_H
#define __FILEIO_H

#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/**
 * Checks if a file exists.
 * @param filename the filename to check. This name can be a relative or 
 * absolute path
 * @return non-zero if a file with the given file name exists, zero if
 * it does not.
 */
int fio_exists(const char *filename);

int fio_get(const char *filename, char **text);
int fio_put(const char *filename, const char *text);
int fio_append(const char *filename, const char *text);

int fio_get_bytes(const char *filename, void **data);
int fio_put_bytes(const char *filename, void *data, int length);
int fio_append_bytes(const char *filename, void *data, int length);

#endif

/*=============================================================================
// end of file: $RCSfile: fileio.h,v $
==============================================================================*/

