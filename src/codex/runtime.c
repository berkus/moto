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

   $RCSfile: runtime.c,v $   
   $Revision: 1.11 $
   $Author: dhakim $
   $Date: 2003/01/29 04:58:42 $
 
==============================================================================*/
#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>

#include "memsw.h"

#include "exception.h"
#include "excpfn.h"
#include "runtime.h"

static Runtime *rtime;

void 
rtime_init() {
   rtime = emalloc(sizeof(Runtime));
   rtime->mpool = mpool_create(4096);
}

void 
rtime_cleanup() {
   mpool_free(rtime->mpool);
   free(rtime);
}

Runtime *
rtime_getRuntime() {
   return rtime;
}

inline MPool *
rtime_getMPool() {
   return rtime->mpool;
}

void 
rtime_clearMPool() {
   mpool_clear(rtime->mpool);
}

Exception *
rtime_getException() {
   return rtime->e;
}

/*=============================================================================
// end of file: $RCSfile: runtime.c,v $
==============================================================================*/

