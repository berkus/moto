Extension: State

Include: "httpd.h"
Include: "module.h"
Include: "state.h"
 
Interface:

tracked State	State::State() =>
	State* state_createDefault();

/** Returns an enumeration of the names of all state variables set in the 
current State **/
tracked Enumeration  State::getKeys() =>
	Enumeration* state_getKeys(State* this);

/** Returns the current value of the state variable named <i>name</i> or
null if it is not set in this State**/
String  State::getValue(String name) =>
	char* state_getValue(State* this, char* name);

/** Sets the value to the State variable named <i>name</i> to the value
<i>value</i> in the this State. This method should not be used lightly as 
it may potentially cause time travel problems. It is best to set state variables
through the submission of forms or as part of URLs **/

void	State::setValue(String name, String value) =>
	void state_setValue(State* this, char* name, char* value);

/** Returns the Session this State is bound to or null if it is not bound
to a Session **/

Session State::getSession() =>
	Session* state_getSession(State* this);

/** Returns the State ID (SID) for this State. This should be passed along 
through generated urls and form submissions in a statefull application **/

String  State::getSID() =>
	char* state_getSID(State* state);

/** Returns the State for the current request **/
State   getState() =>
        State* getState();

/** Convenience function euqivalent to getState().getValue(<i>name</i>) **/
String  getValue(String name) =>
        char* getValue(char* name);

/** Convenience function euqivalent to: <br><br>
getState().getValue(<i>name</i>) != null ? getState().getValue(<i>name</i>) : <i>default</i>
 **/

String  getValue(String name, String default) =>
        char* getValueWDefault(char* name, char* default);

Extension: Session

Include: "httpd.h"
Include: "module.h"
Include: "state.h"

Interface:

tracked Session Session::Session() =>
	Session* sess_createDefault();
void    Session::~Session() =>
        void sess_free(Session* this);

/** Associate a <i>name</i> with <i>value</i> in this session so that
<i>value</i> may be retrieved on subsequent page views. Be sure to
promote() <i>value</i> and any Objects contained in or referenced by
<i>value</i> before putting it in the sessions. Otherwise the storage
for value or Objects it depends on may be released before it is retrieved
causing undefined behavior (usually a SEGFAULT!) **/

void	Session::put(String name, Object value) =>
	void sess_put(Session* session, char* name, void* value);

/** Marks the Object <i>value</i> for deletion only when this Session
is deleted **/
Object  Session::promote(Object value) =>
        void* sess_promote(Session* session, void* value);

/** Retrieve the Object associated with <i>name</i> from this Session **/
Object  Session::get(String name) =>
	void* sess_get(Session* session,char* name);

/** Returns an enumeration of the keys associated with Objects in this Session **/
tracked Enumeration Session::getKeys() =>
	Enumeration* sess_getKeys(Session* this);

/** Return the number of key-Object pairs in this Session **/
int	Session::getObjectCount() =>
	int sess_getObjectCount(Session* this);

Extension: Context

Include: "httpd.h"
Include: "module.h"
Include: "state.h"

Interface:

Context Context::Context() =>
        Context* ctxt_getContext();

/** Return the current Conext (a sigleton) **/
Context getContext() =>
	Context* ctxt_getContext();

/** Associate a <i>name</i> with <i>value</i> in the context so that
<i>value</i> may be retrieved on subsequent page views. Be sure to
promote() <i>value</i> and any Objects contained in or referenced by
<i>value</i> before putting it in the context. Otherwise the storage
for <i>value</i> or Objects it depends on may be released before it is retrieved
causing undefined behavior (usually a SEGFAULT!) **/

void    Context::put(String name, Object value) =>
        void ctxt_put(Context* this, char* name, void* value);

/** Marks the Object <i>value</i> for deletion only when the Context
is deleted i.e. When Apache is terminated or restarted **/
Object  Context::promote(Object value) =>
        void* ctxt_promote(Context* session, void* value);

/** Retrieve the Object associated with <i>name</i> from the Context **/
Object  Context::get(String name) =>
        void* ctxt_get(Context* this,char* name);

/** Returns an enumeration of the keys associated with Objects in the Context **/
tracked Enumeration Context::getKeys() =>
	Enumeration* ctxt_getKeys(Context* this);

/** Returns the number of Objects stored in the Context **/
int	Context::getObjectCount() =>
	int ctxt_getObjectCount(Context* this);

tracked IntEnumeration Context::getSessionKeys() =>
	IntEnumeration* ctxt_getSessionKeys(Context *this);

/** Returns the number of active sessions **/
int 	Context::getSessionCount() =>
	int ctxt_getSessionCount(Context *this);

/** Returns the maximum number of states maintained in a session **/
int     Context::getStatesPerSession() =>
        int ctxt_getStatesPerSession(Context *this);

/** Returns the time in seconds before a session expires due to inactivity **/
int     Context::getSessionTimeout() =>
        time_t ctxt_getSessionTimeout(Context *this);
