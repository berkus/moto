#====================================================================

Extension: String

Include: "mxstring.h"

Interface:

/** Returns "true" if the boolean passed is equal to true. 
   Returns "false" if the boolean passed is equal to false.  **/

tracked String str(boolean x) => 
	char *boolean_toString(char x);

/** Returns a string of length 1 containing the character passed in **/
tracked String str(char x) => 
	char *char_toString(char x); 

/** Returns a string representation of the signed integer corresponding to the byte passed in **/
tracked String str(byte x) => 
	char *byte_toString(signed char x); 
	
/** Returns a string representation of the integer passed in **/
tracked String str(int x) => 
	char *int_toString(int32_t x); 

/** Returns a string representation of the long passed in **/
tracked String str(long x) => 
	char *long_toString(int64_t x); 

/** Returns a string representation of the float passed in **/
tracked String str(float x) => 
	char *float_toString(float x); 

/** Returns a string representation of the double passed in **/
tracked String str(double x) => 
	char *double_toString(double x); 

/** Returns a string created from the char[] passed in **/
tracked String str(char[] x) => 
	char *carr_toString(UnArray* x); 

/** Returns a string created from the byte[] passed in (currently the expected 
character encoding is ASCII ... this may change in the future to the platform's 
default character encoding) **/
tracked String str(byte[] x) => 
	char *yarr_toString(UnArray* x); 
	
/** Returns "true" if the boolean passed is equal to true. 
   Returns "false" if the boolean passed is equal to false.  **/
tracked String String::String(boolean x) => 
	char *boolean_toString(char x);

/** Returns a string of length 1 containing the character passed in **/
tracked String String::String(char x) => 
	char *char_toString(char x); 

/** Returns a string representation of the integer passed in **/
tracked String String::String(int x) => 
	char *int_toString(int32_t x); 

/** Returns a string representation of the signed integer corresponding to the byte passed in **/
tracked String String::String(byte x) => 
	char *byte_toString(signed char x);
	
/** Returns a string representation of the long passed in **/
tracked String String::String(long x) => 
	char *long_toString(int64_t x); 

/** Returns a string representation of the float passed in **/
tracked String String::String(float x) => 
	char *float_toString(float x); 

/** Returns a string representation of the double passed in **/
tracked String String::String(double x) => 
	char *double_toString(double x); 

/** Returns a string created from the char[] passed in **/
tracked String String::String(char[] x) => 
	char *carr_toString(UnArray* x); 

/** Returns a string created from the byte[] passed in (currently the expected 
character encoding is ASCII ... this may change in the future to the platform's 
default character encoding) **/
tracked String String::String(byte[] x) => 
	char *yarr_toString(UnArray* x); 
		
/** Returns the elements in this String as a array of chars **/
char[] String::toArray() =>
	UnArray* mxstr_toArray(const char *s);

/** Returns the elements in this String as a array of bytes (currently the bytes
returned correspond to an ASCII String encoding ... this may change in the future to the 
platform's default character encoding) **/
byte[] String::getBytes() =>
	UnArray* mxstr_getBytes(const char *s);
		
/** Returns the length of the string. **/
int String::length() =>
	int mxstr_len(const char *s);

/** Returns the first index in the string where the character <i>c</i> 
    appears or -1 if <i>c</i> is not present in the string **/
int String::indexOf(char c) =>
	int mxstr_indexOf(const char *s, char c);

/** Returns the last index in the string where the character <i>c</i>
    appears or -1 if <i>c</i> is not present in the string **/
int String::lastIndexOf(char c) =>
	int mxstr_lastIndexOf(const char *s, char c);

/** Returns the character at the specified <i>index</i> in the string. 
    If <i>index</i> &lt; 0
    or <i>index</i> &gt;= the length of the String 
    an ArrayBoundsException will be thrown. **/
    
char String::charAt(int index) =>
	char mxstr_charAt(const char *s, int index);

/** Returns true if the String starts with the specified prefix. 
    Returns false otherwise. **/
boolean String::startsWith(String prefix) =>
        char mxstr_startsWith(const char *this,const char* prefix);

/** Returns true if the string ends with the specified suffix. 
    Returns false otherwise. **/
boolean String::endsWith(String suffix) =>
        char mxstr_endsWith(const char *this,const char* suffix);

/** Return a new string that replaces all instances of the character
    oc with the character nc in this string. **/ 

tracked String String::replaceChar(char oc, char nc) =>
	char *mxstr_replaceChar(const char *s, char oc, char nc);

/** Return a new String that replaces the character at the index
    specified with the character c. If <i>index</i> &lt; 0
    or <i>index</i> &gt;= the length of the String  
    an ArrayBoundsException will be thrown. **/

tracked String String::replaceCharAt(char c, int index) =>
	char *mxstr_replaceCharAt(const char *s, char c, int index);

/** Return a new string that is this string backwards **/

tracked String String::reverse() =>
	char *mxstr_reverse(const char *s);

/** Return a new string that is just like this string but without any
    preceeding or trailing whitespace **/ 

tracked String String::trim() =>
	char *mxstr_trim(const char *s);	

/** Return a new string that replaces all lower case character with
    upper case ones in this string. **/

tracked String String::toUpperCase() =>
	char *mxstr_toUpperCase(const char *s);	

/** Return a new string that replaces all upper case characters with
    lower case ones in this string. **/

tracked String String::toLowerCase() =>
	char *mxstr_toLowerCase(const char *s);	

/** Return a copy of the contents of this string starting at the
    specified index. If <i>sIndex</i> &lt; 0
    or <i>sIndex</i> &gt; the length of the String 
    an ArrayBoundsException will be thrown. **/

tracked String String::substring(int sIndex) =>
        char *mxstr_substr(const char *s, int sIndex);

/** Return a copy of the contents of this string starting at the
    specified index and ending one character before the specified
    end index. If <i>sIndex</i> &lt; 0 or <i>eIndex</i> &lt; 0 
    or <i>sIndex</i> &gt; the length of the String 
    or <i>eIndex</i> &gt; the length of the String 
    or <i>sIndex</i> &gt; <i>eIndex</i> 
    an ArrayBoundsException will be thrown. **/
    
tracked String String::substring(int sIndex, int eIndex) =>
	char *mxstr_substring(const char *s, int sIndex, int eIndex);	

/** Concatenate <i>cop1</i> with <i>cop2</i> returning a 
    new allocated string **/
tracked String +(String cop1, String cop2) =>
	char *mxstr_mstrcat(char* cop1, char* cop2);

/** Returns character at index <i>index</i> in string <i>s</i> **/
char String::[](int index) =>
	char mxstr_charAt(const char *s, int index);

/** Sets character at index <i>index</i> in string <i>s</i> with 
   new character <i>c</i> **/
char String::[](int index, char c) =>
	char mxstr_replaceCharInPlace(char *s, int index, char c);

#====================================================================

