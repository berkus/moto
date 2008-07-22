#====================================================================

Extension: Regex

Include: "cdx_regex.h"

Interface:

/** Constructs a regex from a string representation of a regex **/
tracked Regex Regex::Regex(String s) => 
	Regex *regex_create(char* x);

void Regex::~Regex () => 
	void regex_free(Regex* this); 

/** Returns true if the specified string is matched by this regex **/
boolean Regex::matches(String s) =>
	char regex_matches(Regex* this, char *s);

/** Returns true if the specified string starts with a prefix that
	is matched by this regex **/
boolean Regex::matchesPrefix(String s) =>
	char regex_matchesPrefix(Regex* this, char *s);

/** Returns true if the specified string ends with a suffix that
	is matched by this regex **/
boolean Regex::matchesSuffix(String s) =>
	char regex_matchesSuffix(Regex* this, char *s);

/** Returns true if the specified string contains any substring that
	is matched by this regex **/
boolean Regex::matchesSubstring(String s) =>
	char regex_matchesSubstring(Regex* this, char *s);

/** Returns a Match object for the specified string matched by this regex **/
tracked Match Regex::match(String s) =>
	Match* regex_match(Regex* this, char *s);

/** Returns a Match object for the specified string is matched by this regex requiring
	only the a prefix of s matches **/
tracked Match Regex::matchPrefix(String s) =>
	Match* regex_matchPrefix(Regex* this, char *s);

/** Returns a Match object for the longest suffix of s matched by this regex **/
tracked Match Regex::matchSuffix(String s) =>
	Match* regex_matchSuffix(Regex* this, char *s);

/** Returns a Match object for the leftmost longest match of this regex in s **/
tracked Match Regex::search(String s) =>
	Match* regex_search(Regex* this, char *s);

tracked String Regex::toTNFAString() =>
	char* regex_toTNFAString(Regex* this);

/** Returns a new string with instances of rx in the input string substituted by sub **/		
tracked String sub(String input, Regex rx, String sub) =>
	char* regex_sub(char* s, Regex* rx, char* sub);

/** Returns a new string with instances of rx in the input string substituted by sub **/	
tracked String sub(String input, Regex rx, String(Match) sub) =>
	char* regex_fsub(char* s, Regex* rx, Function* sub);

/** Returns an array of substrings of the input string split up on delimiters matched by rx.
	If rx has specified captures those captures are listed in the array as well. **/
tracked String[] split(String input, Regex rx) =>
	UnArray* regex_split(char* s, Regex* rx);

/** Returns a String joining the strings in the array <i>a</a> together by the specified
	delimiter <i>delim</i> **/
tracked String join(String[] a, String delim) =>
	char* regex_join(UnArray* a, char* s);
	
Extension: Match

Include: "cdx_regex.h"

Interface:

void Match::~Match() =>
	void match_free(Match* m);

/**  Returns the substring of the matched string corresponding to the i'th subexpression of
     the regular expression used. The i'th subexpression is the subexpression started with the i'th
     opening parenthesis in the regular expression. Passing 0 to this method will return the entire 
     string matched **/
tracked String Match::subMatch(int i) =>
	char* match_subMatch(Match* m, int i);

/** Returns the substring of the String matched against occurring prior to the start of the match **/
tracked String Match::preMatch() =>	
	char* match_preMatch(Match* m);

/** Returns the substring of the String matched against occurring after the end of the match **/
tracked String Match::postMatch() =>
	char* match_postMatch(Match* m);

/** Returns the index in the String matched against where the i'th subexpression began. If
	i is set to 0 this method returns the index in the original String where the match begins **/ 
int Match::startIndex(int i) =>
	int match_startIndex(Match* m,int i);

/** Returns the index in the String matched against where the i'th subexpression ends. If
	i is set to 0 this method returns the index in the original String where the match ends **/
int Match::endIndex(int i) =>	
	int match_endIndex(Match* m,int i);
	
tracked String Match::toString() =>
	char* match_toString(Match* m);
#====================================================================

