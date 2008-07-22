#include <stdio.h>
#include "fileio.h"
#include "mxarr.h"

char* fiowrap_get(char* path);
UnArray* fiowrap_get_yarr(char* path);
void fio_put_yarr(char* path, UnArray* contents);
int fio_append_yarr( char *path, UnArray* contents);
int fio_put_yarr_d( char *path, UnArray* contents, int length);
int fio_append_yarr_d( char *path, UnArray* contents, int length);

