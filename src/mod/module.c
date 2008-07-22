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

   $RCSfile: module.c,v $   
   $Revision: 1.42 $
   $Author: dhakim $
   $Date: 2003/03/20 22:33:08 $
 
==============================================================================*/
#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define __MODULE_C

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "kmpstring.h"
#include "tokenizer.h"
#include "mimeentity.h"
#include "symboltable.h"
#include "vector.h"
#include "module.h"
#include "excpfn.h"
#include "mxstring.h"
#include "cookie.h"
#include "integer.h"
#include "memsw.h"

excp_declare(HTTPRedirectException);

request_rec *current_request;
State *current_state;
table *current_options;
Vector *current_parts;
SymbolTable *current_cookies;
char headers_already_sent;

int 
util_read(request_rec *r, const char **rbuf, size_t* length) {
   int rc;
   
   /* Set some default values in case the post was empty */
   *rbuf = "";
   *length = 0;

   if((rc=ap_setup_client_block(r,REQUEST_CHUNKED_ERROR)) != OK) {
      return rc;
   }

   if (ap_should_client_block(r)) {
      char argsbuffer[HUGE_STRING_LEN];
      int rsize, len_read,rpos=0;
      *length = r->remaining;
      *rbuf = ap_pcalloc(r->pool,*length+1);

      ap_hard_timeout("util_read", r);
      while((len_read = ap_get_client_block(r,argsbuffer,HUGE_STRING_LEN))>0) {
         ap_reset_timeout(r);
         if ((rpos + len_read) > *length) {
            rsize=*length - rpos;
         } else {
            rsize=len_read;
         }
         memcpy((char*)*rbuf + rpos,argsbuffer,rsize);
         rpos += rsize;
      }
      ap_kill_timeout(r);
   }
   return rc;
}

State *
addNVPairsFromQS(State *state, request_rec *r, const char *qs) {
   char *key;
   char *val;
   SymbolTable* stab;
   Enumeration* e;

   if(qs) {

      stab = stab_createDefault();

      while(*qs && (val = ap_getword(r->pool, &qs,'&'))) {
         char* oldVal;

         key = ap_getword(r->pool, (const char **)&val ,'=');

         for(oldVal = val; *oldVal; oldVal++)
            if (*oldVal == '+')
               *oldVal = ' ';

         ap_unescape_url((char*)key);
         ap_unescape_url((char*)val);
                  
         if((oldVal = stab_get(stab,key)) == NULL)
            stab_put(stab,key,val);
         else {
            int l1 = strlen(oldVal), l2=strlen(val);
            char* newVal = ap_palloc(r->pool,l1+l2+2);

            strncpy(newVal,oldVal,l1);
            newVal[l1]='|';
            strncpy(newVal+l1+1,val,l2+1);
            stab_put(stab,key,newVal);

         }

      }

      e = stab_getKeys(stab);
      while(enum_hasNext(e)){
         key = (char*)enum_next(e);
         val = (char*)stab_get(stab,key);
         state_setValue(state,key,val);
      }
      enum_free(e);
      stab_free(stab);
   }

   return state;
}

static inline 
char l42h(unsigned char c){
   if(c%16 < 10) return '0'+c%16;
   else return 'A'+c%16-10;
}

static inline 
char h42h(unsigned char c){
   if(c/16%16 < 10) return '0'+c/16%16;
   else return 'A'+c/16%16-10;
}

char* 
urlEncode(unsigned char* s){
   StringBuffer* buf;
   char* encoded;

   if (s==NULL) {
      THROW_D("NullPointerException");
   }

   buf = buf_create(strlen(s)*2);
   while(*s){
      switch(*s){
         case ';': case '/': case '?':
         case ':': case '@': case '=': case '&': 
            /* Reserved Characters */
         case 32:case 34:case 60:case 62:
         case 35:case 37:case 123:case 125:
         case 124:case 92:case 94:case 126:
         case 91:case 93:case 96:
            /* Unsafe characters */ 
            buf_putc(buf,'%');buf_putc(buf,h42h(*s));buf_putc(buf,l42h(*s));break;
         default:
            if(*s<32 || *s==127) {
               /* ASCII Control characters */
               buf_putc(buf,'%');buf_putc(buf,h42h(*s));buf_putc(buf,l42h(*s));
            } else if(*s>127) {
               /* Non ASCII characters */
               buf_putc(buf,'%');buf_putc(buf,h42h(*s));buf_putc(buf,l42h(*s));
            } else {
               buf_putc(buf,*s);
            } 
      }
      s++;
   }
   encoded = buf_toString(buf);
   buf_free(buf);

   return encoded;
}

char*
urlDecode(unsigned char* s){
   StringBuffer* buf;
   char* decoded;
   
   if (s==NULL) {
      THROW_D("NullPointerException");
   }
   
   buf = buf_create(strlen(s));
   while(*s){
   	if(*s=='%') {
   		int h,l;
   		s++; 
   		if(*s && (( *s >= 'A' && *s <= 'F' )||( *s >= '0' && *s <= '9' )) ) 
   			h=*s; 
   		else { 
   			buf_putc(buf,'%'); 
   			continue; 
   		}
   		s++; 
   		if(*s && (( *s >= 'A' && *s <= 'F' )||( *s >= '0' && *s <= '9' )) ) 
   			l=*s; 
   		else { 
   			buf_putc(buf,'%'); 
   			buf_putc(buf,h); 
   			continue; 
   		}
   		buf_putc(buf,(h>='A' && h<='F'?h-'A' +10:h-'0')*16 +(l>='A' && l<='F'?l-'A'+10:l-'0'));
   	} else {
   		buf_putc(buf,*s);
   	}
   	s++;
   }
   
   decoded = buf_toString(buf);
   buf_free(buf);
   return decoded;
}

State *
getParentStateFromQS(request_rec *r, const char *qs) {
   char *key;
   char *val;
   if(qs) {
      while(*qs && (val=ap_getword(r->pool,&qs,'&'))) {
         key = ap_getword(r->pool, (const char **)&val, '=');
         if(!strcmp("sid",key)) {
            return getStateBySID((char *)val);
         }
      }
   }
   return NULL;
}

/*
 * mdfac: The number of regular expressions
 * mdfav: The array of compiled regular expressions
 * s: The character string bieng tokenized
 * ml: The index if mdfav of the match
 *
 * returns: the length of the match or -1 if no match was made
 */

int 
lex_match(int mdfac, MDFA** mdfav, char* s, int* mrx){
   int i,lm = -1;
   for(i=0;i<mdfac;i++){
      int ml = mdfa_starts(mdfav[i],s);
      if (ml > lm){ *mrx = i; lm = ml; }
   }
   return lm;
}

MDFA** 
lex_constructLexer(int rxc, char** rxv){
  int i;
  MDFA** lexer = (MDFA**)emalloc(rxc * sizeof(MDFA*));
  for(i=0;i<rxc;i++){
     lexer[i] = mdfa_createFromRX(rxv[i]);
     /* printf("Parsed regex /%s/\n",rxv[i]); */
  }
  return lexer;
}

static MDFA** cookie_lexer;
static char* cookie_rxs[] =
{
   "[;,]",
   "[ \\t\\n\\r]",
   "=",
   "\"([^\"]|\\\\\")*\"",
   "\\$[A-Za-z0-9_]*",
   "[A-Za-z0-9_]*"
};

void 
cook_init() {
   cookie_lexer = lex_constructLexer(6,cookie_rxs);
}

SymbolTable *
parseCookies(request_rec *r){
   SymbolTable* cookies = stab_createDefault(cookies);
   char* s = (char*)ap_table_get(r->headers_in,"Cookie");
   Cookie* curCookie=NULL;
   char *aname=NULL,*avalue=NULL,*version = NULL;
   char failure='\0';

   if(s==NULL) return cookies;

   while(*s){
      int mrx,ml;

      ml = lex_match(6,cookie_lexer,s,&mrx);
      if (ml <= 0) {fprintf(stderr,"### Failed to parse all cookies from '%s' syntax error before '%s'\n",
                      ap_table_get(r->headers_in,"Cookie"),s); failure='\1'; break;}

      switch(mrx) {
         case 0: /* End of Attribute - Time to find out what goes where */ 
            if (!strcasecmp("$domain",aname)) { curCookie->domain=avalue; free(aname); }
            else if (!strcasecmp("$path",aname)) { curCookie->path=avalue; free(aname); }
            else if (!strcasecmp("$version",aname)) { version = avalue; free(aname);  }
            else { curCookie->name = aname; curCookie->value = avalue; }
            aname = avalue = NULL;
            break;
         case 1: /* Whitespace - Always ignore */ 
            break;
         case 2: /* EQ - For the time bieng I will assume the cookie is well formed */ 
            break;
         case 3: /* Quoted String - Should always be an attribute value, 
                  * need to dequote and set */
            avalue = mxstr_substring(s,1,ml-1); /* FIXME: This is bad dequoting */ 
            break;
         case 4: /* Unquotes String starting with $ - Either $path,$version, or $domain */
            aname = mxstr_substring(s,0,ml);
         case 5: /* Unquoted String - It's either new cookie time or 
                  * we've got an old browser on our hands */
            if (aname != NULL) avalue = mxstr_substring(s,0,ml); /* old browser */
            else {
               if (curCookie != NULL && curCookie->name != NULL) 
                  stab_put(cookies,curCookie->name,curCookie);
               aname = mxstr_substring(s,0,ml);
               curCookie = cook_createDefault();
               curCookie->version = version; 
            }
      }

      /* fprintf(stderr,"Matched /%s/ : %s\n",cookie_rxs[mrx],mxstr_substring(s,0,ml)); */
      s += ml;
   }

   if (aname != NULL && curCookie != NULL && failure == '\0') {

      /*People should really end their headers with a ; */

      if (!strcasecmp("$domain",aname)) { curCookie->domain=avalue; free(aname); }
      else if (!strcasecmp("$path",aname)) { curCookie->path=avalue; free(aname); }
      else if (!strcasecmp("$version",aname)) { version = avalue; free(aname); }
      else {curCookie->name = aname; curCookie->value = avalue; } 
   }

   if (curCookie != NULL && failure == '\0') stab_put(cookies,estrdup(curCookie->name),curCookie);

   return cookies;
}

Vector *
parseMIMEPartsFromPost(request_rec *r){
   char *data;
   size_t postlen;
   char* ctype = (char*)ap_table_get(r->headers_in,"Content-type");
   char* boundary = strstr(ctype,"boundary=")+9;

   if (util_read(r, (const char **)&data,&postlen) == OK) {
      KMPString* kmpBoundary;
      Vector* parts;
      char *partStart,*partEnd;

      parts = vec_createDefault();
      kmpBoundary = kmp_create(boundary);
      partStart = partEnd = data;

      while (1){
         partStart = kmp_memstr(kmpBoundary,partEnd,postlen-(partEnd-data));
         if(partStart != NULL)
            partStart = strchr(partStart,'\n');
         if (partStart != NULL)
            partStart = partStart + 1;
         else
            break;

         partEnd = kmp_memstr(kmpBoundary,partStart,postlen-(partEnd-data));
         if(partEnd != NULL){
            while (partEnd > partStart && *partEnd != '\r')
               partEnd --;
            if (*partEnd != '\r')
               break;
         } else
            break;

         TRY{
            MIMEEntity* mm;
            size_t msglen = partEnd - partStart;
            char* msg = (char*)emalloc(msglen);

            memcpy(msg,(const void*)partStart,msglen);

            mm = mime_create(msg,msglen);
            if(mm->name != NULL)
               vec_add(parts,mm);

            free(msg);
 
         } CATCH("MIMEParseException") {

         } END_TRY

      }

      kmp_free(kmpBoundary);

      return parts;
   } else {
      return NULL;
   }
}

State *
statefullInitFromMIMEParts(Vector* parts){
   State *s= NULL, *ps = NULL;
   MIMEEntity* sidPart = NULL;
   Enumeration* e = NULL;
   SymbolTable* stab = NULL;

   s = state_createDefault();

   /* If this is a statefull application */

   if( ! getBoolConfigOption("Session.Disable")  ) {

      /* Discover the parent state if it was specified */

      e = vec_elements(parts);
      while(enum_hasNext(e)){
         MIMEEntity* curpart = (MIMEEntity*)enum_next(e);
         if (!estrcmp("sid",curpart->name)) sidPart = curpart;
      }

      if(sidPart != NULL){
         char* sid = mime_getBodyAsString(sidPart);
         ps = getStateBySID(sid);
         free(sid);
      }

      /* Attempt to bind the new state to the parent state's session */

      bindStateToSession(ps,s);
   }

   /* Set the state variables to the mime body parts */ 

   stab = stab_createDefault();
   e = vec_elements(parts);
   while(enum_hasNext(e)){
      MIMEEntity* part = (MIMEEntity*)enum_next(e);

      /* iif the filename for the part was not set then it was a
       * regular form variable */

      if (part->filename == NULL) {
         char *oldVal = NULL;
         char *val = mime_getBodyAsString(part);
         char *key = part->name; 

         if((oldVal = stab_get(stab,key)) == NULL)
            stab_put(stab,key,val);
         else {
            int l1 = strlen(oldVal), l2=strlen(val);
            char* newVal = emalloc(l1+l2+2);

            strncpy(newVal,oldVal,l1);
            newVal[l1]='|';
            strncpy(newVal+l1+1,val,l2+1);
            
            free(oldVal);
            stab_put(stab,key,newVal);
         }
      }
   }
   enum_free(e);

   e = stab_getKeys(stab);
   while(enum_hasNext(e)){
      char* key = (char*)enum_next(e);
      char* val = (char*)stab_get(stab,key);
      state_setValue(s,key,val);
      free(val); /* State values are interned */ 
   }
   enum_free(e);
   stab_free(stab);

   return s;
}

State *
statefullInitFromQS(request_rec *r) {
   State *s,*ps;

   s = state_createDefault();

   /* if this is a statefull application */

   if ( ! getBoolConfigOption("Session.Disable") ) {

      /* Attempt to bind the new state to the parent state's session */

      ps = getParentStateFromQS(r,(const char *)r->parsed_uri.query);
      bindStateToSession(ps,s);
   }

   addNVPairsFromQS(s,r,(const char *)r->parsed_uri.query);
   return s;
}

void
bindSessionToCookie(Session* ss){
   char* strSSID = int_toString(ss->ssid);
   char* maxAge = int_toString(ctxt_getContext()->session_timeout);

   Cookie* c = cook_create(
      "SSID",strSSID,"Identifies a session",NULL,maxAge,NULL,'\0',NULL);

   free(strSSID);
   free(maxAge);
   res_setCookie(getResponse(),c);
   cook_free(c);
}

void 
bindStateToSession(State* ps, State* s){
   uint32_t ipaddr = inet_addr(getRequest()->connection->remote_ip);
   Session* ss = NULL, *pss=NULL;
   Cookie* ssid_cookie = req_getCookie(current_request,"SSID");

   /* If we should be discovering sessions from SSID cookies then go try
      and get it */

   if(getBoolConfigOption("Session.SetSSIDCookie")) {
      if(ssid_cookie != NULL && ssid_cookie->value != NULL) {
         int ssid = atoi(ssid_cookie->value);
         ss = getSessionBySSID(ssid);
      }
   }
   
   /* If we are matching IPs and we got a session from an SSID cookie verify the
      IPs match. If not, unset the cookie */

   if(ss != NULL && getBoolConfigOption("Session.MatchClientIP") && ss->ipaddr!=ipaddr) {
      fprintf(stderr,"### IP Address stored in session doesn't match\n");
      ss = NULL;
   }

   if(!ps) {

      /* If the session has not already been found, spawn a new session */
      if(ss==NULL) ss = sess_create(ipaddr);
   
   } else {
      /* Retrieve the session from the parent state */
      pss = state_getSession(ps);

      /* Verify that this session isn't being hijacked */
      if(getBoolConfigOption("Session.SetSSIDCookie") && getBoolConfigOption("Session.MatchSSIDCookie") && ss == NULL) {
         /* We've got a 'parent state' but no SSID cookie ... no good */
         fprintf(stderr,"### Tring to match SSID Cookie but no session retrieved\n");
         ss = sess_create(ipaddr);
      } if(getBoolConfigOption("Session.SetSSIDCookie") && getBoolConfigOption("Session.MatchSSIDCookie") && ss != pss ) {
         /* The session from the 'parent state' and the ssid cookie don't match */
         fprintf(stderr,"### SSID Cookie and SID don't give same session\n");
         ss = sess_create(ipaddr);
      } else if(getBoolConfigOption("Session.MatchClientIP") && pss->ipaddr!=ipaddr) {
         /* The ip address of the user requesting this session doesn't match */
         fprintf(stderr,"### IP Address stored in session doesn't match\n");
         ss = sess_create(ipaddr);
      } else {
         /* Otherwise, inherit state variables from the parent state */
         ss = pss;  
         state_addNVPairsFromState(s,ps);
      }
   }

   /* Attach the new state to the session found or just created */

   state_attachToSession(s,ss);

   /* If the Session ID should be determined by checking a
      client side cookie then bind (or rebind) the session to 
      the SSID cookie */

   if(getBoolConfigOption("Session.SetSSIDCookie")){
      bindSessionToCookie(ss);
   }
}

State *
statefullInitFromPost(request_rec *r) {
   State *s,*ps = NULL;
   char *data;
   size_t postlen;

   s = state_createDefault();

   if (util_read(r, (const char **)&data,&postlen) == OK) {
      
      /* If this is a stateful application */  
      if ( ! getBoolConfigOption("Session.Disable") ) { 

         /* Attempt to bind the new state to the parent state's session */
         ps = getParentStateFromQS(r,data);
         bindStateToSession(ps,s);
      }

      addNVPairsFromQS(s,r,data);
   } else {

      /* If this is a stateful application */
      if ( ! getBoolConfigOption("Session.Disable") ) 

         /* Bind the new state to a new session */
         bindStateToSession(NULL,s);
   }

   return s;
}

char 
getBoolConfigOption(char* name){
   char* value;

   value = (char*)ap_table_get(current_options,name);
   if (value != NULL && strcasecmp(value,"false") != 0)
      return '\1';
   else
      return '\0';
}

char* 
getConfigOption(char* name){
   char* value;

   value = (char*)ap_table_get(current_options,name);
   if (value != NULL)
      return estrdup(value);
   else
      return NULL;
}

HTTPRequest *
getRequest() { return current_request; }

HTTPResponse *
getResponse() { return current_request; }

void 
req_init(HTTPRequest *r ,table* options){

   current_options = options;
   current_request = r;
	headers_already_sent = '\0';
	
   if (r->method_number == M_GET) {
      current_cookies = parseCookies(r);
      current_state = statefullInitFromQS(r);
      current_parts = vec_createDefault();
   } else if (r->method_number == M_POST) {
      char* ctype = (char*)ap_table_get(r->headers_in,"Content-type");
      if(ctype != NULL && !strncmp(ctype,"multipart",9)){
         current_cookies = parseCookies(r);
         current_parts = parseMIMEPartsFromPost(r);
         current_state = statefullInitFromMIMEParts(current_parts);
      } else {
         current_cookies = parseCookies(r);
         current_state = statefullInitFromPost(r);
         current_parts = vec_createDefault();
      }
   } else {
      current_cookies = NULL;
      current_state = NULL; /* we shouldn't handle any other HTTP commands */
      current_parts = NULL;
   }

}

void 
req_cleanup(){
   /* Free MIME parts */

   if (current_parts!=NULL) {
      Enumeration* e=vec_elements(current_parts);
      while(enum_hasNext(e)){
         MIMEEntity* curPart = (MIMEEntity*)enum_next(e);
         mime_free(curPart);
      }
      enum_free(e);

      vec_free(current_parts);
   }

   /* Free Cookies */

   if (current_cookies!=NULL) {
      Enumeration* e=stab_getValues(current_cookies);
      while(enum_hasNext(e)){
         Cookie* curCookie = (Cookie*)enum_next(e);
         cook_free(curCookie);
      }
      enum_free(e);

      stab_free(current_cookies);
   }

   /* Free the State if it's not being maintained */

   if( getBoolConfigOption("Session.Disable") ) {
      state_free(current_state);
   } else {

   /* If we are maintaining sessions for this application then expire old ones */

      ctxt_expireSessions(ctxt_getContext());
   }
}

char* 
req_getFileName(HTTPRequest* r){
   return estrdup((char*)r->filename);
}

char* 
req_getPathInfo(HTTPRequest* r){
   return estrdup((char*)r->path_info);
}

char* 
req_getURI(HTTPRequest* r){
   return estrdup((char*)(r->uri));
}

char* 
req_getMethod(HTTPRequest* r){
   return estrdup((char*)r->method);
}

char* 
req_getHostName(HTTPRequest* r){
   return estrdup((char*)r->hostname);
}

char* 
req_getProtocol(HTTPRequest* r){
   return estrdup((char*)r->protocol);
}

char* 
req_getHeader(HTTPRequest* r,char* header){
   char* hval = (char*)ap_table_get(r->headers_in,header);

   if(hval != NULL)
      return estrdup(hval);

   return NULL;  
}

char*
req_getClientIP(HTTPRequest* r){
   return estrdup((char*)r->connection->remote_ip);
}

Enumeration* 
req_getMIMEParts(HTTPRequest* r){
   return vec_elements(current_parts); 
}

MIMEEntity* 
req_getMIMEPart(HTTPRequest* r,char* name){
   int i=0;
   for(i=0;i<vec_size(current_parts);i++)
      if(!estrcmp(name,((MIMEEntity*)vec_get(current_parts,i))->name))
         return (MIMEEntity*)vec_get(current_parts,i);
   return NULL;
}

Cookie* 
req_getCookie(HTTPRequest* r,char* name){
   return (Cookie*)stab_get(current_cookies, name);
}

Enumeration* 
req_getCookies(HTTPRequest* r){
   return stab_getValues(current_cookies);
}

State *
getState() {
   return current_state;
}

char *
getValue(char* name) {
   return state_getValue(current_state,name);
}

char* 
getValueWDefault(char* name, char* value) {
   char* result;
   result = state_getValue(current_state,name);
   if (result == NULL)
      result = value;
   return result;
}

void 
res_setCookie(HTTPResponse* r, Cookie* c){
   char* cstring = cook_toString(c);
   res_setHeader(r,"Set-Cookie", cstring);
   free(cstring);
}

void 
res_setHeader(HTTPResponse* r, char* name, char* value){

   /* Internally, apache calls ap_pstrdup on both the name and the
      value so whe don't need to worry about deleting either */

   if(estrcasecmp(name,"Content-type")==0)
   	/* set the appropriate response type*/
   	r->content_type = ap_pstrdup(r->pool,value);
   
   ap_table_set(r->headers_out,name,value);
}

void
res_sendHeaders(HTTPResponse* r){
	ap_send_http_header(r);
	headers_already_sent = '\1';
}

extern StringBuffer* out; // FIXME!!!! Huge hack to get output for flush()

void
res_flushOutput(HTTPResponse* r){
	if(!headers_already_sent){
		ap_send_http_header(r);
		headers_already_sent = '\1';
	}
	ap_rputs(buf_data(out),r);
	ap_rflush(r);
	buf_clear(out);
}

void 
sendRedirect(char* redirect){
   request_rec* r = current_request;

   if (redirect == NULL ) {
      THROW_D("NullPointerException");
   }
	if ( redirect[0] == '/' || strncmp(redirect,"http://",7) == 0 || strncmp(redirect,"https://",8) == 0) {
		/* Fully qualified */
   } else {
      char* newredirect = (char*)ap_palloc(r->pool,
         (strrchr(r->uri,'/') - r->uri)+strlen(redirect)+2);
      strncpy(newredirect,r->uri,strrchr(r->uri,'/') - r->uri + 1);
      strcpy(newredirect + (strrchr(r->uri,'/') - r->uri) + 1,redirect);
      redirect = newredirect;
   }

   ap_table_add(r->headers_out,"Location",redirect);

   THROW_D("HTTPRedirectException");

}

void 
logExecutionTime(request_rec *r, char *tmpl, float exectime) {
   ap_log_error(
      APLOG_MARK,
      APLOG_NOERRNO|APLOG_INFO,
      r->server,
      "%s executed in , %f seconds. At this rate we could execute %f tps.",
      tmpl ,
      exectime ,
      (float)1 / exectime
   );

}

struct sigaction sa;
struct sigaction oasegv,oabus;

void 
signalHandler(int signal){
   char* signame= "???";

   if (signal == SIGSEGV)
      signame = "SIGSEGV";
   else if (signal == SIGBUS)
      signame = "SIGBUS";

   THROW("SignalException","The signal %s was raised during page execution",signame);;
}

void
installSignalHandlers(){
   /* If we want signals to be wrapped in exceptions */

   if((char*)ap_table_get(current_options,"SignalsThrowExceptions") != NULL) {
      sigemptyset(&sa.sa_mask);
      sa.sa_handler = signalHandler;

#if defined(SA_ONESHOT)
      sa.sa_flags = SA_ONESHOT;
#elif defined(SA_RESETHAND)
      sa.sa_flags = SA_RESETHAND;
#endif

      sigaction(SIGSEGV, &sa, &oasegv);
      sigaction(SIGBUS, &sa, &oabus);
   }
}

void
unInstallSignalHandlers(){
   if((char*)ap_table_get(current_options,"SignalsThrowExceptions") != NULL) {
      sigaction(SIGSEGV, &oasegv, NULL);
      sigaction(SIGBUS, &oabus, NULL);
   }
}
   
/*=============================================================================
// end of file: $RCSfile: module.c,v $
==============================================================================*/

