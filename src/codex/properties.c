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

   $RCSfile: properties.c,v $   
   $Revision: 1.9 $
   $Author: kocienda $
   $Date: 2000/04/30 23:10:02 $
 
==============================================================================*/
#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "memsw.h"

#include "properties.h"
#include "stringbuffer.h"
#include "fileio.h"
#include "exception.h"
#include "excpfn.h"
#include "log.h"
#include "util.h"

Properties *prop_create(){
   Properties *props = emalloc(sizeof(Properties));
 
   props->properties = htab_create(
      10,
      string_hash,
      (int(*)(const void*, const void*))strcmp,
      0
   );
   return props;
}

void prop_free(Properties *props) {
   htab_free(props->properties);
   free(props);
}

Properties *prop_addFromFile(Properties *props, char* fname){
   char* propFileData;
   char* freep;
   StringBuffer* curMacroValue = NULL;
   StringBuffer* curMacroName = NULL; 

   fio_get(fname, &propFileData);
   /* save the orginal address of the text data so we can free it later */
   freep = propFileData;

   while(*propFileData){
      char* firstIndexOfEOF = strchr(propFileData,'\0');
      char* firstIndexOfEOL = strchr(propFileData,'\n');
      char* firstIndex = (firstIndexOfEOL < firstIndexOfEOF && firstIndexOfEOL != 0)
         ? firstIndexOfEOL + 1 : firstIndexOfEOF;

      if (*propFileData == '#') {

         propFileData = firstIndex;
         continue;

      } else if (isspace(*propFileData)){
         if (curMacroName != NULL) 
            buf_put(curMacroValue,propFileData, firstIndex - propFileData);

         propFileData = firstIndex;
         continue;

      } else {
         char* firstIndexOfWhite = strpbrk(propFileData," \t\n\r\f");

         if(curMacroName != NULL) {
            htab_put(props->properties,buf_toString(curMacroName),buf_toString(curMacroValue));
            
            buf_clear(curMacroName);
            buf_clear(curMacroValue);
         } else {
            curMacroName = buf_createDefault();
            curMacroValue = buf_createDefault();
         }

         buf_put(
            curMacroName,
            propFileData,
            firstIndexOfWhite - propFileData
         );

         propFileData = firstIndexOfWhite + 1;
         continue;
      }
   }

   if(curMacroName != NULL) {
      htab_put(props->properties,buf_toString(curMacroName),buf_toString(curMacroValue));
   }

   free(freep);
   if (curMacroName != NULL) {
      buf_free(curMacroName);
   }
   if (curMacroValue != NULL) {
      buf_free(curMacroValue);
   }

   return props;
}

Enumeration *prop_getNames(Properties *props){
   return htab_getKeys(props->properties);
}

char *prop_get(Properties *props, char* name){
   return htab_get(props->properties,name);
}

/*=============================================================================
// end of file: $RCSfile: properties.c,v $
==============================================================================*/

