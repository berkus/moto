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

   $RCSfile: state.h,v $   
   $Revision: 1.14 $
   $Author: dhakim $
   $Date: 2003/05/29 02:14:08 $
 
==============================================================================*/
#ifndef __STATE_H
#define __STATE_H

#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <time.h>

#include "hashtable.h"
#include "inthashtable.h"
#include "stringset.h"
#include "symboltable.h"
#include "integer.h"
#include "mman.h"

#define MAX_STATES 50

typedef struct state{
   unsigned int sid;     /* State identifier */
   char strsid[9];       /* Static string form of the sid */
   SymbolTable* nvpairs; /* The name value pairs associated with this state */
} State;

typedef struct session{
   unsigned int ssid;
   unsigned int last_state;
   State* states[MAX_STATES];
   SymbolTable* objects;
   MPool* mpool;
   StringSet* svvalues;       /* Stores canonical values for state vars */
   StringSet* svnames;        /* Stores canonical names for state vars and 
                                 session objects */
   time_t last_accessed;      /* The time (seconds since the epoch) since this
                                 session was last accessed */ 
   uint32_t ipaddr;           /* The ip address this session is bound to */
} Session;

typedef struct context{
   unsigned int last_session;	/* The id of the last allocated session */
   int states_per_session;      /* The max number of states held in a session */
   time_t session_timeout;      /* The number of seconds before a session expires */
   IntHashtable* sessions; 
   StringSet* conames;          /* Stores canonical names for context objects */
   SymbolTable* objects;
   MPool* mpool;

   /* spinlock_t *lock; */
} Context;

void ctxt_init(time_t session_timeout,int states_per_session);

int ctxt_initLocking(char* globalLockDir);

Context* ctxt_getContext();

char* state_getValue(State* state, char* name);

void state_setValue(State* state, char* name, char* value);

Enumeration* state_getKeys(State* state);

Session* state_getSession(State* state);

Session *getSessionBySSID(unsigned int ssid);

/*
 * Constructors
 */

State* state_createDefault();

Session* sess_createDefault();

Session* sess_create(uint32_t ipaddr);

/*
 * Destructors
 */

void state_free(State* state);

void sess_free(Session* session);

void ctxt_cleanup();

/*
 * Reporting Functions
 */

Enumeration* ctxt_getKeys(Context *context);

int ctxt_getObjectCount(Context* context);

Enumeration* sess_getKeys(Session *session);

int sess_getObjectCount(Session* session);

IntEnumeration* ctxt_getSessionKeys(Context *context);

int ctxt_getSessionCount(Context *context);

int ctxt_getStatesPerSession (Context *context);

time_t ctxt_getSessionTimeout (Context * context);

/*
 * Session expiration functions
 */
 
void sess_stamp(Session* session);

void ctxt_expireSessions(Context *context);

/*
 * Functions for state building
 */

State* state_attachToSession(State* state, Session* session);
  
State* state_addNVPairsFromState(State* child, State* parent);

/*
 * Functions dealing with the sid string
 */

char* state_getSID(State* state);

State* getStateBySID(char* SID);

Session* getSessionBySID(char* SID);

/*
 * Functions dealing with session and context objects
 */

void* sess_get(Session* session, char* name);

void sess_put(Session* session, char* name, void* value);

void* ctxt_get(Context* context, char* name);

void ctxt_put(Context* context, char* name, void* value);

/*
 * Memory promotion functions
 */

void* sess_promote(Session* session, void* ptr);

void* ctxt_promote(Context* context, void* ptr);

#endif

/*=============================================================================
// end of file: $RCSfile: state.h,v $
==============================================================================*/

