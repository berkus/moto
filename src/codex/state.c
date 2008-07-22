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

   $RCSfile: state.c,v $   
   $Revision: 1.23 $
   $Author: dhakim $
   $Date: 2003/03/20 22:33:08 $
 
==============================================================================*/
#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/file.h>
#include <fcntl.h>

#include "memsw.h"

#include "exception.h"
#include "excpfn.h"
#include "state.h"
#include "enumeration.h"
#include "util.h"
#include "intstack.h"

int CTXT_LOCK = 0;

#define LOCK ctxt_lock(); 
#define UNLOCK ctxt_unlock();

#ifdef HAVE_FLOCK

inline
void ctxt_lock(){
   if (CTXT_LOCK){
      flock(CTXT_LOCK,LOCK_EX);
   }
}

inline
void ctxt_unlock(){
   if (CTXT_LOCK){
      flock(CTXT_LOCK,LOCK_UN);
   }
}

int
ctxt_initLocking(char* globalLockDir){
   char lockPath[1024];
   char* lockDir = (globalLockDir != NULL) ? globalLockDir : "/tmp/";

   if(strlen(lockDir)+strlen("ctxt.lock")+1 < 1024) {
     sprintf(lockPath,"%s/ctxt.lock",lockDir);

     if (CTXT_LOCK == 0){
       if((CTXT_LOCK = open(lockPath,O_RDWR | O_CREAT,S_IRWXU|S_IRWXO|S_IRWXG))<0){
         fprintf(stderr,"### Could not initialize context locking !!!\n");
         return -1; /* could not create lock */
       }
     }
     return 1;
   } else {
     fprintf(stderr,"### Could not initialize context locking: ");
     fprintf(stderr,"Directory %s is too long.\n",lockDir);
     return -1;
   }
}

#else

static struct flock lock_it;
static struct flock unlock_it;

int
ctxt_initLocking(char* globalLockDir){

    lock_it.l_whence = SEEK_SET;        /* from current point */
    lock_it.l_start = 0;                /* -"- */
    lock_it.l_len = 0;                  /* until end of file */
    lock_it.l_type = F_WRLCK;           /* set exclusive/write lock */
    lock_it.l_pid = 0;                  /* pid not actually interesting */
    unlock_it.l_whence = SEEK_SET;      /* from current point */
    unlock_it.l_start = 0;              /* -"- */
    unlock_it.l_len = 0;                /* until end of file */
    unlock_it.l_type = F_UNLCK;         /* set exclusive/write lock */
    unlock_it.l_pid = 0;                /* pid not actually interesting */

   char lockPath[1024];
   char* lockDir = (globalLockDir != NULL) ? globalLockDir : "/tmp/";

   if(strlen(lockDir)+strlen("ctxt.lock")+1 < 1024) {
     sprintf(lockPath,"%s/ctxt.lock",lockDir);

     if (CTXT_LOCK == 0){
       if((CTXT_LOCK = open(lockPath,O_RDWR | O_CREAT,S_IRWXU|S_IRWXO|S_IRWXG))<0) {
         fprintf(stderr,"### Could not initialize context locking !!!\n");
         return -1; /* could not create lock */
       }
     }
     return 1;
   } else {
     fprintf(stderr,"### Could not initialize context locking: ");
     fprintf(stderr,"Directory %s is too long.\n",lockDir);
     return -1;
   }
}

inline void
ctxt_lock(){
   if (CTXT_LOCK){
      int ret;

      while ((ret = fcntl(CTXT_LOCK, F_SETLKW, &lock_it)) < 0 && errno == EINTR) {
         /* nop */
      }

      if (ret < 0) {fprintf(stderr,"Could not lock shared memory segment.\n"); }
   }
}

inline void
ctxt_unlock(){
   if (CTXT_LOCK){
      int ret;

      while ((ret = fcntl(CTXT_LOCK, F_SETLKW, &unlock_it)) < 0 && errno == EINTR) {
         /* nop */
      }
      if (ret < 0) {fprintf(stderr,"Could not lock shared memory segment.\n"); }
   }
}

#endif

Context *context = NULL;

void 
ctxt_init(time_t session_timeout,int states_per_session){
   if (context == NULL) {
      context=(Context*)emalloc(sizeof(Context));
      context->sessions=ihtab_createDefault();
      context->objects=stab_createDefault();
      context->conames=sset_createDefault();
      context->mpool=mpool_create(10);
      context->last_session=0;
      context->session_timeout=session_timeout;
      context->states_per_session=states_per_session;

   } 
   else {
      printf("\n### Hey, somebody already initialized the context\n");
   }
   srand(time(NULL));
}

void 
ctxt_cleanup(Context *context){
   Enumeration* e;

   for(e=sset_elements(context->conames);enum_hasNext(e);) {
      free(enum_next(e));
   }
   enum_free(e);

   ihtab_free(context->sessions);
   sset_free(context->conames);
   stab_free(context->objects);
   mpool_free(context->mpool);

   free(context);
}

Enumeration* 
ctxt_getKeys(Context *context){
   return stab_getKeys(context->objects);
}
int 
ctxt_getObjectCount(Context* context){
   return stab_size(context->objects);
}
Enumeration* 
sess_getKeys(Session *session){
   return stab_getKeys(session->objects);
}
int 
sess_getObjectCount(Session* session){
   return stab_size(session->objects);
}
IntEnumeration* 
ctxt_getSessionKeys(Context *context){
   return ihtab_getKeys(context->sessions);
}
int 
ctxt_getSessionCount(Context *context){
   return ihtab_size(context->sessions);
}
int 
ctxt_getStatesPerSession (Context *context){
   return context->states_per_session;
}
time_t 
ctxt_getSessionTimeout (Context * context){
   return context->session_timeout;
}
void 
sess_stamp(Session* session){
   session->last_accessed = time(NULL);
}

void 
ctxt_expireSessions(Context *context){
   IntEnumeration* ie; 
   IntStack* is;
   time_t expire_time;

   LOCK;
   ie = ihtab_getKeys(context->sessions);
   is = istack_createDefault(); 
   expire_time = time(NULL);

   while(ienum_hasNext(ie)){
      int ssid = ienum_next(ie);
      Session* session = ihtab_get(context->sessions,ssid);
      if(session->last_accessed + context->session_timeout < expire_time)
         istack_push(is,ssid);
   }
   ienum_free(ie);

   while(istack_size(is) > 0){
      int ssid = istack_pop(is);
      Session* session = ihtab_get(context->sessions,ssid);
      ihtab_remove(context->sessions,ssid);
      sess_free(session);
   }
   istack_free(is);

   UNLOCK;
}

Context *
ctxt_getContext(){
   return context;
}

Session *
sess_createDefault(){
   return(sess_create(0));
}

Session *
sess_create(uint32_t ipaddr){
   Session *session=(Session *)emalloc(sizeof(Session));
   Context *context=ctxt_getContext();
   int i;

   session->last_state = 0;
   session->objects = stab_createDefault();
   session->mpool=mpool_create(10);
   session->last_accessed = time(NULL);
   session->ipaddr = ipaddr;
   session->svvalues=sset_createDefault();
   session->svnames=sset_createDefault();

   LOCK ;
   while(1) if(ihtab_get(context->sessions,(session->ssid = rand() % 65536))==NULL) break; 
   ihtab_put(context->sessions,session->ssid,session);
   UNLOCK;

   for(i = 0; i < context->states_per_session; i++) {
      session->states[i] = NULL;
   }

   return session;
}

void 
sess_free(Session *session){
   int i;
   Enumeration *e;
   Context* context = ctxt_getContext();

   if (session) {
      
      /* Free states */
      for(i = 0; i < context->states_per_session; i++) {
         if(session->states[i] != NULL) {
            state_free(session->states[i]);
         }
      }

      /* Free state variable and session object names */
      for(e=sset_elements(session->svnames);enum_hasNext(e);) {
         free(enum_next(e));
      }
      enum_free(e);

      /* Free state variable values */
      for(e=sset_elements(session->svvalues);enum_hasNext(e);) {
         free(enum_next(e));
      }
      enum_free(e);

      sset_free(session->svvalues);
      sset_free(session->svnames);
      stab_free(session->objects);
      mpool_free(session->mpool);
      free(session);
   }

}

/* This function will set the state's sid */

State *
state_attachToSession(State *state, Session *session){
   Context* context = ctxt_getContext();

   state->sid = (session->ssid << 16) | session->last_state;
   sprintf(state->strsid,"%08X",state->sid);

   if(session->states[session->last_state % context->states_per_session] != NULL) {
      state_free(session->states[session->last_state % context->states_per_session]);
   }

   session->states[session->last_state % context->states_per_session] = state;
   session->last_state++;
   sess_stamp(session);

   return state;
}

Session *
getSessionBySSID(unsigned int ssid){
   Session *session = NULL;
   LOCK;
   session = ihtab_get(context->sessions,ssid);
   UNLOCK;
   return session;
}

Session *
getSessionBySID(char *SID){
   Session *session = NULL;
   unsigned int ssid = (strtoul(SID,0,16) >> 16) & 0xFFFF;
   LOCK;
   session = ihtab_get(context->sessions,ssid);
   UNLOCK;
   return session;
}

State *
getStateBySID(char *SID){
   Session *session;
   unsigned int sid; 
   State *state;
   Context* context = ctxt_getContext();

   if(SID == NULL) 
      return NULL;

   sid = strtoul(SID,0,16);
   
   LOCK;
   session = ihtab_get(context->sessions,((sid>>16)&0xFFFF));
   UNLOCK;

   if (session == NULL)
      return NULL;

   state = session->states[(sid & 0xFFFF) % context->states_per_session];
   if(state == NULL || state->sid != sid)
      return NULL;

   return state;
}

/* Dest should be at least 9 characters long */

char *
state_getSID(State *state){
   return state->strsid;
}

Session *
state_getSession(State *state){
   Session *session = NULL;
   unsigned int ssid = (state->sid >> 16) & 0xFFFF;
   LOCK;
   session = ihtab_get(context->sessions, ssid);
   UNLOCK;
   return session;
}

Enumeration* 
state_getKeys(State * state){
   return stab_getKeys(state->nvpairs);
}

char *
state_getValue(State *state, char *name){
   void *result = NULL;
   result = stab_get(state->nvpairs,name);
   return result;
}

void 
state_setValue(State *state, char *name, char *value){
   char *newname;
   char *newvalue;
   Session *session = state_getSession(state);

   if(session == NULL) {
      newname = estrdup(name);
   } else if((newname = sset_get(session->svnames,name))== NULL) {
      newname = estrdup(name);
      sset_add(session->svnames,newname);
   }
   
   if(session == NULL) {
      newvalue = estrdup(value);
   } else if((newvalue = sset_get(session->svvalues,value))== NULL) {
      newvalue = estrdup(value);
      sset_add(session->svvalues,newvalue);
   }
   
   stab_put(state->nvpairs,newname,newvalue);
}

State *
state_createDefault(){
   State *state;
   state = (State*)emalloc(sizeof(State));
   state->nvpairs = stab_createDefault();
   state->sid = 0;			/* Create sessionless sid */
   strcpy(state->strsid,"00000000");
   return state;
}

void 
state_free(State *state){

   /* We must free the keys and values of an unbound state */

   if(state->sid == 0) {
      Enumeration* e = stab_getValues(state->nvpairs);
      while (enum_hasNext(e)){ free(enum_next(e)); }
      enum_free(e);
      e = stab_getKeys(state->nvpairs);
      while (enum_hasNext(e)){ free(enum_next(e)); }
      enum_free(e);
   }
   stab_free(state->nvpairs);
   free(state);  
}

State *
state_addNVPairsFromState(State *child, State *parent){
   Enumeration* e;
   for(e = stab_getKeys(parent->nvpairs); enum_hasNext(e); ) {
      char *curkey = (char*)enum_next(e);
      if(*curkey != '_') {
         stab_put(child->nvpairs,curkey,stab_get(parent->nvpairs,curkey));
      }
   }
   enum_free(e);
   return child;
}

void *
sess_get(Session *session,char *name){
   void *result = NULL;
   result = stab_get(session->objects,name);
   return result;
}

void 
sess_put(Session *session, char *name, void *value){
   char* newname;

   /* So that we don't ever forget to free the keys we will intern
      then in the session */

   if((newname = sset_get(session->svnames,name))== NULL) {
      newname = estrdup(name);
      sset_add(session->svnames,newname);
   }
   stab_put(session->objects,newname,value);

}

void *
sess_promote(Session *session, void *ptr){
   void *result = NULL;
   result = mman_trackf(session->mpool, ptr,NULL);
   return result;
}
   
void *
ctxt_get(Context *context, char *name){
   void *result = NULL;
   result = stab_get(context->objects, name);
   return result;
}

void 
ctxt_put(Context *context, char *name, void *value){
   char* newname;

   /* So that we don't ever forget to free the keys we will intern
      then in the context */

   if((newname = sset_get(context->conames,name))== NULL) {
      newname = estrdup(name);
      sset_add(context->conames,newname);
   }
   stab_put(context->objects,newname, value);

}

void *
ctxt_promote(Context *context, void *ptr){
   void *result = NULL;
   result = mman_trackf(context->mpool, ptr,NULL);
   return result;
}

/*=============================================================================
// end of file: $RCSfile: state.c,v $
==============================================================================*/

