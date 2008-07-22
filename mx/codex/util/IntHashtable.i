Extension: IntHashtable

Include: "inthashtable.h"
Include: "intenumeration.h"

Interface:

/** Constructs a Hashtable object that maps ints to Objects **/
tracked IntHashtable IntHashtable::IntHashtable() =>
	IntHashtable *ihtab_createDefault();
	
void IntHashtable::~IntHashtable() =>
	void *ihtab_free(IntHashtable *this);

/** Associate the int <i>key</i> with the Object <i>value</i> **/
void IntHashtable::put(int key, Object value) =>
	void ihtab_put(IntHashtable *this, int key, void *value);

/** Returns the object associated with <i>key</i> **/
Object IntHashtable::get(int key) =>
	void *ihtab_get(IntHashtable *this, int key);

/** Remove <i>key</i> and it's associated Object from this
IntHashtable **/   
Object IntHashtable::remove(int key) =>
	void *ihtab_remove(IntHashtable *this, int key);

/** Remove all keys and associated objects from this IntHashtable **/
void IntHashtable::clear() =>
	void *ihtab_clear(IntHashtable *this);

/** Return an IntEnumeration of all keys in this IntHashtable **/
tracked IntEnumeration IntHashtable::keys() =>
	IntEnumeration *ihtab_getKeys(IntHashtable *this);

/** Return the number of associations in this IntHashtable **/
int IntHashtable::size() =>
	int ihtab_size(IntHashtable *this);

/** Returns true if this IntHashtable contains the same elements as <i>S</i> **/
boolean IntHashtable::equals(IntHashtable s) =>
	int ihtab_equals(IntHashtable *this, IntHashtable* S);

/** Returns true if IntHashtable x contains the same elements as <i>y</i> **/
boolean eq(IntHashtable x,IntHashtable y) =>
	int ihtab_equals(IntHashtable *x, IntHashtable* y);

/** Returns true if IntHashtable x does not contains the same elements as <i>y</i> **/
boolean ne(IntHashtable x,IntHashtable y) =>
	int ihtab_notequals(IntHashtable *x, IntHashtable* y);