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

   $RCSfile: mimeentity.c,v $
   $Revision: 1.6 $
   $Author: dhakim $
   $Date: 2002/12/05 02:40:48 $

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
#include "mimeentity.h"
#include "excpfn.h"
#include "mxstring.h"
#include "tokenizer.h"

excp_declare(MIMEParseException);

void mime_free(MIMEEntity* mm){
   Enumeration *e = stab_getValues(mm->headers);

   /* free the values */
   while(enum_hasNext(e)){
      char* curVal = (char*)enum_next(e);
      free(curVal);
   }
   enum_free(e);

   /* free the keys */ 
   e = stab_getKeys(mm->headers);
   while(enum_hasNext(e))
      free(enum_next(e));
   enum_free(e); 

   /* free the member variables */
   if (mm->name != NULL) 
      free(mm->name);
   if (mm->type != NULL) 
      free(mm->type);
   if (mm->encoding != NULL) 
      free(mm->encoding);
   if (mm->charset != NULL) 
      free(mm->charset);
   if (mm->filename != NULL) 
      free(mm->filename);
   if (mm->disposition != NULL) 
      free(mm->disposition);
   if (mm->body != NULL)
      free(mm->body);

   stab_free(mm->headers);
   free(mm);
}

MIMEEntity* mime_create(char* msg,size_t msglen){
   char* eol;
   MIMEEntity * mm;

   char* curHeader = msg;

   mm = (MIMEEntity*)emalloc(sizeof(MIMEEntity));
   mm->headers= stab_createDefault();

   /* Content-disposition header fields */
   mm->name = NULL;
   mm->filename = NULL;
   mm->disposition = NULL;

   /* Content-type header fields */ /* Remind me ... why are we providing defaults for these ? */
   mm->type = estrdup("text/plain");
   mm->charset = estrdup("us-ascii");

   /* Content-transfer-encoding header fields */
   mm->encoding = estrdup("7bit");  /* 7bit, 8bit, binary, quoted-printable, 
                                     * base64, ietf-token, x-token */

   /* parse headers */

   while((eol = strstr(curHeader,"\r\n")) != NULL){
      char curHeaderLine[255];
      char* eoh;
      int hls = (int)(eol - curHeader);

      if(hls == 0)  
         break;

      if(hls < 254){
         strncpy(curHeaderLine,curHeader,hls);
         curHeaderLine[hls] = '\0';
      }else {
         THROW("MIMEParseException","Header line too long");
      }

      curHeader = eol + 2;

      eoh = strchr(curHeaderLine,':');
      if (eoh==NULL) { /* FIXME: Why are we ignoring this again ? */
         continue;
         /*
         THROW("MIMEParseException","Invalid header"); */
      }
      *eoh = '\0';

      stab_put(mm->headers,estrdup(curHeaderLine),estrdup(eoh + 1));
   
      if(!strcasecmp("Content-disposition",curHeaderLine)){
         Tokenizer* ctok = tok_createCTok(eoh+1,';');
         char* curtok;

         if((curtok=tok_next(ctok)) != NULL){
            if (mm->disposition != NULL) free (mm->disposition);
            mm->disposition = mxstr_trim(curtok); 
            free(curtok);
         }
         while((curtok = tok_next(ctok)) != NULL){
            char* varn = mxstr_substring(curtok,0,mxstr_indexOf(curtok,'='));
            char* varv = mxstr_substr(curtok,mxstr_indexOf(curtok,'=')+1);

            char* varnTrimmed = mxstr_trim(varn);
            char* varvTrimmed = mxstr_trim(varv);

            /* FIXME: Pick off open and close quotes */

            if(varvTrimmed[0]=='\"' && varvTrimmed[strlen(varvTrimmed)-1]=='\"'){
               char* tmp = varvTrimmed;
               varvTrimmed = mxstr_substring(varvTrimmed,1,strlen(varvTrimmed)-1);
               free(tmp);
            }
 
            if(!strcasecmp("name",varnTrimmed)){
               if (mm->name != NULL) free(mm->name);
               mm->name = varvTrimmed;
            } else if (!strcasecmp("filename",varnTrimmed)){
               if (mm->filename != NULL) free(mm->filename);
               mm->filename = varvTrimmed;
            } else {
               free(varvTrimmed);
            }

            free(varn);
            free(varv);
            free(varnTrimmed);
            free(curtok);
         }
         tok_free(ctok);
      } else if(!strcasecmp("Content-type",curHeaderLine)){
         Tokenizer* ctok = tok_createCTok(eoh+1,';');
         char* curtok;

         if((curtok=tok_next(ctok)) != NULL){
            if (mm->type != NULL) free(mm->type);
            mm->type = mxstr_trim(curtok);
            free(curtok);
         }
         while((curtok = tok_next(ctok)) != NULL){
            char* varn = mxstr_substring(curtok,0,mxstr_indexOf(curtok,'='));
            char* varv = mxstr_substr(curtok,mxstr_indexOf(curtok,'=')+1);

            char* varnTrimmed = mxstr_trim(varn);
            char* varvTrimmed = mxstr_trim(varv);

            /* FIXME: Pick off open and close quotes */

            if(varvTrimmed[0]=='\"' && varvTrimmed[strlen(varvTrimmed)-1]=='\"'){
               char* tmp = varvTrimmed;
               varvTrimmed = mxstr_substring(varvTrimmed,1,strlen(varvTrimmed)-1);
               free(tmp);
            }

            if(!strcasecmp("charset",varnTrimmed)){
               if(mm->charset != NULL) free(mm->charset);
               mm->charset = varvTrimmed;
            } else {
               free(varvTrimmed);
            }

            free(varn);
            free(varv);
            free(varnTrimmed);
            free(curtok);
         }
         tok_free(ctok);

      } else if(!strcasecmp("Content-transfer-encoding",curHeaderLine)){
         Tokenizer* ctok = tok_createCTok(eoh+1,';');
         char* curtok;

         if((curtok=tok_next(ctok)) != NULL){
            mm->encoding = mxstr_trim(curtok);
            free(curtok);
         }
         tok_free(ctok);
      }
   }
   
   if(eol != NULL){
      mm->bodylen = msglen-((eol+2)-msg);
      if(mm->bodylen > 0)
         mm->body = emalloc(mm->bodylen);
      else {
         THROW("MIMEParseException","No Body");
      }
      memcpy(mm->body,eol+2,mm->bodylen);
   }

   return mm;

}

/*
char* mime_getVersion(MIMEEntity* mm){
   return mm->version;
}
*/

char* mime_getContentType(MIMEEntity* mm){
   return mm->type !=NULL ? estrdup((char*)mm->type) : NULL;
}

char* mime_getContentTransferEncoding(MIMEEntity* mm){
   return mm->encoding!=NULL ? estrdup((char*)mm->encoding) : NULL;
}

char* mime_getName(MIMEEntity* mm){
   return mm->name!=NULL ? estrdup((char*)mm->name) : NULL;
}

char* mime_getFileName(MIMEEntity* mm){
   return mm->filename!=NULL ? estrdup((char*)mm->filename) : NULL;
}

char* mime_getHeader(MIMEEntity* mm, char* hdr){
   return (char*)stab_get(mm->headers,hdr)!=NULL ? estrdup((char*)stab_get(mm->headers,hdr)) : NULL;
}

size_t mime_getBodyLength(MIMEEntity* mm){
   return mm->bodylen;
}

char* mime_getBody(MIMEEntity* mm){
   char* r = (char*)emalloc(mm->bodylen);
   memcpy(r,mm->body,mm->bodylen);
   return r;
}

UnArray* mime_getBody_yarr(MIMEEntity* mm){
   UnArray*	arr;
   
   arr = arr_create(1,0,0,(int*)&mm->bodylen,ARR_BYTE_TYPE);
   memcpy(&arr->ya.data[0],mm->body,mm->bodylen);
   return arr;
}

char* mime_getBodyAsString(MIMEEntity* mm){
   char* r = (char*)emalloc(mm->bodylen + 1);
   memcpy(r,mm->body,mm->bodylen);
   r[mm->bodylen]='\0';
   return r;
}

/*
MIMEPart* mime_getPart(int part){
}

Enumeration* mime_getParts(){
}
*/

