Extension: Vector

Include: "vector.h"
Include: "enumeration.h"

Interface:

/** Constructs a new Vector (a dynamic Array of Objects) **/
tracked Vector Vector::Vector() =>
	Vector *vec_createDefault();

/** Constructs a new Vector from an Object[] **/
tracked Vector Vector::Vector(Object[] objs) =>
	Vector *vec_createFromArray(UnArray* objs);
	
void Vector::~Vector() =>
	void vec_free(Vector *v);

/** Adds the Object <i>obj</i> to the end of this Vector **/
int Vector::add(Object obj) =>
	int vec_add(Vector *this, void *obj);

/** Returns the elements in this array as a vector of Objects **/
Object[] Vector::toArray() =>
	UnArray* vec_toArray(Vector *this);

/** Clears the contents of this Vector **/
void Vector::clear() =>
	void vec_clear(Vector *this);

/** Returns true if the Vector contains <i>obj</i> **/
boolean Vector::contains(Object obj) =>
	int vec_contains(Vector *this, void *obj);

/** Returns an Enumeration of the Objects in this Vector. Note
that it is not a good idea to remove objects from a vector while
you're enumerating over it **/
tracked Enumeration Vector::elements() =>
	Enumeration *vec_elements(Vector *this);

/** Returns the Object <i>index</i> elements from the front of this Vector 
<br>
<br>
Throws ArrayBoundsException if index &lt; 0 or index &gt;= size() **/

Object Vector::get(int index) =>
	void *vec_get(Vector *this, int index);
	
/** Inserts the Object <i>obj</i> at index <i>index</i> shifting all elements
with indices >= <i>index</i> up one 
<br>
<br>
Throws ArrayBoundsException if index &lt; 0 or index &gt; size() **/

void Vector::insert(Object obj, int index) =>
	void vec_insert(Vector *this, void *obj, int index);

/** Removes the Object <i>obj</i> from this Vector. Returns true if 
<i>obj</i> was present, false otherwise **/

boolean Vector::remove(Object obj) =>
	int vec_remove(Vector *this, void *obj);

/** Removes the Object <i>index</i> elements from the front of this Vector
shifting all subsequent elements down one.
<br>
<br>
Throws ArrayBoundsException if index &lt; 0 or index &gt;= size() **/

void Vector::removeAt(int index) =>
	void vec_removeAt(Vector *this, int index);

/** Sets the object at the specified index to <i>obj</i> replacing the
object that was there 
<br>
<br>
Throws ArrayBoundsException if index &lt; 0 or index &gt;= size() **/
void Vector::setAt(Object obj, int index) =>
	void vec_setAt(Vector *this, void *obj, int index);

/** Shuffles the contents of this Vector. Note that randomness of the shuffling depends
on the random seed set by the srand function from cstdlib **/
void Vector::shuffle() =>
	void vec_shuffle(Vector *this);

/** Returns the number of Objects in this Vector **/ 
int Vector::size() =>
	int vec_size(Vector *this);
	
/** Sorts the vector in place using the given comparison function. 
The comparison function must return an integer less than, equal to, or 
greater than zero if the first argument is considered to be respectively 
less than, equal to, or greater than the second.
When this function returns, the vector is sorted in ascending order. **/ 
void Vector::sort(int(Object,Object) compfn) =>
	void vec_sort_cmp(Vector *this, Function* compfn);	

/** Returns true iff the Vector <i>that</i> contains the same elements in the 
same order as this Vector **/	
boolean Vector::equals(Vector that) =>
	int vec_equals(Vector* this, Vector* that); 

/** Returns true if v1 equals v2 otherwise false **/
boolean eq(Vector i1, Vector i2) =>
	int vec_equals(Vector * i1, Vector * i2);

/** Returns true if v1 does not equal v2 otherwise false **/
boolean ne(Vector i1, Vector i2) =>
	int vec_notequals(Vector * i1, Vector * i2);
		
/** Returns a new Vector that contains the same elemnts in the same order as this Vector **/	
tracked Vector Vector::clone() =>
	Vector* vec_clone(Vector* this);
	
/** Returns the Object <i>index</i> elements from the front of this Vector 
<br>
<br>
Throws ArrayBoundsException if index &lt; 0 or index &gt;= size() **/
Object Vector::[](int index) =>
	void *vec_get(Vector *this, int index);

/** Sets the object at the specified index to <i>obj</i> replacing the
object that was there. The inserted object is returned.
<br>
<br>
Throws ArrayBoundsException if index &lt; 0 or index &gt;= size() **/
Object Vector::[](int index, Object obj) =>
	void vec_setAtOverloaded(Vector *this, int index, void *obj);
