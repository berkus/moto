#include <ctype.h>
#include <string.h>
#include <stdio.h>

#include "memsw.h"
#include "excpfn.h"
#include "mman.h"
#include "runtime.h"
#include "stringbuffer.h"
#include "mxstring.h"

char *
boolean_toString(char x) {
   char *s;
   if (x) {
      s = emalloc(5);
      strcpy(s, "true");
   }
   else {
      s = emalloc(6);
      strcpy(s, "false");
   }
   return s;
}

char *
char_toString(char x) {
   char *s = emalloc(2);
   s[0]=x; s[1]='\0';
   return s;
}

char *
int_toString(int32_t x) {
   char *s = emalloc(12);
   sprintf(s, "%d", x);
   return s;
}

char *
byte_toString(signed char x) {
   char *s = emalloc(4);
   sprintf(s, "%d", (int)x);
   return s;
}

char *
long_toString(int64_t x) {
   char *s = emalloc(24);
   sprintf(s, "%lld", x);
   return s;
}

char *
float_toString(float x) {
   char *s = emalloc(128);
   sprintf(s, "%f", x);
   return s;
}

char *
double_toString(double x) {
   char *s = emalloc(256);
   sprintf(s, "%e", x);
   return s;
}

char* 
obj_toString(void* x){
   char* s;
   StringBuffer* sb = buf_createDefault();
   buf_putp(sb,x);
   s= buf_toString(sb);
   buf_free(sb);
   return s;
}

char* 
carr_toString(UnArray* x){
   char* s;
   StringBuffer* sb = buf_createDefault();
   buf_put(sb,x->ca.data,x->meta.length);
   s= buf_toString(sb);
   buf_free(sb);
   return s;
}

char* 
yarr_toString(UnArray* x){
   char* s;
   StringBuffer* sb = buf_createDefault();
   buf_put(sb,x->ca.data,x->meta.length);
   s= buf_toString(sb);
   buf_free(sb);
   return s;
}

int 
mxstr_len(const char *s) {
	if (s == NULL) {
		THROW_D("NullPointerException");
	}	
   return strlen(s);
}

char 
mxstr_charAt(const char *s, int index) {
	int len;
	if (s == NULL) {
		THROW_D("NullPointerException");
	}
	len = strlen(s);	
	if (index < 0 || index >= len) {
		THROW_D("ArrayBoundsException");
	}
   return s[index];
}

char *
mxstr_replaceChar(const char *s, char oc, char nc) {
	int i;
   int len;
	char *r;
   if (s == NULL) {
		THROW_D("NullPointerException");
	}
	len = strlen(s);	
   r = emalloc(len + 1);
   for (i = 0; i < len; i++) {
      if (s[i] == oc) {
         r[i] = nc;
      }
      else {
         r[i] = s[i];
      }
   }
   r[len] = '\0';
   return r;
}

char *
mxstr_replaceCharAt(const char *s, char c, int index) {
   int len;
   char *r;
   if (s == NULL) {
		THROW_D("NullPointerException");
	}
	len = strlen(s);	
	if (index < 0 || index > len - 1) {
		THROW_D("ArrayBoundsException");
	}
   r = emalloc(len + 1);
   strcpy(r,s);
   r[index] = c;
   return r;
}

char
mxstr_replaceCharInPlace(char *s, int index, char c) {
	s[index] = c;
	return c;
}

int 
mxstr_indexOf(const char *s, char c) {
	int r = -1;
   int i;
   int len;   
   if (s == NULL) {
		THROW_D("NullPointerException");
	}
	len = strlen(s);
   for (i = 0; i < len; i++) {
      if (s[i] == c) {
         r = i;
         break;
      }
   }
   return r;
}

int 
mxstr_lastIndexOf(const char *s, char c) {
	int r = -1;
   int i;
   int len;   
   if (s == NULL) {
		THROW_D("NullPointerException");
	}
	len = strlen(s);
   for (i = len - 1; i >= 0; i--) {
      if (s[i] == c) {
         r = i;
         break;
      }
   }
   return r;
}

char *
mxstr_toUpperCase(const char *s) {
	int i;
   int len;
	char *r;
   if (s == NULL) {
		THROW_D("NullPointerException");
	}
	len = strlen(s);	
   r = emalloc(len + 1);
   for (i = 0; i < len; i++) {
      r[i] = toupper(s[i]);
   }
   r[len] = '\0';
   return r;
}

char *
mxstr_toLowerCase(const char *s) {
	int i;
   int len;
	char *r;
   if (s == NULL) {
		THROW_D("NullPointerException");
	}
	len = strlen(s);	
   r = emalloc(len + 1);
   for (i = 0; i < len; i++) {
      r[i] = tolower(s[i]);
   }
   r[len] = '\0';
   return r;
}

char *
mxstr_reverse(const char *s) {
	int i;
   int len;
   int index;
	char *r;
   if (s == NULL) {
		THROW_D("NullPointerException");
	}
	index = len = strlen(s);	
   r = emalloc(len + 1);
   index--;
   for (i = 0; i < len; i++) {
      r[i] = s[index--];
   }
   r[len] = '\0';
   return r;
}

char *
mxstr_trim(const char *s) {
   int i;
   int b;
   int e;
   int len;
	char *r;
   if (s == NULL) {
		THROW_D("NullPointerException");
	}
	len = strlen(s);	
   i = 0;
   while (isspace(s[i]) && i < len) {
      i++;
   }
   b = i;
   i = len - 1;
   while (isspace(s[i]) && i >= 0) {
      i--;
   }
   e = i;
   if (b <= e) {
      int len = e - b + 1;
      r = emalloc(len + 1);
      strncpy(r, s + b, len);
      r[len] = '\0';
   }
   else {
      r = emalloc(2);
      r[0] = '\0';
   }
   return r;
}

char 
mxstr_startsWith(const char *s, const char* prefix){
   if (strncmp(s,prefix,strlen(prefix)) ==0) 
      return '\1';
   return '\0';
}

char 
mxstr_endsWith(const char *s, const char* suffix){
   if (strncmp(s+strlen(s) - strlen(suffix),suffix,strlen(suffix)) == 0) 
      return '\1';
   return '\0';
}

char *
mxstr_substr(const char *s, int sIndex) {
   char *r = NULL;
   int len,eIndex;
   if (s == NULL ) {
      THROW_D("NullPointerException");
   }
   len = strlen(s);
   eIndex = len;
   if (sIndex < 0 || sIndex > len){
      THROW_D("ArrayBoundsException");
   }
   if (sIndex <= eIndex) {
      r = emalloc(eIndex - sIndex+1);
      strncpy(r,s+sIndex,eIndex - sIndex);
      r[eIndex - sIndex] = '\0';
   }
   return r;
}

char *
mxstr_substring(const char *s, int sIndex, int eIndex) {
   char *r = NULL;
   int len;   
   if (s == NULL ) {
      THROW_D("NullPointerException");
   }
   len = strlen(s);
   if (sIndex < 0 || sIndex > len || eIndex < 0 || eIndex > len || sIndex > eIndex){
      THROW_D("ArrayBoundsException");
   }
   if (sIndex <= eIndex) {
      r = emalloc(eIndex - sIndex+1);
      strncpy(r,s+sIndex,eIndex - sIndex);
      r[eIndex - sIndex] = '\0';
   }
   return r;
}

UnArray *
mxstr_toArray(const char *s){
	if (s == NULL ) {
      THROW_D("NullPointerException");
   }
   {
		int i,length = strlen(s);
		
		UnArray* arr = arr_create(1,0,0,&length,ARR_CHAR_TYPE);
		for (i=0;i<length;i++){
			*csub(arr,i) = s[i];
		}
		return arr;
	}
}

UnArray *
mxstr_getBytes(const char *s){
	if (s == NULL ) {
      THROW_D("NullPointerException");
   }
   {
		int i,length = strlen(s);
		
		UnArray* arr = arr_create(1,0,0,&length,ARR_BYTE_TYPE);
		for (i=0;i<length;i++){
			*ysub(arr,i) = s[i];
		}
		return arr;
	}
}

char* 
mxstr_mstrcat(char* cop1, char* cop2){
   char* cptr;

   if(cop1 == NULL) cop1 = "null";
   if(cop2 == NULL) cop2 = "null";

   cptr = mman_malloc(rtime_getMPool(),strlen(cop1) + strlen(cop2) + 1);

   strcpy(cptr, cop1);
   strcat(cptr, cop2);

   return cptr;
}

