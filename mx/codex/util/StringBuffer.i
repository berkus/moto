Extension: StringBuffer

Include: "stringbuffer.h"

Interface:

/** Creates a new StringBuffer. StringBuffers provide an efficient means to
concatenate multiple strings together that does not result in the creation
of temporary Strings **/
tracked StringBuffer StringBuffer::StringBuffer() =>
	StringBuffer *buf_createDefault();

void StringBuffer::~StringBuffer() =>
	void buf_free(StringBuffer *buf);

/** Clears the contents of this StringBuffer **/ 
void StringBuffer::clear() =>
	void buf_clear(StringBuffer *buf);

/** Appends an int onto this StringBuffer **/  	
void StringBuffer::append(int n) =>
	void buf_puti(StringBuffer *buf, int n);

/** Appends a long onto this StringBuffer **/
void StringBuffer::append(long n) =>
	void buf_putl(StringBuffer *buf, int64_t n);

/** Appends a float onto this StringBuffer **/
void StringBuffer::append(float n) =>
	void buf_putf(StringBuffer *buf, float n);

/** Appends a double onto this StringBuffer **/
void StringBuffer::append(double n) =>
	void buf_putd(StringBuffer *buf, double n);

/** Appends a character onto this StringBuffer **/
void StringBuffer::append(char c) =>
	void buf_putc(StringBuffer *buf, char c);

/** Appends a "true" if b is true "false" if b is false **/
void StringBuffer::append(boolean b) =>
	void buf_putb(StringBuffer *buf, char c);

/** Appends the signed integer value of the byte y onto the string buffer **/
void StringBuffer::append(byte y) =>
	void buf_puty(StringBuffer *buf, signed char c);
		
/** Appends a character array onto this StringBuffer **/
void StringBuffer::append(char[] carr) =>
	void buf_putca(StringBuffer *buf, UnArray* carr);

/** Appends a byte array onto this StringBuffer **/
void StringBuffer::append(byte[] carr) =>
	void buf_putya(StringBuffer *buf, UnArray* carr);
		
/** Appends a String onto this StringBuffer **/
void StringBuffer::append(String s) =>
	void buf_puts(StringBuffer *buf, char *s);

/** Appends the contents of another StringBuffer onto this StringBuffer **/
void StringBuffer::append(StringBuffer buf) =>
	void buf_cat(StringBuffer *buf1, StringBuffer buf2);

/** Returns the length of this StringBuffer **/
int StringBuffer::length() =>
	int buf_size(StringBuffer *buf);

/** Returns the character at the specified index in this StringBuffer **/ 
char StringBuffer::charAt(int index) =>
	char buf_charAt(StringBuffer *buf, int index);

/** Returns the contents of this StringBuffer as a String **/
tracked String StringBuffer::toString() =>
	char *buf_toString(StringBuffer *buf);

