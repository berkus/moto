Extension: Tokenizer

Include: "tokenizer.h"
Include: "kmpstring.h"
Include: "cdx_regex.h"

Interface:

/** Construct a new Tokenizer that will tokenize the String <i>string</i> on
the character <i>c</i>. Each time you call the next() method on this tokenizer
it will return the next substring of <i>string</i> that is terminated by c 
or '\0' and starting one character after the end previous substring (or at
the beginning for the first call). The character <i>c</i> will never be present 
in the result of any call to the next() **/

tracked Tokenizer Tokenizer::Tokenizer(String string,char c) =>
	Tokenizer *tok_createCTok(char* string, char c);


/** Construct a new Tokenizer that will tokenize the String <i>string</i> on
the Regex<i>rx</i>. Each time you call the next() method on this tokenizer
it will return the next substring of <i>string</i> that is terminated by rx 
or '\0' and starting after the end previous substring (or at
the beginning for the first call). The string matched by <i>rx</i> will never be present  
in the result of any call to the next() **/

tracked Tokenizer Tokenizer::Tokenizer(String string,Regex rx) =>
	Tokenizer *tok_createRXTok(char* string, Regex* rx);

/** Construct a new Tokenizer that will tokenize the String <i>string</i> on
the KMPString<i>kmp</i>. Each time you call the next() method on this tokenizer
it will return the next substring of <i>string</i> that is terminated by kmp 
or '\0' and starting kmp.length() characters after the end previous substring (or at
the beginning for the first call). The string <i>kmp</i> will never be present  
in the result of any call to the next() **/
	
tracked Tokenizer Tokenizer::Tokenizer(String string,KMPString kmp) =>
	Tokenizer *tok_createKMPTok(char* string, KMPString* kmp);

void Tokenizer::~Tokenizer() =>
	void tok_free(Tokenizer *this);

/** Returns the next substring matched by this Tokenizer or null if there are
no more substrings to return **/
tracked String Tokenizer::next() =>
	char* tok_next(Tokenizer *this);

/** Instructs the tokenizer whether or not to return matched delimiters as tokens **/
void Tokenizer::setReturnDelimiters(boolean b) =>
	void tok_setReturnDelimiters(Tokenizer *this,char b);
