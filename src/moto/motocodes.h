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

   $RCSfile: motocodes.h,v $   
   $Revision: 1.6 $
   $Author: dhakim $
   $Date: 2002/10/29 00:21:53 $
 
==============================================================================*/
#ifndef __MOTOCODES_H
#define __MOTOCODES_H

/* result codes */
enum {
   MOTO_OK =           0,
   MOTO_FAIL =         1,
};

/* frame types */
enum {
   MAIN_FRAME =        1,
   SUB_FRAME =         2,
   INCLUDE_FRAME =     3,
   READDEFS_FRAME =    4,
   MACRO_FRAME =       5,
   FN_FRAME = 	       6
};

/* flags */
enum {
	DEBUG_FLAG =        0x0000001,
   MEM_LEAK_FLAG =     0x0000002,
	DEF_DL_FLAG =       0x0000004,	
   MACRO_DEBUG_FLAG =  0x0000008,
   MMAN_LEVEL1_FLAG =  0x0000010,
   MMAN_LEVEL2_FLAG =  0x0000020,
	EXEC_TEXT_FLAG =    0x0000040,
   /* execution modes */
   MOTOPP_FLAG =       0x0100000,
   MOTOV_FLAG =        0x0200000,
   MOTOI_FLAG =        0x0400000,
   MOTOC_FLAG =        0x0800000,
   MOTOD_FLAG =        0x1000000,
   MOTODIP_FLAG =	0x2000000,
 
};

/* limits */
enum {
	MAX_FRAMES =   500,
};

/* error codes */
enum {
   ERR_RUNAWAY =       1,
};

/* mx code types */
enum {
   MX_FN =             1,
   MX_METHOD =         2,
   MX_CONSTRUCTOR =    3,
};

#endif

/*=============================================================================
// end of file: $RCSfile: motocodes.h,v $
==============================================================================*/

