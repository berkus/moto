/*=============================================================================

   Copyright (C) 2000 David Hakim.
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

   $RCSfile: forms.h,v $
   $Revision: 1.4 $
   $Author: dhakim $
   $Date: 2002/01/23 22:31:23 $

==============================================================================*/
#ifndef __FORMS_H
#define __FORMS_H

#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

char* form_textarea( char* name, char* value, int rows, int cols);

char* form_textfieldNameOnly( char* name);

char* form_textfieldNameAndValue( char* name,char* value);

char* form_textfieldNameAndLength( char* name,int length);

char* form_textfieldNameValueAndLength( char* name,char* value,int length);

char* form_textfield( char* name, char* value, int length, int maxlength);

char* form_checkbox(char* name, char* value, char checked);

char* form_multicheckbox(char* name, char* value, char checked);

char* form_radio(char* name, char* value, char checked);

char* form_option(char* name, char* value, char* selected);

char* form_multioption(char* name, char* value, char* selected);

char* form_hidden(char* name, char* value);

#endif

/*=============================================================================
// end of file: $RCSfile: forms.h,v $
==============================================================================*/
