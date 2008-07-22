Extension: math

Library: -lm

Include: <math.h>

Interface:

/** Returns the value of e (the base of natural logarithms) raised to 
   the power of the x **/
double exp(double x) => 
	double exp(double x); 

/** Returns the natural log of x **/
double log(double x) => 
	double log(double x); 

/** Returns the log base ten of x **/
double log10(double x) => 
	double log10(double x); 

/** Returns x raised to the power y **/
double pow(double x, double y) => 
	double pow(double x, double y); 

/** Returns the square root of x **/
double sqrt(double x) => 
	double sqrt(double x); 

/** Returns x rounded upwards to the nearest integer **/
double ceil(double x) => 
	double ceil(double x); 

/** Returns x rounded downwards to the nearest integer **/
double floor(double x) => 
	double floor(double x); 

/** Returns the absolute value of x **/
double fabs(double x) => 
	double fabs(double x); 

/** Returns the result of multiplying the floating-point 
number x by 2 raised to the power **/
double ldexp(double x, int n) => 
	double ldexp(double x, int n); 

/** Splits the number x into a normalized fraction which is returned 
    and an exponent which is stored in exp. **/
double frexp(double x, Integer exp) => 
	double frexp(double x, int *exp); 

/** Breaks the argument x into an integral part and a fractional part, 
    each of which has the same sign as x.  The integral part is stored 
    in ip. The fractional part is returned **/
double modf(double x, Double ip) => 
	double modf(double x, double *ip); 

/** Returns the remainder of dividing x by y an integral number of time **/
double fmod(double x, double y) => 
	double fmod(double x, double y); 

/** Returns sine of x where x is given in radians **/
double sin(double x) => 
	double sin(double x); 

/** Returns the cosine of x where x is given in radians **/
double cos(double x) => 
	double cos(double x); 

/** Returns the tangent of x where x is given in radians **/
double tan(double x) => 
	double tan(double x); 

/** Returns the arc sine of x (the value whose sine is x). If
    x is not between 1 and -1 this function will fail **/
double asin(double x) => 
	double asin(double x); 

/** Returns the arc cosine of x (the value whose cosine is x). If
    x is not between 1 and -1 this function will fail **/
double acos(double x) => 
	double acos(double x); 

/** Returns the arc tangent of x (the value whose tangent is x). **/
double atan(double x) => 
	double atan(double x); 

/** Returns the arc tangent of the two variables x and y. This is 
similar to the arc tangent of y / x, except that the signs of both 
arguments are used to determine the quadrant of the result. **/

double atan2(double y, double x) => 
	double atan2(double y, double x); 

/** Returns the hyperbolic sine of x defined as (exp(x) - exp(-x)) / 2 **/
double sinh(double x) => 
	double sinh(double x); 

/** Returns the hyperbolic cosine of x defined as (exp(x) + exp(-x)) / 2 **/
double cosh(double x) => 
	double cosh(double x); 

/** Returns the hyperbolic tangent of x defined as sinh(x)/cosh(x) **/
double tanh(double x) => 
	double tanh(double x); 
