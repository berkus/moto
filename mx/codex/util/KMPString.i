Extension: KMPString

Include: "kmpstring.h"

Interface:

/** Creates a new KMPString (Knuth Morris Pratt string) from the
specified <i>string</i> . This KMPString may then be used to create
a Tokenizer to tokenize other Strings with **/ 

tracked KMPString KMPString::KMPString(String string) =>
	KMPString *kmp_create(char* string);

/** Returns the first index in <i>string</i> where this KMPString
appears or -1 if it's not present **/

int	KMPString::firstIndexIn(String string) =>
	int kmp_indexOf(KMPString* kmp, char* string);

void KMPString::~KMPString() =>
	void kmp_free(KMPString *this);
