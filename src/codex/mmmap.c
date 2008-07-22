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

   $RCSfile: mmmap.c,v $   
   $Revision: 1.8 $
   $Author: dhakim $
   $Date: 2002/02/21 07:38:54 $
 
==============================================================================*/
#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <sys/mman.h>

#ifdef LINUX_OS

#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "err.h"

#define DEV_ZERO "/dev/zero"
#define TMP_TMPL "/tmp/mmmap-XXXXXX"

static void err_close(int);
static int is_this_file(int, const char *);

caddr_t mmmap(caddr_t addr, size_t len, int prot, int flags, int fd, off_t o) {
  if ((flags & MAP_SHARED) && 
		((flags & MAP_ANON) || is_this_file(fd, DEV_ZERO))) {

		int tmpfd;
		caddr_t retval;
      char tmpname[18] = TMP_TMPL;

      tmpfd = mkstemp(tmpname);
      if (tmpfd == -1) {
         return (caddr_t)-1;
      }

		if (unlink(tmpname) < 0) {
         err_w("IOError");
      }

		if (lseek(tmpfd, len - 1, SEEK_SET) == -1) {
			 err_close(tmpfd);
			 return MAP_FAILED;
		}

		if (write(tmpfd, "", 1) != 1) {
			 err_close(tmpfd);
			 return MAP_FAILED;
		}

		/* call mmap with flags clear of MAP_ANON bit */
		retval = mmap(addr, len, prot, flags & ~MAP_ANON, tmpfd, o);

		if (close(tmpfd) < 0) {
         err_w("IOError");
      }

		return retval;
  }
  return mmap(addr, len, prot, flags, fd, o);
}


/* 
 * Check whether FD refers to FILENAME. 
 * Return nonzero if so, zero otherwise.
 */
static int is_this_file(int fd, const char *filename) {
  struct stat fdinfo, nameinfo;
  /* i-node number is not unique ACROSS file systems */
  return !stat(filename, &nameinfo) && !fstat(fd, &fdinfo)
  && (fdinfo.st_ino == nameinfo.st_ino)
  && (fdinfo.st_dev == nameinfo.st_dev);
}


/* close FD with care on errno value */
static void err_close(int fd) {
  /* save original error number */
  int old_err;
  old_err = errno;
  if (close(fd) < 0) {
     err_w("IOError");
  }
  errno = old_err;
  return;
}

#else 
caddr_t mmmap(caddr_t addr, size_t len, int prot, int flags, int fd, off_t o) {
#ifdef MAP_ANON
      return mmap(addr, len, prot, flags | MAP_ANON , -1 , o);
#else
      return mmap(addr, len, prot, flags, fd, o);
#endif
}
#endif

/*=============================================================================
// end of file: $RCSfile: mmmap.c,v $
==============================================================================*/

