Extension: IntStack

Include: "intstack.h"
Include: "enumeration.h"

Interface:

/** Creates a new IntStack (a stack that holds ints) **/
tracked IntStack IntStack::IntStack() =>
	IntStack *istack_createDefault();

void IntStack::~IntStack() =>
	void istack_free(IntStack *this);

/** Clears the contents of the IntStack **/
void IntStack::clear() =>
	void istack_clear(IntStack *this);

/** Returns true if this IntStack contains <i>i</i> **/ 
boolean IntStack::contains(int i) =>
	int istack_contains(IntStack *this, int i);

/** Returns an IntEnumeration of the elements in this IntStack **/
tracked IntEnumeration IntStack::elements() =>
	IntEnumeration *istack_elements(IntStack *this);

/** Returns the top element in this IntStack **/
int IntStack::peek() =>
	void *istack_peek(IntStack *this);

/** Returns the element <i>index</i> - 1 from the top of this IntStack **/ 
int IntStack::peekAt(int index) =>
	void *istack_peekAt(IntStack *this, int index);

/** Pops the top element off of this IntStack and returns it **/
int IntStack::pop() =>
	void *istack_pop(IntStack *this);

/** Pushes the int <i>i</i> onto the top of this IntStack **/
void IntStack::push(int i) =>
	void *istack_push(IntStack *this, int i);

/** Returns the number of ints on this IntStack **/
int IntStack::size() =>
	int istack_size(IntStack *this);

/** Returns true iff the IntStack <i>that</i> contains the same elements in the 
same order as this IntStack **/	
boolean IntStack::equals(IntStack that) =>
	int istack_equals(IntStack* this, IntStack* that); 

/** Returns true if v1 equals v2 otherwise false **/
boolean eq(IntStack i1, IntStack i2) =>
	int istack_equals(IntStack * i1, IntStack * i2);

/** Returns true if v1 does not equal v2 otherwise false **/
boolean ne(IntStack i1, IntStack i2) =>
	int istack_notequals(IntStack * i1, IntStack * i2);