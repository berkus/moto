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

   $RCSfile: motofn.h,v $   
   $Revision: 1.11 $
   $Author: dhakim $
   $Date: 2002/10/29 00:21:53 $
 
==============================================================================*/
#ifndef __MOTOFN_H
#define __MOTOFN_H

/* Calls shared_init, mxdl_init, mman_init, err_init, log_init, and moto_errInit in
   the proper order */
void moto_init(size_t shared_heap,char* xpath);

/* Calls the necessary cleanup functions */
void moto_cleanup();

int moto_run(char *filename, int flags);
int moto_stdin(int flags);
int moto_files(int argc, char **argv, int flags,size_t shared_heap,char* mxpath);

void moto_out(int code);
void moto_done(int code);

void motopp_start(char *filename, int flags);
void motopp();
void motopp_out(int code);
void motopp_done(int code);

#endif

/*=============================================================================
// end of file: $RCSfile: motofn.h,v $
==============================================================================*/

