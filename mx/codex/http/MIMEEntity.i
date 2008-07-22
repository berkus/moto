Extension: MIMEEntity

Include: "mimeentity.h"

Interface:

/** Parse the specified message <i>msg</i> with lenght <i>length</i>
into a MIME Entity **/
MIMEEntity MIMEEntity::MIMEEntity(String msg, int length) =>
        MIMEEntity* mime_create(char* msg,size_t msglen);

/** Return the name of this MIMEEntity **/
tracked String MIMEEntity::getName() =>
        char* mime_getName(MIMEEntity* this);

/** Return the content type of this MIMEEntity e.g. "text/html" **/
tracked String MIMEEntity::getContentType() =>
	char* mime_getContentType(MIMEEntity* this);

/** Return the content transfer encoding of this MIMEEntity e.g. "7bit" **/
tracked String MIMEEntity::getContentTransferEncoding() =>
	char* mime_getContentTransferEncoding(MIMEEntity* this);

/** Return the file name of this MIMEEntity. A filename header is set on
file uploads the tells you the original name of the file being uploaded **/  
tracked String MIMEEntity::getFileName() =>
	char* mime_getFileName(MIMEEntity* this);

/** Returns the value of an arbitrary header <i>hdr</i> or null if this
header is not set **/
tracked String MIMEEntity::getHeader(String hdr) =>
        char* mime_getHeader(MIMEEntity* this, char* hdr);

/** Returns the body of this MIMEEntity as a byte[] **/
tracked byte[] MIMEEntity::getBody() =>
        void* mime_getBody_yarr(MIMEEntity* this);

/** Returns the body of this MIMEEntity as a String. This is really only
usefull if this MIME entity has a text body **/
tracked String MIMEEntity::getBodyAsString() =>
	char* mime_getBodyAsString(MIMEEntity* this);

/** Returns the length of the body of this MIME Entity in bytes (same as chars). 
This is usefull for saving the contents of a file upload to disk:<br>
<br>
&nbsp;&nbsp;&nbsp; $do(<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; putFileBytes(<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; "/usr/local/wa/dhakim/httpd/htdocs/foo.gif",<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; getRequest().getMIMEPart("uploaded_file").getBody(),<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; )<br>
&nbsp;&nbsp;&nbsp; )<br>

**/
int MIMEEntity::getBodyLength() =>
        size_t mime_getBodyLength(MIMEEntity* this);
