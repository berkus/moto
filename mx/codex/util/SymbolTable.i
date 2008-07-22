Extension: SymbolTable

Include: "symboltable.h"
Include: "enumeration.h"

Interface:

/** Constructs a new SymbolTable, a Hashtable that associates Strings
to Objects **/
tracked SymbolTable SymbolTable::SymbolTable() =>
	SymbolTable *stab_createDefault();
	
void SymbolTable::~SymbolTable() =>
	void *stab_free(SymbolTable *this);

/** Add a new association between <i>key</i> and <i>value</i> **/
void SymbolTable::put(String key, Object value) =>
	void stab_put(SymbolTable *this, char *key, void *value);

Object SymbolTable::[](String key, Object value) =>
	void * stab_put(SymbolTable *this, char *key, void *value);

/** Get the Object associate with <i>key</i>. If <i>key</i> is not
present in this SymbolTable null is returned **/
Object SymbolTable::get(String key) =>
	void *stab_get(SymbolTable *this, char *key);

Object SymbolTable::[](String key) =>
	void *stab_get(SymbolTable *this, char *key);

/** Returns true if the symbol table contains <i>key</i> **/
boolean SymbolTable::contains(String key) =>
	int stab_contains(SymbolTable *this, char *key);
	
/** Remove <i>key</i> and its associate value from this SymbolTable **/ 
void SymbolTable::remove(String key) =>
	void *stab_remove(SymbolTable *this, char *key);

/** Clear all associations from this SymbolTable **/	
void SymbolTable::clear() =>
	void *stab_clear(SymbolTable *this);

/** Returns an Enumeration of all keys from this SymbolTable **/
tracked Enumeration SymbolTable::keys() =>
	Enumeration *stab_getKeys(SymbolTable *this);

/** Returns an Enumeration of all values from this SymbolTable **/
tracked Enumeration SymbolTable::values() =>
	Enumeration *stab_getValues(SymbolTable *this);
	
/** Returns the number of key value pairs present in this SymbolTable **/
int SymbolTable::size() =>
	int stab_size(SymbolTable *table);

/** Returns true if this SymbolTable contains the same elements as <i>S</i> **/
boolean SymbolTable::equals(SymbolTable s) =>
	int stab_equals(SymbolTable *this, SymbolTable* S);

/** Returns true if this SymbolTable x contains the same elements as <i>y</i> **/
boolean eq(SymbolTable x,SymbolTable y) =>
	int stab_equals(SymbolTable *x, SymbolTable* y);

/** Returns true if this SymbolTable x does not contain the same elements as <i>y</i> **/
boolean ne(SymbolTable x,SymbolTable y) =>
	int stab_notequals(SymbolTable *x, SymbolTable* y);
