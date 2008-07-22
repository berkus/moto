#====================================================================

Extension: Exception

Include: "exception.h"

Interface:

/** Construct a new Exception **/
tracked Exception Exception::Exception() => 
	Exception* excp_createDefault();

/** Construct a new Exception with a message **/
tracked Exception Exception::Exception(String msg) => 
	Exception* excp_create(char* msg);

/** Exception destructor **/	
void Exception::~Exception() => 
	void excp_free(Exception* this);

/** Returns a String representation of this exception **/
tracked String Exception::toString() =>
	char* excp_toString(Exception* e);

/** Returns the message, if any, this exception was constructed with **/
tracked String Exception::getMessage() => 
	char* excp_getMessage(Exception* e);
	
/** Returns the name of the file this exception was thrown from **/
tracked String Exception::getFile() =>
	char* excp_getFile(Exception* e);
	
/** Returns the line number this exception was thrown on **/
int Exception::getLine() =>
	int excp_getLine(Exception* e);

tracked String Exception::getStackTrace() =>
   char* excp_getStackTrace(Exception *e);

Extension: ArrayBoundsException

Include: "exception.h"

Interface:

/** Construct a new Exception **/
tracked ArrayBoundsException ArrayBoundsException::ArrayBoundsException() => 
	ArrayBoundsException* ArrayBoundsException_createDefault();

/** Construct a new Exception with a message **/
tracked ArrayBoundsException ArrayBoundsException::ArrayBoundsException(String msg) => 
	ArrayBoundsException* ArrayBoundsException_create(char* msg);

/** Exception destructor **/	
void ArrayBoundsException::~ArrayBoundsException() => 
	void excp_free(ArrayBoundsException* this);
	
/** Returns a String representation of this exception **/
tracked String ArrayBoundsException::toString() =>
	char* excp_toString(ArrayBoundsException* e);

/** Returns the message, if any, this exception was constructed with **/
tracked String ArrayBoundsException::getMessage() => 
	char* excp_getMessage(ArrayBoundsException* e);
	
/** Returns the name of the file this exception was thrown from **/
tracked String ArrayBoundsException::getFile() =>
	char* excp_getFile(ArrayBoundsException* e);
	
/** Returns the line number this exception was thrown on **/
int ArrayBoundsException::getLine() =>
	int excp_getLine(ArrayBoundsException* e);

Extension: NullPointerException

Include: "exception.h"

Interface:

/** Construct a new Exception **/
tracked NullPointerException NullPointerException::NullPointerException() => 
	NullPointerException* NullPointerException_createDefault();

/** Construct a new Exception with a message **/
tracked NullPointerException NullPointerException::NullPointerException(String msg) => 
	NullPointerException* NullPointerException_create(char* msg);

/** Exception destructor **/	
void NullPointerException::~NullPointerException() => 
	void excp_free(NullPointerException* this);
	
/** Returns a String representation of this exception **/
tracked String NullPointerException::toString() =>
	char* excp_toString(NullPointerException* e);

/** Returns the message, if any, this exception was constructed with **/
tracked String NullPointerException::getMessage() => 
	char* excp_getMessage(NullPointerException* e);
	
/** Returns the name of the file this exception was thrown from **/
tracked String NullPointerException::getFile() =>
	char* excp_getFile(NullPointerException* e);
	
/** Returns the line number this exception was thrown on **/
int NullPointerException::getLine() =>
	int excp_getLine(NullPointerException* e);	
	
Extension: MathException

Include: "exception.h"

Interface:

/** Construct a new Exception **/
tracked MathException MathException::MathException() => 
	MathException* MathException_createDefault();

/** Construct a new Exception with a message **/
tracked MathException MathException::MathException(String msg) => 
	MathException* MathException_create(char* msg);

/** Exception destructor **/	
void MathException::~MathException() => 
	void excp_free(MathException* this);
	
/** Returns a String representation of this exception **/
tracked String MathException::toString() =>
	char* excp_toString(MathException* e);

/** Returns the message, if any, this exception was constructed with **/
tracked String MathException::getMessage() => 
	char* excp_getMessage(MathException* e);
	
/** Returns the name of the file this exception was thrown from **/
tracked String MathException::getFile() =>
	char* excp_getFile(MathException* e);
	
/** Returns the line number this exception was thrown on **/
int MathException::getLine() =>
	int excp_getLine(MathException* e);	
	
Extension: IllegalArgumentException

Include: "exception.h"

Interface:

/** Construct a new Exception **/
tracked IllegalArgumentException IllegalArgumentException::IllegalArgumentException() => 
	IllegalArgumentException* IllegalArgumentException_createDefault();

/** Construct a new Exception with a message **/
tracked IllegalArgumentException IllegalArgumentException::IllegalArgumentException(String msg) => 
	IllegalArgumentException* IllegalArgumentException_create(char* msg);

/** Exception destructor **/	
void IllegalArgumentException::~IllegalArgumentException() => 
	void excp_free(IllegalArgumentException* this);
	
/** Returns a String representation of this exception **/
tracked String IllegalArgumentException::toString() =>
	char* excp_toString(IllegalArgumentException* e);

/** Returns the message, if any, this exception was constructed with **/
tracked String IllegalArgumentException::getMessage() => 
	char* excp_getMessage(IllegalArgumentException* e);
	
/** Returns the name of the file this exception was thrown from **/
tracked String IllegalArgumentException::getFile() =>
	char* excp_getFile(IllegalArgumentException* e);
	
/** Returns the line number this exception was thrown on **/
int IllegalArgumentException::getLine() =>
	int excp_getLine(IllegalArgumentException* e);					
	
#====================================================================

