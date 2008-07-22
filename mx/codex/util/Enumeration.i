Extension: Enumeration

Include: "enumeration.h"

Interface:

void Enumeration::~Enumeration() =>
	void enum_free(Enumeration *this);

/** Returns true if there are elements left in this enumeration **/
boolean Enumeration::hasNext() =>
	int enum_hasNext(Enumeration *this);

/** Returns the next element in this enumeration **/
Object Enumeration::next() =>
	void *enum_next(Enumeration *this);

Object Enumeration::++() =>
	void *enum_next(Enumeration *this);
	
/** Calls the function f on each remaining element of the enumeration. **/
void Enumeration::each(void(Object) f) =>
	void *enum_each(Enumeration *this,Function *f);

