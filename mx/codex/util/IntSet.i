Extension: IntSet

Include: "intset.h"
Include: "intset_op.h"
Include: "intenumeration.h"

Interface:

/** Create an IntSet (a set of ints) **/
		tracked IntSet IntSet::IntSet() =>
	IntSet *iset_createDefault();

void IntSet::~IntSet() =>
	void iset_free(IntSet *this);

/** Add an int to this IntSet **/
void IntSet::add(int i) =>
	void iset_add(IntSet *this, int i);

/** Add an int to this IntSet **/
tracked IntSet IntSet::+=(int i) =>
	IntSet * iset_pluseq(IntSet *this, int i);

/** Add an IntSet to this IntSet **/
tracked IntSet IntSet::+=(IntSet i) =>
	IntSet * iset_pluseq2(IntSet *this, IntSet *i);

/** Remove all ints from this IntSet **/
void IntSet::clear() =>
	void iset_clear(IntSet *this);

/** Returns true if this IntSet contains <i>i</i> **/
boolean IntSet::contains(int i) =>
	int iset_contains(IntSet *this, int i);

/** Returns an IntEnumeration of all ints in this IntSet **/
tracked IntEnumeration IntSet::elements() =>
	IntEnumeration *iset_elements(IntSet *this);

/** Remove the int <i>i</i> from this IntSet **/
boolean IntSet::remove(int i) =>
	int iset_remove(IntSet *this, int i);

/** Remove the int <i>i</i> from this IntSet **/
tracked IntSet IntSet::-=(int i) =>
	IntSet * iset_minuseq(IntSet *this, int i);

/** Remove an IntSet to this IntSet **/
tracked IntSet IntSet::-=(IntSet i) =>
	IntSet * iset_minuseq2(IntSet *this, IntSet *i);

/** Return the number of ints in this IntSet **/
int IntSet::size() =>
	int iset_size(IntSet *this);

/** Returns true if i1 equals (in size) i2 otherwise false **/
boolean eq(IntSet i1, IntSet i2) =>
	int iset_equal(IntSet * i1, IntSet * i2);

/** Returns true if i1 differs (in size) from i2 otherwise false **/
boolean ne(IntSet i1, IntSet i2) =>
	int iset_notequal(IntSet * i1, IntSet * i2);

/** Returns the union of i1 and i2 **/
tracked IntSet +(IntSet i1, IntSet i2) =>
	IntSet * iset_plus(IntSet * i1, IntSet * i2);

/** Returns the difference of i1 and i2 **/
tracked IntSet -(IntSet i1, IntSet i2) =>
	IntSet * iset_minus(IntSet * i1, IntSet * i2);
