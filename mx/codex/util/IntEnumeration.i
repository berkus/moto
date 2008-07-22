Extension: IntEnumeration

Include: "intenumeration.h"

Interface:

void IntEnumeration::~IntEnumeration() =>
	void ienum_free(IntEnumeration *e);

/** Returns true if there are integers left in this
Enumeration **/  
boolean IntEnumeration::hasNext() =>
	int ienum_hasNext(Enumeration *e);

/** Returns the next int in this enumeration **/
int IntEnumeration::next() =>
	int ienum_next(IntEnumeration *e);


