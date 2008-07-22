Extension: ctype

Include: <ctype.h>

Interface:

/** Returns true if the specified character is a letter or a number **/
boolean isalnum(char c) =>
	int isalnum(int c);

/** Returns true if the specified character is a letter **/	
boolean isalpha(char c) =>
	int isalpha(int c);

/** Returns true if the specified character is a control character **/
boolean iscntrl(char c) => 
	int iscntrl(int c); 

/** Returns true if the specified character is a number **/
boolean isdigit(char c) => 
	int isdigit(int c); 

/** Returns true if the specified character is printable and not a space ' ' **/
boolean isgraph(char c) => 
	int isgraph(int c); 

/** Returns true if the specified character is a lower case letter **/
boolean islower(char c) => 
	int islower(int c); 

/** Returns true if the specified character is printable **/
boolean isprint(char c) => 
	int isprint(int c); 

/** Returns true if the specified character is printable but is not a space
   or any alphanumeric character **/
boolean ispunct(char c) => 
	int ispunct(int c); 

/** Returns true if the specified character is one of '\n','\r','\f','\t','\v' or
   space **/
boolean isspace(char c) => 
	int isspace(int c); 

/** Returns true if the specified character is an upper case letter **/
boolean isupper(char c) => 
	int isupper(int c); 

/** Returns true if the specified character is one of 
one of '0' '1' '2' '3' '4' '5' '6' '7' '8' '9' 'a' 'b' 
'c' 'd' 'e' 'f' 'A' 'B' 'C' 'D' 'E' 'F' **/
boolean isxdigit(char c) => 
	int isxdigit(int c); 

/** Converts the specified character to lower case **/
char tolower(char c) => 
	int tolower(int c); 

/** Converts the specified character to upper case **/
char toupper(char c) => 
	int toupper(int c); 
