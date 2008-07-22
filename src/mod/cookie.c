/*=============================================================================

   Copyright (C) 2001 David Hakim.
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

   $RCSfile: cookie.c,v $
   $Revision: 1.2 $
   $Author: dhakim $
   $Date: 2002/04/06 22:36:51 $

==============================================================================*/

#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define __MIMEENTITY_C

#include <string.h>
#include <stdio.h>

#include "memsw.h"
#include "cookie.h"
#include "stringbuffer.h"
#include "excpfn.h"
#include "mxstring.h"
#include "tokenizer.h"

Cookie* cook_createDefault(){
   Cookie* c = (Cookie*)ecalloc(1,sizeof(Cookie));
   return c;
}

Cookie* cook_create(
   char* name,
   char* value,
   char* comment,
   char* domain,
   char* maxage,
   char* path,
   char secure,
   char* version
){
   Cookie* c = (Cookie*)malloc(sizeof(Cookie));

   c->secure=secure;
   c->maxage=(maxage!=NULL?estrdup(maxage):NULL);
   c->comment=(comment!=NULL?estrdup(comment):NULL);
   c->name=(name!=NULL?estrdup(name):NULL);
   c->value=(value!=NULL?estrdup(value):NULL);
   c->domain=(domain!=NULL?estrdup(domain):NULL);
   c->path=(path!=NULL?estrdup(path):NULL);
   c->version=(version!=NULL?estrdup(version):NULL); 

   return c;
}

void cook_free(Cookie* c){
   if (c->maxage!=NULL) free(c->maxage);
   if (c->comment!=NULL) free(c->comment);
   if (c->name!=NULL) free(c->name);
   if (c->value!=NULL) free(c->value);
   if (c->domain!=NULL) free(c->domain);
   if (c->version!=NULL) free(c->version);

   free(c);
}

char* cook_toString(Cookie* c) {
   StringBuffer* buf = buf_createDefault();
   char* rval;

   buf_printf(buf,"%s=\"%s\";",c->name,c->value); /* FIXME: need to escape quotes */
   if (c->version != NULL) buf_printf(buf,"Version=\"%s\";",c->version);
   if (c->comment != NULL) buf_printf(buf,"Comment=\"%s\";",c->comment);
   if (c->domain != NULL) buf_printf(buf,"Domain=\"%s\";",c->domain);
   if (c->maxage != NULL) buf_printf(buf,"Max-Age=\"%s\";",c->maxage);
   if (c->path != NULL) buf_printf(buf,"Path=\"%s\";",c->path);
   if (c->secure) buf_printf(buf,"Secure");

   rval=buf_toString(buf);
   buf_free(buf);

   return rval;
}

Cookie* cook_parse(char* s,char isSetCookie) {
   Cookie* c = (Cookie*)ecalloc(1,sizeof(Cookie));
   Tokenizer* ctok = tok_createCTok(s,';'); 
   char* curtok;

   while((curtok = tok_next(ctok)) != NULL){
      char* varn = NULL,*varnTrimmed=NULL;
      char* varv = NULL,*varvTrimmed=NULL; 

      if(mxstr_indexOf(curtok,'=') != -1) {
         varn = mxstr_substring(curtok,0,mxstr_indexOf(curtok,'='));
         varv = mxstr_substr(curtok,mxstr_indexOf(curtok,'=')+1);
      } else {
         varn=curtok;
      }

      varnTrimmed = mxstr_trim(varn);
      varvTrimmed = mxstr_trim(varv);

      /* Pick off open and close quotes */

      if(varvTrimmed[0]=='\"' && varvTrimmed[strlen(varvTrimmed)-1]=='\"'){
         char* tmp = varvTrimmed;
         varvTrimmed = mxstr_substring(varvTrimmed,1,strlen(varvTrimmed)-1);
         free(tmp);
      }

      if(!strcasecmp((!isSetCookie?"$path":"path"),varnTrimmed)){
         c->path = varvTrimmed;
      } else if (!strcasecmp((!isSetCookie?"$domain":"domain"),varnTrimmed)){
         c->domain = varvTrimmed;
      } else if (!strcasecmp((!isSetCookie?"$version":"version"),varnTrimmed)){
         c->version = varvTrimmed;
      } else if (isSetCookie && !strcasecmp("max-age",varnTrimmed)){
         c->maxage = varvTrimmed;
      } else if (isSetCookie && !strcasecmp("comment",varnTrimmed)){
         c->comment = varvTrimmed;
      } else if (isSetCookie && !strcasecmp("secure",varnTrimmed)){
         c->secure = '\1';
      } else {
         c->name = estrdup(varnTrimmed);
         c->value = varvTrimmed;
      }
 
      free(varn);
      free(varv);
      free(varnTrimmed);
      free(curtok);
   }

   tok_free(ctok);

   return c;
}

char* cook_getName(Cookie* c) {return c->name;}
char* cook_getValue(Cookie* c) {return c->value;}
char* cook_getComment(Cookie* c) {return c->comment;}
char* cook_getDomain(Cookie* c) {return c->domain;}
char* cook_getMaxAge(Cookie* c) {return c->maxage;}
char* cook_getPath(Cookie* c) {return c->path;}
char* cook_getVersion(Cookie* c) {return c->version;}
char cook_isSecure(Cookie* c) {return c->secure;}

void cook_setName(Cookie* c,char* name){
   if (c->name != NULL) free(c->name); 
   c->name=(name != NULL?estrdup(name):NULL);
}
void cook_setValue(Cookie* c,char* value){
   if (c->value != NULL) free(c->value); 
   c->value=(value != NULL?estrdup(value):NULL);
}
void cook_setComment(Cookie* c,char* comment){
   if (c->comment != NULL) free(c->comment); 
   c->comment=(comment != NULL?estrdup(comment):NULL);
}
void cook_setDomain(Cookie* c,char* domain){
   if (c->domain != NULL) free(c->domain); 
   c->domain=(domain != NULL?estrdup(domain):NULL);
}
void cook_setMaxAge(Cookie* c,char* maxage){
   if (c->maxage != NULL) free(c->maxage); 
   c->maxage=(maxage != NULL?estrdup(maxage):NULL);
}
void cook_setPath(Cookie* c,char* path){
   if (c->path != NULL) free(c->path); 
   c->path=(path != NULL?estrdup(path):NULL);
}
void cook_setVersion(Cookie* c,char* version){
   if (c->version != NULL) free(c->version); 
   c->version=(version != NULL?estrdup(version):NULL);
}
void cook_setSecure(Cookie* c,char secure){ c->secure=secure;}

