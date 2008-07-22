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

   $RCSfile: motopplex.h,v $   
   $Revision: 1.2 $
   $Author: kocienda $
   $Date: 2000/04/30 23:10:03 $
 
==============================================================================*/
#ifndef __MOTOPPLEX_H
#define __MOTOPPLEX_H

#include "pp.h"

void motopp_freeYYBuffer(struct yy_buffer_state *yybuf);
void motopp_beginInitial();
void motopp_freeYYStack();
void motopp_prepareFrameForParenthood(MotoPP *ppenv);
int motopp_prepareMainBuffer(MotoPP *ppenv, char *filename);
void motopp_prepareFrameBuffer(MotoPP *ppenv, char *text);
int motopp_prepareFileFrameBuffer(MotoPP *ppenv, char *filename);
void motopp_prepareMacroFrameForExec(MotoPP *ppenv, MotoMacro *m);
void motopp_prepareFrameForExec(MotoPP *ppenv, char *filename);

void motopp_freeRYYBuffer();
int motopp_prepareReaddefsFrameBuffer(MotoPP *ppenv, char *filename);

#endif

/*=============================================================================
// end of file: $RCSfile: motopplex.h,v $
==============================================================================*/

