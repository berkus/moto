Extension: stdlib

Include: <stdlib.h>

Interface:

/** Returns the absolute value of the int argument i **/
int abs(int i) =>
	int abs(int i);

/** Parses the string s into a float **/
float atof(String s) =>
	float atof(const char *s);

/** Parses the string s into an int **/
int atoi(String s) =>
	int atoi(const char *s);

/** Return an integer greater than, equal to, or
     less than 0, according as the string s1 is greater than, equal to, or
     less than the string s2 **/
int strcmp(String s1,String s2) =>
	int strcmp(const char *s1, const char *s2);
	
/** Returns the value of the environment variable with the 
    specified name **/
String getenv(String name) =>
	char *getenv(const char *name);

/** Returns a pseudo-random integer **/
int rand() =>
	int rand();

/** Sets the specified integer as the seed for a new sequence of 
pseudo-random numbers generated by rand() **/
void srand(int seed) =>
	void srand(int seed);