Extension: Stack

Include: "stack.h"
Include: "enumeration.h"

Interface:

/** Creates a new Stack **/
tracked Stack Stack::Stack() =>
	Stack *stack_createDefault();

void Stack::~Stack() =>
	void stack_free(Stack *this);

/** Clears out the objects in this Stack **/
void Stack::clear() =>
	void stack_clear(Stack *this);

/** Returns true if this Stack contains the specified Object <i>obj</i> **/
boolean Stack::contains(Object obj) =>
	int stack_contains(Stack *this, void *obj);

/** Returns an Enumeration of the Objects in this Stack **/
tracked Enumeration Stack::elements() =>
	Enumeration *stack_elements(Stack *this);

/** Returns the Object at the top of this Stack **/
Object Stack::peek() =>
	void *stack_peek(Stack *this);

/** Returns the Object <i>index</i>-1 elements down from the top of this Stack **/
Object Stack::peekAt(int index) =>
	void *stack_peekAt(Stack *this, int index);

/** Pops the top element off of this Stack and returns it **/
Object Stack::pop() =>
	void *stack_pop(Stack *this);

/** Pushes a new element on to the top of this Stack **/
void Stack::push(Object obj) =>
	void *stack_push(Stack *this, void *obj);

/** Returns the number of Objects in this Stack **/
int Stack::size() =>
	int stack_size(Stack *this);

/** Returns true iff the Stack <i>that</i> contains the same elements in the 
same order as this Stack **/	
boolean Stack::equals(Stack that) =>
	int stack_equals(Stack* this, Stack* that); 

/** Returns true if v1 equals v2 otherwise false **/
boolean eq(Stack i1, Stack i2) =>
	int stack_equals(Stack * i1, Stack * i2);

/** Returns true if v1 does not equal v2 otherwise false **/
boolean ne(Stack i1, Stack i2) =>
	int stack_notequals(Stack * i1, Stack * i2);
	
/** Returns a new Stack that contains the same elemnts in the same order as this Stack **/	
tracked Stack Stack::clone() =>
	Stack* stack_clone(Stack* this);