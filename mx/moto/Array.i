#====================================================================

Extension: Array

Include: "mxarr.h"

Interface:

/** Returns the length of the specified boolean[] **/
int length(boolean[] x) => 
	int arr_length(UnArray* x);
	
/** Returns the length of the specified byte[] **/
int length(byte[] x) => 
	int arr_length(UnArray* x);

/** Returns the length of the specified char[] **/
int length(char[] x) => 
	int arr_length(UnArray* x);

/** Returns the length of the specified double[] **/
int length(double[] x) => 
	int arr_length(UnArray* x);

/** Returns the length of the specified float[] **/
int length(float[] x) => 
	int arr_length(UnArray* x);

/** Returns the length of the specified int[] **/
int length(int[] x) => 
	int arr_length(UnArray* x);
					
/** Returns the length of the specified long[] **/
int length(long[] x) => 
	int arr_length(UnArray* x);	

/** Returns the length of the specified Object[] **/
int length(Object[] x) => 
	int arr_length(UnArray* x);

#====================================================================

