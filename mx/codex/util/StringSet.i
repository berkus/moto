Extension: StringSet

Include: "stringset.h"
Include: "enumeration.h"

Interface:

/** Create a new StringSet (a set of Strings) **/
tracked StringSet StringSet::StringSet() =>
	StringSet *sset_createDefault();

void StringSet::~StringSet() =>
	void sset_free(StringSet *set);

/** Add the String <i>s</i> to this StringSet. If a matching String
was already present it is replaces with <i>s</i> **/
void StringSet::add(String s) =>
	void sset_add(StringSet *set, char *s);

/** Add the contents of StringSet <i>s</i> to this StringSet. If a matching String
was already present it is replaces with <i>s</i> **/
void StringSet::addSet(StringSet s) =>
	void sset_addSet(StringSet *this, StringSet *p);

/** Clear out the contents of this StringSet **/
void StringSet::clear() =>
	void sset_clear(StringSet *set);

/** Clones the contents of this StringSet **/
tracked StringSet StringSet::clone() =>
	StringSet* sset_clone(StringSet *this);
	
/** Returns true if this StringSet contains <i>s</i> **/
boolean StringSet::contains(String s) =>
	int sset_contains(StringSet *set, char *s);

/** Returns true if this StringSet contains the same elements as <i>S</i> **/
boolean StringSet::equals(StringSet s) =>
	int sset_equals(StringSet *this, StringSet* S);

/** Returns true if this StringSet x contains the same elements as <i>y</i> **/
boolean eq(StringSet x,StringSet y) =>
	int sset_equals(StringSet *x, StringSet* y);

/** Returns true if this StringSet x does not contain the same elements as <i>y</i> **/
boolean ne(StringSet x,StringSet y) =>
	int sset_notequals(StringSet *x, StringSet* y);
	
/** Returns an enumeration of the Strings in this StringSet **/
tracked Enumeration StringSet::elements() =>
	Enumeration *sset_elements(StringSet *set);

/** Returns the instance of the String that matches <i>s</i> out of this
StringSet if this StringSet contains <i>s</i>. Otherwise null is returned **/
String StringSet::get(String s) =>
	char *sset_get(StringSet *set, char *s);

/** Removes the instance of the String that matches <i>s</i> from this
StringSet. Returns true if it was present, false otherwise **/
boolean StringSet::remove(String s) =>
	int sset_remove(StringSet *set, char *s);

/** Removes the contents of StringSet s that are contained in this
StringSet **/
void StringSet::removeSet(StringSet s) =>
	void sset_removeSet(StringSet *this, StringSet *p);

/** Returns the number Strings in this stringset **/
int StringSet::size() =>
	int sset_size(StringSet *set);

/** Returns the union of two StringSets **/
tracked StringSet StringSet::+=(StringSet i) =>
	StringSet * sset_union(StringSet *this, StringSet *i);

/** Returns the intersection of StringSet i1 and StringSet i2 **/
tracked StringSet StringSet::*=(StringSet i) =>
	StringSet * sset_intersection(StringSet *this, StringSet *i);
	
/** Returns the difference StringSet i1 and StringSet i2 **/
tracked StringSet StringSet::-=(StringSet i) =>
	StringSet * sset_difference(StringSet *this, StringSet *i);

/** Returns the union of two StringSets - {x:x is in s1 or x is in s2 } **/
tracked StringSet +(StringSet s1, StringSet s2) =>
	StringSet * sset_union(StringSet * s1, StringSet * s2);

/** Returns the intersection of two StringSets - {x:x is in s1 and x is in s2 } **/
tracked StringSet *(StringSet s1, StringSet s2) =>
	StringSet * sset_intersection(StringSet * s1, StringSet * s2);
	
/** Returns the difference StringSet i1 and StringSet i2 - {x:x is in s1, x is not in s2}  **/
tracked StringSet -(StringSet s1, StringSet s2) =>
	StringSet * sset_difference(StringSet * s1, StringSet * s2);	
	
/** Returns the union of two StringSets - {x:x is in s1 or x is in s2 } **/
tracked StringSet union(StringSet s1, StringSet s2) =>
	StringSet * sset_union(StringSet * s1, StringSet * s2);

/** Returns the intersection of two StringSets - {x:x is in s1 and x is in s2 } **/
tracked StringSet intersection(StringSet s1, StringSet s2) =>
	StringSet * sset_intersection(StringSet * s1, StringSet * s2);
	
/** Returns the difference StringSet i1 and StringSet i2 - {x:x is in s1, x is not in s2}  **/
tracked StringSet difference(StringSet s1, StringSet s2) =>
	StringSet * sset_difference(StringSet * s1, StringSet * s2);

/** Returns the elements in this StringSet as a array of Strings **/
tracked String[] StringSet::toArray() =>
	UnArray* sset_toArray(StringSet *this);
