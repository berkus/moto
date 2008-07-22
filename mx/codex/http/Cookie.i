Extension: Cookie

Include: "cookie.h"

Interface:

/** Constructs a new Cookie object (and Object representing a client-side cookie)
form the specified arguments that may then be set on the client browser. Once
created a Cookie can get set on a client browser by calling the setCookie() method
on the HTTPResponse Object. Note that cookies cannot be set on a page that then
redirects to another page **/

tracked Cookie Cookie::Cookie(
   String name, 
   String value, 
   String comment, 
   String domain, 
   String maxage, 
   String path, 
   boolean secure, 
   String version 
) => Cookie* cook_create( 
   char* name, 
   char* value, 
   char* comment, 
   char* domain, 
   char* maxage, 
   char* path, 
   char secure, 
   char* version
);

/** Constructs a new Cookie object with no attributes set **/
tracked Cookie Cookie::Cookie() =>
   Cookie* cook_createDefault();

void Cookie::~Cookie() =>
   void cook_free(Cookie* c);

/** Returns the name attibute of this Cookie **/
String Cookie::getName() => char* cook_getName(Cookie* this);

/** Returns the value attibute of this Cookie **/
String Cookie::getValue() => char* cook_getValue(Cookie* this);

/** Returns the comment attibute of this Cookie. Note that Cookies sent 
by clients to the server do not include the comment attribute. Rather
it is something the client sees when a cookie is sent to their browser **/
String Cookie::getComment() => char* cook_getComment(Cookie* this);

/** Returns the domain attibute of this Cookie. Note that Cookies sent  
by clients to the server do not include the domain attribute. If
the client is sent this cookie at all then your domain was specified 
in the cookie **/
String Cookie::getDomain() => char* cook_getDomain(Cookie* this);

/** Returns the Max-Age attribute of this Cookie. Note that Cookies sent
by clients to the server do not include the Max-Age attribute. Rather
it is something set by the server in the client side cookie telling
the client browser when to expire it. **/

String Cookie::getMaxAge() => char* cook_getMaxAge(Cookie* this);

/** Returns the path attribute of this Cookie. Note that Cookies sent
by clients to the server do not include the path attribute. If 
the client sent this cookie at all then the requested
URL was within the specified path **/
String Cookie::getPath() => char* cook_getPath(Cookie* this);

/** Returns the version of the Cookie **/
String Cookie::getVersion() => char* cook_getVersion(Cookie* this);

/** If set this cookie will only be sent over ssl connections **/
boolean Cookie::isSecure() => char cook_isSecure(Cookie* this);

/** Sets the name attribute for this Cookie **/
void Cookie::setName(String name) => void cook_setName(Cookie* c,char* name);

/** Sets the value attribute for this Cookie **/
void Cookie::setValue(String value) => void cook_setValue(Cookie* c,char* value);

/** Sets a comment for this Cookie for users to see when it gets set explaining
what information it stores and why it's necessary **/
void Cookie::setComment(String comment) => void cook_setComment(Cookie* c,char* comment);

/** Sets the domain for this Cookie. Once set on the client side this cookie
will only be sent to servers in the specified domain. Not setting this attribute
will default it's value to the setting domain  **/

void Cookie::setDomain(String domain) => void cook_setDomain(Cookie* c,char* domain);

/** Sets the time at which this Cookie will no longer be valid (and the
browser can expire it). <i>maxage</i> is the number seconds
of seconds after the cookie is set that it should expire. If the Max-Age
is not specified the client will expire the cookie when it quits **/

void Cookie::setMaxAge(String maxage) => void cook_setMaxAge(Cookie* c,char* maxage);

/** Sets the path for which this cookie applies. For instance setting a path
of /moto will imply that the client browser should only send this cookie
to URLs that begin with /moto. If the path is not specified it defaults to 
the path of the request URL that generated the Set-Cookie response, up to, 
but not including, the right-most / **/

void Cookie::setPath(String path) => void cook_setPath(Cookie* c,char* path);

/** Sets the version of the Cookie header used to set this cookie **/
void Cookie::setVersion(String version) => void cook_setVersion(Cookie* c,char* version);

/** If <i>isSecure</i> is set to true than this Cookie will only be send
over ssl connections **/
void Cookie::setSecurity(boolean isSecure) => void cook_setSecure(Cookie* c,char secure);

/** Returns a string representation of this Cookie **/
tracked String Cookie::toString() => char* cook_toString(Cookie* c);
