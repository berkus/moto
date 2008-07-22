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

   $RCSfile: motohook.c,v $   
   $Revision: 1.6 $
   $Author: dhakim $
   $Date: 2002/03/14 09:07:26 $
 
==============================================================================*/
#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "motohook.h"

extern MotoEnv *env;
extern MotoPP *ppenv;

void 
moto_setEnv(MotoEnv *envp) {
   env = envp;   
}

void 
motopp_setEnv(MotoPP *ppenvp) {
   ppenv = ppenvp;   
}

MotoEnv *
moto_getEnv() {
   return env;
}

MotoPP *
motopp_getEnv() {
   return ppenv;
}

/*=============================================================================
// end of file: $RCSfile: motohook.c,v $
==============================================================================*/

