Extension: ItoIHashtable

Include: "itoihashtable.h"
Include: "intenumeration.h"

Interface:

/** Creates a new ItoIHashtable, a hashtable that maps ints to ints **/
tracked ItoIHashtable ItoIHashtable::ItoIHashtable() =>
	ItoIHashtable *itoi_createDefault();
	
void ItoIHashtable::~ItoIHashtable() =>
	void *itoi_free(ItoIHashtable *this);

/** Associates <i>key</i> with <i>value</i> **/	
void ItoIHashtable::put(int key, int value) =>
	void itoi_put(ItoIHashtable *this, int key, int value);

/** Returns the int associated with <i>key</i> **/ 	
int ItoIHashtable::get(int key) =>
	int itoi_get(ItoIHashtable *this, int key);

/** Returns true if this ItoIHashtable contains <i>key</i> **/ 
boolean ItoIHashtable::contains(int key) =>
        int itoi_contains(ItoIHashtable *this, int key);

/** Removes <i>key</i> and its associated int**/
int ItoIHashtable::remove(int key) =>
	int itoi_remove(ItoIHashtable *this, int key);

/** Clears out all keys and values from this ItoIHashtable **/
void ItoIHashtable::clear() =>
	void itoi_clear(ItoIHashtable *this);

/** Returns an IntEnumeration of the keys in this ItoIHashtable **/
tracked IntEnumeration ItoIHashtable::keys() =>
	IntEnumeration *itoi_getKeys(ItoIHashtable *this);

/** Returns the number of key value pairs in this ItoIHashtable **/
int ItoIHashtable::size() =>
	int itoi_size(ItoIHashtable *this);

/** Returns true if this ItoIHashtable contains the same elements as <i>S</i> **/
boolean ItoIHashtable::equals(ItoIHashtable s) =>
	int itoi_equals(ItoIHashtable *this, ItoIHashtable* S);

/** Returns true if this ItoIHashtable x contains the same elements as <i>y</i> **/
boolean eq(ItoIHashtable x,ItoIHashtable y) =>
	int itoi_equals(ItoIHashtable *x, ItoIHashtable* y);

/** Returns true if this ItoIHashtable y does not contain the same elements as <i>y</i> **/
boolean ne(ItoIHashtable x,ItoIHashtable y) =>
	int itoi_notequals(ItoIHashtable *x, ItoIHashtable* y);
