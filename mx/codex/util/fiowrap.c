#include "fiowrap.h"
#include "memsw.h"

char* fiowrap_get(char* path){ char* foo; fio_get(path, &foo); return foo;}

UnArray* fiowrap_get_yarr(char* path){ 
	UnArray* arr; 
	int i,len; 
	char* s; 
	len = fio_get(path, &s); 
	arr = arr_create(1,0,0,&len,ARR_BYTE_TYPE);
	for (i=0;i<len;i++){
		*ysub(arr,i) = s[i];
	}
	free(s);
	return arr;
}

void fio_put_yarr(char* path, UnArray* contents){
	if (contents == NULL) {
		THROW_D("NullPointerException");
	}
	fio_put_bytes(path,&contents->ya.data[0],contents->meta.length);
}

int fio_append_yarr( char *path, UnArray* contents){
	if (contents == NULL) {
		THROW_D("NullPointerException");
	}
	return fio_append_bytes(path,&contents->ya.data[0],contents->meta.length);
}

int fio_put_yarr_d( char *path, UnArray* contents, int length){
	if (contents == NULL) {
		THROW_D("NullPointerException");
	}
	return fio_put_bytes(path,&contents->ya.data[0],length);
}

int fio_append_yarr_d( char *path, UnArray* contents, int length){
	if (contents == NULL) {
		THROW_D("NullPointerException");
	}
	return fio_append_bytes(path,&contents->ya.data[0],length);
}


