#ifndef __MXSTRING_H
#define __MXSTRING_H

#include "mxarr.h"

/* Returns "true" if the boolean passed is equal to true. 
   Returns "false" if the boolean passed is equal to false.  */
char *boolean_toString(char x);

/* Returns a string representation of the signed integer 
corresponding to the byte passed in */
char *byte_toString(signed char x);

/* Returns a string of length 1 containing the character passed in */
char *char_toString(char x);

/* Returns a string representation of the integer passed in */
char *int_toString(int32_t x);

/* Returns a string representation of the long passed in */
char *long_toString(int64_t x);

/* Returns a string representation of the float passed in */
char *float_toString(float x);

/* Returns a string representation of the double passed in */
char *double_toString(double x);


char *obj_toString(void* x);

/* Returns a string created from the char[] passed in */
char *carr_toString(UnArray* x);

/* Returns a string created from the byte[] passed in (currently the expected 
character encoding is ASCII ... this may change in the future to the platform's 
default character encoding) */
char *yarr_toString(UnArray* x);


int mxstr_len(const char *s);

/* Returns the character at the specified <i>index</i> in the string. 
    If <i>index</i> &lt; 0
    or <i>index</i> &gte; the length of the String 
    an ArrayBoundsException will be thrown. */
char mxstr_charAt(const char *s, int index);

/* Returns the first index in the string where the character <i>c</i> 
    appears or -1 if <i>c</i> is not present in the string */
int mxstr_indexOf(const char *s, char c);

/* Returns the last index in the string where the character <i>c</i>
    appears or -1 if <i>c</i> is not present in the string */
int mxstr_lastIndexOf(const char *s, char c);

/* Return a new string that replaces all instances of the character
    oc with the character nc in this string. */
char *mxstr_replaceChar(const char *s, char oc, char nc);

/* Return a new String that replaces the character at the index
    specified with the character c. If <i>index</i> &lt; 0
    or <i>index</i> &gte; the length of the String  
    an ArrayBoundsException will be thrown. */
char *mxstr_replaceCharAt(const char *s, char c, int index);

/* Replaces the char at position <i>index</i> in string <i>s</i> 
   with new character <i>c</i>. Returns the replaced char. */
char mxstr_replaceCharInPlace(char *s, int index, char c);

/* Return a new string that replaces all lower case character with
    upper case ones in this string. */
char *mxstr_toUpperCase(const char *s);

/* Return a new string that replaces all upper case characters with
    lower case ones in this string. */
char *mxstr_toLowerCase(const char *s);

/* Return a new string that is this string backwards */
char *mxstr_reverse(const char *s);

/* Returns the elements in this String as a array of chars */
UnArray *mxstr_toArray(const char *s);

/* Returns the elements in this String as a array of bytes (currently the bytes
returned correspond to an ASCII String encoding ... this may change in the future to the 
platform's default character encoding) */
UnArray *mxstr_getBytes(const char *s);

/* Return a new string that is just like this string but without any
    preceeding or trailing whitespace */ 
char *mxstr_trim(const char *s);

/* Return a copy of the contents of this string starting at the
    specified index and ending one character before the specified
    end index. If <i>sIndex</i> &lt; 0 or <i>eIndex</i> &lt; 0 
    or <i>sIndex</i> &gt; the length of the String 
    or <i>eIndex</i> &gt; the length of the String 
    or <i>sIndex</i> &gt; <i>eIndex</i> 
    an ArrayBoundsException will be thrown. */
char *mxstr_substring(const char *s, int sIndex, int eIndex);	

/* Return a copy of the contents of this string starting at the
    specified index. If <i>sIndex</i> &lt; 0
    or <i>sIndex</i> &gt; the length of the String 
    an ArrayBoundsException will be thrown. */
char *mxstr_substr(const char *s, int sIndex);

/* Returns true if the String starts with the specified prefix. 
    Returns false otherwise. */
char mxstr_startsWith(const char *s, const char* prefix);

/* Returns true if the string ends with the specified suffix. 
    Returns false otherwise. */
char mxstr_endsWith(const char *s, const char* suffix);

char *mxstr_mstrcat(char* cop1, char* cop2);

#endif
