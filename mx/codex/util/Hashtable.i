Extension: Hashtable

Include: "hashtable.h"
Include: "enumeration.h"

Interface:

tracked Hashtable Hashtable::Hashtable() =>
	Hashtable *htab_createDefault();
	
void Hashtable::~Hashtable() =>
	void *htab_free(Hashtable *this);
	
void Hashtable::put(Object key, Object value) =>
	void htab_put(Hashtable *this, void *key, void *value);
	
Object Hashtable::get(Object key) =>
	void *htab_get(Hashtable *this, void *key);

Object Hashtable::remove(Object key) =>
	void *htab_remove(Hashtable *this, void *key);
	
void Hashtable::clear() =>
	void htab_clear(Hashtable *this);

tracked Enumeration Hashtable::keys() =>
	Enumeration *htab_getKeys(Hashtable *this);

int Hashtable::size() =>
	int htab_size(Hashtable *table);

