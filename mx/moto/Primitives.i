#====================================================================
#
# Primitives.i
#
#====================================================================

Extension: Integer

Include: <string.h>
Include: "primitives.h"

Interface:

/** Constructs an Integer object which wraps int typed variables
    so they are suitable for storage in Vectors or Hashtables or
    other object containers. Its initial value is set to the
    specified int. **/
tracked Integer Integer::Integer(int n) =>
	Integer *inum_create(int32_t n);
	
void Integer::~Integer() =>
	void inum_free(Integer *p);

/** Gets the int value this Integer object is wrapping **/
int Integer::getValue() =>
	int32_t inum_getValue(Integer *p);

/** Sets the int value this Integer object is wrapping **/
void Integer::setValue(int n) =>
	void inum_setValue(Integer* this, int32_t n);

/** Returns the ascii ordinal value of the specified character **/
int int(char c) =>
	int char_toInt(char c);
	
#====================================================================

Extension: Long

Include: "primitives.h"

Interface:

/** Constructs a Long object which wraps long typed variables
    so they are suitable for storage in Vectors or Hashtables or
    other object containers. Its initial value is set to the
    specified long. **/

tracked Long Long::Long(long n) =>
	Long *lnum_create(int64_t n);
	
void Long::~Long() =>
	void lnum_free(Long *p);

/** Gets the long value this Long object is wrapping **/	
long Long::getValue() =>
	int64_t lnum_getValue(Long *p);

/** Sets the long value this Long object is wrapping **/
void Long::setValue(long n) =>
	void inum_setValue(Long* this, int64_t n);

#====================================================================

Extension: Float

Include: "primitives.h"

Interface:

/** Constructs an Float object which wraps float typed variables
    so they are suitable for storage in Vectors or Hashtables or
    other object containers. Its initial value is set to the
    specified float. **/

tracked Float Float::Float(float n) =>
	Float *fnum_create(float n);
	
void Float::~Float() =>
	void fnum_free(Float *p);

/** Gets the float value this Float object is wrapping **/	
float Float::getValue() =>
	float fnum_getValue(Float *p);

/** Sets the float value this Float object is wrapping **/
void 	Float::setValue(float n) =>
	void fnum_setValue(Float* this, float n);

#====================================================================

Extension: Double

Include: "primitives.h"

Interface:

/** Constructs a Double object which wraps double typed variables
    so they are suitable for storage in Vectors or Hashtables or
    other object containers. Its initial value is set to the
    specified double. **/

tracked Double Double::Double(double n) =>
	Double *dnum_create(double n);
	
void Double::~Double() =>
	void dnum_free(Double *p);

/** Gets the double value this Double object is wrapping **/
	
double Double::getValue() =>
	double dnum_getValue(Double *p);

/** Sets the double value this Double object is wrapping **/
void    Double::setValue(double n) =>
        void dnum_setValue(Double* this, double n);
#====================================================================

Extension: Boolean

Include: "primitives.h"

Interface:

/** Constructs a Boolean object which wraps boolean typed variables
    so they are suitable for storage in Vectors or Hashtables or
    other object containers. Its initial value is set to the
    specified boolean. **/

tracked Boolean Boolean::Boolean(boolean n) =>
	Boolean *bool_create(char n);
	
void    Boolean::~Boolean() =>
	void bool_free(Boolean *p);

/** Gets the boolean value this Boolean object is wrapping **/	
boolean Boolean::getValue() =>
	char bool_getValue(Boolean *p);

/** Sets the boolean value this Boolean object is wrapping **/
void    Boolean::setValue(boolean n) =>
        void bool_setValue(Boolean* this, char n);
#====================================================================

Extension: Character

Include: "primitives.h"

Interface:

/** Constructs a Character object which wraps char typed variables
    so they are suitable for storage in Vectors or Hashtables or
    other object containers. Its initial value is set to the
    specified char. **/

tracked Character Character::Character(char n) =>
	Character *char_create(char n);
	
void    Character::~Character() =>
	void char_free(Character *p);

/** Gets the char value this Character object is wrapping **/	
char    Character::getValue() =>
	char char_getValue(Character *p);

/** Sets the char value this Character object is wrapping **/
void    Character::setValue(char n) =>
        void char_setValue(Character* this, char n);
#====================================================================
	
